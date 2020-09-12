// SPDX-License-Identifier: GPL-2.0
/*
 * A fast, small, non-recursive O(nlog n) sort for the Linux kernel
 *
 * Jan 23 2005  Matt Mackall <mpm@selenic.com>
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sort.h>

static int alignment_ok(const void *base, int align)
{
	return IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS) ||
		((unsigned long)base & (align - 1)) == 0;
}

static void u32_swap(void *a, void *b, int size)
{
	u32 t = *(u32 *)a;
	*(u32 *)a = *(u32 *)b;
	*(u32 *)b = t;
}

static void u64_swap(void *a, void *b, int size)
{
	u64 t = *(u64 *)a;
	*(u64 *)a = *(u64 *)b;
	*(u64 *)b = t;
}

static void generic_swap(void *a, void *b, int size)
{
	char t;

	do {
		t = *(char *)a;
		*(char *)a++ = *(char *)b;
		*(char *)b++ = t;
	} while (--size > 0);
}

/**
 * sort - sort an array of elements
 * @base: pointer to data to sort
 * @num: number of elements
 * @size: size of each element
 * @cmp_func: pointer to comparison function
 * @swap_func: pointer to swap function or NULL
 *
 * This function does a heapsort on the given array. You may provide a
 * swap_func function optimized to your element type.
 *
 * Sorting time is O(n log n) both on average and worst-case. While
 * qsort is about 20% faster on average, it suffers from exploitable
 * O(n*n) worst-case behavior and extra memory requirements that make
 * it less suitable for kernel use.
 */ // 참조 - https://www.youtube.com/watch?v=S42s_ANn4c4 https://gmlwjd9405.github.io/2018/05/10/algorithm-heap-sort.html

void sort(void *base, size_t num, size_t size,
	  int (*cmp_func)(const void *, const void *),
	  void (*swap_func)(void *, void *, int size))
{
	/* pre-scale counters for performance */
	int i = (num/2 - 1) * size, n = num * size, c, r;

	if (!swap_func) { // size와 base의 align에 따라 swap 함수 포인터 설정
		if (size == 4 && alignment_ok(base, 4))
			swap_func = u32_swap;
		else if (size == 8 && alignment_ok(base, 8))
			swap_func = u64_swap;
		else
			swap_func = generic_swap;
	}

	/* heapify */ // max heap
	for ( ; i >= 0; i -= size) { // 제일 마지막 노드의 부모에서 부터 시작
		for (r = i; r * 2 + size < n; r  = c) { // 유효한 노드면, 부모의 부모로 이동
			c = r * 2 + size; // 부모인덱스 : r, 자식인덱스 : c, c + 1
			if (c < n - size &&
					cmp_func(base + c, base + c + size) < 0) // 부모(r)의 두 자식간의 비교
				c += size;
			if (cmp_func(base + r, base + c) >= 0) // 부모가 더 큰경우
				break;
			swap_func(base + r, base + c, size); // 부모와 자식을 swap
		}
	}

	/* sort */ // 모든 노드에 대해 오름 차순 정렬 (최상위 노드가 최소값)
	for (i = n - size; i > 0; i -= size) { // 가장 마지막 노드에서 시작 
		swap_func(base, base + i, size); // 최상위 노드와 swap하여 큰값을 하위로 보냄
		for (r = 0; r * 2 + size < i; r = c) // 위의 heapify와 동일코드
			c = r * 2 + size;
			if (c < i - size &&
					cmp_func(base + c, base + c + size) < 0)
				c += size;
			if (cmp_func(base + r, base + c) >= 0)
				break;
			swap_func(base + r, base + c, size);
		}
	}
}

EXPORT_SYMBOL(sort);
