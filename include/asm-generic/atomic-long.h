// SPDX-License-Identifier: GPL-2.0

// Generated by scripts/atomic/gen-atomic-long.sh
// DO NOT MODIFY THIS FILE DIRECTLY

#ifndef _ASM_GENERIC_ATOMIC_LONG_H
#define _ASM_GENERIC_ATOMIC_LONG_H

#include <asm/types.h>

#ifdef CONFIG_64BIT
typedef atomic64_t atomic_long_t;
#define ATOMIC_LONG_INIT(i)		ATOMIC64_INIT(i)
#define atomic_long_cond_read_acquire	atomic64_cond_read_acquire
#define atomic_long_cond_read_relaxed	atomic64_cond_read_relaxed
#else
typedef atomic_t atomic_long_t;
#define ATOMIC_LONG_INIT(i)		ATOMIC_INIT(i)
#define atomic_long_cond_read_acquire	atomic_cond_read_acquire
#define atomic_long_cond_read_relaxed	atomic_cond_read_relaxed
#endif

#ifdef CONFIG_64BIT

static inline long
atomic_long_read(const atomic_long_t *v)
{
	return atomic64_read(v);
}

static inline long
atomic_long_read_acquire(const atomic_long_t *v)
{
	return atomic64_read_acquire(v);
}

static inline void
atomic_long_set(atomic_long_t *v, long i)
{
	atomic64_set(v, i);
}

static inline void
atomic_long_set_release(atomic_long_t *v, long i)
{
	atomic64_set_release(v, i);
}

static inline void
atomic_long_add(long i, atomic_long_t *v)
{
	atomic64_add(i, v);
}

static inline long
atomic_long_add_return(long i, atomic_long_t *v)
{
	return atomic64_add_return(i, v);
}

static inline long
atomic_long_add_return_acquire(long i, atomic_long_t *v)
{
	return atomic64_add_return_acquire(i, v);
}

static inline long
atomic_long_add_return_release(long i, atomic_long_t *v)
{
	return atomic64_add_return_release(i, v);
}

static inline long
atomic_long_add_return_relaxed(long i, atomic_long_t *v)
{
	return atomic64_add_return_relaxed(i, v);
}

static inline long
atomic_long_fetch_add(long i, atomic_long_t *v)
{
	return atomic64_fetch_add(i, v);
}

static inline long
atomic_long_fetch_add_acquire(long i, atomic_long_t *v)
{
	return atomic64_fetch_add_acquire(i, v);
}

static inline long
atomic_long_fetch_add_release(long i, atomic_long_t *v)
{
	return atomic64_fetch_add_release(i, v);
}

static inline long
atomic_long_fetch_add_relaxed(long i, atomic_long_t *v)
{
	return atomic64_fetch_add_relaxed(i, v);
}

static inline void
atomic_long_sub(long i, atomic_long_t *v)
{
	atomic64_sub(i, v);
}

static inline long
atomic_long_sub_return(long i, atomic_long_t *v)
{
	return atomic64_sub_return(i, v);
}

static inline long
atomic_long_sub_return_acquire(long i, atomic_long_t *v)
{
	return atomic64_sub_return_acquire(i, v);
}

static inline long
atomic_long_sub_return_release(long i, atomic_long_t *v)
{
	return atomic64_sub_return_release(i, v);
}

static inline long
atomic_long_sub_return_relaxed(long i, atomic_long_t *v)
{
	return atomic64_sub_return_relaxed(i, v);
}

static inline long
atomic_long_fetch_sub(long i, atomic_long_t *v)
{
	return atomic64_fetch_sub(i, v);
}

static inline long
atomic_long_fetch_sub_acquire(long i, atomic_long_t *v)
{
	return atomic64_fetch_sub_acquire(i, v);
}

static inline long
atomic_long_fetch_sub_release(long i, atomic_long_t *v)
{
	return atomic64_fetch_sub_release(i, v);
}

static inline long
atomic_long_fetch_sub_relaxed(long i, atomic_long_t *v)
{
	return atomic64_fetch_sub_relaxed(i, v);
}

