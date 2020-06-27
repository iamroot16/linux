// SPDX-License-Identifier: GPL-2.0
/*
 * sparse memory mappings.
 */
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/mmzone.h>
#include <linux/memblock.h>
#include <linux/compiler.h>
#include <linux/highmem.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

#include "internal.h"
#include <asm/dma.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>

/*
 * Permanent SPARSEMEM data:
 *
 * 1) mem_section	- memory sections, mem_map's for valid memory
 */
#ifdef CONFIG_SPARSEMEM_EXTREME
struct mem_section **mem_section;
#else
struct mem_section mem_section[NR_SECTION_ROOTS][SECTIONS_PER_ROOT]
	____cacheline_internodealigned_in_smp;
#endif
EXPORT_SYMBOL(mem_section);

#ifdef NODE_NOT_IN_PAGE_FLAGS
/*
 * If we did not store the node number in the page then we have to
 * do a lookup in the section_to_node_table in order to find which
 * node the page belongs to.
 */
#if MAX_NUMNODES <= 256
static u8 section_to_node_table[NR_MEM_SECTIONS] __cacheline_aligned;
#else
static u16 section_to_node_table[NR_MEM_SECTIONS] __cacheline_aligned;
#endif

int page_to_nid(const struct page *page)
{
	return section_to_node_table[page_to_section(page)];
}
EXPORT_SYMBOL(page_to_nid);

static void set_section_nid(unsigned long section_nr, int nid)
{
	section_to_node_table[section_nr] = nid;
}
#else /* !NODE_NOT_IN_PAGE_FLAGS */
static inline void set_section_nid(unsigned long section_nr, int nid)
{
}
#endif

#ifdef CONFIG_SPARSEMEM_EXTREME
static noinline struct mem_section __ref *sparse_index_alloc(int nid)
{
	struct mem_section *section = NULL;
	unsigned long array_size = SECTIONS_PER_ROOT *
				   sizeof(struct mem_section);

	if (slab_is_available()) {
		section = kzalloc_node(array_size, GFP_KERNEL, nid);
	} else {
		section = memblock_alloc_node(array_size, SMP_CACHE_BYTES,
					      nid);
		if (!section)
			panic("%s: Failed to allocate %lu bytes nid=%d\n",
			      __func__, array_size, nid);
	}

	return section;
}

// CONFIG_SPARSEMEM_EXTREME 커널 옵션을 사용하는 경우 dynamic하게 mem_section 테이블을 할당 받아 구성한다.
static int __meminit sparse_index_init(unsigned long section_nr, int nid)
{
	// 섹션 번호로 루트 번호를 구한다
    // root = section_nr / SECTION_PER_ROOT(==256)
	unsigned long root = SECTION_NR_TO_ROOT(section_nr);
	struct mem_section *section;

	// 해당 루트 인덱스의 루트 섹션에 값이 존재하는 경우 이미 mem_section[] 테이블이 구성되었으므로 함수를 빠져나간다.
	if (mem_section[root])
		return -EEXIST;

	// 해당 노드에서 2 단계용 섹션 테이블(section)을 할당받아 구성한다. 핫플러그 메모리를 위해 각 mem_section[] 테이블은 해당 노드에 위치해야 한다.
	section = sparse_index_alloc(nid);
	if (!section)
		return -ENOMEM;

	// 루트 번호에 해당하는 1단계 mem_section[] 포인터 배열에 새로 할당받은 2 단계 mem_section[] 테이블의 시작 주소를 설정한다.
	mem_section[root] = section;

	return 0;
}
#else /* !SPARSEMEM_EXTREME */
static inline int sparse_index_init(unsigned long section_nr, int nid)
{
	return 0;
}
#endif

#ifdef CONFIG_SPARSEMEM_EXTREME
int __section_nr(struct mem_section* ms)
{
	unsigned long root_nr;
	struct mem_section *root = NULL;

	for (root_nr = 0; root_nr < NR_SECTION_ROOTS; root_nr++) {
		root = __nr_to_section(root_nr * SECTIONS_PER_ROOT);
		if (!root)
			continue;

		if ((ms >= root) && (ms < (root + SECTIONS_PER_ROOT)))
		     break;
	}

	VM_BUG_ON(!root);

	return (root_nr * SECTIONS_PER_ROOT) + (ms - root);
}
#else
int __section_nr(struct mem_section* ms)
{
	return (int)(ms - mem_section[0]);
}
#endif

