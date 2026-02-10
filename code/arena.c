#include <stdio.h>
#include <stdlib.h>
#include "arena.h"

arena* arena_create(u64 reserve_size, u64 commit_size){
    //reserve and commit sizes must fit page boundaries
    u32 pgsize = get_pagesize();
    reserve_size = ALIGN_UP_POW2(reserve_size, pgsize);
    commit_size = ALIGN_UP_POW2(commit_size, pgsize);

    arena* ar = (arena*)mem_reserve(reserve_size);
    if (ar == NULL) {
        fprintf(stderr, "Error Reserving VA Space");
        exit(EXIT_FAILURE);
    }
    if(!mem_commit(ar, commit_size)){
        fprintf(stderr, "Error Comitting to VA Space");
        exit(EXIT_FAILURE);
    }

    ar->reserve_size = reserve_size;
    ar->commit_size = commit_size;
    ar->pos = ARENA_BASE_POS;
    ar->commit_pos = commit_size;

    return ar;
}

void* arena_push(arena* ar, u64 size){
    //ensures our allocations start aligned with the word size
    u64 pos_align = ALIGN_UP_POW2(ar->pos, ARENA_ALIGN);
    u64 new_pos = pos_align + size;

    if(new_pos > ar->reserve_size) {
        fprintf(stderr, "Failed to push past reserved space");
        exit(EXIT_FAILURE);
    }

    if(new_pos> ar->commit_pos){
        //round up based on commit size for less commit calls
        u64 new_comm = new_pos;
        new_comm += ar->commit_size - 1;
        new_comm -= new_comm % ar->commit_size;
        new_comm = MIN(new_comm, ar->reserve_size);


        u8* ptr = (u8*)ar + ar->commit_pos; //exact byte math (no ptr arithmetic)
        u64 commit_size = new_comm - ar->commit_pos;
        if(!mem_commit(ptr, commit_size)){
            fprintf(stderr, "Failed to push new commit");
            exit(EXIT_FAILURE);
        }
        ar->commit_pos = new_comm;
    }

    //update the location for the next alloc, return position of current alloc
    ar->pos = new_pos;
    return (u8*)ar + pos_align;
}

void arena_pop(arena* ar, u64 size){
    u64 old_pos = ar->pos;
    size = MIN(size, ar->pos - ARENA_BASE_POS);
    ar->pos -= size;
    
    //align our old and new pos to commit_size boundaries
    u64 old_align = old_pos + ar->commit_size - 1;
    old_align -= old_align % ar->commit_size;
    u64 new_align = ar->pos + ar->commit_size - 1;
    new_align -= new_align % ar->commit_size;

    if(new_align < old_align){
        u64 decommit_size = old_align-new_align;
        if(!mem_decommit((void*)new_align, decommit_size)){
            fprintf(stderr, "Failed to decommit on pop");
            exit(EXIT_FAILURE);
        }
        ar->commit_pos -= decommit_size;
    }
}

void arena_pop_to(arena* ar, u64 pos){
    u64 size = pos < ar->pos ? ar->pos - pos : 0;
    if(size != 0) arena_pop(ar, size);
}

void arena_clear(arena* ar){
    arena_pop_to(ar, ARENA_BASE_POS);
}

b32 arena_destroy(arena* ar){
    return mem_release(ar, ar->reserve_size);
}


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
