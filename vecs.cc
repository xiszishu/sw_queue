#include <sys/mman.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "mmp_user.h"
#include "mmp_init.h"
#include "lat.h"

#define VEC_SZ 5000//65536

int main(int argc, char *argv[])
{
  void *a_p, *b_p, *c_p;
  long *a, *b, *c, a_val, b_val;
  int i, j, temp;
  rt_mem_t *rt_mem = get_mmp_initializer()->initialize();


  // mmapped areas to store 3 vectors
  a_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  b_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  c_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  a = (long *) a_p;
  b = (long *) b_p;
  c = (long *) c_p;

  // Outer for loop is just to increase computation time
  timer_begin=GetWallTime();

  for(j = 0; j < 100; j++)
  {

      // Initialize a & b
      for(i = 0; i < VEC_SZ; i++)
      {
          temp=i;
          rt_mem->write_literal(&temp, sizeof(long), &a[i]);
          //emulate_latency_ns_fence(2000);
          temp=i+VEC_SZ;
          rt_mem->write_literal(&temp, sizeof(long), &b[i]);
          //emulate_latency_ns_fence(2000);
      }

      //c = a + b;
      for(i = 0; i < VEC_SZ; i++)
      {
          //rt_mem->txend();
         a_val = *((long *) rt_mem->read(&a[i]));
         //a_val=a[i];
         //emulate_latency_ns_fence(2000);
         b_val = *((long *) rt_mem->read(&b[i]));
         //b_val=b[i];
         temp=a_val+b_val;
         rt_mem->write_literal(&temp, sizeof(long), &c[i]);
         // emulate_latency_ns_fence(2000);
      }
      rt_mem->do_transfer();
      printf("Finished computation for j = %d\n", j);
  }

  timer_end=GetWallTime();
  sum+=timer_end-timer_begin;

  printf("time: %.15lf\n",sum);
  // for (i=0;i<VEC_SZ;i++)
  // {
  //     printf("%ld\n",*((long *) rt_mem->read(&a[i])));
  //     printf("%ld\n", *((long *) rt_mem->read(&b[i])));
  //     printf("%ld\n", *((long *) rt_mem->read(&c[i])));
  // }
  munmap(a_p,VEC_SZ * sizeof(long));
  munmap(b_p,VEC_SZ * sizeof(long));
  munmap(c_p,VEC_SZ * sizeof(long));

  return 0;
}
