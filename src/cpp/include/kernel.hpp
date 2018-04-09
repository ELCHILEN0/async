#ifndef KERNEL_H
#define KERNEL_H

#include <atomic>
#include <memory>
#include <vector>
#include "context.hpp"

#include "msync.hpp"

#include <stdlib.h>

#include "../../c/include/multicore.h"

class Kernel;

extern "C" spinlock_t newlib_lock;

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

    std::unique_ptr<ClientLock> resource_lock;
    
    Kernel() :
    cpu({
        std::shared_ptr<CPU>(new CPU()),
        std::shared_ptr<CPU>(new CPU()),
        std::shared_ptr<CPU>(new CPU()),
    }),
    resource_lock(new ClientLock(0x1234567899999))
    { };
    Kernel(Kernel const&)           = delete;
    void operator=(Kernel const&)   = delete;

public:
    void create_task(void *(*start_routine)(void *), void *arg);
    void delete_task(std::shared_ptr<Task> task);

    std::shared_ptr<Task> next();

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