/*
 * During early boot, before section_mem_map is used for an actual
 * mem_map, we use section_mem_map to store the section's NUMA
 * node.  This keeps us from having to use another data structure.  The
 * node information is cleared just before we store the real mem_map.
 */
// SECTION_NID_SHIFT = 3
static inline unsigned long sparse_encode_early_nid(int nid)
{
	return (nid << SECTION_NID_SHIFT);
}

static inline int sparse_early_nid(struct mem_section *section)
{
	return (section->section_mem_map >> SECTION_NID_SHIFT);
}

/* Validate the physical addressing limitations of the model */
// 인자로 사용된 시작 pfn, 끝 pfn 값이 물리 메모리 주소 최대 pfn 값을 초과하지 않도록 제한한다.
void __meminit mminit_validate_memmodel_limits(unsigned long *start_pfn,
						unsigned long *end_pfn)
{
    // max_sparsemem_pfn = 1UL << 36 = 64G
	unsigned long max_sparsemem_pfn = 1UL << (MAX_PHYSMEM_BITS-PAGE_SHIFT);

	/*
	 * Sanity checks - do not allow an architecture to pass
	 * in larger pfns than the maximum scope of sparsemem:
	 */
	// 시작 pfn이 max_sparsemem_pfn보다 크면 경고를 출력하고, 
	// start_pfn과 end_pfn에 max_sparsemem_pfn을 설정한다.
	if (*start_pfn > max_sparsemem_pfn) {
		mminit_dprintk(MMINIT_WARNING, "pfnvalidation",
			"Start of range %lu -> %lu exceeds SPARSEMEM max %lu\n",
			*start_pfn, *end_pfn, max_sparsemem_pfn);
		WARN_ON_ONCE(1);
		*start_pfn = max_sparsemem_pfn;
		*end_pfn = max_sparsemem_pfn;
	} else if (*end_pfn > max_sparsemem_pfn) { // 끝 pfn이 max_sparsemem_pfn보다 크면 경고를 출력하고, end_pfn에 max_sparsemem_pfn을 설정한다.
		mminit_dprintk(MMINIT_WARNING, "pfnvalidation",
			"End of range %lu -> %lu exceeds SPARSEMEM max %lu\n",
			*start_pfn, *end_pfn, max_sparsemem_pfn);
		WARN_ON_ONCE(1);
		*end_pfn = max_sparsemem_pfn;
	}
}

/*
 * There are a number of times that we loop over NR_MEM_SECTIONS,
 * looking for section_present() on each.  But, when we have very
 * large physical address spaces, NR_MEM_SECTIONS can also be
 * very large which makes the loops quite long.
 *
 * Keeping track of this gives us an easy way to break out of
 * those loops early.
 */
int __highest_present_section_nr;
static void section_mark_present(struct mem_section *ms)
{
	int section_nr = __section_nr(ms);

	if (section_nr > __highest_present_section_nr)
		__highest_present_section_nr = section_nr;

	ms->section_mem_map |= SECTION_MARKED_PRESENT;
}

static inline int next_present_section_nr(int section_nr)
{
	do {
		section_nr++;
		if (present_section_nr(section_nr))
			return section_nr;
	} while ((section_nr <= __highest_present_section_nr));

	return -1;
}
#define for_each_present_section_nr(start, section_nr)		\
	for (section_nr = next_present_section_nr(start-1);	\
	     ((section_nr != -1) &&				\
	      (section_nr <= __highest_present_section_nr));	\
	     section_nr = next_present_section_nr(section_nr))

static inline unsigned long first_present_section_nr(void)
{
	return next_present_section_nr(-1);
}

