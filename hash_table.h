#pragma once
#include"pch.h"

typedef struct _kv{
    char *key;
    void *val;
    unsigned int hash;
    struct _kv *next;
}kv;

typedef struct {
    kv* *table;
    int cnt;
    int table_size;
}hash_table;

hash_table * new_hash_table(int init_capacity);

void * hash_table_put(hash_table * ht, char * key, void * val);

void * hash_table_get(hash_table * ht, char * key);

void * hash_table_remove(hash_table * ht, char * key);

void hash_table_free(hash_table * ht);