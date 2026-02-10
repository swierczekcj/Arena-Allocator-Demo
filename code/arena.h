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

u32 get_pagesize(void);
void* mem_reserve(u64 size);
b32 mem_commit(void* ptr, u64 size);
b32 mem_decommit(void* ptr, u64 size);
b32 mem_release(void* ptr, u64 size);

