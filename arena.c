#include "arena.h"

#if defined(_WIN32)
#include <windows.h>
//using the Win API to use virtual allocation instead of malloc() and free()

u32 get_pagesize(void){
    SYSTEM_INFO s ={0};
    GetSystemInfo(&s);
    return s.dwPageSize;
}

#endif