static inline void
atomic_long_inc(atomic_long_t *v)
{
	atomic64_inc(v);
}

static inline long
atomic_long_inc_return(atomic_long_t *v)
{
	return atomic64_inc_return(v);
}

static inline long
atomic_long_inc_return_acquire(atomic_long_t *v)
{
	return atomic64_inc_return_acquire(v);
}

static inline long
atomic_long_inc_return_release(atomic_long_t *v)
{
	return atomic64_inc_return_release(v);
}

static inline long
atomic_long_inc_return_relaxed(atomic_long_t *v)
{
	return atomic64_inc_return_relaxed(v);
}

static inline long
atomic_long_fetch_inc(atomic_long_t *v)
{
	return atomic64_fetch_inc(v);
}

static inline long
atomic_long_fetch_inc_acquire(atomic_long_t *v)
{
	return atomic64_fetch_inc_acquire(v);
}

static inline long
atomic_long_fetch_inc_release(atomic_long_t *v)
{
	return atomic64_fetch_inc_release(v);
}

static inline long
atomic_long_fetch_inc_relaxed(atomic_long_t *v)
{
	return atomic64_fetch_inc_relaxed(v);
}

static inline void
atomic_long_dec(atomic_long_t *v)
{
	atomic64_dec(v);
}

static inline long
atomic_long_dec_return(atomic_long_t *v)
{
	return atomic64_dec_return(v);
}

static inline long
atomic_long_dec_return_acquire(atomic_long_t *v)
{
	return atomic64_dec_return_acquire(v);
}

static inline long
atomic_long_dec_return_release(atomic_long_t *v)
{
	return atomic64_dec_return_release(v);
}

static inline long
atomic_long_dec_return_relaxed(atomic_long_t *v)
{
	return atomic64_dec_return_relaxed(v);
}

static inline long
atomic_long_fetch_dec(atomic_long_t *v)
{
	return atomic64_fetch_dec(v);
}

static inline long
atomic_long_fetch_dec_acquire(atomic_long_t *v)
{
	return atomic64_fetch_dec_acquire(v);
}

static inline long
atomic_long_fetch_dec_release(atomic_long_t *v)
{
	return atomic64_fetch_dec_release(v);
}

static inline long
atomic_long_fetch_dec_relaxed(atomic_long_t *v)
{
	return atomic64_fetch_dec_relaxed(v);
}

static inline void
atomic_long_and(long i, atomic_long_t *v)
{
	atomic64_and(i, v);
}

static inline long
atomic_long_fetch_and(long i, atomic_long_t *v)
{
	return atomic64_fetch_and(i, v);
}

static inline long
atomic_long_fetch_and_acquire(long i, atomic_long_t *v)
{
	return atomic64_fetch_and_acquire(i, v);
}

static inline long
atomic_long_fetch_and_release(long i, atomic_long_t *v)
{
	return atomic64_fetch_and_release(i, v);
}

static inline long
atomic_long_fetch_and_relaxed(long i, atomic_long_t *v)
{
	return atomic64_fetch_and_relaxed(i, v);
}

static inline void
atomic_long_andnot(long i, atomic_long_t *v)
{
	atomic64_andnot(i, v);
}

static inline long
atomic_long_fetch_andnot(long i, atomic_long_t *v)
{
	return atomic64_fetch_andnot(i, v);
}

static inline long
atomic_long_fetch_andnot_acquire(long i, atomic_long_t *v)
{
	return atomic64_fetch_andnot_acquire(i, v);
}

static inline long
atomic_long_fetch_andnot_release(long i, atomic_long_t *v)
{
	return atomic64_fetch_andnot_release(i, v);
}

static inline long
atomic_long_fetch_andnot_relaxed(long i, atomic_long_t *v)
{
	return atomic64_fetch_andnot_relaxed(i, v);
}
/*
 * args (이전 함수에서 보내는 arguments)
 * i = 1 (i의 뜻은 인덱스를 의미하는것으로 추정)
 * v = cpumask 의 주소 &(unsigned long bits[0])
 */
static inline void
atomic_long_or(long i, atomic_long_t *v)
{
	atomic64_or(i, v);
}

