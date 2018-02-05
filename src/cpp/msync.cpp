#include "msync.hpp"

core_mailbox_t *core_mailboxes = (core_mailbox_t *) 0x40000080;

void ClientLock::acquire() {
    uint32_t core_id = get_core_id();    
    core_mailboxes[core_id].set[0] = (this->id & LOCK_ID_MASK) | ACQUIRE_FLAG;
    // asm("SEV");
    // asm("WFE"); TODO ... signal handling
    while(core_mailboxes[core_id].rd_clr[0] != 0);
}

void ClientLock::release() {
    uint32_t core_id = get_core_id();
    core_mailboxes[core_id].set[0] = (this->id & LOCK_ID_MASK) | RELEASE_FLAG;
    while(core_mailboxes[core_id].rd_clr[0] != 0); // TODO ... Policy choice, release immediately or wait for signal
}

void ProducerLock::assign(int8_t owner) {
    printf("[producer] Lock assigned to %d\r\n", owner);
    this->owner = owner;
}

void ProducerLock::revoke(int8_t owner) {
    printf("[producer] Lock revoked from %d\r\n", owner);    
    this->owner = -1;
}

bool ProducerLock::is_assigned() {
    return this->owner != -1;
}

void Producer::dispatch() {
    printf("[producer] Dispatch...\r\n");
    
    while (true) {
        for (int core_id = 1; core_id < 3; core_id++) {
            uint32_t lock_id = core_mailboxes[core_id].rd_clr[0];
            
            if (lock_id & ~ACQUIRE_FLAG)  {
                auto lock = this->get_lock(lock_id);
                if (!lock->is_assigned()) {
                    lock->assign(core_id);
                }
            } else if (lock_id & ~RELEASE_FLAG) {
                auto lock = this->get_lock(lock_id);
                if (lock->is_assigned() && lock->owner == core_id) {
                    lock->revoke(core_id);
                } 
            }
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

    auto lock = new ProducerLock();
    this->locks.push_back(lock);
    return lock;
}