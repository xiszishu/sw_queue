#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <deque>
#include <iostream>
#define SIZE 10000
#include "mmp_user.h"
#include <mutex>

static int is_initialized = 0, initializing = 0, transferring = 0;
//static buffer_t buffer;
static std::deque<buffer_t> cmtq;
static bool dirties[SIZE];
static buffer_t fake;
struct timespec tim, tim2;
static std::mutex num_mutex;

void my_write(void *data, int len, void *location);
void my_write_literal(void *d_data, int len, void *location); // The d_data "pointer" is actually the data, don't dereference or else!
void *my_read(void *location);
void my_xfer();
void my_txend();

static rt_mem_t glob_rt_mem = {
    .write = my_write,
    .write_literal = my_write_literal,
    .read = my_read,
    .do_transfer = my_xfer,
    .txend=my_txend
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
            //This thread updated initialization var, so continue with idxs
            //buffer.read_idx = 0;
            //buffer.write_idx = 0;
            //buffer.and_seed = WRITE_BUFFER_SIZE - 1;
            //initializxe the fake
            fake.ele.data=NULL;
            fake.ele.write_to=NULL;
            fake.ele.len=0;
            fake.st=-1;
            fake.cmt=0;

            tim.tv_sec = 0;
            tim.tv_nsec = 10;

            is_initialized = 1;

        }
        //std::cout<<cmtq.size()<<std::endl;
    }
}
void my_txend()
{
    glob_rt_mem.curtxid++;
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
  temp->ele.data = data;
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
  //buffer_t *temp;

  //wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1) & buffer.and_seed;
  //wIdx = __sync_fetch_and_add(&(buffer.write_idx), 1);
  //if(wIdx >= WRITE_BUFFER_SIZE) {
  //  wIdx = wIdx & buffer.and_seed;
 // }
  //std::cout<<cmtq.size()<<std::endl;
  while (cmtq.size()>=SIZE);

  num_mutex.lock();
  cmtq.push_back(fake);
  num_mutex.unlock();
  std::deque<buffer_t>::iterator it=cmtq.end()-1;

  //temp=&cmtq.back();
  //write_t *ele = &buffer.elements[wIdx];
  //ele->write_to = location;
  //ele->direct_val = 1;
  it->ele.write_to=location;
  it->txid=glob_rt_mem.curtxid;
  //temp->ele.data=data;
  memcpy(&(it->ele.data), &data, len); // Treat the void pointer as a literal value

  // For reads to know if this value hasn't been moved to NVRAM yet
  dirty_idx = hash_addr((long) location);
  r = __sync_fetch_and_add(&(dirties[dirty_idx]), 1);
  while(!r)
  {
      r = __sync_fetch_and_add(&(dirties[dirty_idx]), 1);
  }
  //if (len==-1)
  //std::cout<<"invalid"<<std::endl;

	// Len is set last because it is used to determine if the data is ready to be moved from buffer to nvm
  //std::cout<<cmtq.size()<<" "<<len<<std::endl;
  //std::cout<<"add"<<cmtq.size()<<std::endl;
  //std::cout<<"add"<<cmtq.front().ele.len<<std::endl;
  //std::cout<<"**********"<<std::endl;
  it->ele.len = len;
}

void *my_read(void *location)
{
    int dirty_idx = hash_addr((long) location);
    if(dirties[dirty_idx])
    {
        //value is in the buffer
        //Wait for the data to transfer
        my_xfer();
    }

    return (void *) location;
}

// void my_check_self()
// {

// 	//  If the buffer indexes have surpassed the buffer size, do a quick fix to put it back
// 	if(buffer.write_idx >= WRITE_BUFFER_SIZE) {
// 		__sync_fetch_and_and(&(buffer.write_idx), buffer.and_seed);
// 	}
// }

// inline void inline_check_self() {

// 	//  If the buffer indexes have surpassed the buffer size, do a quick fix to put it back
// 	if(buffer.write_idx >= WRITE_BUFFER_SIZE) {
// 		__sync_fetch_and_and(&(buffer.write_idx), buffer.and_seed);
// 	}
// }

