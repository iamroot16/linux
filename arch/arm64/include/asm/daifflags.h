/*
 * Copyright (C) 2017 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_DAIFFLAGS_H
#define __ASM_DAIFFLAGS_H

#include <linux/irqflags.h>

#include <asm/cpufeature.h>

#define DAIF_PROCCTX		0
#define DAIF_PROCCTX_NOIRQ	PSR_I_BIT
#define DAIF_ERRCTX		(PSR_I_BIT | PSR_A_BIT)

/* mask/save/unmask/restore all exceptions, including interrupts. */
static inline void local_daif_mask(void)
{
	asm volatile(
		"msr	daifset, #0xf		// local_daif_mask\n"
		:
		:
		: "memory");
	trace_hardirqs_off();
}

static inline unsigned long local_daif_save(void)
{
	unsigned long flags;

	flags = read_sysreg(daif);

	if (system_uses_irq_prio_masking()) {
		/* If IRQs are masked with PMR, reflect it in the flags */
		if (read_sysreg_s(SYS_ICC_PMR_EL1) <= GIC_PRIO_IRQOFF)
			flags |= PSR_I_BIT;
	}

	local_daif_mask();

	return flags;
}

/*
 * [ARM Generic Interrupt Controller Architecture Specification Document 66page]
 *
 * For Non-secure access to ICC_PMR_EL1 and ICC_RPR_EL1 when SCR_EL3.FIQ == 1:
 * • If the current priority mask value is in the range of 0x00-0x7F:
 *    — A read access returns the value 0x00.
 *    — The GIC ignores a write access to ICC_PMR_EL1
 */
static inline void local_daif_restore(unsigned long flags)
{
	bool irq_disabled = flags & PSR_I_BIT;

	if (!irq_disabled) {
		trace_hardirqs_on();

		if (system_uses_irq_prio_masking())
			arch_local_irq_enable();
	} else if (!(flags & PSR_A_BIT)) {
		/* I flag masked, A bit unmasked */
		/*
		 * If interrupts are disabled but we can take
		 * asynchronous errors, we can take NMIs
		 */
		if (system_uses_irq_prio_masking()) {
			/* I flag는 0이 된다.(unmasked)*/
			flags &= ~PSR_I_BIT;
			/*
			 * There has been concern that the write to daif
			 * might be reordered before this write to PMR.
			 * From the ARM ARM DDI 0487D.a, section D1.7.1
			 * "Accessing PSTATE fields":
			 *   Writes to the PSTATE fields have side-effects on
			 *   various aspects of the PE operation. All of these
			 *   side-effects are guaranteed:
			 *     - Not to be visible to earlier instructions in
			 *       the execution stream.
			 *     - To be visible to later instructions in the
			 *       execution stream
			 *
			 * Also, writes to PMR are self-synchronizing, so no
			 * interrupts with a lower priority than PMR is signaled
			 * to the PE after the write.
			 *
			 * So we don't need additional synchronization here.
			 */
			arch_local_irq_disable();
		}
	}
	/* I flag A flag
	 *    1     0     ->  0       0 -> using pmr 
	 *    1     1     ->  1       1 -> using daif
	 */
	write_sysreg(flags, daif);

	if (irq_disabled)
		trace_hardirqs_off();
}

#endif
