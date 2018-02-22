#include "msync.hpp"

// Producer interrupt handler
void producer_dispatch_handler() {
    // TODO: Round robin instead of ffs.
    uint8_t requesting_core = (__builtin_ffs(core_interrupt_src_irq[Producer::instance().core_id()]) - 1) - 4;
    Producer::instance().handle_request(requesting_core);
}

extern "C" void __enable_interrupts(void);

void Producer::dispatch() {
    printf("[producer] Dispatch...\r\n");

    register_interrupt_handler(this->core_id(), false, 5, (interrupt_vector_t) { .identify = NULL, .handle = producer_dispatch_handler });
    register_interrupt_handler(this->core_id(), false, 6, (interrupt_vector_t) { .identify = NULL, .handle = producer_dispatch_handler });
    register_interrupt_handler(this->core_id(), false, 7, (interrupt_vector_t) { .identify = NULL, .handle = producer_dispatch_handler });
    core_mailbox_interrupt_routing(this->core_id(), MB1_IRQ | MB2_IRQ | MB3_IRQ);
    __enable_interrupts();
    
    while (true) asm("WFI");
}

void Producer::handle_request(uint8_t requestor) {
    uint32_t lock_id = core_mailbox->rd_clr[this->core_id()][requestor];
            
    if (lock_id & ACQUIRE_FLAG)  {
        auto lock = this->get_lock(lock_id);
        if (!lock->is_assigned()) {
            lock->assign(requestor);
        }
    } else if (lock_id & RELEASE_FLAG) {
        auto lock = this->get_lock(lock_id);
        if (lock->is_assigned() && lock->owner == requestor) {
            lock->revoke(requestor);
        } 
    }
}

bool Producer::lock_exists(uint32_t lock_id) {
    for (const class ProducerLock *lock : this->locks) {
        if (lock->id == (lock_id & LOCK_ID_MASK))
            return true;
    }

    return false;
}

ProducerLock* Producer::get_lock(uint32_t lock_id) {
    for (class ProducerLock *lock : this->locks) {
        if (lock->id == (lock_id & LOCK_ID_MASK))
            return lock;
    }

    auto lock = new ProducerLock(lock_id);
    this->locks.push_back(lock);
    return lock;
}