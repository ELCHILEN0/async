#include <cstdlib> 
#include <new> 

extern "C" {
    int __aeabi_atexit( 
        void *object, 
        void (*destructor)(void *), 
        void *dso_handle) 
    { 
        static_cast<void>(object); 
        static_cast<void>(destructor); 
        static_cast<void>(dso_handle); 
        return 0; 
    } 

    // __dso_handle defined in cstubs, not linking properly when defined here.
}

void* operator new(size_t size) noexcept 
{
    return malloc(size); 
} 

void operator delete(void *p) noexcept 
{ 
    free(p); 
} 

void* operator new[](size_t size) noexcept 
{ 
    return malloc(size);

void operator delete[](void *p) noexcept 
{
    free(p);
} 

void* operator new(size_t size, std::nothrow_t) noexcept 
{ 
    return malloc(size); 
} 

void operator delete(void *p,  std::nothrow_t) noexcept 
{ 
    free(p);
} 

void* operator new[](size_t size, std::nothrow_t) noexcept 
{ 
    return malloc(size); 
} 

void operator delete[](void *p,  std::nothrow_t) noexcept 
{ 
    free(p);
}

void *operator new(size_t, void *p)     noexcept { return p; }
void *operator new[](size_t, void *p)   noexcept { return p; }
void  operator delete  (void *, void *) noexcept { };
void  operator delete[](void *, void *) noexcept { };