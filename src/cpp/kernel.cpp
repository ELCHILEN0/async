#include "include/kernel.hpp"

void kernel_interrupt_handler() {
    __spin_lock(&newlib_lock);
    printf("syscall\r\n");
    __spin_unlock(&newlib_lock);

    while(true);
}

// void Kernel::dispatch() {

// }

int syscall(enum system_call req_id, std::vector<std::experimental::fundamentals_v1::any> args) {
    int ret_code;

    asm volatile("STP %0, %1, [SP, #-16]!" :: "r" (req_id), "r" (&args));
    asm volatile("SVC 0x80");
    asm volatile("ADD SP, SP, #16");
    asm volatile("MOV %0, X0" : "=g" (ret_code));

    return ret_code;
}

void Kernel::create_task(void *(*start_routine)(void *), void *arg) {
    std::shared_ptr<Task> task = std::shared_ptr<Task>(new Task());

    // __spin_lock(&newlib_lock);
    resource_lock->acquire();
    task->id = next_id++;
    task->stack_size = 4096;
    task->stack_base = malloc(task->stack_size);

    task->frame = new (task->stack_base + task->stack_size - sizeof(aarch64_frame_t)) Frame(start_routine, arg);
    
    tasks.push_back(task);

    resource_lock->release();
    // __spin_unlock(&newlib_lock);
}

void Kernel::delete_task(std::shared_ptr<Task> task) {
    auto curr = tasks.begin();
    while (curr != tasks.end()) {
        if ((*curr)->id == task->id) {
            tasks.erase(curr);
            break;
        } else {
            curr = std::next(curr);
        }
    }
}

std::shared_ptr<Task> Kernel::next() {
    // __spin_lock(&newlib_lock);
    resource_lock->acquire();

    auto task = tasks.front();
    tasks.erase(tasks.begin());
    
    resource_lock->release();    
    // __spin_unlock(&newlib_lock);

    return task;
}

std::shared_ptr<Task> CPU::next() {
    auto task = Kernel::instance().next();
    current = task;
    return current;
}