/* Record a memory area against a node. */
// http://jake.dothome.co.kr/sparsemem/
void __init memory_present(int nid, unsigned long start, unsigned long end)
{
	unsigned long pfn;

// CONFIG_SPARSEMEM_EXTREME 커널 옵션을 사용하는 경우 
// 처음 mem_section이 초기화되지 않은 경우 mem_section[] 배열을 생성한다

// mem_section은 이중 포인터(논리적으로, 2차 배열)로 구성되어 있음
// mem_section index 크기 : NR_SECTION_ROOTS * SECTION_PER_ROOT
// NR_SECTION_ROOTS = 1024
// SECTION_PER_ROOT = 256 (See mmzone.h)
#ifdef CONFIG_SPARSEMEM_EXTREME
	if (unlikely(!mem_section)) { // 1단계 배열 할당 확인
		unsigned long size, align;

		size = sizeof(struct mem_section*) * NR_SECTION_ROOTS;
		// INTERNODE_CACHE_SHIFT = 6 (@ L1 cache line)
		align = 1 << (INTERNODE_CACHE_SHIFT);
		// size = 8K, align = 64
		mem_section = memblock_alloc(size, align);
		if (!mem_section)
			panic("%s: Failed to allocate %lu bytes align=0x%lx\n",
			      __func__, size, align);
	}
#endif

	// 요청한 범위의 시작 pfn을 섹션 단위로 내림 정렬한 주소로 변환한 후 
	// 해당 범위의 pfn이 실제로 지정할 수 있는 물리 메모리 범위에 포함되는지 검증하고 초과 시 그 주소를 강제로 제한한다.
	// start를 section 단위(PAGE_PER_SECTION)에 맞춰 내림 (PAGE_SECTION_MASK = (~(256K-1)))
	start &= PAGE_SECTION_MASK;
	mminit_validate_memmodel_limits(&start, &end);
	// 시작 pfn부터 섹션 단위로 증가시키며 이 값으로 section 번호를 구한다.
	for (pfn = start; pfn < end; pfn += PAGES_PER_SECTION) { // 섹션 단위로 증가
		unsigned long section = pfn_to_section_nr(pfn);
		struct mem_section *ms;

		// CONFIG_SPARSEMEM_EXTREME 커널 옵션을 사용하는 경우에만 동작하며, 
		// 이 함수에서 해당 섹션에 대한 mem_section[]을 동적으로(dynamic) 할당받는다. 
		// CONFIG_SPARSEMEM_STATIC 커널 옵션을 사용하는 경우에는 이미 정적(static) 배열이 준비되어 있으므로 
		// 아무런 동작도 요구되지 않는다
		sparse_index_init(section, nid);
		// NODE_NOT_IN_PAGE_FLAGS가 정의된 경우 
		// 별도의 전역 section_to_node_table[] 배열에 해당 섹션을 인덱스로 해당 노드 id를 가리키게 한다.
		// NODE_NOT_IN_PAGE_FLAGS 커널 옵션은 page 구조체의 flags 필드에 노드 번호를 저장할 비트가 
		// 부족한 32비트 아키텍처에서 사용되는 옵션이다.
		set_section_nid(section, nid);

		// 해당 섹션의 mem_section 구조체내의 section_mem_map에 노드 id와 online 및 present 플래그를 설정한다.
 
		// SECTION_IS_ONLINE   (1UL<<2)
		// 섹션넘버에 해당하는 memory section 구조체의 주소를 리턴
		ms = __nr_to_section(section);
		if (!ms->section_mem_map) { // memory section 구조체를 설정
			// ms->section_mem_map = nid <<3 | 1UL << 2
			ms->section_mem_map = sparse_encode_early_nid(nid) |
							SECTION_IS_ONLINE;
			section_mark_present(ms);
		}
	}
}

/*
 * Mark all memblocks as present using memory_present(). This is a
 * convienence function that is useful for a number of arches
 * to mark all of the systems memory as present during initialization.
 */
void __init memblocks_present(void)
{
	struct memblock_region *reg;

	for_each_memblock(memory, reg) {
		memory_present(memblock_get_region_node(reg),
			       memblock_region_memory_base_pfn(reg),
			       memblock_region_memory_end_pfn(reg));
	}
}

/*
 * Subtle, we encode the real pfn into the mem_map such that
 * the identity pfn - section_mem_map will return the actual
 * physical page frame number.
 */
