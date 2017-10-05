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

static inline void flush(volatile intptr_t *addr,unsigned int length)
{
    volatile intptr_t *p=addr;
    for (int i=length;i>0;i-=64,p+=64)
    asm_clflush(p);
}
