#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>

#include <sys/types.h>

typedef struct Frame {
    uint64_t spsr;    
    uint64_t elr;
    uint64_t reg[32];

    Frame(void *(*start_routine)(void *), void *arg) :
        reg({ 0 }) 
    { 
        this->spsr = 0b00100;
        this->elr = (uint64_t) start_routine;
        this->reg[0] = (uint64_t) arg;
    }
} aarch64_frame_t;

typedef struct Task {
    uint64_t    id;
    uint64_t    sys_ret;
    uint64_t    sys_arg;

    uint64_t    stack_size;
    void        *stack_base;
    aarch64_frame_t *frame; // SP_EL0 - sizeof(aarch64_frame_t)

    void        **status;

    void switch_from();
    void switch_to();
} task_t;

#endif