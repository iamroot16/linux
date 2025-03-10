/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 *
 * libfdt is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"

/*
 * Minimal sanity check for a read-only tree. fdt_ro_probe_() checks
 * that the given buffer contains what appears to be a flattened
 * device tree with sane information in its header.
 */
int fdt_ro_probe_(const void *fdt)
{
	if (fdt_magic(fdt) == FDT_MAGIC) {
		/* Complete tree */
		if (fdt_version(fdt) < FDT_FIRST_SUPPORTED_VERSION)
			return -FDT_ERR_BADVERSION;
		if (fdt_last_comp_version(fdt) > FDT_LAST_SUPPORTED_VERSION)
			return -FDT_ERR_BADVERSION;
	} else if (fdt_magic(fdt) == FDT_SW_MAGIC) {
		/* Unfinished sequential-write blob */
		if (fdt_size_dt_struct(fdt) == 0)
			return -FDT_ERR_BADSTATE;
	} else {
		return -FDT_ERR_BADMAGIC;
	}

	return 0;
}

static int check_off_(uint32_t hdrsize, uint32_t totalsize, uint32_t off)
{
	return (off >= hdrsize) && (off <= totalsize);
}

static int check_block_(uint32_t hdrsize, uint32_t totalsize,
			uint32_t base, uint32_t size)
{
	if (!check_off_(hdrsize, totalsize, base))
		return 0; /* block start out of bounds */
	if ((base + size) < base)
		return 0; /* overflow */
	if (!check_off_(hdrsize, totalsize, base + size))
		return 0; /* block end out of bounds */
	return 1;
}

size_t fdt_header_size_(uint32_t version)
{
	if (version <= 1)
		return FDT_V1_SIZE;
	else if (version <= 2)
		return FDT_V2_SIZE;
	else if (version <= 3)
		return FDT_V3_SIZE;
	else if (version <= 16)
		return FDT_V16_SIZE;
	else
		return FDT_V17_SIZE;
}

int fdt_check_header(const void *fdt)
{
	size_t hdrsize;

	if (fdt_magic(fdt) != FDT_MAGIC)
		return -FDT_ERR_BADMAGIC;
	hdrsize = fdt_header_size(fdt);
	// fdt_header의version에 따라서 fdt_header의 size를 내보내줌 
	if ((fdt_version(fdt) < FDT_FIRST_SUPPORTED_VERSION)
	    || (fdt_last_comp_version(fdt) > FDT_LAST_SUPPORTED_VERSION))
		return -FDT_ERR_BADVERSION;
	if (fdt_version(fdt) < fdt_last_comp_version(fdt))
		return -FDT_ERR_BADVERSION;
	// fdt_header의 version과 last_comp_version이 동일해야한다.

	if ((fdt_totalsize(fdt) < hdrsize)
	    || (fdt_totalsize(fdt) > INT_MAX))
		return -FDT_ERR_TRUNCATED;

	/* Bounds check memrsv block */
	if (!check_off_(hdrsize, fdt_totalsize(fdt), fdt_off_mem_rsvmap(fdt)))
		return -FDT_ERR_TRUNCATED;

	/* Bounds check structure block */
	if (fdt_version(fdt) < 17) {
		if (!check_off_(hdrsize, fdt_totalsize(fdt),
				fdt_off_dt_struct(fdt)))
			return -FDT_ERR_TRUNCATED;
	} else {
		if (!check_block_(hdrsize, fdt_totalsize(fdt),
				  fdt_off_dt_struct(fdt),
				  fdt_size_dt_struct(fdt)))
			return -FDT_ERR_TRUNCATED;
	}

	/* Bounds check strings block */
	if (!check_block_(hdrsize, fdt_totalsize(fdt),
			  fdt_off_dt_strings(fdt), fdt_size_dt_strings(fdt)))
		return -FDT_ERR_TRUNCATED;

	return 0;
}

const void *fdt_offset_ptr(const void *fdt, int offset, unsigned int len)
{
	unsigned absoffset = offset + fdt_off_dt_struct(fdt);
	// offset = structure block 안에서의 offset
	// fdt_off_dt_struct -> fdt 시작 주소 ~ structure block 시작주소
	//absolute offset = fdt 시작 주소부터의 offset 계산

	if ((absoffset < offset)
	    || ((absoffset + len) < absoffset)
	    || (absoffset + len) > fdt_totalsize(fdt))
		return NULL;
	// len = FDT_TAGSIZE
	if (fdt_version(fdt) >= 0x11)
		if (((offset + len) < offset)
		    || ((offset + len) > fdt_size_dt_struct(fdt)))
			return NULL;

	return fdt_offset_ptr_(fdt, offset);
	// offset에 해당되는 가상 주소를 return 
}

