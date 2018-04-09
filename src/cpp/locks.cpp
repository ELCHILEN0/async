#include "include/msync.hpp"

void client_lock_handler() {
    // Immediately clear the ACK
    core_mailbox->rd_clr[get_core_id()][0] = ~(0);
}

void ClientLock::acquire() {
    uint8_t core_id = get_core_id();
    core_mailbox->set[Producer::instance().core_id()][core_id] = (this->id & LOCK_ID_MASK) | ACQUIRE_FLAG;
    // while(core_mailbox->rd_clr[Producer::instance().core_id()][core_id] != 0);
    asm("WFI"); // For more complex systems the result should be verified
}

void ClientLock::release() {
    uint8_t core_id = get_core_id();
    core_mailbox->set[Producer::instance().core_id()][core_id] = (this->id & LOCK_ID_MASK) & ~ACQUIRE_FLAG;
    // while(core_mailbox->rd_clr[Producer::instance().core_id()][core_id] != 0); // TODO ... Policy choice, release immediately or wait for signal
    asm("WFI"); // For more complex systems the result should be verified
}

void ProducerLock::assign(int8_t owner) {
    this->owner = owner;
    this->prev_count = pmu_read_ccnt();
    core_mailbox->rd_clr[Producer::instance().core_id()][owner] = ~(0);
    core_mailbox->set[owner][0] = true;

    // printf("assigned %d\r\n", owner);
}

void ProducerLock::revoke(int8_t owner) {
    // printf("revoking %d\r\n", owner);
    
    this->owner = -1;    
    this->hold_count = pmu_read_ccnt() - this->prev_count;
    core_mailbox->rd_clr[Producer::instance().core_id()][owner] = ~(0);
    core_mailbox->set[owner][0] = true;
}

bool ProducerLock::is_assigned() {
    return this->owner != -1;
}