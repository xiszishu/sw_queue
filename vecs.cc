#include <sys/mman.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "mmp_user.h"
#include "mmp_init.h"

#define VEC_SZ 65536

int main(int argc, char *argv[])
{
  void *a_p, *b_p, *c_p;
  long *a, *b, *c, a_val, b_val;
  int i, j;
  rt_mem_t *rt_mem = get_mmp_initializer()->initialize();


  // mmapped areas to store 3 vectors
  a_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  b_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  c_p = mmap(0, VEC_SZ * sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  a = (long *) a_p;
  b = (long *) b_p;
  c = (long *) c_p;

  // Outer for loop is just to increase computation time
  for(j = 0; j < 10; j++)
  {

      // Initialize a & b
      for(i = 0; i < VEC_SZ; i++)
      {
          rt_mem->txend();
          rt_mem->write_literal((void *) ((long) i), sizeof(long), &a[i]);
          rt_mem->write_literal((void *) ((long) i + VEC_SZ), sizeof(long), &b[i]);
      }

      //c = a + b;
      for(i = 0; i < VEC_SZ; i++)
      {
         rt_mem->txend();
         a_val = *((long *) rt_mem->read(&a[i]));
         b_val = *((long *) rt_mem->read(&b[i]));
         rt_mem->write_literal((void *) ((long) a_val + b_val), sizeof(long), &c[i]);
      }

      printf("Finished computation for j = %d\n", j);
  }
  rt_mem->appfinish=1;

  pthread_join(rt_mem->th1, NULL);

  for (i=0;i<VEC_SZ;i++)
  {
      printf("%ld\n",*((long *) rt_mem->read(&a[i])));
      printf("%ld\n", *((long *) rt_mem->read(&b[i])));
      printf("%ld\n", *((long *) rt_mem->read(&c[i])));
  }
  munmap(a_p,VEC_SZ * sizeof(long));
  munmap(b_p,VEC_SZ * sizeof(long));
  munmap(c_p,VEC_SZ * sizeof(long));

  return 0;
}