static inline long
atomic_long_fetch_or(long i, atomic_long_t *v)
{
	return atomic64_fetch_or(i, v);
}

static inline long
atomic_long_fetch_or_acquire(long i, atomic_long_t *v)
{
	return atomic64_fetch_or_acquire(i, v);
}

static inline long
atomic_long_fetch_or_release(long i, atomic_long_t *v)
{
	return atomic64_fetch_or_release(i, v);
}

static inline long
atomic_long_fetch_or_relaxed(long i, atomic_long_t *v)
{
	return atomic64_fetch_or_relaxed(i, v);
}

static inline void
atomic_long_xor(long i, atomic_long_t *v)
{
	atomic64_xor(i, v);
}

static inline long
atomic_long_fetch_xor(long i, atomic_long_t *v)
{
	return atomic64_fetch_xor(i, v);
}

static inline long
atomic_long_fetch_xor_acquire(long i, atomic_long_t *v)
{
	return atomic64_fetch_xor_acquire(i, v);
}

static inline long
atomic_long_fetch_xor_release(long i, atomic_long_t *v)
{
	return atomic64_fetch_xor_release(i, v);
}

static inline long
atomic_long_fetch_xor_relaxed(long i, atomic_long_t *v)
{
	return atomic64_fetch_xor_relaxed(i, v);
}

static inline long
atomic_long_xchg(atomic_long_t *v, long i)
{
	return atomic64_xchg(v, i);
}

static inline long
atomic_long_xchg_acquire(atomic_long_t *v, long i)
{
	return atomic64_xchg_acquire(v, i);
}

static inline long
atomic_long_xchg_release(atomic_long_t *v, long i)
{
	return atomic64_xchg_release(v, i);
}

static inline long
atomic_long_xchg_relaxed(atomic_long_t *v, long i)
{
	return atomic64_xchg_relaxed(v, i);
}

static inline long
atomic_long_cmpxchg(atomic_long_t *v, long old, long new)
{
	return atomic64_cmpxchg(v, old, new);
}

static inline long
atomic_long_cmpxchg_acquire(atomic_long_t *v, long old, long new)
{
	return atomic64_cmpxchg_acquire(v, old, new);
}

static inline long
atomic_long_cmpxchg_release(atomic_long_t *v, long old, long new)
{
	return atomic64_cmpxchg_release(v, old, new);
}

static inline long
atomic_long_cmpxchg_relaxed(atomic_long_t *v, long old, long new)
{
	return atomic64_cmpxchg_relaxed(v, old, new);
}

static inline bool
atomic_long_try_cmpxchg(atomic_long_t *v, long *old, long new)
{
	return atomic64_try_cmpxchg(v, (s64 *)old, new);
}

static inline bool
atomic_long_try_cmpxchg_acquire(atomic_long_t *v, long *old, long new)
{
	return atomic64_try_cmpxchg_acquire(v, (s64 *)old, new);
}

static inline bool
atomic_long_try_cmpxchg_release(atomic_long_t *v, long *old, long new)
{
	return atomic64_try_cmpxchg_release(v, (s64 *)old, new);
}

static inline bool
atomic_long_try_cmpxchg_relaxed(atomic_long_t *v, long *old, long new)
{
	return atomic64_try_cmpxchg_relaxed(v, (s64 *)old, new);
}

static inline bool
atomic_long_sub_and_test(long i, atomic_long_t *v)
{
	return atomic64_sub_and_test(i, v);
}

static inline bool
atomic_long_dec_and_test(atomic_long_t *v)
{
	return atomic64_dec_and_test(v);
}

static inline bool
atomic_long_inc_and_test(atomic_long_t *v)
{
	return atomic64_inc_and_test(v);
}

static inline bool
atomic_long_add_negative(long i, atomic_long_t *v)
{
	return atomic64_add_negative(i, v);
}

static inline long
atomic_long_fetch_add_unless(atomic_long_t *v, long a, long u)
{
	return atomic64_fetch_add_unless(v, a, u);
}