void my_xfer()
{
    int i, r,nowtx;
    long dirty_idx;
    write_t *to_write;
    buffer_t *beg;
    std::deque<buffer_t>::iterator it=cmtq.begin();

    if(transferring)
    {
        //Another thread is already transferring, just wait for it to finish and return
        goto wait_for_finish;
    }

    r = __sync_bool_compare_and_swap(&transferring, 0, 1);
    //printf("this r:%d\n",r);
    if(!r)
    {
        //Another thread is already transferring, just wait for it to finish and return
        goto wait_for_finish;
    }

    //std::cout<<"transfer:"<<cmtq.size()<<" "<<cmtq.empty()<<std::endl;
    //std::cout<<it->txid<<" "<<it->ele.len<<" "<<glob_rt_mem.curtxid<<std::endl;
    //if (!cmtq.empty()) return;

    //if (cmtq.empty()) return;
    //std::cout<<"transfer:"<<cmtq.size()<<" "<<cmtq.empty()<<std::endl;
    //std::cout<<it->txid<<" "<<it->ele.len<<" "<<glob_rt_mem.curtxid<<std::endl;

    // it=cmtq.begin();
    // num_mutex.lock();
    // while (it!=cmtq.end())
    // {
    //     std::cout<<"TXID:"<<it->txid<<std::endl;
    //     it++;
    // }
    // if (it==cmtq.end()) return;
    // num_mutex.unlock();

    it=cmtq.begin();
    //std::cout<<"here:"<<it->txid<<" "<<glob_rt_mem.curtxid<<" "<<(long) it->ele.data<<std::endl;
    //if (it->txid < glob_rt_mem.curtxid) printf("what the fuck!\n");
    //num_mutex.lock();
    nowtx=glob_rt_mem.curtxid;
    while ((it!=cmtq.end())&&(it->txid < nowtx))
    //while (!cmtq.empty())
    {
        //beg=&cmtq.front();
        //if (it->txid < 0)
        //{
        //    printf("txid:%d %p\n",it->txid,it);
        //    break;
        //}
        //it=cmtq.begin();
        while (it->ele.len==-1)
        {
            //std::cout<<"stuck"<<cmtq.size()<<std::endl;
            //std::cout<<"stuck"<<cmtq.front().ele.len<<std::endl;
            nanosleep(&tim, &tim2);
        }
        //if (beg->ele.direcet_val==1)
        //{
        //    memcpy(beg->ele.write_to, &(beg->ele.data), beg->ele.len);
        //}
        //else
        //{
        //std::cout<<cmtq.size()<<std::endl;
        //printf("%p %d %d\n",beg->ele.write_to,beg->ele.data,beg->ele.len);
        memcpy(it->ele.write_to, &(it->ele.data), it->ele.len);
        //}
        dirty_idx = hash_addr((long) it->ele.write_to);
        r = __sync_fetch_and_sub(&(dirties[dirty_idx]), 1);

        it->cmt=1;
        //printf("");
        //std::cout<<"here:"<<it->txid<<" "<<glob_rt_mem.curtxid<<" "<<cmtq.size()<<" "<<nowtx<<std::endl;
        //std::cout<<"number:"<<(long)(it->ele.data)<<std::endl;
        //std::cout<<"segfault"<<std::endl;
        //num_mutex.lock();
        ++it;
        //num_mutex.unlock();
        //if (it==cmtq.end()) break;
        //goto outofloop;
        //if (it->txid<0) printf("id:%d\n",it->txid);
        //num_mutex.lock();
        //std::cout<<cmtq.size()<<std::endl;
        //cmtq.pop_front();
        //num_mutex.unlock();
        //std::cout<<"size"<<cmtq.size()<<std::endl;
    }
    //num_mutex.unlock();

 outofloop:
    num_mutex.lock();
   while ((!cmtq.empty())&&(cmtq.begin()->cmt))
   {
       cmtq.pop_front();
       //std::cout<<"pop"<<std::endl;
   }
   num_mutex.unlock();
  // while(buffer.read_idx != (buffer.write_idx & buffer.and_seed))
  // {
  //   i = buffer.read_idx;

  //   to_write = &(buffer.elements[i]);

  //   // Len is written to last and used here as a "ready" flag
	// 	while(to_write->len == 0)
  //   {
  //     //if(buffer.read_idx == buffer.write_idx & buffer.and_seed) {
  //       //goto xfer_quit;
  //     //}
	// 		//	Write index has been moved up, but the data isn't ready yet. Sleep a tad to give the other thread time
	// 		nanosleep(&tim, &tim2); // 10 nanosecs?
	// 	}

  //   if(to_write->direct_val == 1) {
	// 		//	Treat the value stored in the pointer as literal
  //     memcpy(to_write->write_to, &(to_write->data), to_write->len);
  //   } else {
	// 		// Not literal, do a copy like the data is an actual pointer
  //     memcpy(to_write->write_to, to_write->data, to_write->len);
  //   }

	// 	//	Cleanup
	// 	to_write->direct_val = 0;
	// 	to_write->len = 0;
  //   dirty_idx = hash_addr((long) to_write->write_to);
  //   r = __sync_fetch_and_sub(&(buffer.dirties[dirty_idx]), 1);

  //   buffer.read_idx = buffer.read_idx + 1;
  //   if(buffer.read_idx >= WRITE_BUFFER_SIZE) {
  //     buffer.read_idx = buffer.read_idx & buffer.and_seed;
  //   }
  // }

xfer_quit:
  // no need for atomic here because there is only 1 thread doing this at a time
  transferring = 0;
  //__sync_bool_compare_and_swap(&transferring, 1, 0);
  return;

wait_for_finish:
  while(transferring) {
    nanosleep(&tim, &tim2);
  }
  return;
}

rt_mem_t *get_rt_mem()
{
    if(!is_initialized)
    {
        internal_init();
    }

    return &glob_rt_mem;
}
