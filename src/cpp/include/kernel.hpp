#ifndef KERNEL_H
#define KERNEL_H

#include <atomic>
#include <memory>
#include <vector>
#include "context.hpp"

#include <stdlib.h>

#include "../../c/include/multicore.h"

class Kernel;

extern "C" spinlock_t newlib_lock;

// Global Kernel
class StaticKernel {
private:
    StaticKernel() {};
    StaticKernel(StaticKernel const&)   = delete;
    void operator=(StaticKernel const&) = delete;

public:
    std::unique_ptr<Kernel> cpu[3];
    std::vector<std::shared_ptr<Task>> tasks;

    std::atomic<int64_t> next_id; 

    void create_task(void *(*start_routine)(void *), void *arg);
    void delete_task(std::shared_ptr<Task> task);

    std::shared_ptr<Task> next();

    static StaticKernel& instance()
    {
        static StaticKernel instance;
        return instance;
    }
};

// CPU Kernel
class Kernel {
public:
    std::shared_ptr<Task> current;

    std::shared_ptr<Task> next();
};

#endif