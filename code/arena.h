#include <stdint.h>
#include <stdbool.h>

//typing longer than necessary sucks
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef i8 b8;
typedef i32 b32;

//same here
#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)


#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ALIGN_UP_POW2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))

#define ARENA_BASE_POS (sizeof(arena))
#define ARENA_ALIGN (sizeof(void*))

typedef struct {
    u64 reserve_size;
    u64 commit_size;

    u64 pos;
    u64 commit_pos;
} arena;

arena* arena_create(u64 reserve_size, u64 commit_size);
b32 arena_destroy(arena* ar);
void* arena_push(arena* ar, u64 size);
void arena_pop(arena* ar, u64 size);
void arena_pop_to(arena* ar, u64 pos);
void arena_clear(arena* ar);

#if defined(_WIN32)

#include <windows.h>
//using the Win API to use virtual allocation instead of malloc and free 

u32 get_pagesize(void){
    SYSTEM_INFO s ={0};
    GetSystemInfo(&s);
    return s.dwPageSize;
}

//reserve virtual address space
void* mem_reserve(u64 size){
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

//allocate actual memory charges inside reserved space
b32 mem_commit(void* ptr, u64 size){
    void* loc = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    return loc != NULL;
}

//returns pages to reserve state
b32 mem_decommit(void* ptr, u64 size){
    return VirtualFree(ptr, size, MEM_DECOMMIT);
}

//releases VA space
b32 mem_release(void* ptr, u64 size){
    return VirtualFree(ptr, size, MEM_RELEASE);
}

#elif defined (__linux__)
//using linux memory manager to use VA spaces instead of malloc and free

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

//release VA space
b32 mem_release(void* ptr, u64 size){
    i32 ret = munmap(ptr, size);
    return ret == 0;
}

#endif