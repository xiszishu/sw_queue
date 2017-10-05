#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <deque>
#include <iostream>
#include <Judy.h>
#include "mmp_user.h"
#include "lat.h"

#define SIZE 100000

static int is_initialized = 0, initializing = 0, transferring = 0;
//static buffer_t buffer;
static std::deque<buffer_t> cmtq;
static bool dirties[SIZE];
static buffer_t fake;
struct timespec tim, tim2;
static pthread_mutex_t lock;
std::deque<buffer_t>::iterator giter;

Pvoid_t  PJLArray;//list for address hash
Word_t *PValue;
Word_t Index;

void my_write(void *data, int len, void *location);
void my_write_literal(void *d_data, int len, void *location); // The d_data "pointer" is actually the data, don't dereference or else!
void *my_read(void *location);
void my_xfer();
void my_txend();
void my_finish();

static rt_mem_t glob_rt_mem = {
    .write = my_write,
    .write_literal = my_write_literal,
    .read = my_read,
    .do_transfer = my_xfer,
    .txend=my_txend,
    .finish=my_finish
    //.check_self = my_check_self
};

long hash_addr(long addr)
{
    //std::cout<<addr<<std::endl;
    return SIZE & (addr);
}

void internal_init()
{
    printf("initializing\n");
    if(!is_initialized)
    {
        int r = __sync_bool_compare_and_swap(&initializing, 0, 1);
        if(r)
        {
            //fake.ele.data=NULL;
            fake.ele.write_to=NULL;
            fake.ele.len=0;
            fake.st=-1;
            fake.cmt=0;

            tim.tv_sec = 0;
            tim.tv_nsec = 10;

            giter=cmtq.begin();
            cmtq.resize(SIZE);

            is_initialized = 1;
        }
        PJLArray= (Pvoid_t) NULL;
        memset(dirties,0,sizeof(dirties));
        //std::cout<<cmtq.size()<<std::endl;
    }
}
void my_txend()
{
    glob_rt_mem.curtxid++;
    return;
}
void my_xfer()
{
    int i, r,nowtx;
    long dirty_idx;
    write_t *to_write;
    buffer_t *now;
    Word_t Index1;

    Index1=0;
    JLF(PValue,PJLArray, Index1);
    //if (PValue!=NULL)
    //   printf("%p %d\n",*PValue,*(int *)(*PValue));
    while (PValue!=NULL)
    {
        now=(buffer_t *)*PValue;
        memcpy(now->ele.write_to,&(now->ele.data),now->ele.len);
        emulate_latency_ns_fence(1000);
        JLN(PValue,PJLArray, Index1);
    }

    giter=cmtq.begin();

    PJLArray= (Pvoid_t) NULL;
    //num_mutex.unlock();
    return;
}
void my_finish()
{
    my_xfer();
    cmtq.clear();
    return;
}
void my_write(void *data, int len, void *location)
{
  int wIdx, r;
  long dirty_idx;

  //wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1) & buffer.and_seed;

  buffer_t *temp;
  cmtq.push_back(fake);
  temp=&cmtq.back();
  //temp->ele.data = data;
  temp->ele.write_to = location;
  //temp.ele.len=len;
  temp->st=2;
  //ele->direct_val = 0;


  // For reads to know if this value hasn't been moved to NVRAM yet
  dirty_idx = hash_addr((long) location);
  r = __sync_fetch_and_add(&(dirties[dirty_idx]), 1);
  while(!r) {
    r = __sync_fetch_and_add(&(dirties[dirty_idx]), 1);
  }

	// Len is set last because it is used to determine if the data is ready to be moved from buffer to nvm
  temp->ele.len = len;
}

/*
 * data doesn't actually point to a value. Instead, the pointer of data is the actual value and continues for len.
 */

void my_write_literal(void *data, int len, void *location)
{
  int wIdx, r;
  long dirty_idx;

  if (giter==cmtq.end()) my_xfer();

  std::deque<buffer_t>::iterator it1=giter++;

  it1->ele.write_to=location;
  it1->txid=glob_rt_mem.curtxid;

  //temp->ele.data=data;
  memcpy(it1->ele.data, data, len); // Treat the void pointer as a literal value

  JLI(PValue,PJLArray,location);
  *PValue= &(*it1);

  it1->ele.len = len;

}

void *my_read(void *location)
{
    int dirty_idx = hash_addr((long) location);
    void *p;
    Word_t Index=location;


    //***********optimization of Judy***************
    JLL(PValue,PJLArray,Index);
    if (PValue!=NULL)
    {
        buffer_t *now=(buffer_t *)*PValue;
        return (void *)&(now->ele.data);
    }
    else
        return location;

    //****************

    //printf("Here\n");
    //while (dirties[dirty_idx]);
    //num_mutex.lock();
    // if(dirties[dirty_idx])
    // {
    //     //value is in the buffer
    //     //Wait for the data to transfer
    //     #ifdef OR
    //     //num_mutex.lock();
    //     if (cmtq.empty())
    //     {
    //         num_mutex.unlock();
    //         goto functionend;
    //     }
    //     std::deque<buffer_t>::iterator it=cmtq.end()-1;
    //     //printf("%p %p\n",it->ele.write_to,location);
    //     //printf("******************\n");
    //     while ((it->ele.write_to!=location)&&(it!=cmtq.begin()))
    //     {
    //         //printf("%d %d\n",cmtq.size(),cmtq.empty());
    //         //printf("%p %p\n",it->ele.write_to,location);
    //         it--;
    //     }
    //     if (it->ele.write_to!=location)
    //     {
    //         //std::cout<<"find it"<<std::endl;
    //         //num_mutex.unlock();
    //         return (void *) location;
    //     }
    //     //printf("%p %p\n",it->ele.write_to,location);
    //     //num_mutex.lock();
    //     //p=(void *)malloc(it->ele.len);
    //     //memcpy();
    //     //memcpy(p,&it->ele.data,it->ele.len);
    //     //num_mutex.unlock();
    //     //printf("done!\n");
    //     return &it->ele.data;
    //     #else
    //     //my_xfer();
    //     //znum_mutex.unlock();
    //     #endif
    // }

functionend:
    return (void *) location;
}

rt_mem_t *get_rt_mem()
{
    if(!is_initialized)
    {
        internal_init();
    }

    return &glob_rt_mem;
}
