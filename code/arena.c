#include <stdio.h>
#include "arena.h"

arena* arena_create(u64 reserve_size, u64 commit_size){
    //reserve and commit sizes must fit page boundaries
    u32 pgsize = get_pagesize();
    reserve_size = ALIGN_UP_POW2(reserve_size, pgsize);
    commit_size = ALIGN_UP_POW2(commit_size, pgsize);

    arena* ar = mem_resereve(reserve_size);
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

b32 arena_destroy(arena* ar) {
    return plat_mem_release(ar, ar->reserve_size);
}

void* arena_push(arena* ar, u64 size, b32 non_zero){
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



