#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include "../mmp_user.h"
#include "../mmp_init.h"
#include "../lat.h"

using namespace std;

//typedef mystring ch[256];
typedef pair <int, char *> Int_Pair;


double timer_begin,timer_end,sum;
rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

int build_array(vector<char *>& a, int n)
{
    char ch[256]={};
    //ch[21]='\0';
    srand(time(NULL));
    //memset(c);
    for (int i = 0; i < n; i++)
    {
        for (int j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        a[i]=malloc(256);
        strcpy(a[i],ch);
        //a[i]=rand();
        //cout<<sizeof(a[i])<<endl;
        //cout<<a[i]<<endl;
        //cout<<"****************"<<endl;
    }
    return 0;
}

void array_swap(vector<char *>& a, int n, int i)
{
  //mcsim_skip_instrs_begin();
  int  k1, k2;
  //mystring temp;
  char *temp,*a1,*a2;
  //string a1,a2;
  long b[1000];

  srand(time(NULL)+i*i);
  k1 = rand() % n;
  k2 = rand() % n;

  a1 = ((char *) rt_mem->read(a[k1]));
  a2 = ((char *) rt_mem->read(a[k2]));
  //rt_mem->write_literal((void *) ((long) b[k2]), sizeof(long), &b[k1]);
  //rt_mem->write_literal((void *) ((long) b[k1]), sizeof(long), &b[k2]);

  //rt_mem->txend();

  rt_mem->write_literal(a2, 256, a[k1]);
  //a[k1] = a[k2];
  rt_mem->write_literal(a1, 256, a[k2]);
  //a[k2] = temp;
}

void print_array(vector<char *>& a, int n, ofstream& file)
{
  int i;

  for (i = 0; i < n; i++)
    file << a[i] << endl;
}

int main(int argc, char **argv)
{
    int i,swaps,item_count;
    std::ifstream file1;
    file1.open("sps.txt");
    file1>>item_count>>swaps;
    vector<char *> array(item_count);

    // Initialization: build an array with random intergers
    if (build_array(array, item_count))
    {
        cerr << "Fails to build an array" << endl;
        return -1;
    }

    //mcsim_skip_instrs_end();

#ifdef SPS_DEBUG
    ofstream orig;
    orig.open("orig.debug");
    print_array(array, item_count, orig);
#endif

    sum=0;
    int batch=1;
    for (int k=1;k<=batch;k++)
    {
        timer_begin=GetWallTime();
        // randomly swaps between entries
        for (i = 0; i < swaps; i++)
        {
            array_swap(array, item_count, i); // swap two entris at a time
            //printf("%d\n",i);
            //mcsim_mem_fence();  //clflush+fence
        }
        //printf("finish swaps\n");

        timer_end=GetWallTime();
        sum+=timer_end-timer_begin;

        //mcsim_skip_instrs_begin();
        //cout << "done swaps " << i << endl;

        // make sure log structures are not dummy, will not discard by compile
        //cout << "dummy: undolog.size= " << undolog.size() << endl;
        //cout << "dummy: redolog.size= " << redolog.size() << endl;
    }
    rt_mem->finish;

    printf("%.15lf\n",sum/batch);
    cout << "done swaps " << i << endl;

    for (int i=0;i<item_count;i++)
        free(array[i]);
#ifdef SPS_DEBUG
    ofstream now;
    now.open("now.debug");
    print_array(array, item_count, now);
#endif

    return 0;
    //mcsim_skip_instrs_end();
}
