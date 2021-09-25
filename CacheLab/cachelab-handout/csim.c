#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    int s, E, b, opt;
    char* file;
    printf("%s\n", argv[argc - 1]);
    while(-1 != (opt = getopt(argc, argv, "s:E:b:t:"))){
        switch(opt){
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                printf("%s", optarg);
                file = optarg;
                break;
        }
    }
    printf("%d %d %d %s\n", s, E, b, file);
    return 0;
}
