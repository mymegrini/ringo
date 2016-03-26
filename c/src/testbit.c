#include <stdio.h>

int main(int argc, char *argv[])
{
    struct f {
        char ip[4];
        unsigned short port : 14;
        unsigned short id : 16;
    }f = {
        .ip = { 'A', 'B', 'C', 'D'},
        .port = 4242,
        .id = 48
    };
    
    printf("%u", sizeof(f));
    return 0;
}

