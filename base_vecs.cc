#include <sys/mman.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "lat.h"
#include <queue>
#define VEC_SZ 65536

typedef std::pair <long, long> dual;
std::queue<dual> log_s;
//double timer_begin,timer_end,sum;

int main(int argc, char *argv[]) {
  void *a_p, *b_p, *c_p;
  long *a, *b, *c, a_val, b_val;
  int i, j;

  a_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  b_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  c_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  a = (long *) a_p;
  b = (long *) b_p;
  c = (long *) c_p;
  timer_begin=GetWallTime();

  for(j = 0; j < 100; j++) {
    for(i = 0; i < VEC_SZ; i++)
    {
        //log_s.push(dual(i,i+VEC_SZ));
        //emulate_latency_ns_fence(40000);
      a[i] = i;
      emulate_latency_ns_fence(20000);
      //log_s.push(dual(i,i+VEC_SZ));
      //emulate_latency_ns_fence(40000);
      b[i] = i + VEC_SZ;
      emulate_latency_ns_fence(20000);
    }

    for(i = 0; i < VEC_SZ; i++) {
      a_val = a[i];
      b_val = b[i];
      //log_s.push(dual(i,i+VEC_SZ));
      //emulate_latency_ns_fence(40000);
      c[i] = a_val + b_val;
      emulate_latency_ns_fence(20000);
      //emulate_latency_ns_fence(20000);
    }

    printf("Finished computation for j = %d\n", j);
  }
  timer_end=GetWallTime();
  sum+=timer_end-timer_begin;
  printf("time: %.15lf\n",sum);

  munmap(a_p,VEC_SZ * sizeof(long));
  munmap(b_p,VEC_SZ * sizeof(long));
  munmap(c_p,VEC_SZ * sizeof(long));

  return 0;

}
