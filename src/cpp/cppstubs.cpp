#include <cstdlib> 
#include <new> 

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