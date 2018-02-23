#include "msync.hpp"

void client_lock_handler() {
    core_mailbox->rd_clr[get_core_id()][0] = ~(0);
}

ClientLock::ClientLock(uint32_t id) : Lock(id)
{
    // TODO: Doesn't necesarily belong in constructor
    uint8_t core_id = get_core_id();
    register_interrupt_handler(core_id, false, 4, (interrupt_vector_t) { .identify = NULL, .handle = client_lock_handler });
    core_mailbox_interrupt_routing(core_id, MB0_IRQ);
    __enable_interrupts();
}

void ClientLock::acquire() {
    uint8_t core_id = get_core_id();
    core_mailbox->set[Producer::instance().core_id()][core_id] = (this->id & LOCK_ID_MASK) | ACQUIRE_FLAG;
    // while(core_mailbox->rd_clr[Producer::instance().core_id()][core_id] != 0);
    asm("WFI"); // For more complex systems the result should be verified
}

void ClientLock::release() {
    uint8_t core_id = get_core_id();
    core_mailbox->set[Producer::instance().core_id()][core_id] = (this->id & LOCK_ID_MASK) | RELEASE_FLAG;
    // while(core_mailbox->rd_clr[Producer::instance().core_id()][core_id] != 0); // TODO ... Policy choice, release immediately or wait for signal
    asm("WFI"); // For more complex systems the result should be verified
}

void ProducerLock::assign(int8_t owner) {
    this->owner = owner;
    core_mailbox->rd_clr[Producer::instance().core_id()][owner] = ~(0);
    core_mailbox->set[owner][0] = true;
}

void ProducerLock::revoke(int8_t owner) {
    core_mailbox->rd_clr[Producer::instance().core_id()][owner] = ~(0);
    core_mailbox->set[owner][0] = true;    
    this->owner = -1;    
}

bool ProducerLock::is_assigned() {
    return this->owner != -1;
}