#include <stddef.h>
#include <stdbool.h>

void * ff_malloc(size_t size);
void * bf_malloc(size_t size);
void ff_free(void * ptr);
void bf_free(void * ptr);
//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);



//block
typedef struct BlockHeader block_t;
struct BlockHeader{
    bool isfree;
    size_t size;
    block_t * next_free;
    block_t * prev_free;
    block_t * next_phys;
    block_t * prev_phys;
};

unsigned long get_total_free_size(void);
unsigned long get_largest_free_data_segment_size(void);

static block_t * free_head = NULL;
static block_t * phys_tail = NULL;