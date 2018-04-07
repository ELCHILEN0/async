#include "include/context.hpp"

/*
 * The switch_from function shall save a process stack.
 */
void Task::switch_from() {
    uint64_t stack_pointer;
    asm volatile(".global _switch_from    \n\
    _switch_from:                         \n\
        MSR SPSel, #0       \n\
        MOV %0, SP          \n\
        MSR SPSel, #1       \n\
    " : "=r" (stack_pointer));
    
    this->frame = (aarch64_frame_t *) stack_pointer;
}

/*
 * The switch_to function shall load a process stack to be restored on an ERET.
 */
void Task::switch_to() {
    // Place previous interrupt return code in x0
    this->frame->reg[0] = this->sys_ret;

    asm volatile(".global _switch_to    \n\
    _switch_to:                         \n\
        MSR SPSel, #0       \n\
        MOV SP, %0          \n\
        MSR SPSel, #1       \n\
    " :: "r" (this->frame));
}

