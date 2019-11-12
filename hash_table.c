#include"pch.h"
#include"hash_table.h"

static unsigned int hashcode(char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash = hash * 33 + *key;
        key++;
    }
    return hash;
}

hash_table *new_hash_table(int init_capacity, unsigned int(*hash_func)(void *key),
    int(*cmp_func)(const void *k, const void *key), void(*free_key)(void *ptr), void(*free_value)(void *ptr)){
    float f = (float)(init_capacity - 1);
    init_capacity = 1 << ((*(unsigned int*)(&f) >> 23) - 126);
    if (init_capacity < 16) init_capacity = 16;
    hash_table *ht = (hash_table *)malloc(sizeof(hash_table));
    ht->table = (kv **)malloc(sizeof(kv *)*init_capacity);
    memset(ht->table, 0, sizeof(kv *)*init_capacity);
    ht->cnt = 0;
    ht->table_size = init_capacity;
    ht->hash_func = hash_func ? hash_func : hashcode;
    ht->cmp_func = cmp_func ? cmp_func : strcmp;
    ht->free_key = free_key ? free_key : free;
    ht->free_value = free_value ? free_value : free;
    return ht;
}

void extend(hash_table *ht) {
    ht->cnt++;
    if (ht->cnt*1.0 / ht->table_size > 0.75 && ht->table_size << 1 <= 1073741824) {//1073741824    1<<30
        int old_size = ht->table_size;
        ht->table_size <<= 1;
        ht->table = (kv **)realloc(ht->table, sizeof(kv *)*ht->table_size);
        memset(ht->table + old_size, 0, sizeof(kv *)*old_size);
        kv **table = ht->table;
        
        //move all kv to new table
        int i = 0;
        for (; i < old_size; i++) {
            if (table[i]) {
                kv *kv_list = table[i];
                kv *lowHead = NULL, *lowTail = NULL, *highHead = NULL, *highTail = NULL;
                do {//move the list
                    if (kv_list->hash & old_size) {//move
                        if (highHead == NULL) { highHead = kv_list; }
                        else { highTail->next = kv_list; }
                        highTail = kv_list;
                    }
                    else {//not move
                        if (lowHead == NULL) { lowHead = kv_list; }
                        else { lowTail->next = kv_list; }
                        lowTail = kv_list;
                    }
                } while (kv_list = kv_list->next);
                if(lowTail) lowTail->next = NULL;
                if (highTail) { 
                    highTail->next = NULL;
                    ht->table[i] = lowHead;
                    ht->table[i + old_size] = highHead; 
                }
            }
        }
    }
}

void *hash_table_put(hash_table *ht, void *key, void *val) {//return oldval when update else return NULL
    int hash = ht->hash_func(key);
    int h = hash & (ht->table_size - 1);
    kv *kv_list = ht->table[h];
    while (kv_list) {
        if (hash == kv_list->hash && ht->cmp_func(key, kv_list->key) == 0) {
            void *old = kv_list->val;
            kv_list->val = val;
            return old;
        }
        else kv_list = kv_list->next;
    }
    kv *new_kv = (kv *)malloc(sizeof(kv));
    new_kv->next = ht->table[h];
    new_kv->key = key;
    new_kv->val = val;
    new_kv->hash = hash;
    ht->table[h] = new_kv;
    extend(ht);
    return NULL;
}

void *hash_table_get(hash_table *ht, void *key) {
    int hash = ht->hash_func(key);
    int h = hash & (ht->table_size - 1);
    kv *kv_list = ht->table[h];
    while (kv_list) {
        if (hash == kv_list->hash && ht->cmp_func(kv_list->key, key) == 0)return kv_list->val;
        else kv_list = kv_list->next;
    }
    return NULL;
}

void *hash_table_remove(hash_table *ht, void *key) {// free key in kv
    int hash = ht->hash_func(key);
    int h = hash & (ht->table_size - 1);
    kv *kv_list = ht->table[h];
    if (kv_list) {
        kv *obj = NULL;
        if (hash == kv_list->hash && ht->cmp_func(key, kv_list->key) == 0) { obj = kv_list; ht->table[h] = kv_list->next; }
        else while (kv_list->next) {
            if (hash == kv_list->next->hash && ht->cmp_func(key, kv_list->next->key) == 0) {
                obj = kv_list->next;
                kv_list->next = obj->next;
                break;
            }
            kv_list = kv_list->next;
        }
        if (obj) {
            ht->free_key(obj->key);
            void *val = obj->val;
            free(obj);
            ht->cnt--;
            return val;
        }
    }
    return NULL;
}

void hash_table_free(hash_table *ht) {//free key and value
    int cnt = ht->cnt, i = 0;
    kv *del = NULL;
    void (*ht_free_key)() = ht->free_key;
    void (*ht_free_val)() = ht->free_value;
    while (cnt) {
        while (ht->table[i]) {
            del = ht->table[i];
            ht->table[i] = del->next;
            ht_free_key(del->key);
            ht->free_value(del->val);
            free(del);
            cnt--;
        }
        i++;
    }
    free(ht->table);
    free(ht);
}