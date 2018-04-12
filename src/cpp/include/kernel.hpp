#ifndef KERNEL_H
#define KERNEL_H

#include <atomic>
#include <memory>
#include <vector>
#include <array>

#include "context.hpp"

#include "msync.hpp"

#include <stdlib.h>
#include <experimental/any>

#include "../../c/include/multicore.h"
#include "../../c/include/gpio.h"

extern "C" spinlock_t newlib_lock;

class Kernel;

enum system_call {
    SYS_ACQUIRE,
    SYS_RELEASE,
    SYS_EXIT,
};

void kernel_interrupt_handler();

// int syscall(enum system_call req_id, std::vector<std::experimental::fundamentals_v1::any> *args);
int sys_exit();
int sys_acquire();
int sys_release();

// CPU Kernel
class CPU {
public:
    std::shared_ptr<Task> current;
    std::shared_ptr<Task> next();
};

// Global Kernel
class Kernel {
private:
    std::shared_ptr<CPU> cpu[3];
    std::atomic<int64_t> next_id;     
    std::vector<std::shared_ptr<Task>> tasks;

public:
    std::unique_ptr<ClientLock> resource_lock;
    std::unique_ptr<ClientLock> usr_task_lock;

    Kernel() :
    cpu({
        std::shared_ptr<CPU>(new CPU()),
        std::shared_ptr<CPU>(new CPU()),
        std::shared_ptr<CPU>(new CPU()),
    }),
    resource_lock(new ClientLock(0x1111111111111)),
    usr_task_lock(new ClientLock(0x2222222222222))
    { };
    Kernel(Kernel const&)           = delete;
    void operator=(Kernel const&)   = delete;

public:
    void create_task(void *(*start_routine)(void *), void *arg);
    void delete_task(std::shared_ptr<Task> task);

    std::shared_ptr<Task> next();
    void ready(std::shared_ptr<Task> task);

    void dispatch();

    static Kernel& instance()
    {
        static Kernel instance;
        return instance;
    }

    std::shared_ptr<CPU> get(int cpu_id) {
        return cpu[cpu_id - 1]; // offset for cpu 0 being used as another
    } 
};

#endif