// 다음 노드의 offset를 리턴하고 출력 인수에 depth를 저장한다.
// http://jake.dothome.co.kr/dtb-fdt-api/
uint32_t fdt_next_tag(const void *fdt, int startoffset, int *nextoffset)
{
	const fdt32_t *tagp, *lenp;
	uint32_t tag;
	int offset = startoffset;
	// offset = 현재 node의 시작 주소
	const char *p;

	*nextoffset = -FDT_ERR_TRUNCATED;
	tagp = fdt_offset_ptr(fdt, offset, FDT_TAGSIZE);
	// tagp = (void *) 현재 node(tag)의 시작 주소(가상 주소)
	if (!tagp)
		return FDT_END; /* premature end */
	tag = fdt32_to_cpu(*tagp);
	// 현재 node(혹은 tag)의 시작 주소값(가상 주소)의  endian을 local system에 맞춰서 tag에 저장 
	offset += FDT_TAGSIZE;

	*nextoffset = -FDT_ERR_BADSTRUCTURE;
	switch (tag) {
	case FDT_BEGIN_NODE:
		/* skip name */
		do {
			p = fdt_offset_ptr(fdt, offset++, 1);
		} while (p && (*p != '\0'));
		if (!p)
			return FDT_END; /* premature end */
		break;

	case FDT_PROP:
		lenp = fdt_offset_ptr(fdt, offset, sizeof(*lenp));
		if (!lenp)
			return FDT_END; /* premature end */
		/* skip-name offset, length and value */
		offset += sizeof(struct fdt_property) - FDT_TAGSIZE
			+ fdt32_to_cpu(*lenp);
		if (fdt_version(fdt) < 0x10 && fdt32_to_cpu(*lenp) >= 8 &&
		    ((offset - fdt32_to_cpu(*lenp)) % 8) != 0)
			offset += 4;
		break;

	case FDT_END:
	case FDT_END_NODE:
	case FDT_NOP:
		break;

	default:
		return FDT_END;
	}

	if (!fdt_offset_ptr(fdt, startoffset, offset - startoffset))
		return FDT_END; /* premature end */

	*nextoffset = FDT_TAGALIGN(offset);
	return tag;
}

int fdt_check_node_offset_(const void *fdt, int offset)
{
	if ((offset < 0) || (offset % FDT_TAGSIZE)
	    || (fdt_next_tag(fdt, offset, &offset) != FDT_BEGIN_NODE))
		return -FDT_ERR_BADOFFSET;

	return offset;
}

int fdt_check_prop_offset_(const void *fdt, int offset)
{
	if ((offset < 0) || (offset % FDT_TAGSIZE)
	    || (fdt_next_tag(fdt, offset, &offset) != FDT_PROP))
		return -FDT_ERR_BADOFFSET;

	return offset;
}

// 다음 노드의 offset를 리턴하고 출력 인수에 depth를 저장한다.
// http://jake.dothome.co.kr/dtb-fdt-api/
int fdt_next_node(const void *fdt, int offset, int *depth)
{
	int nextoffset = 0;
	uint32_t tag;

	// 주어진 offset가 노드가 아닌 경우 null을 리턴한다.
	if (offset >= 0)
		if ((nextoffset = fdt_check_node_offset_(fdt, offset)) < 0)
			return nextoffset;

	// 루프를 돌며 태그 번호와 다음 태그 offset을 알아오고 태그에 따라 다음과 같이 동작한다.
	do {
		offset = nextoffset;
		tag = fdt_next_tag(fdt, offset, &nextoffset);

		switch (tag) {
		case FDT_PROP:
		case FDT_NOP:
			break;

		case FDT_BEGIN_NODE: 
			// depth 증가하게 되고 루프를 벗어나게 된다.
			if (depth)
				(*depth)++;
			break;

		case FDT_END_NODE: 
			// depth가 0이상이면서 감소 시킨 depth가 0보다 작은 경우 
			// nextoffset를 리턴한다. (보통 또 다른 시작 노드)
			if (depth && ((--(*depth)) < 0))
				return nextoffset;
			break;

		case FDT_END:
			//  음수 갑ㅅ인 에러를 리턴한다.
			if ((nextoffset >= 0)
			    || ((nextoffset == -FDT_ERR_TRUNCATED) && !depth))
				return -FDT_ERR_NOTFOUND;
			else
				return nextoffset;
		}
	} while (tag != FDT_BEGIN_NODE);

	return offset;
}

int fdt_first_subnode(const void *fdt, int offset)
{
	int depth = 0;

	offset = fdt_next_node(fdt, offset, &depth);
	if (offset < 0 || depth != 1)
		return -FDT_ERR_NOTFOUND;

	return offset;
}

int fdt_next_subnode(const void *fdt, int offset)
{
	int depth = 1;

	/*
	 * With respect to the parent, the depth of the next subnode will be
	 * the same as the last.
	 */
	do {
		offset = fdt_next_node(fdt, offset, &depth);
		if (offset < 0 || depth < 1)
			return -FDT_ERR_NOTFOUND;
	} while (depth > 1);

	return offset;
}

const char *fdt_find_string_(const char *strtab, int tabsize, const char *s)
{
	int len = strlen(s) + 1;
	const char *last = strtab + tabsize - len;
	const char *p;

	for (p = strtab; p <= last; p++)
		if (memcmp(p, s, len) == 0)
			return p;
	return NULL;
}

int fdt_move(const void *fdt, void *buf, int bufsize)
{
	FDT_RO_PROBE(fdt);

	if (fdt_totalsize(fdt) > bufsize)
		return -FDT_ERR_NOSPACE;

	memmove(buf, fdt, fdt_totalsize(fdt));
	return 0;
}
