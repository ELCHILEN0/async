#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char *__text_start;
extern char *__text_end;
extern char *__data_start;
extern char *__data_end;
extern char *__bss_start;
extern char *__bss_end;
extern char *__heap_start;
extern char *__heap_end;
extern char *__stack_start;
extern char *__stack_end;
extern char *__irq_stack_start;
extern char *__irq_stack_end;

extern void cinit_core();
extern void __libc_init_array();

// stub for __libc_init_array
void _init() { };

void init_bss() {
    char *bss = (char*) &__bss_start;

    while ( (unsigned long) bss < (unsigned long) &__bss_end )
        *bss++ = 0;
}

void cstartup( uint32_t r0, uint32_t r1, uint32_t atags ) {
    init_bss();
    __libc_init_array();

    cinit_core();
    
    while (1) { }
}

