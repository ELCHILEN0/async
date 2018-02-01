#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "peripheral.h"
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "interrupts.h"

#include "mailbox.h"
#include "multicore.h"

// extern "C"
// {
// #include "../cpp/msync.cpp"
// }

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void __enable_interrupts(void);
extern void __disable_interrupts(void);
extern void _init_core(void);

static spinlock_t slave_lock;

void slave_core() {
    int core_id = get_core_id();
    int core_gpio[3] = { 6, 13, 19 };

    while (true) {
        for (int i = 0; i < 0x10000; i++);
        gpio_write(core_gpio[core_id - 1], true);

        for (int i = 0; i < 0x10000; i++);
        gpio_write(core_gpio[core_id - 1], false);   
    }
    while (true);
}

// void context_switch() {
//     static bool next_blinker_state = true;
//     gpio_write(21, next_blinker_state);
//     next_blinker_state = !next_blinker_state;
// }

// void time_slice() {
//     local_timer_reset();

//     static bool next_blinker_state = true;
//     gpio_write(13, next_blinker_state);
//     next_blinker_state = !next_blinker_state;
// }

#define GPPUD					(PERIPHERAL_BASE + 0x00200094)
#define GPPUDCLK0				(PERIPHERAL_BASE + 0x00200098)
#define GPPUDCLK1				(PERIPHERAL_BASE + 0x0020009C)

void init_jtag() {
    mmio_write(GPPUD, 0);
	for(int i = 0; i < 150; i++) asm("nop");
	mmio_write(GPPUDCLK0, (1 << 22) | (1 << 24) | (1 << 25) | (1 << 26) | (1 << 27));
	for(int i = 0; i < 150; i++) asm("nop");
	mmio_write(GPPUDCLK0, 0);

    // JTAG
    gpio_fsel(22, SEL_ALT4); // TRST
    gpio_fsel(24, SEL_ALT4); // TDO
    gpio_fsel(25, SEL_ALT4); // TCK
    gpio_fsel(26, SEL_ALT4); // TDI
    gpio_fsel(27, SEL_ALT4); // TMS
    // gpio_fsel(4, SEL_ALT5);
}

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
	(void) r0;
	(void) r1;
	(void) atags;

    // OK status (use till GPIO working)
    act_message[6] = 1;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);

    gpio_fsel(5, SEL_OUTPUT);
    gpio_fsel(6, SEL_OUTPUT);
    gpio_fsel(13, SEL_OUTPUT);
    gpio_fsel(19, SEL_OUTPUT);
    gpio_fsel(21, SEL_INPUT);

    uart_init(115200);
    init_jtag();

    printf("[kernel] Kernel started at 0x%X on core %d.\r\n", kernel_main, get_core_id());
    // __enable_interrupts();

    slave_lock.flag = 0;
    
    core_enable(1, (uint32_t) _init_core);
    core_enable(2, (uint32_t) _init_core);
    core_enable(3, (uint32_t) _init_core);

    Producer p;

    // register_interrupt_handler(vector_table_svc, 0x80, &context_switch);
    // register_interrupt_handler(vector_table_svc, 0x81, context_switch);
    // register_interrupt_handler(vector_table_irq, 11, time_slice);
    
    while (true) {       
        for (int i = 0; i < 0x10000 * 3; i++);
        gpio_write(5, true);

        for (int i = 0; i < 0x10000 * 3; i++);
        gpio_write(5, false);
    }

    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}