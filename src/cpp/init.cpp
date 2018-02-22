#include "../cpp/msync.hpp"

#ifdef __cplusplus
    extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "../c/peripheral.h"
#include "../c/gpio.h"
#include "../c/timer.h"
#include "../c/uart.h"
#include "../c/interrupts.h"

#include "../c/mailbox.h"
#include "../c/multicore.h"

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void __enable_interrupts(void);
extern void __disable_interrupts(void);
extern void _init_core(void);
extern void enable_mmu(void);

spinlock_t newlib_lock;

void master_core () {
    __spin_lock(&newlib_lock);
    printf("[core%d] Executing from 0x%lX\r\n", get_core_id(), (uint64_t) master_core);
    __spin_unlock(&newlib_lock);   

    Producer::instance().dispatch();

    while (true) {
        for (int i = 0; i < 0x100000 * 30; i++);
        gpio_write(5, true);

        for (int i = 0; i < 0x100000 * 30; i++);
        gpio_write(5, false);
    }
}

void slave_core() {
    int core_id = get_core_id();
    int core_gpio[3] = { 6, 13, 19 };

    __spin_lock(&newlib_lock);
    printf("[core%d] Executing from 0x%lX!\r\n", core_id, (uint64_t) slave_core);
    __spin_unlock(&newlib_lock);

    ClientLock *lock = new ClientLock(0x12345678);

    while (true) {
        for (int i = 0; i < 0x10000 * (core_id + 1) * 30; i++);
        lock->acquire();
        gpio_write(core_gpio[core_id - 1], true);
        lock->release();

        for (int i = 0; i < 0x10000 * (core_id + 1) * 30; i++);
        lock->acquire();
        gpio_write(core_gpio[core_id - 1], false);  
        lock->release();
    }
}

void cinit_core(void) {    
    // OK status (use till GPIO working)
    act_message[6] = 1;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (intptr_t) &act_message);

    int core_id = get_core_id();

    core_timer_interrupt_routing(core_id, CT_IRQ_NON_SECURE);  

    switch(core_id) {
        case 0:
        {
            // timer_frequency = (2e31/prescaler) * input_frequency
            // APB vs CRY ...   1/2 CPU Frequency vs Fixed Time Frequency
            //                  Fixed Execution Instructions vs Fixed Execution Time
            core_timer_init( CT_CTRL_SRC_APB, CT_CTRL_INC2, 0x80000000);
            // core_timer_init( CT_CTRL_SRC_CRY, CT_CTRL_INC1, 0x80000000);

            core_enable(1, (uint64_t) _init_core);
            core_enable(2, (uint64_t) _init_core);
            core_enable(3, (uint64_t) _init_core); 

            // init_vector_tables();
            // register_interrupt_handler(vector_table_irq, 0x80, test_handler);

            gpio_fsel(5, SEL_OUTPUT);
            gpio_fsel(6, SEL_OUTPUT);
            gpio_fsel(13, SEL_OUTPUT);
            gpio_fsel(19, SEL_OUTPUT);
            gpio_fsel(21, SEL_OUTPUT);

            gpio_write(5, true);
            gpio_write(6, true);
            gpio_write(13, true);
            gpio_write(19, true);
            // gpio_write(21, true);

            // Important notes... in general J-TAG and UART will need to be initialized especially when
            // starting from 0x0.  However, with config.txt adding enable_uart, enable_jtag allow bypassing
            // this device specific initialization
            // enable_jtag();            
            // uart_init(115200);

            enable_mmu();  
            master_core();
        }
        break;

        default:
            enable_mmu();          
            slave_core();
            break;
    }
    
    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (intptr_t) &act_message);
}

#ifdef __cplusplus
    }
#endif