static unsigned long sparse_encode_mem_map(struct page *mem_map, unsigned long pnum)
{
	unsigned long coded_mem_map =
		(unsigned long)(mem_map - (section_nr_to_pfn(pnum)));
	BUILD_BUG_ON(SECTION_MAP_LAST_BIT > (1UL<<PFN_SECTION_SHIFT));
	BUG_ON(coded_mem_map & ~SECTION_MAP_MASK);
	return coded_mem_map;
}

/*
 * Decode mem_map from the coded memmap
 */
struct page *sparse_decode_mem_map(unsigned long coded_mem_map, unsigned long pnum)
{
	/* mask off the extra low bits of information */
	coded_mem_map &= SECTION_MAP_MASK;
	return ((struct page *)coded_mem_map) + section_nr_to_pfn(pnum);
}

/*
 * sparse_init_one_section   545  sparse_init_one_section(__nr_to_section(pnum), pnum, map, usemap)
 */

static void __meminit sparse_init_one_section(struct mem_section *ms,
		unsigned long pnum, struct page *mem_map,
		unsigned long *pageblock_bitmap)
{
	ms->section_mem_map &= ~SECTION_MAP_MASK; // *** &= 0b111
	ms->section_mem_map |= sparse_encode_mem_map(mem_map, pnum) |
							SECTION_HAS_MEM_MAP; // *** |= encoded_stuff | 2
 	ms->pageblock_flags = pageblock_bitmap; // usemap -> page_block_flags로 넣어준다.
	// pageblock_flags는 usemap을 가리키는 주소이다.
}

unsigned long usemap_size(void)
{
	return BITS_TO_LONGS(SECTION_BLOCKFLAGS_BITS) * sizeof(unsigned long);
}

#ifdef CONFIG_MEMORY_HOTPLUG
static unsigned long *__kmalloc_section_usemap(void)
{
	return kmalloc(usemap_size(), GFP_KERNEL);
}
#endif /* CONFIG_MEMORY_HOTPLUG */

#ifdef CONFIG_MEMORY_HOTREMOVE
static unsigned long * __init
sparse_early_usemaps_alloc_pgdat_section(struct pglist_data *pgdat,
					 unsigned long size)
{
	unsigned long goal, limit;
	unsigned long *p;
	int nid;
	/*
	 * A page may contain usemaps for other sections preventing the
	 * page being freed and making a section unremovable while
	 * other sections referencing the usemap remain active. Similarly,
	 * a pgdat can prevent a section being removed. If section A
	 * contains a pgdat and section B contains the usemap, both
	 * sections become inter-dependent. This allocates usemaps
	 * from the same section as the pgdat where possible to avoid
	 * this problem.
	 */
	goal = __pa(pgdat) & (PAGE_SECTION_MASK << PAGE_SHIFT);
	limit = goal + (1UL << PA_SECTION_SHIFT);
	nid = early_pfn_to_nid(goal >> PAGE_SHIFT);
again:
	p = memblock_alloc_try_nid(size, SMP_CACHE_BYTES, goal, limit, nid);
	if (!p && limit) {
		limit = 0;
		goto again;
	}
	return p;
}

/*
*  check_usemap_section_nr   540  check_usemap_section_nr(nid, usemap)
*/