static inline bool
atomic_long_add_unless(atomic_long_t *v, long a, long u)
{
	return atomic64_add_unless(v, a, u);
}

static inline bool
atomic_long_inc_not_zero(atomic_long_t *v)
{
	return atomic64_inc_not_zero(v);
}

static inline bool
atomic_long_inc_unless_negative(atomic_long_t *v)
{
	return atomic64_inc_unless_negative(v);
}

static inline bool
atomic_long_dec_unless_positive(atomic_long_t *v)
{
	return atomic64_dec_unless_positive(v);
}

static inline long
atomic_long_dec_if_positive(atomic_long_t *v)
{
	return atomic64_dec_if_positive(v);
}

#else /* CONFIG_64BIT */

static inline long
atomic_long_read(const atomic_long_t *v)
{
	return atomic_read(v);
}

static inline long
atomic_long_read_acquire(const atomic_long_t *v)
{
	return atomic_read_acquire(v);
}

static inline void
atomic_long_set(atomic_long_t *v, long i)
{
	atomic_set(v, i);
}

static inline void
atomic_long_set_release(atomic_long_t *v, long i)
{
	atomic_set_release(v, i);
}

static inline void
atomic_long_add(long i, atomic_long_t *v)
{
	atomic_add(i, v);
}

static inline long
atomic_long_add_return(long i, atomic_long_t *v)
{
	return atomic_add_return(i, v);
}

static inline long
atomic_long_add_return_acquire(long i, atomic_long_t *v)
{
	return atomic_add_return_acquire(i, v);
}

static inline long
atomic_long_add_return_release(long i, atomic_long_t *v)
{
	return atomic_add_return_release(i, v);
}

static inline long
atomic_long_add_return_relaxed(long i, atomic_long_t *v)
{
	return atomic_add_return_relaxed(i, v);
}

static inline long
atomic_long_fetch_add(long i, atomic_long_t *v)
{
	return atomic_fetch_add(i, v);
}

static inline long
atomic_long_fetch_add_acquire(long i, atomic_long_t *v)
{
	return atomic_fetch_add_acquire(i, v);
}

static inline long
atomic_long_fetch_add_release(long i, atomic_long_t *v)
{
	return atomic_fetch_add_release(i, v);
}

static inline long
atomic_long_fetch_add_relaxed(long i, atomic_long_t *v)
{
	return atomic_fetch_add_relaxed(i, v);
}

static inline void
atomic_long_sub(long i, atomic_long_t *v)
{
	atomic_sub(i, v);
}

static inline long
atomic_long_sub_return(long i, atomic_long_t *v)
{
	return atomic_sub_return(i, v);
}

static inline long
atomic_long_sub_return_acquire(long i, atomic_long_t *v)
{
	return atomic_sub_return_acquire(i, v);
}

static inline long
atomic_long_sub_return_release(long i, atomic_long_t *v)
{
	return atomic_sub_return_release(i, v);
}

static inline long
atomic_long_sub_return_relaxed(long i, atomic_long_t *v)
{
	return atomic_sub_return_relaxed(i, v);
}

static inline long
atomic_long_fetch_sub(long i, atomic_long_t *v)
{
	return atomic_fetch_sub(i, v);
}

static inline long
atomic_long_fetch_sub_acquire(long i, atomic_long_t *v)
{
	return atomic_fetch_sub_acquire(i, v);
}

static inline long
atomic_long_fetch_sub_release(long i, atomic_long_t *v)
{
	return atomic_fetch_sub_release(i, v);
}

static inline long
atomic_long_fetch_sub_relaxed(long i, atomic_long_t *v)
{
	return atomic_fetch_sub_relaxed(i, v);
}

static inline void
atomic_long_inc(atomic_long_t *v)
{
	atomic_inc(v);
}

static inline long
atomic_long_inc_return(atomic_long_t *v)
{
	return atomic_inc_return(v);
}

static inline long
atomic_long_inc_return_acquire(atomic_long_t *v)
{
	return atomic_inc_return_acquire(v);
}

static inline long
atomic_long_inc_return_release(atomic_long_t *v)
{
	return atomic_inc_return_release(v);
}

