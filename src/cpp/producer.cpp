#include "msync.hpp"

// Producer interrupt handler
void producer_dispatch_handler() {
    auto requests = &Producer::instance().requests;

    // An interrupt source will not be cleared until the request is handled
    uint32_t interrupt_src = core_interrupt_src_irq[Producer::instance().core_id()];
    while (interrupt_src != 0) {
        uint8_t src_bit = __builtin_ffs(interrupt_src) - 1;
        interrupt_src &= ~(1 << src_bit);

        uint8_t requesting_core = src_bit - 4;
        if (std::find(std::begin(*requests), std::end(*requests), requesting_core) == std::end(*requests)) {
            requests->push_back(requesting_core);
        }
    }  

    auto pending = requests->begin();
    while (pending != requests->end()) {
        if (Producer::instance().handle_request(*pending)) {
            pending = requests->erase(pending);
        } else {
            pending = std::next(pending);
        }
    }
}

void Producer::dispatch() {
    // core_timer_init( CT_CTRL_SRC_APB, CT_CTRL_INC2, 0x80000000 );

    pmu_enable();

    pmu_enable_ccnt();
    pmu_reset_ccnt();

    register_interrupt_handler(this->core_id(), false, 5, (interrupt_vector_t) { .identify = NULL, .handle = producer_dispatch_handler });
    register_interrupt_handler(this->core_id(), false, 6, (interrupt_vector_t) { .identify = NULL, .handle = producer_dispatch_handler });
    register_interrupt_handler(this->core_id(), false, 7, (interrupt_vector_t) { .identify = NULL, .handle = producer_dispatch_handler });
    core_mailbox_interrupt_routing(this->core_id(), MB1_IRQ | MB2_IRQ | MB3_IRQ);
    __enable_interrupts();
    
    while (true) asm("WFI");
}

bool Producer::handle_request(uint8_t requestor) {
    uint32_t lock_id = core_mailbox->rd_clr[this->core_id()][requestor];
    auto lock = this->get_lock(lock_id);    

    if (lock_id & ACQUIRE_FLAG) {
        if (!lock->is_assigned()) {
            lock->assign(requestor);
            return true;
        }
    } else {
        if (lock->is_assigned() && lock->owner == requestor) {
            lock->revoke(requestor);
            return true;
        } 
    }

    return false;
}

bool Producer::lock_exists(uint64_t lock_id) {
    lock_id &= LOCK_ID_MASK;
    
    std::map<uint64_t, ProducerLock*>::iterator lock = locks.find(lock_id);
    if (lock != locks.end())
        return true;

    return false;
}

ProducerLock* Producer::get_lock(uint64_t lock_id) {
    lock_id &= LOCK_ID_MASK;

    std::map<uint64_t, ProducerLock*>::iterator lock = locks.find(lock_id);
    if (lock != locks.end())
        return lock->second;

    auto new_lock = new ProducerLock(lock_id);
    locks[lock_id] = new_lock;
    return new_lock;
}