#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

typedef struct cline{
    // cache中的一行
    int valid;
    int tag;
    int LRUcounter;
}cline;
cline** cache;

typedef enum{ cache_miss, cache_hit, cache_eviction } cache_status;

char filename[1000];
char buff[1000];
int miss_count, hit_count, evict_count;
int s, E, b;
int counter = 1;
int has_info = 0;

void init();
void cache_free();

int find_lru(int set_index);
cache_status visit_cache(uint64_t address);
void write_cache(uint64_t address, int set_index, int line_index);
void load_data(uint64_t address);
void store_data(uint64_t address);
void modify_data(uint64_t address);


int main(int argc, char** argv) {
    int opt, ss, data_size;
    uint64_t address;
    FILE* fp;
    while(-1 != (opt = getopt(argc, argv, "s:E:b:t:v"))){
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
                ss = strlen(optarg);
                strncpy(filename, optarg, ss);
                break;
            case 'v':
                has_info = 1;
                break;
        }
    }
    init();
    fp = fopen(filename, "r");
    while(fgets(buff, 15, fp)){
        if(buff[0] != ' ') continue;
        sscanf(buff + 3, "%lx,%d", &address, &data_size);
        if(has_info) printf("%c %lx,%d ", buff[1], address, data_size);
        if(buff[1] == 'L'){
            load_data(address);
        }else if(buff[1] == 'S'){
            store_data(address);
        }else if(buff[1] == 'M'){
            modify_data(address);
        }
    }
    printSummary(hit_count ,miss_count, evict_count);
    cache_free();
    fclose(fp);
    return 0;
}

void init(){
    int S = pow(2, s);
    cache = (cline**)malloc(sizeof(cline*) * S); // 2^s组
    for(int i = 0; i < S; i++){
        cache[i] = (cline*)malloc(sizeof(cline) * E);
        for(int j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].LRUcounter = 0;
            cache[i][j].tag = 0;
        }
    }
}

void cache_free(){
    int S = pow(2, s);
    for(int i = 0; i < S; i++){
        free(cache[i]);
    }
    free(cache);
}

void write_cache(uint64_t address, int set_index, int line_index) {
    cache[set_index][line_index].valid = 1;
    cache[set_index][line_index].LRUcounter = counter++;
    cache[set_index][line_index].tag = address >> (b + s);
}

int find_lru(int set_index) {
    int min_count_index = -1, min_count = 1e9 + 7;
    for(int i = 0; i < E; i++){
        if(cache[set_index][i].LRUcounter < min_count) {
            min_count = cache[set_index][i].LRUcounter;
            min_count_index = i;
        }
    }
    return min_count_index;
}

cache_status visit_cache(uint64_t addr) {
    int set_index = 0, tag = 0, replace_index = -1;
    uint64_t address = addr >> b;
    tag = address >> s;
    set_index = address - (tag << s);
    for(int i = 0; i < E; i++) {
        if(cache[set_index][i].valid == 1 && cache[set_index][i].tag == tag){
            cache[set_index][i].LRUcounter = counter++;
            return cache_hit;
        }
        if(replace_index == -1 && cache[set_index][i].valid == 0){
            replace_index = i;
        }
    }
    if(replace_index != -1) {
        write_cache(addr, set_index, replace_index);
        return cache_miss;
    }else{
        replace_index = find_lru(set_index);
        write_cache(addr, set_index, replace_index);
        return cache_eviction;
    }
}

void load_data(uint64_t address){
    cache_status status = visit_cache(address);
    if(status == cache_hit){
        if(has_info) printf("hit\n");
        hit_count++;
    }else {
        miss_count++;
        if(has_info) printf("miss");
        if(status == cache_eviction) {
            evict_count++;
            if(has_info) printf(" eviction");
        }
        if(has_info) printf("\n");
    }
}

void store_data(uint64_t address){
    cache_status status = visit_cache(address);
    if(status == cache_hit) {
        if(has_info) printf("hit\n");
        hit_count++;
    }else {
        miss_count++;
        if(has_info) printf("miss");
        if(status == cache_eviction) {
            evict_count++;
            if(has_info) printf(" eviction");
        }
        if(has_info) printf("\n");
    }
}

void modify_data(uint64_t address){
    cache_status status = visit_cache(address);
    if(status == cache_hit) {
        if(has_info) printf("hit");
        hit_count++;
    }else {
        miss_count++;
        if(has_info) printf("miss");
        if(status == cache_eviction) {
            evict_count++;
            if(has_info) printf(" eviction");
        }
    }
    hit_count++;
    if(has_info) printf(" hit\n");
}