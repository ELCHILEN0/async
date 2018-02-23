#include "msync.hpp"

// Producer interrupt handler
void producer_dispatch_handler() {
    uint32_t interrupt_src = core_interrupt_src_irq[Producer::instance().core_id()];
    std::vector<uint8_t> requests;
    while (interrupt_src != 0) {
        uint8_t src_bit = __builtin_ffs(interrupt_src) - 1;
        interrupt_src &= ~(1 << src_bit);

        requests.push_back(src_bit - 4);
    }

    auto waiters = Producer::instance().waiters;
    for (auto request : requests) {
        if (std::find(std::begin(waiters), std::end(waiters), request) == std::end(waiters)) {
            waiters.push_back(request);
        }
    }

    if (!waiters.empty()) {
        Producer::instance().handle_request(waiters.front());
        waiters.erase(waiters.begin());
    }

    // TODO: Round robin instead of ffs.
    // uint8_t requesting_core = (__builtin_ffs(core_interrupt_src_irq[Producer::instance().core_id()]) - 1) - 4;
    // Producer::instance().handle_request(requesting_core);

    
}

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
    std::map<uint32_t, ProducerLock*>::iterator lock = locks.find(lock_id);
    if (lock != locks.end())
        return true;

    return false;
}

ProducerLock* Producer::get_lock(uint32_t lock_id) {
    std::map<uint32_t, ProducerLock*>::iterator lock = locks.find(lock_id);
    if (lock != locks.end())
        return lock->second;

    auto new_lock = new ProducerLock(lock_id);
    locks[lock_id] = new_lock;
    return new_lock;
}