static void __init check_usemap_section_nr(int nid, unsigned long *usemap)
{
	unsigned long usemap_snr, pgdat_snr;
	static unsigned long old_usemap_snr;
	static unsigned long old_pgdat_snr;
	struct pglist_data *pgdat = NODE_DATA(nid);
	int usemap_nid;

	/* First call */
	if (!old_usemap_snr) {
		old_usemap_snr = NR_MEM_SECTIONS;
		old_pgdat_snr = NR_MEM_SECTIONS;
	}

	usemap_snr = pfn_to_section_nr(__pa(usemap) >> PAGE_SHIFT);
	pgdat_snr = pfn_to_section_nr(__pa(pgdat) >> PAGE_SHIFT);
	if (usemap_snr == pgdat_snr)
		return;

	if (old_usemap_snr == usemap_snr && old_pgdat_snr == pgdat_snr)
		/* skip redundant message */
		return;

	old_usemap_snr = usemap_snr;
	old_pgdat_snr = pgdat_snr;

	usemap_nid = sparse_early_nid(__nr_to_section(usemap_snr));
	if (usemap_nid != nid) {
		pr_info("node %d must be removed before remove section %ld\n",
			nid, usemap_snr);
		return;
	}
	/*
	 * There is a circular dependency.
	 * Some platforms allow un-removable section because they will just
	 * gather other removable sections for dynamic partitioning.
	 * Just notify un-removable section's number here.
	 */
	pr_info("Section %ld and %ld (node %d) have a circular dependency on usemap and pgdat allocations\n",
		usemap_snr, pgdat_snr, nid);
}
#else
static unsigned long * __init
sparse_early_usemaps_alloc_pgdat_section(struct pglist_data *pgdat,
					 unsigned long size)
{
	return memblock_alloc_node(size, SMP_CACHE_BYTES, pgdat->node_id);
}

static void __init check_usemap_section_nr(int nid, unsigned long *usemap)
{
}
#endif /* CONFIG_MEMORY_HOTREMOVE */

#ifdef CONFIG_SPARSEMEM_VMEMMAP
static unsigned long __init section_map_size(void)
{
	return ALIGN(sizeof(struct page) * PAGES_PER_SECTION, PMD_SIZE);
}

#else
static unsigned long __init section_map_size(void)
{
	return PAGE_ALIGN(sizeof(struct page) * PAGES_PER_SECTION);
}

struct page __init *sparse_mem_map_populate(unsigned long pnum, int nid,
		struct vmem_altmap *altmap)
{
	unsigned long size = section_map_size();
	struct page *map = sparse_buffer_alloc(size);
	phys_addr_t addr = __pa(MAX_DMA_ADDRESS);

	if (map)
		return map;

	map = memblock_alloc_try_nid(size,
					  PAGE_SIZE, addr,
					  MEMBLOCK_ALLOC_ACCESSIBLE, nid);
	if (!map)
		panic("%s: Failed to allocate %lu bytes align=0x%lx nid=%d from=%pa\n",
		      __func__, size, PAGE_SIZE, nid, &addr);

	return map;
}
#endif /* !CONFIG_SPARSEMEM_VMEMMAP */

static void *sparsemap_buf __meminitdata;
static void *sparsemap_buf_end __meminitdata;

static void __init sparse_buffer_init(unsigned long size, int nid)
{
	phys_addr_t addr = __pa(MAX_DMA_ADDRESS);
	WARN_ON(sparsemap_buf);	/* forgot to call sparse_buffer_fini()? */
	sparsemap_buf =
		memblock_alloc_try_nid_raw(size, PAGE_SIZE,
						addr,
						MEMBLOCK_ALLOC_ACCESSIBLE, nid);
	sparsemap_buf_end = sparsemap_buf + size;
}

/*
 * sparse_buffer_fini (finish아닐까 생각함)
 * sparse햇던 sparsemap_buf 에 대해서 유효한지 체크하고
 * memblock을 할당해제한다.
 */
static void __init sparse_buffer_fini(void)
{
	unsigned long size = sparsemap_buf_end - sparsemap_buf;

	if (sparsemap_buf && size > 0) // 유효체크 
		memblock_free_early(__pa(sparsemap_buf), size);
	sparsemap_buf = NULL;
}

void * __meminit sparse_buffer_alloc(unsigned long size)
{
	void *ptr = NULL;

	if (sparsemap_buf) {
		ptr = PTR_ALIGN(sparsemap_buf, size);
		if (ptr + size > sparsemap_buf_end)
			ptr = NULL;
		else
			sparsemap_buf = ptr + size;
	}
	return ptr;
}

void __weak __meminit vmemmap_populate_print_last(void)
{
}

/*
 * Initialize sparse on a specific node. The node spans [pnum_begin, pnum_end)
 * And number of present sections in this node is map_count.
usemap : memory allign ? 할때 연속된 공간을 확보하기위한 사전정보
 */
