#ifndef __MSYNC_H_INCLUDED__
#define __MSYNC_H_INCLUDED__

#include <iostream>
#include <cstdint>
// #include <vector>

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
    protected:
        uint32_t id;
};

class ClientLock: public Lock {
    public:
        void acquire();
        void release();
        uint8_t owner();
};

class ProducerLock: public Lock {
    private:
        uint8_t owner;

    public:
        void assign(uint8_t owner);
        void revoke(uint8_t owner);
        bool is_assigned();
};

class Producer {
    private:
        // std::vector<ProducerLock> locks;
    
    public:
        void dispatch();
        bool lock_exists();
        ProducerLock get_lock();
};

#endif