static inline long
atomic_long_inc_return_relaxed(atomic_long_t *v)
{
	return atomic_inc_return_relaxed(v);
}

static inline long
atomic_long_fetch_inc(atomic_long_t *v)
{
	return atomic_fetch_inc(v);
}

static inline long
atomic_long_fetch_inc_acquire(atomic_long_t *v)
{
	return atomic_fetch_inc_acquire(v);
}

static inline long
atomic_long_fetch_inc_release(atomic_long_t *v)
{
	return atomic_fetch_inc_release(v);
}

static inline long
atomic_long_fetch_inc_relaxed(atomic_long_t *v)
{
	return atomic_fetch_inc_relaxed(v);
}

static inline void
atomic_long_dec(atomic_long_t *v)
{
	atomic_dec(v);
}

static inline long
atomic_long_dec_return(atomic_long_t *v)
{
	return atomic_dec_return(v);
}

static inline long
atomic_long_dec_return_acquire(atomic_long_t *v)
{
	return atomic_dec_return_acquire(v);
}

static inline long
atomic_long_dec_return_release(atomic_long_t *v)
{
	return atomic_dec_return_release(v);
}

static inline long
atomic_long_dec_return_relaxed(atomic_long_t *v)
{
	return atomic_dec_return_relaxed(v);
}

static inline long
atomic_long_fetch_dec(atomic_long_t *v)
{
	return atomic_fetch_dec(v);
}

static inline long
atomic_long_fetch_dec_acquire(atomic_long_t *v)
{
	return atomic_fetch_dec_acquire(v);
}

static inline long
atomic_long_fetch_dec_release(atomic_long_t *v)
{
	return atomic_fetch_dec_release(v);
}

static inline long
atomic_long_fetch_dec_relaxed(atomic_long_t *v)
{
	return atomic_fetch_dec_relaxed(v);
}

static inline void
atomic_long_and(long i, atomic_long_t *v)
{
	atomic_and(i, v);
}

static inline long
atomic_long_fetch_and(long i, atomic_long_t *v)
{
	return atomic_fetch_and(i, v);
}

static inline long
atomic_long_fetch_and_acquire(long i, atomic_long_t *v)
{
	return atomic_fetch_and_acquire(i, v);
}

static inline long
atomic_long_fetch_and_release(long i, atomic_long_t *v)
{
	return atomic_fetch_and_release(i, v);
}

static inline long
atomic_long_fetch_and_relaxed(long i, atomic_long_t *v)
{
	return atomic_fetch_and_relaxed(i, v);
}

static inline void
atomic_long_andnot(long i, atomic_long_t *v)
{
	atomic_andnot(i, v);
}

static inline long
atomic_long_fetch_andnot(long i, atomic_long_t *v)
{
	return atomic_fetch_andnot(i, v);
}

static inline long
atomic_long_fetch_andnot_acquire(long i, atomic_long_t *v)
{
	return atomic_fetch_andnot_acquire(i, v);
}

static inline long
atomic_long_fetch_andnot_release(long i, atomic_long_t *v)
{
	return atomic_fetch_andnot_release(i, v);
}

static inline long
atomic_long_fetch_andnot_relaxed(long i, atomic_long_t *v)
{
	return atomic_fetch_andnot_relaxed(i, v);
}

static inline void
atomic_long_or(long i, atomic_long_t *v)
{
	atomic_or(i, v);
}

static inline long
atomic_long_fetch_or(long i, atomic_long_t *v)
{
	return atomic_fetch_or(i, v);
}

static inline long
atomic_long_fetch_or_acquire(long i, atomic_long_t *v)
{
	return atomic_fetch_or_acquire(i, v);
}

static inline long
atomic_long_fetch_or_release(long i, atomic_long_t *v)
{
	return atomic_fetch_or_release(i, v);
}

static inline long
atomic_long_fetch_or_relaxed(long i, atomic_long_t *v)
{
	return atomic_fetch_or_relaxed(i, v);
}

static inline void
atomic_long_xor(long i, atomic_long_t *v)
{
	atomic_xor(i, v);
}

