#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include "lat.h"

double timer_begin,timer_end,sum;
//typedef uint64_t hrtime_t;
//static int global_cpu_speed_mhz = 0;
//static int global_write_latency_ns = 0;

size_t string_to_size(char* str)
{
    size_t factor = 1;
    size_t size;
    long   val;
    char*  endptr = 0;

    val = strtoull(str, &endptr, 10);
    while(endptr && (endptr - str) < strlen(str) && !isalpha(*endptr)) {endptr++;}

    switch (endptr[0]) {
    case 'K': case 'k':
        factor = 1024LLU;
        break;
    case 'M': case 'm':
        factor = 1024LLU*1024LLU;
        break;
    case 'G': case 'g':
        factor = 1024LLU*1024LLU*1024LLU;
        break;
    default:
        factor = 1;
    }
    size = factor * val;
    return size;
}

char *cpuinfo(char *valname)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL)
    {
        return NULL;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (strstr(line, valname))
        {
            char *colon = strchr(line, ':');
            int len = colon - line;
            char *buf = (char *) malloc(strlen(line) - len);
            strcpy(buf, &line[len + 2]);
            free(line);
            fclose(fp);
            return buf;
        }
    }

    free(line);
    fclose(fp);
    return NULL;
}

int cpu_speed_mhz()
{
    size_t val;
    char *str = cpuinfo("cpu MHz");
    val = string_to_size(str);
    free(str);
    return val;
}

void init_pflush(int cpu_speed_mhz, int write_latency_ns)
{
    global_cpu_speed_mhz = cpu_speed_mhz;
    global_write_latency_ns = write_latency_ns;
}

inline hrtime_t ns_to_cycles(int cpu_speed_mhz, hrtime_t ns)
{
    return (ns*cpu_speed_mhz/1000);
}

double GetWallTime(void)
{
    struct timeval tp;
    static long start=0, startu;
    if (!start)
    {
        gettimeofday(&tp, NULL);
        start = tp.tv_sec;
        startu = tp.tv_usec;
        return(0.0);
    }
    gettimeofday(&tp, NULL);
    return( ((double) (tp.tv_sec - start)) + (tp.tv_usec-startu)/1000000.0 );
}

static inline void emulate_latency_ns(int ns)
{
    hrtime_t cycles;
    hrtime_t start;
    hrtime_t stop;
    
    start = asm_rdtsc();
    cycles = ns_to_cycles(global_cpu_speed_mhz, ns);

    do { 
        /* RDTSC doesn't necessarily wait for previous instructions to complete 
         * so a serializing instruction is usually used to ensure previous 
         * instructions have completed. However, in our case this is a desirable
         * property since we want to overlap the latency we emulate with the
         * actual latency of the emulated instruction. 
         */
        stop = asm_rdtsc();
    } while (stop - start < cycles);
}
/* int main() */
/* { */
/*     int i,j; */
/*     int write_latency=1000; */
/*     init_pflush(cpu_speed_mhz(), write_latency); */
/*     for (i=0;i<1000000;i++) */
/*     emulate_latency_ns(1000); */
/*     return 0; */
/* } */
