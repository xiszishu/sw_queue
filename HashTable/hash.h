#define SIZE 100
#define LOGSIZE 1000000

struct DataItem
{
    char data[256];
    int key;
};

typedef struct DataItem DataItem;
typedef std::pair <DataItem*, DataItem> dual;
