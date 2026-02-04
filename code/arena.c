#include "arena.h"

#if defined(_WIN32)

#include <windows.h>
//using the Win API to use virtual allocation instead of malloc() and free()

u32 get_pagesize(void){
    SYSTEM_INFO s ={0};
    GetSystemInfo(&s);
    return s.dwPageSize;
}

//reserve virtual address space
void* mem_reserve(u64 size){
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

//allocate actual memory inside a reserved space
b32 mem_commit(void* ptr, u64 size){
    void* loc = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    return loc != NULL;
}

b32 mem_decommit(void* ptr, u64 size){
    return VirtualFree(ptr, size, MEM_DECOMMIT);
}

b32 mem_release(void* ptr, u64 size){
    return VirtualFree(ptr, size, MEM_RELEASE);
}

#elif defined (__linux__)

#define __DEFAULT_SOURCE

#include <unistd.h>
#include <sys/mman.h>

u32 get_pagesize(void){
    return (u32)sysconf(_SC_PAGESIZE);
}

//reserve virtual address space
void* mem_reserve(u64 size){
    void* res = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (res == MAP_FAILED) {
        return NULL;
    }
    return res;
}

//mark reserved space for reading and writing
b32 mem_commit(void* ptr, u64 size){
    i32 ret = mprotect(ptr, size, PROT_READ| PROT_WRITE);
    return ret == 0;
}

//mark reserved space as unusable
b32 mem_decommit(void* ptr, u64 size){
    i32 ret = mprotect(ptr, size, PROT_NONE);
    if(ret != 0) return false;
    ret = madvise(ptr, size, MADV_FREE);
    return ret == 0;
}

b32 mem_release(void* ptr, u64 size){
    i32 ret = munmap(ptr, size);
    return ret == 0;
}

#endif

