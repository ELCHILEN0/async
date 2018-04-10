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

    void* __dso_handle = NULL;
}

constexpr size_t compute_alignment(size_t size) {
    return (size + 15) & ~15;
}

void* operator new(size_t size) noexcept 
{
    return malloc(compute_alignment(size));
} 

void operator delete(void *p) noexcept 
{ 
    free(p); 
} 

void* operator new[](size_t size) noexcept 
{ 
    return malloc(compute_alignment(size));
}

void operator delete[](void *p) noexcept 
{
    free(p);
} 

void* operator new(size_t size, std::nothrow_t) noexcept 
{ 
    return malloc(compute_alignment(size)); 
} 

void operator delete(void *p,  std::nothrow_t) noexcept 
{ 
    free(p);
} 

void* operator new[](size_t size, std::nothrow_t) noexcept 
{ 
    return malloc(compute_alignment(size)); 
} 

void operator delete[](void *p,  std::nothrow_t) noexcept 
{ 
    free(p);
}

// void *operator new(size_t, void *p)     noexcept { return p; }
// void *operator new[](size_t, void *p)   noexcept { return p; }
// void  operator delete  (void *, void *) noexcept { };
// void  operator delete[](void *, void *) noexcept { };