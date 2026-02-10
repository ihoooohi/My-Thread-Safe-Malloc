#define _DEFAULT_SOURCE
#include "stddef.h"
#include "my_malloc.h"
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#define MIN_SPLIT 8

__thread block_t *tls_free_head = NULL;

//global lock
pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;
//just lock sbrk()
pthread_mutex_t sbrk_lock = PTHREAD_MUTEX_INITIALIZER;

//when free
void insert_free(block_t * b){
    b->isfree = true;
    
    b->prev_free = NULL;
    b->next_free = free_head;
    if (free_head != NULL) {
        free_head->prev_free = b;
    }
    free_head = b;
}

//when malloc or merge
void remove_free(block_t * b) {
    if (b->prev_free == NULL) {
        free_head = b->next_free;
    } else {
        b->prev_free->next_free = b->next_free;
    }

    if (b->next_free) b->next_free->prev_free = b->prev_free;

    b->prev_free = NULL;
    b->next_free = NULL;
}

//when sbrk()
void append_phys(block_t * b) {
    b->prev_phys = phys_tail;
    b->next_phys = NULL;
    if (phys_tail) phys_tail->next_phys = b;
    phys_tail = b;
}

//merge close free block
block_t * coalesce(block_t * b) {
    //merge left
    block_t * L = b->prev_phys;
    if (L && L->isfree) {
        remove_free(L);
        L->size += sizeof(block_t) + b->size;

        L->next_phys = b->next_phys;
        if (b->next_phys) b->next_phys->prev_phys = L;
        else phys_tail = L;

        b = L;
    }

    //merge right
    block_t * R = b->next_phys;
    if (R && R->isfree) {
        remove_free(R);
        b->size += sizeof(block_t) + R->size;

        b->next_phys = R->next_phys;
        if (R->next_phys) R->next_phys->prev_phys = b;
        else phys_tail = b;
    }

    return b;
}

void m_free(void *ptr){
    block_t * b = ((block_t*)ptr) - 1;

    b->isfree = true;
    b = coalesce(b);
    insert_free(b);
}

void bf_free(void *ptr) {
    m_free(ptr);   
}


block_t *find_best_fit(size_t req) {
    block_t *best = NULL;
    size_t best_size = 0;

    for (block_t *cur = free_head; cur != NULL; cur = cur->next_free) {
        if (!cur->isfree) continue;              
        if (cur->size < req) continue;          
        if (best == NULL || cur->size < best_size) {
            best = cur;
            best_size = cur->size;
            if (best_size == req) break;        
        }
    }
    return best;
}


size_t align_size(size_t n) {
    size_t a = sizeof(void*);
    return (n + a - 1) & ~(a - 1);
}


void split_block(block_t * b, size_t req) {
    if (b->size < req + sizeof(block_t) + MIN_SPLIT) return;

    block_t *newb = (block_t *)((char *)(b + 1) + req);
    newb->size = b->size - req - sizeof(block_t);
    newb->isfree = true;

    newb->prev_phys = b;
    newb->next_phys = b->next_phys;
    if (b->next_phys) b->next_phys->prev_phys = newb;
    else phys_tail = newb;
    b->next_phys = newb;
    b->size = req;

    newb->next_free = newb->prev_free = NULL;
    insert_free(newb);
}

void * bf_malloc(size_t size) {
    if (size == 0) return NULL;
    size_t req = align_size(size);

    block_t * b = find_best_fit(req);
    if (b != NULL) {
        remove_free(b);
        b->isfree = false;

        split_block(b, req);     
        return (void *)(b + 1);
    }

    // sbrk to extend when no free space
    void * p = sbrk((intptr_t)(sizeof(block_t) + req));
    if (p == (void *)-1) return NULL;

    b = (block_t *)p;
    b->size = req;
    b->isfree = false;
    b->next_free = b->prev_free = NULL;

    b->next_phys = b->prev_phys = NULL;
    append_phys(b);

    return (void *)(b + 1);
}