static void __init sparse_init_nid(int nid, unsigned long pnum_begin,
				   unsigned long pnum_end,
				   unsigned long map_count)
{
	unsigned long pnum, usemap_longs, *usemap;
	struct page *map;

	usemap_longs = BITS_TO_LONGS(SECTION_BLOCKFLAGS_BITS);
	usemap = sparse_early_usemaps_alloc_pgdat_section(NODE_DATA(nid),
							  usemap_size() *
							  map_count);
	if (!usemap) {
		pr_err("%s: node[%d] usemap allocation failed", __func__, nid);
		goto failed;
	}
	sparse_buffer_init(map_count * section_map_size(), nid);
	for_each_present_section_nr(pnum_begin, pnum) {
		if (pnum >= pnum_end)
			break;

		map = sparse_mem_map_populate(pnum, nid, NULL);
		if (!map) {
			pr_err("%s: node[%d] memory map backing failed. Some memory will not be available.",
			       __func__, nid);
			pnum_begin = pnum;
			goto failed;
		}
		check_usemap_section_nr(nid, usemap); // default로 비어있는 함수로 실행되기에 아무것도 수행되지 않는다.
		// for_each를 돌때마다 *usemap 과 pnum이 각각 가산되서 init을 한다.
		// mem_map 영역을 엔코딩하여 가상주소를 mem_section에 연결하고 usemap 영역도 연결한다.
		sparse_init_one_section(__nr_to_section(pnum), pnum, map, usemap);
		usemap += usemap_longs;
	}
	// free sparsemap_buf which was allocated @ sparse_buffer_init()
	sparse_buffer_fini();
	return;
failed:
	/* We failed to allocate, mark all the following pnums as not present */
	/*
	 * 1. usemap이 없는겨우
	 * 2. for each를 돌다가 특정 nid에 대한 memory map을 가져오는데 실패한경우
	 */
	for_each_present_section_nr(pnum_begin, pnum) {
		struct mem_section *ms;

		if (pnum >= pnum_end)
			break;
		ms = __nr_to_section(pnum);
		ms->section_mem_map = 0;
	}
	/*
	 * 실패하면 memsection에 메모리가 없다고 0으로 flag를 clear한다.
	 */
}

/*
 * Allocate the accumulated non-linear sections, allocate a mem_map
 * for each and record the physical to section mapping.
 */
void __init sparse_init(void)
{
	// 첫번째 섹션의 NID를 mem_section으로 부터 구해서 nid_begin에 저장
	unsigned long pnum_begin = first_present_section_nr();
	int nid_begin = sparse_early_nid(__nr_to_section(pnum_begin));
	unsigned long pnum_end, map_count = 1;

	/* Setup pageblock_order for HUGETLB_PAGE_SIZE_VARIABLE */
	// ARM64 시스템의 디폴트 설정은 pageblock_order 값에 9을 사용한다.
	set_pageblock_order();

	for_each_present_section_nr(pnum_begin + 1, pnum_end) {
		int nid = sparse_early_nid(__nr_to_section(pnum_end));

		if (nid == nid_begin) {
			map_count++;
			continue;
		}
		/* Init node with sections in range [pnum_begin, pnum_end) */
		// for_each_present_section_nr(pnum_begin, pnum) 
		// for_each로 sparse할때에 겉에서는 전체 section에 대해 반복하면서
		// nid가 이전과 동일하면 continue하지만, 달라지면 같은 nid끼리 처리를 한다.
		sparse_init_nid(nid_begin, pnum_begin, pnum_end, map_count);
		nid_begin = nid;
		pnum_begin = pnum_end;
		map_count = 1;
		// nid_begin ... map_count까지는 sparse_init_nid에서 처리했던 nid의 다음 nid를 위한 초기화를 진행한다.
	}
	/* cover the last node */
	sparse_init_nid(nid_begin, pnum_begin, pnum_end, map_count); // 마지막 nid는 손수 sparse하는데 nid_beging을 마지막에 건들진 않는다.
	vmemmap_populate_print_last();
}

#ifdef CONFIG_MEMORY_HOTPLUG