static inline long
atomic_long_fetch_xor(long i, atomic_long_t *v)
{
	return atomic_fetch_xor(i, v);
}

static inline long
atomic_long_fetch_xor_acquire(long i, atomic_long_t *v)
{
	return atomic_fetch_xor_acquire(i, v);
}

static inline long
atomic_long_fetch_xor_release(long i, atomic_long_t *v)
{
	return atomic_fetch_xor_release(i, v);
}

static inline long
atomic_long_fetch_xor_relaxed(long i, atomic_long_t *v)
{
	return atomic_fetch_xor_relaxed(i, v);
}

static inline long
atomic_long_xchg(atomic_long_t *v, long i)
{
	return atomic_xchg(v, i);
}

static inline long
atomic_long_xchg_acquire(atomic_long_t *v, long i)
{
	return atomic_xchg_acquire(v, i);
}

static inline long
atomic_long_xchg_release(atomic_long_t *v, long i)
{
	return atomic_xchg_release(v, i);
}

static inline long
atomic_long_xchg_relaxed(atomic_long_t *v, long i)
{
	return atomic_xchg_relaxed(v, i);
}

static inline long
atomic_long_cmpxchg(atomic_long_t *v, long old, long new)
{
	return atomic_cmpxchg(v, old, new);
}

static inline long
atomic_long_cmpxchg_acquire(atomic_long_t *v, long old, long new)
{
	return atomic_cmpxchg_acquire(v, old, new);
}

static inline long
atomic_long_cmpxchg_release(atomic_long_t *v, long old, long new)
{
	return atomic_cmpxchg_release(v, old, new);
}

static inline long
atomic_long_cmpxchg_relaxed(atomic_long_t *v, long old, long new)
{
	return atomic_cmpxchg_relaxed(v, old, new);
}

static inline bool
atomic_long_try_cmpxchg(atomic_long_t *v, long *old, long new)
{
	return atomic_try_cmpxchg(v, (int *)old, new);
}

static inline bool
atomic_long_try_cmpxchg_acquire(atomic_long_t *v, long *old, long new)
{
	return atomic_try_cmpxchg_acquire(v, (int *)old, new);
}

static inline bool
atomic_long_try_cmpxchg_release(atomic_long_t *v, long *old, long new)
{
	return atomic_try_cmpxchg_release(v, (int *)old, new);
}

static inline bool
atomic_long_try_cmpxchg_relaxed(atomic_long_t *v, long *old, long new)
{
	return atomic_try_cmpxchg_relaxed(v, (int *)old, new);
}

static inline bool
atomic_long_sub_and_test(long i, atomic_long_t *v)
{
	return atomic_sub_and_test(i, v);
}

static inline bool
atomic_long_dec_and_test(atomic_long_t *v)
{
	return atomic_dec_and_test(v);
}

static inline bool
atomic_long_inc_and_test(atomic_long_t *v)
{
	return atomic_inc_and_test(v);
}

static inline bool
atomic_long_add_negative(long i, atomic_long_t *v)
{
	return atomic_add_negative(i, v);
}

static inline long
atomic_long_fetch_add_unless(atomic_long_t *v, long a, long u)
{
	return atomic_fetch_add_unless(v, a, u);
}

static inline bool
atomic_long_add_unless(atomic_long_t *v, long a, long u)
{
	return atomic_add_unless(v, a, u);
}

static inline bool
atomic_long_inc_not_zero(atomic_long_t *v)
{
	return atomic_inc_not_zero(v);
}

static inline bool
atomic_long_inc_unless_negative(atomic_long_t *v)
{
	return atomic_inc_unless_negative(v);
}

static inline bool
atomic_long_dec_unless_positive(atomic_long_t *v)
{
	return atomic_dec_unless_positive(v);
}

static inline long
atomic_long_dec_if_positive(atomic_long_t *v)
{
	return atomic_dec_if_positive(v);
}

#endif /* CONFIG_64BIT */
#endif /* _ASM_GENERIC_ATOMIC_LONG_H */
// 77558968132ce4f911ad53f6f52ce423006f6268
