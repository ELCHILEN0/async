#include "include/kernel.hpp"

void Kernel::create_task(void *(*start_routine)(void *), void *arg) {
    std::shared_ptr<Task> task = std::shared_ptr<Task>(new Task());

    __spin_lock(&newlib_lock);
    // resource_lock->acquire();
    task->id = next_id++;
    task->stack_size = 4096;
    task->stack_base = malloc(task->stack_size);

    task->frame = new (task->stack_base + task->stack_size - sizeof(aarch64_frame_t)) Frame(start_routine, arg);
    task->switch_to();
    
    // TODO: locking with the api...
    tasks.push_back(task);

    // resource_lock->release();
    __spin_unlock(&newlib_lock);
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
    __spin_lock(&newlib_lock);
    // resource_lock->acquire();

    auto task = tasks.front();
    tasks.erase(tasks.begin());
    
    // resource_lock->release();    
    __spin_unlock(&newlib_lock);

    return task;
}

std::shared_ptr<Task> CPU::next() {
    auto task = Kernel::instance().next();
    current = task;
    return current;
}