/* Mark all memory sections within the pfn range as online */
void online_mem_sections(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long pfn;

	for (pfn = start_pfn; pfn < end_pfn; pfn += PAGES_PER_SECTION) {
		unsigned long section_nr = pfn_to_section_nr(pfn);
		struct mem_section *ms;

		/* onlining code should never touch invalid ranges */
		if (WARN_ON(!valid_section_nr(section_nr)))
			continue;

		ms = __nr_to_section(section_nr);
		ms->section_mem_map |= SECTION_IS_ONLINE;
	}
}

#ifdef CONFIG_MEMORY_HOTREMOVE
/* Mark all memory sections within the pfn range as offline */
void offline_mem_sections(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long pfn;

	for (pfn = start_pfn; pfn < end_pfn; pfn += PAGES_PER_SECTION) {
		unsigned long section_nr = pfn_to_section_nr(pfn);
		struct mem_section *ms;

		/*
		 * TODO this needs some double checking. Offlining code makes
		 * sure to check pfn_valid but those checks might be just bogus
		 */
		if (WARN_ON(!valid_section_nr(section_nr)))
			continue;

		ms = __nr_to_section(section_nr);
		ms->section_mem_map &= ~SECTION_IS_ONLINE;
	}
}
#endif

#ifdef CONFIG_SPARSEMEM_VMEMMAP
static inline struct page *kmalloc_section_memmap(unsigned long pnum, int nid,
		struct vmem_altmap *altmap)
{
	/* This will make the necessary allocations eventually. */
	return sparse_mem_map_populate(pnum, nid, altmap);
}
static void __kfree_section_memmap(struct page *memmap,
		struct vmem_altmap *altmap)
{
	unsigned long start = (unsigned long)memmap;
	unsigned long end = (unsigned long)(memmap + PAGES_PER_SECTION);

	vmemmap_free(start, end, altmap);
}
#ifdef CONFIG_MEMORY_HOTREMOVE
static void free_map_bootmem(struct page *memmap)
{
	unsigned long start = (unsigned long)memmap;
	unsigned long end = (unsigned long)(memmap + PAGES_PER_SECTION);

	vmemmap_free(start, end, NULL);
}
#endif /* CONFIG_MEMORY_HOTREMOVE */
#else
static struct page *__kmalloc_section_memmap(void)
{
	struct page *page, *ret;
	unsigned long memmap_size = sizeof(struct page) * PAGES_PER_SECTION;

	page = alloc_pages(GFP_KERNEL|__GFP_NOWARN, get_order(memmap_size));
	if (page)
		goto got_map_page;

	ret = vmalloc(memmap_size);
	if (ret)
		goto got_map_ptr;

	return NULL;
got_map_page:
	ret = (struct page *)pfn_to_kaddr(page_to_pfn(page));
got_map_ptr:

	return ret;
}

static inline struct page *kmalloc_section_memmap(unsigned long pnum, int nid,
		struct vmem_altmap *altmap)
{
	return __kmalloc_section_memmap();
}

static void __kfree_section_memmap(struct page *memmap,
		struct vmem_altmap *altmap)
{
	if (is_vmalloc_addr(memmap))
		vfree(memmap);
	else
		free_pages((unsigned long)memmap,
			   get_order(sizeof(struct page) * PAGES_PER_SECTION));
}

#ifdef CONFIG_MEMORY_HOTREMOVE
static void free_map_bootmem(struct page *memmap)
{
	unsigned long maps_section_nr, removing_section_nr, i;
	unsigned long magic, nr_pages;
	struct page *page = virt_to_page(memmap);

	nr_pages = PAGE_ALIGN(PAGES_PER_SECTION * sizeof(struct page))
		>> PAGE_SHIFT;

	for (i = 0; i < nr_pages; i++, page++) {
		magic = (unsigned long) page->freelist;

		BUG_ON(magic == NODE_INFO);

		maps_section_nr = pfn_to_section_nr(page_to_pfn(page));
		removing_section_nr = page_private(page);

		/*
		 * When this function is called, the removing section is
		 * logical offlined state. This means all pages are isolated
		 * from page allocator. If removing section's memmap is placed
		 * on the same section, it must not be freed.
		 * If it is freed, page allocator may allocate it which will
		 * be removed physically soon.
		 */
		if (maps_section_nr != removing_section_nr)
			put_page_bootmem(page);
	}
}
#endif /* CONFIG_MEMORY_HOTREMOVE */
#endif /* CONFIG_SPARSEMEM_VMEMMAP */

