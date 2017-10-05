#include "../mmp_user.h"
#include "../mmp_init.h"
#include "../lat.h"
#include "../flush.h"

#define SIZE 100
#define LOGSIZE 1000000

struct DataItem
{
    char data[256];
    int key;
};

typedef struct DataItem DataItem;
