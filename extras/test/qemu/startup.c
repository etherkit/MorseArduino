/*
 * Minimal Cortex-M0 startup for running the Morse tests under QEMU
 * (machine "microbit"). Output and the exit status go to the host via
 * ARM semihosting.
 *
 * QEMU loads every ELF segment, including .data, straight to its run
 * address, so only .bss needs to be cleared here.
 */

#include <stdio.h>
#include <stdlib.h>

extern unsigned long __stack_top;
extern unsigned long __bss_start__;
extern unsigned long __bss_end__;

extern int main(void);
extern void initialise_monitor_handles(void);
extern void __libc_init_array(void);

// Normally provided by crti/crtn and crtbegin, which -nostartfiles omits
void *__dso_handle = 0;
void _init(void) {}
void _fini(void) {}

// Report the exit status to QEMU with semihosting SYS_EXIT_EXTENDED, so that
// `make qemu` fails when a test fails
void _exit(int status)
{
  static unsigned long block[2];
  block[0] = 0x20026;               // ADP_Stopped_ApplicationExit
  block[1] = (unsigned long)status;

  register unsigned long r0 __asm__("r0") = 0x20;  // SYS_EXIT_EXTENDED
  register unsigned long *r1 __asm__("r1") = block;
  __asm__ __volatile__("bkpt 0xab" : : "r"(r0), "r"(r1) : "memory");

  for (;;) {}
}

void Reset_Handler(void)
{
  for (unsigned long *p = &__bss_start__; p < &__bss_end__; )
  {
    *p++ = 0;
  }

  initialise_monitor_handles();
  __libc_init_array();
  exit(main());
}

void Fault_Handler(void)
{
  puts("FAIL: CPU fault");
  exit(2);
}

__attribute__((section(".vectors"), used))
void (* const vectors[16])(void) =
{
  (void (*)(void))&__stack_top,
  Reset_Handler,
  Fault_Handler,  // NMI
  Fault_Handler,  // HardFault
  Fault_Handler, Fault_Handler, Fault_Handler, Fault_Handler,
  Fault_Handler, Fault_Handler, Fault_Handler, Fault_Handler,
  Fault_Handler, Fault_Handler, Fault_Handler, Fault_Handler,
};
