.section .text

// See ARM section A2.2 (Processor Modes)
.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVC,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_UNDEFINED,    0x1B
.equ    CPSR_MODE_SYSTEM,       0x1F

// See ARM section A2.5 (Program status registers)
.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20

.equ	SCTLR_ENABLE_DATA_CACHE,        0x4
.equ	SCTLR_ENABLE_BRANCH_PREDICTION, 0x800
.equ	SCTLR_ENABLE_INSTRUCTION_CACHE, 0x1000

.global _vectors
_vectors:
    b _init_core
    b interrupt_handler_udef
    b interrupt_handler_svc
    b interrupt_handler_pabt
    b interrupt_handler_dabt
    nop
    b interrupt_handler_irq
    b interrupt_handler_fiq

hang:
    b hang

/**
 * On startup the armstub runs which is located from 0x0 - 0x100
 * TODO (Optional): Rewite armstub to follow https://www.kernel.org/doc/Documentation/arm64/booting.txt semantics
 *
 * armstubs: https://github.com/raspberrypi/tools/tree/master/armstubs
 * - HYP
 * - Core 0 jumps to 0x8000, 0x80000 on aarch64
 * - Cores 1-3 are stuck in a WFE loop
 * - On SVR they jump to the address in mailbox 3

 * Therefore common startup code should:
 * - HYP -> SVR (no need for virtualization extensions)
 * - Setup execution level stacks (this varies on aarch64)
 * - Jump to execution code

 * Initialization routine is as follows:
 * - HYP -> SVC (all cores)
 * - Jump to core vector to perform core specific initialization
 * - Jump to core execution
 */

.global _init_core
_init_core:
    // HYP -> SVC
    mrs r0, cpsr
    bic r0, r0, #CPSR_MODE_SYSTEM
    orr r0, r0, #CPSR_MODE_SVC
    msr spsr_cxsf,   r0
    add r0, pc, #4
    msr ELR_hyp,  r0
    eret

/*
    // R0 = System Control Register
    mrc p15,0,r0,c1,c0,0
	
    // Enable caches and branch prediction
    orr r0,#SCTLR_ENABLE_BRANCH_PREDICTION
    orr r0,#SCTLR_ENABLE_DATA_CACHE
    orr r0,#SCTLR_ENABLE_INSTRUCTION_CACHE

    // System Control Register = R0
    mcr p15,0,r0,c1,c0,0
  */
    /**
     * Jump to addr = _core_vectors + (core_id * 4)
     */
    mrc     p15, 0, r0, c0, c0, 5
    ubfx    r0, r0, #0, #2 

    adr r1, _core_vectors
    mov r2, #4
    mla r1, r0, r2, r1
    ldr pc, [r1]
    b hang

_core_vectors:
    .word _init_core_0
    .word _init_core_1
    .word _init_core_2
    .word _init_core_3

_init_core_0:
    /**
    * Instead of copying the vector table to 0x0, update the vector base register
    */
    ldr     r4, =_vectors
    mcr     p15, #0, r4, c12, c0, #0

    mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    ldr sp, =__irq_stack_end_core_0

    mov r0, #(CPSR_MODE_FIQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    ldr sp, =__fiq_stack_end_core_0

    mov r0, #(CPSR_MODE_ABORT | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    ldr sp, =__abt_stack_end_core_0

    mov r0, #(CPSR_MODE_UNDEFINED | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    ldr sp, =__udef_stack_end_core_0

    mov r0, #(CPSR_MODE_USER | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    ldr sp, =__svr_stack_end_core_0

    b hang
    /**
    * Finally branch to higher level c routines.
    */
    bl cstartup
    b hang

_init_core_1:
  mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =__irq_stack_end_core_1

  mov r0, #(CPSR_MODE_SVC | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =__svr_stack_end_core_1

  bl cinit_core
  b hang

_init_core_2:
  mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =__irq_stack_end_core_2

  mov r0, #(CPSR_MODE_SVC | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =__svr_stack_end_core_2

  bl cinit_core
  b hang

_init_core_3:
  mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =__irq_stack_end_core_3

  mov r0, #(CPSR_MODE_SVC | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =__svr_stack_end_core_3

  bl cinit_core
  b hang

/**
 * __enable_interrupts()
 */
.global __enable_interrupts
__enable_interrupts:
  cpsie aif
  bx lr

/**
 * __disable_interrupts()
 */
.global __disable_interrupts
__disable_interrupts:
  cpsid aif
  bx lr
