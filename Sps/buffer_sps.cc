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
#include "sps.h"
//#include "defines.h"

using namespace std;

//typedef string ch[256];
typedef pair <int, char*> Int_Pair;
vector<char *> ay;

double timer_begin,timer_end,sum;
rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

int build_array(vector<char *>& a, int n)
{
    int i,j;
    char ch[256]={};
    //ch[21]='\0';
    srand(time(NULL));
    //memset(c);
    for (i = 0; i < n; i++)
    {
        for (j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        a[i]=malloc(256);
        strcpy(a[i],ch);
        //a[i]=ch;
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
  //string temp;
  // string temp,a1,a2;
  //char a1[256],a2[256];
  char *a1,*a2;
  //string a1,a2;
  long b[1000];

  srand(time(NULL)+i*i);
  k1 = rand() % n;
  k2 = rand() % n;

  a1 = rt_mem->read((void *)a[k1]);
  a2 = rt_mem->read((void *)a[k2]);

  rt_mem->write_literal(a2, 255, (void *)a[k1]);
  //a[k1] = a[k2];
  rt_mem->write_literal(a1, 255, (void *)a[k2]);
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
    file1.close();
    ay.resize(item_count);
    //vector<string> array(item_count);
    //map<int, long> undolog, redolog;

  // Initialization: build an array with random intergers
  if (build_array(ay, item_count)) {
    cerr << "Fails to build an array" << endl;
    return -1;
  }

  //mcsim_skip_instrs_end();

#ifdef SPS_DEBUG
  ofstream orig;
  orig.open("orig.debug");
  print_array(ay, item_count, orig);
#endif

  sum=0;
  int batch=100;
  for (int k=1;k<=batch;k++)
  {
      timer_begin=GetWallTime();
      for (i = 0; i < swaps; i++)
      {
          array_swap(ay,  item_count, i); // swap two entris at a time
      }
      timer_end=GetWallTime();
      sum+=timer_end-timer_begin;
  }
  //rt_mem->appfinish=1;
  //rt_mem->do_transfer();
  rt_mem->finish();
  //pthread_join(rt_mem->th1, NULL);
  for (i=0;i<item_count;i++) free(ay[i]);
  printf("%.15lf\n",sum/batch);
  cout << "done swaps " << i << endl;
#ifdef SPS_DEBUG
  ofstream now;
  now.open("now.debug");
  print_array(ay, item_count, now);
#endif
  return 0;
  //mcsim_skip_instrs_end();
}
