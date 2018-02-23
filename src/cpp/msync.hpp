#ifndef MSYNC_H
#define MSYNC_H

#include <iostream>
#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>

#include <stdio.h>
#include "../c/mailbox.h"
#include "../c/multicore.h"
#include "../c/interrupts.h"

#define LOCK_ID_MASK 0xFFFFFFF0
#define ACQUIRE_FLAG (1 << 0)
#define RELEASE_FLAG (1 << 1)

extern "C" void __enable_interrupts(void);

class Lock {
    public:
        Lock(uint32_t id) : id(id)
        {

        }
        uint32_t id;
};

class ClientLock: public Lock {
    public:
        ClientLock(uint32_t id);
        void acquire();
        void release();
};

class ProducerLock: public Lock {
    public:
        int8_t owner;

    public:
        ProducerLock(uint32_t id) : Lock(id & LOCK_ID_MASK), owner(-1)
        {

        }
        void assign(int8_t owner);
        void revoke(int8_t owner);
        bool is_assigned();
};

class Producer {
    private:
        Producer() {};
        Producer(Producer const&)           = delete;
        void operator=(Producer const&)     = delete;

        std::map<uint32_t, ProducerLock*> locks;
        std::vector<uint8_t> waiters;
    
    public:
        friend void producer_dispatch_handler();
        // Action Methods
        void dispatch();
        void handle_request(uint8_t requestor);

        // Utility Methods
        bool lock_exists(uint32_t lock_id);
        ProducerLock *get_lock(uint32_t lock_id);

        const uint8_t core_id() {
            return 0; // TODO: initial core
        }

        static Producer& instance()
        {
            static Producer instance;
            return instance;
        }
};

inline core_mailbox_interrupt_t operator|(core_mailbox_interrupt_t l, core_mailbox_interrupt_t r)
{ return static_cast<core_mailbox_interrupt_t>(static_cast<int>(l) | static_cast<int>(r)); }

#endif