unsigned long get_total_free_size(void) {
    unsigned long total = 0;
    block_t *cur = free_head;

    while (cur != NULL) {
        if (cur->isfree) {
            total += (unsigned long)cur->size;
        }
        cur = cur->next_free;
    }
    return total;
}

unsigned long get_largest_free_data_segment_size(void) {
    unsigned long largest = 0;
    block_t *cur = free_head;
    while (cur != NULL) {
        if (cur->isfree && (unsigned long)cur->size > largest) {
            largest = (unsigned long)cur->size;
        }
        cur = cur->next_free;
    }
    return largest;
}

//=================================================================
//                      hw2 thread safe
//==================================================================

//-----------------------Version1 Lock Version-------------------------
void* ts_malloc_lock(size_t size){
    void* ret;
    pthread_mutex_lock(&malloc_lock);
    ret = bf_malloc(size);
    pthread_mutex_unlock(&malloc_lock);
    return ret;
}

void ts_free_lock(void *ptr){
    pthread_mutex_lock(&malloc_lock);
    m_free(ptr);
    pthread_mutex_unlock(&malloc_lock);   
}

//---------------------Version2 Nonlock Version-------------------------------

// Helper functions for TLS free list
void insert_free_tls(block_t *b) {
    b->isfree = true;
    b->prev_free = NULL;
    b->next_free = tls_free_head;
    
    if (tls_free_head != NULL) {
        tls_free_head->prev_free = b;
    }
    tls_free_head = b;
}

void remove_free_tls(block_t *b) {
    if (b->prev_free == NULL) {
        tls_free_head = b->next_free;
    } else {
        b->prev_free->next_free = b->next_free;
    }
    
    if (b->next_free) {
        b->next_free->prev_free = b->prev_free;
    }
    
    b->prev_free = NULL;
    b->next_free = NULL;
}

block_t *find_best_fit_tls(size_t req) {
    block_t *best = NULL;
    size_t best_size = 0;
    
    for (block_t *cur = tls_free_head; cur != NULL; cur = cur->next_free) {
        if (!cur->isfree) continue;
        if (cur->size < req) continue;
        
        if (best == NULL || cur->size < best_size) {
            best = cur;
            best_size = cur->size;
            if (best_size == req) break;
        }
    }
    return best;
}

// Split block (same logic, but use TLS free list)
void split_block_tls(block_t *b, size_t req) {
    if (b->size < req + sizeof(block_t) + MIN_SPLIT) return;
    
    block_t *newb = (block_t *)((char *)(b + 1) + req);
    newb->size = b->size - req - sizeof(block_t);
    newb->isfree = true;
    
    // Don't maintain physical list for simplicity (or you can maintain it)
    newb->prev_phys = NULL;
    newb->next_phys = NULL;
    
    b->size = req;
    
    newb->next_free = newb->prev_free = NULL;
    insert_free_tls(newb);
}
void * bf_malloc_tls(size_t size) {
    if (size == 0) return NULL;
    size_t req = align_size(size);
    
    // Step 1: Try to find a block in this thread's free list
    block_t *b = find_best_fit_tls(req);
    
    if (b != NULL) {
        // Found a free block
        remove_free_tls(b);
        b->isfree = false;
        split_block_tls(b, req);
        return (void *)(b + 1);
    }
    
    // Step 2: No free block, need to call sbrk (LOCK ONLY HERE)
    pthread_mutex_lock(&sbrk_lock);
    void *p = sbrk((intptr_t)(sizeof(block_t) + req));
    pthread_mutex_unlock(&sbrk_lock);
    
    if (p == (void *)-1) return NULL;
    
    // Step 3: Initialize the new block
    b = (block_t *)p;
    b->size = req;
    b->isfree = false;
    b->next_free = b->prev_free = NULL;
    b->next_phys = b->prev_phys = NULL;
    
    return (void *)(b + 1);
}

void *ts_malloc_nolock(size_t size){
    return bf_malloc_tls(size);
}

void ts_free_nolock(void *ptr){
     block_t * b = ((block_t*)ptr) - 1;

    b->isfree = true;
    insert_free_tls(b);
}




