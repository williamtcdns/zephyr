/*
 * Copyright (c) 2024 Cadence Design Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>
#include <zephyr/toolchain.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/sys_io.h>

#include <kernel_internal.h>

#include <xtensa/config/core-isa.h>

#include <zephyr/zsr.h>
//#include <soc.h>
#include <zephyr/cache.h>

volatile struct cpustart_rec {
	uint32_t        cpu;
	arch_cpustart_t	fn;
	void            *arg;
} z_mp_start = {0, 0, 0};

/* Simple array of CPUs that are active and available for an IPI. */
bool soc_cpus_active[CONFIG_MP_MAX_NUM_CPUS];

__asm__(".section .text.z_mp_asm_entry, \"x\" \n\t"
	".align 4                   \n\t"
	".global z_mp_asm_entry     \n\t"
	"z_mp_asm_entry:            \n\t"
	"  movi  a0, 0x4002f        \n\t" /* WOE | UM | INTLEVEL(max) */
	"  wsr   a0, PS             \n\t"
	"  movi  a0, 0              \n\t"
	"  wsr   a0, WINDOWBASE     \n\t"
	"  movi  a0, 1              \n\t"
	"  wsr   a0, WINDOWSTART    \n\t"
	"  rsync                    \n\t"
	"  rsr   a2, PRID           \n\t"
	// Compute and set stack pointer register.
	"  movi  a1, z_interrupt_stacks \n\t"
	"  movi  a3, CONFIG_ISR_STACK_SIZE \n\t"
	"  mull  a4, a3, a2 \n\t"
	"  add   a4, a4, a3 \n\t"
	"  add   a1, a1, a4 \n\t"
	// Call z_mp_entry().
	"  call4 z_mp_entry         \n\t");

static void __used z_mp_entry(void)
{
	extern char _memmap_mem_sram_start[];
	xthal_mpu_set_region_attribute((void *)_memmap_mem_sram_start,
	                               0x4000000,
	                               XTHAL_AR_RWXrwx,
	                               XTHAL_MEM_NON_CACHEABLE | XTHAL_MEM_SYSTEM_SHAREABLE,
	                               0);

	int prid = arch_proc_id();
	while (z_mp_start.cpu != prid) {
		// TODO: Wait for interrupt.
	}

	// Set up the CPU pointer.
	_cpu_t *cpu = &_kernel.cpus[z_mp_start.cpu];
	__asm__ volatile("wsr %0, " ZSR_CPU_STR :: "r"(cpu));

	soc_cpus_active[z_mp_start.cpu] = true;
	z_mp_start.fn(z_mp_start.arg);
	__ASSERT(false, "arch_cpu_start() handler should never return");
}

bool arch_cpu_active(int cpu_num)
{
	return soc_cpus_active[cpu_num];
}

void arch_cpu_start(int cpu_num, k_thread_stack_t *stack, int sz,
		    arch_cpustart_t fn, void *arg)
{
	__ASSERT_NO_MSG(!soc_cpus_active[cpu_num]);

	// It must be the same stack that will be computed in z_mp_asm_entry().
	__ASSERT_NO_MSG(stack == z_interrupt_stacks[cpu_num]);

	z_mp_start.fn = fn;
	z_mp_start.arg = arg;

	/* must be done last. */
	z_mp_start.cpu = cpu_num;
	__asm__ volatile("rsync" ::: "memory");
}
