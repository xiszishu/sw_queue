#include <stdio.h>
#include <iostream>
#include <stdlib.h>

static inline void asm_clflush(volatile intptr_t *addr)
{
    __asm__ __volatile__ ("clflush %0 "::"m"(*addr));
}
static inline void asm_mfence(void)
{
    __asm__ __volatile__ ("mfence");
}