/*
 * returns the number of sections whose mem_maps were properly
 * set.  If this is <=0, then that means that the passed-in
 * map was not consumed and must be freed.
 */
int __meminit sparse_add_one_section(int nid, unsigned long start_pfn,
				     struct vmem_altmap *altmap)
{
	unsigned long section_nr = pfn_to_section_nr(start_pfn);
	struct mem_section *ms;
	struct page *memmap;
	unsigned long *usemap;
	int ret;

	/*
	 * no locking for this, because it does its own
	 * plus, it does a kmalloc
	 */
	ret = sparse_index_init(section_nr, nid);
	if (ret < 0 && ret != -EEXIST)
		return ret;
	ret = 0;
	memmap = kmalloc_section_memmap(section_nr, nid, altmap);
	if (!memmap)
		return -ENOMEM;
	usemap = __kmalloc_section_usemap();
	if (!usemap) {
		__kfree_section_memmap(memmap, altmap);
		return -ENOMEM;
	}

	ms = __pfn_to_section(start_pfn);
	if (ms->section_mem_map & SECTION_MARKED_PRESENT) {
		ret = -EEXIST;
		goto out;
	}

	/*
	 * Poison uninitialized struct pages in order to catch invalid flags
	 * combinations.
	 */
	page_init_poison(memmap, sizeof(struct page) * PAGES_PER_SECTION);

	section_mark_present(ms);
	sparse_init_one_section(ms, section_nr, memmap, usemap);

out:
	if (ret < 0) {
		kfree(usemap);
		__kfree_section_memmap(memmap, altmap);
	}
	return ret;
}

#ifdef CONFIG_MEMORY_HOTREMOVE
#ifdef CONFIG_MEMORY_FAILURE
static void clear_hwpoisoned_pages(struct page *memmap, int nr_pages)
{
	int i;

	if (!memmap)
		return;

	/*
	 * A further optimization is to have per section refcounted
	 * num_poisoned_pages.  But that would need more space per memmap, so
	 * for now just do a quick global check to speed up this routine in the
	 * absence of bad pages.
	 */
	if (atomic_long_read(&num_poisoned_pages) == 0)
		return;

	for (i = 0; i < nr_pages; i++) {
		if (PageHWPoison(&memmap[i])) {
			atomic_long_sub(1, &num_poisoned_pages);
			ClearPageHWPoison(&memmap[i]);
		}
	}
}
#else
static inline void clear_hwpoisoned_pages(struct page *memmap, int nr_pages)
{
}
#endif

static void free_section_usemap(struct page *memmap, unsigned long *usemap,
		struct vmem_altmap *altmap)
{
	struct page *usemap_page;

	if (!usemap)
		return;

	usemap_page = virt_to_page(usemap);
	/*
	 * Check to see if allocation came from hot-plug-add
	 */
	if (PageSlab(usemap_page) || PageCompound(usemap_page)) {
		kfree(usemap);
		if (memmap)
			__kfree_section_memmap(memmap, altmap);
		return;
	}

	/*
	 * The usemap came from bootmem. This is packed with other usemaps
	 * on the section which has pgdat at boot time. Just keep it as is now.
	 */

	if (memmap)
		free_map_bootmem(memmap);
}

void sparse_remove_one_section(struct zone *zone, struct mem_section *ms,
		unsigned long map_offset, struct vmem_altmap *altmap)
{
	struct page *memmap = NULL;
	unsigned long *usemap = NULL;

	if (ms->section_mem_map) {
		usemap = ms->pageblock_flags;
		memmap = sparse_decode_mem_map(ms->section_mem_map,
						__section_nr(ms));
		ms->section_mem_map = 0;
		ms->pageblock_flags = NULL;
	}

	clear_hwpoisoned_pages(memmap + map_offset,
			PAGES_PER_SECTION - map_offset);
	free_section_usemap(memmap, usemap, altmap);
}
#endif /* CONFIG_MEMORY_HOTREMOVE */
#endif /* CONFIG_MEMORY_HOTPLUG */
