#ifndef MSYNC_H
#define MSYNC_H

#include <iostream>
#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>

#include <stdio.h>
#include "../c/perf.h"
#include "../c/mailbox.h"
#include "../c/multicore.h"
#include "../c/interrupts.h"

#define ACQUIRE_FLAG    (1 << 0)
#define SPAN_FLAG       (1 << 1)
#define LOCK_ID_MASK    ~(SPAN_FLAG | ACQUIRE_FLAG)

extern "C" void __enable_interrupts(void);

class Lock {
    public:
        Lock(uint64_t id) : id(id)
        {

        }
        uint64_t id;
};

class ClientLock: public Lock {
    public:
        ClientLock(uint64_t id);
        void acquire();
        void release();
};

class ProducerLock: public Lock {
    public:
        uint64_t perf_prev;
        uint64_t hold_count;
        uint64_t cont_count;
        uint8_t owner;

    public:
        ProducerLock(uint64_t id) : Lock(id & LOCK_ID_MASK), owner(-1)
        {

        }
        void assign(uint8_t owner);
        void revoke(uint8_t owner);
        bool is_assigned();
};

class Producer {
    private:
        Producer() {};
        Producer(Producer const&)           = delete;
        void operator=(Producer const&)     = delete;

        std::map<uint64_t, ProducerLock*> locks;
        std::vector<uint8_t> requests;
    
    public:
        friend void producer_dispatch_handler();
        // Action Methods
        void dispatch();
        bool handle_request(uint8_t requestor, uint64_t time_count);

        // Utility Methods
        bool lock_exists(uint64_t lock_id);
        ProducerLock *get_lock(uint64_t lock_id);

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
