#ifndef _MMP_USER_
#define _MMP_USER_

#include <stdio.h>
#include <stdlib.h>

#define WRITE_BUFFER_SIZE 131072

typedef struct rt_mem_t rt_mem_t;
typedef struct buffer_t buffer_t;
typedef struct write_t write_t;

struct rt_mem_t {
    void (*write)(void *data, int len, void *location);
    void (*write_literal)(void *d_data, int len, void *location); // The d_data "pointer" is actually the data, don't dereference or else!
    void *(*read)(void *location);
    void (*do_transfer)(void);
    void (*txend)(void);
    void (*finish)(void);

    //void (*check_self)(void);
    pthread_t th1;
    bool appfinish;
    int curtxid;
};

struct write_t {
  char data[300];
  void *write_to;
  int len;
    //int direct_val;
};

struct buffer_t
{
    //write_t elements[WRITE_BUFFER_SIZE];
    //int dirties[WRITE_BUFFER_SIZE];
    write_t ele;
    short st;
    int txid;
    bool cmt;
    //int read_idx;
    //int write_idx;
    //long and_seed;
};


//Functions
rt_mem_t *get_rt_mem(void);

#endif
