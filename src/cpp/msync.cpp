#include "msync.hpp"

core_mailbox_t *core_mailboxes = (core_mailbox_t *) 0x40000080;

class ClientLock: public Lock {
    public:
        void acquire() {
            core_mailboxes[0].set[0] = (this->id & LOCK_ID_MASK) | ACQUIRE_FLAG;
            // asm("SEV");
            // asm("WFE"); TODO ... signal handling
            while(core_mailboxes[0].rd_clr[0] != 0);
        };

        void release() {
            core_mailboxes[0].set[0] = (this->id & LOCK_ID_MASK) | RELEASE_FLAG;
            while(core_mailboxes[0].rd_clr[0] != 0);   // TODO ... Policy choice, release immediately or wait for confirmation...
        };

        virtual uint8_t owner(); 
};

class ProducerLock: public Lock {
    private:
        uint8_t owner;

    public:
        void assign(uint8_t owner) {
            this->owner = owner;
        }

        void revoke(uint8_t owner) {
            this->owner = -1;
        }

        bool is_assigned() {
            return this->owner != -1;
        }
};

class Producer {
    private:
        std::vector<ProducerLock> locks;

    public:
        void dispatch() {
            std::cout << "[producer] Dispatch..." << "\r\n";
            while (true) {

            }
        }

        virtual bool lock_exists();
        virtual ProducerLock get_lock();
};