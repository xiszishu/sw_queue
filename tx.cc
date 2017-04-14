#include <iostream>       // std::cin, std::cout
#include <queue>          // std::queue
#include <cstring>
#define WRITE_BUFFER_SIZE 20;

struct write_t
{
    void *data;
    void *write_to;
    int len;
    int direct_val;
};

struct buffer_t
{
    //write_t elements[WRITE_BUFFER_SIZE];
    //int dirties[WRITE_BUFFER_SIZE];
    write_t ele;
    short state; //available=0;commited=1;active=2;
    //int read_idx;
    //int write_idx;
    //long and_seed;
};

int main ()
{
    std::queue<buffer_t> myqueue;
    buffer_t myele,temp;
    int sum=10;
    int c=40,b=20;

    int a;
    myele.ele.data=(void *)&a;
    myele.ele.write_to=(void *)&a;
    myele.ele.len=sizeof(int);
    myele.state=2;


    memcpy(&b,&c,sizeof(int));
    std::cout<<b<<" "<<c<<std::endl;

    std::cout << "Please enter some integers (enter 0 to end):\n";

    do {
        //std::cin >> myint;
        myqueue.push (myele);
        sum--;
    } while (sum);

    std::cout << "myqueue contains: ";
    while (!myqueue.empty())
    {
        temp=myqueue.front();
        std::cout << ' ' << temp.ele.len<< std:: endl;
        myqueue.pop();
    }
    std::cout << '\n';

    return 0;
}
