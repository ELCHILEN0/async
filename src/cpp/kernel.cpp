#include "include/kernel.hpp"

void kernel_interrupt_handler() {
    Kernel::instance().dispatch();
}

void *idle_proc(void *arg) {
    while (true);
}

void Kernel::dispatch() {
    int core_id = get_core_id();

    auto self = get(core_id);

    self->current->switch_from();

    // syscall ABI dependent
    unsigned long *params = (unsigned long *) self->current->frame + sizeof(aarch64_frame_t);
    auto request = params[0];
    auto args = (std::vector<std::experimental::fundamentals_v1::any> *) params[1];

    switch (request) {
        case SYS_ACQUIRE:
            resource_lock->acquire();
            self->current->sys_ret = 0;
            break;

        case SYS_RELEASE:
            resource_lock->release();
            self->current->sys_ret = 0;            
            break;

        case SYS_EXIT:
            create_task(idle_proc, NULL);
            self->next();
            break;
    }
    
    self->current->switch_to();
}

int syscall(enum system_call req_id, std::vector<std::experimental::fundamentals_v1::any> *args) {
    int ret_code;

    asm volatile("STP %0, %1, [SP, #-16]!" :: "r" (req_id), "r" (args));
    asm volatile("SVC 0x80");
    asm volatile("ADD SP, SP, #16");
    asm volatile("MOV %0, X0" : "=g" (ret_code));

    return ret_code;
}

template<typename ... T>
int syscall(enum system_call req_id, T ... args)
{
    std::vector<std::experimental::fundamentals_v1::any> params = {args ...};

    return syscall(req_id, &params);
}

int sys_exit() {
    return syscall(SYS_EXIT, 1, "two", 3.0);
}

int sys_acquire() {
    return syscall(SYS_ACQUIRE);
}

int sys_release() {
    return syscall(SYS_RELEASE);
}

void Kernel::create_task(void *(*start_routine)(void *), void *arg) {
    std::shared_ptr<Task> task = std::shared_ptr<Task>(new Task());

    // __spin_lock(&newlib_lock);
    // resource_lock->acquire();
    task->id = next_id++;
    task->stack_size = 4096;
    task->stack_base = malloc(task->stack_size);

    task->frame = new (task->stack_base + task->stack_size - sizeof(aarch64_frame_t)) Frame(start_routine, arg);
    // resource_lock->release();

    Kernel::instance().resource_lock->acquire();
    Kernel::instance().resource_lock->release();
    Kernel::instance().resource_lock->acquire();
    Kernel::instance().resource_lock->release();    

    ready(task);    
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
    // resource_lock->acquire();
    Kernel::instance().resource_lock->acquire();

    auto task = tasks.front();
    tasks.erase(tasks.begin());
    
    Kernel::instance().resource_lock->release();    
    // resource_lock->release();    
    // __spin_unlock(&newlib_lock);

    return task;
}

void Kernel::ready(std::shared_ptr<Task> task) {
    // resource_lock->acquire();
    Kernel::instance().resource_lock->acquire();    
    
    tasks.push_back(task);

    Kernel::instance().resource_lock->release();    
    // resource_lock->release();
}

std::shared_ptr<Task> CPU::next() {
    auto task = Kernel::instance().next();
    current = task;
    return current;
}