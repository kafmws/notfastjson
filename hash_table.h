#pragma once
#include"pch.h"

typedef struct _kv {
    void *key;
    void *val;
    unsigned int hash;
    struct _kv *next;
}kv;

typedef struct {
    kv* *table;
    unsigned int(*hash_func)(void *key);
    int (*cmp_func)(const void *k, const void *key);
    void (*free_key)(void *ptr);
    void (*free_value)(void *ptr);
    int cnt;
    int table_size;
}hash_table;

hash_table *new_hash_table(int init_capacity, unsigned int(*hash_func)(void *key), 
    int (*cmp_func)(const void *k, const void *key), void (*free_key)(void *ptr), void (*free_value)(void *ptr));

void * hash_table_put(hash_table * ht, void * key, void * val);

void * hash_table_get(hash_table * ht, void * key);

void * hash_table_remove(hash_table * ht, void * key);//free key in kv

void hash_table_free(hash_table * ht);//free key and value