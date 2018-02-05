#ifndef __MSYNC_H_INCLUDED__
#define __MSYNC_H_INCLUDED__

#include <iostream>
#include <cstdint>
#include <vector>
#include "../c/multicore.h"

#define LOCK_ID_MASK 0xFFFFFFF0
#define ACQUIRE_FLAG (1 << 0)
#define RELEASE_FLAG (1 << 1)

typedef struct {
    uint32_t set[4];
    uint32_t _RESERVED_0[(0xC0 - 0x8C) / sizeof(uint32_t) - 1];
    uint32_t rd_clr[4];
} core_mailbox_t;

extern core_mailbox_t *core_mailboxes;

class Lock {
    public:
        uint32_t id;
};

class ClientLock: public Lock {
    public:
        void acquire();
        void release();
};

class ProducerLock: public Lock {
    public:
        int8_t owner;

    public:
        void assign(int8_t owner);
        void revoke(int8_t owner);
        bool is_assigned();
};

class Producer {
    private:
        std::vector<ProducerLock*> locks;
    
    public:
        void dispatch();
        bool lock_exists(uint32_t lock_id);
        ProducerLock* get_lock(uint32_t lock_id);

    // friend Lock;
    // friend ProducerLock;
};

#endif
