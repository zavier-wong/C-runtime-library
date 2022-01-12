//--------------------------------------------------
//  Process Heap Manager
//
//  the basic c run time library of heap
//
//  use simple merge , alloc and search algorithm
//
//  note: It's search time complexity is O(n);
//
//--------------------------------------------------
#include "crt.h"

#ifdef WIN32
#include <Windows.h>
#endif

//
//  heap block header
//
typedef struct _heap_header{
    enum{
        BLOCK_FREE = 0x52013140,    //magic nmuber of free block
        BLOCK_USED = 0xacacacac     //magic number of used block
    }type;
    int size;                       //block size (include the head size)
    struct _heap_header *prev;      //next block
    struct _heap_header *next;      //prev block
} heap_header;

//heap block list
static heap_header* heap_list=NULL;

#define HEAD_SIZE (sizeof(heap_header))
#define addr_offset(addr,offset) ((char*)addr+offset)
#define CHUNKSIZE (1<<12)
#define ALIGN 8

static size_t round_up(size_t size) {
	return (((size) + ALIGN-1) & ~(ALIGN - 1));
}

#ifdef __linux
//brk system call
static void* brk(void *end_of_data){
    void* ret = 0;
#if __SIZEOF_POINTER__ == 4
    asm("push %eax    \n"
        "push %ebx    \n");
    asm("movl $45,%%eax \n"
        "movl %1,%%ebx  \n"
        "int $0x80      \n"
        "movl %%eax,%0  \n"
        :"=m"(ret)
        :"m"(end_of_data));
    asm("pop %ebx    \n"
        "pop %eax    \n");
    return ret;
#elif __SIZEOF_POINTER__ == 8
    asm("movq $12,%%rax \n"
        "movq %1,%%rdi  \n"
        "syscall        \n"
        "movq %%rax,%0  \n"
        :"=m"(ret)
        :"m"(end_of_data));
#endif
}
static void* sbrk(int increment){
    void *ret=brk(0);
    if(increment > 0){
        void *p=addr_offset(ret,increment);
        brk(p);
    }
    return ret;
}
#endif

static void* crt_extend_heap(heap_header* h, size_t size){
	size_t asize = round_up(size);
	void* base = NULL;
	heap_header* tmp = NULL;
#ifdef WIN32
    base = VirtualAlloc(0,asize,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
    if(base == NULL)
        return 0;
#else
    base = sbrk(asize);
    if(!base)
        return 0;
#endif
	h->next = (heap_header*)base;
	tmp = (heap_header*)base;
    tmp->type = BLOCK_FREE;
    tmp->size = asize;
    tmp->prev = h;
    tmp->next = NULL;
    return tmp;
}
//initialize the heap 
int crt_heap_init(){
    int heap_size = CHUNKSIZE;
    void* base = NULL;
#ifdef WIN32
    base = VirtualAlloc(0,heap_size,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
    if(base == NULL)
        return 1;
#else
    base = sbrk(heap_size);
    if(!base)
        return 1;
#endif
    heap_list = (heap_header*)base;
    heap_list->type = BLOCK_FREE;
    heap_list->size = heap_size;
    heap_list->prev = NULL;
    heap_list->next = NULL;
    return 0;
}

//When reclaiming memory, merge the free blocks before and after
void free(void *p){
    if(!p)
        return;
    heap_header* ph = (heap_header*)addr_offset(p,-HEAD_SIZE);
    if(ph->type != BLOCK_USED)
        return;
    ph->type = BLOCK_FREE;
    //ph->next->type;
    if(ph->next != NULL && ph->next->type == BLOCK_FREE){
        ph->size += ph->next->size;
        ph->next = ph->next->next;
    }
    if(ph->prev != NULL && ph->prev->type == BLOCK_FREE){
        ph = ph->prev;
        ph->size += ph->next->size;
        ph->next = ph->next->next;
    }
}

//If the request size is larger than the free block, the free block is cut
void* malloc(size_t size){
    if(size == 0)
        return NULL;
    size_t asize = round_up(size) + HEAD_SIZE;
    heap_header* ph = heap_list;
    while(ph) {
    	if(ph->next == NULL) {
			size_t extendsize = asize > CHUNKSIZE ? asize : CHUNKSIZE;
			ph = crt_extend_heap(ph, extendsize);
		}
		if(ph->type == BLOCK_USED){
			ph = ph->next;
            continue;
        }
        if(ph->size > asize && ph->size < asize + HEAD_SIZE ){
            ph->type = BLOCK_USED;
            return addr_offset(ph, HEAD_SIZE);
        } 
        if(ph->size > asize + HEAD_SIZE){
            ph->type=BLOCK_USED; 
            heap_header* tmp = addr_offset(ph, asize);
            tmp->type = BLOCK_FREE;
            tmp->next = ph->next;
            tmp->prev = ph;
            tmp->size = ph->size-asize;
            ph->next = tmp;
            ph->size = asize;
            return addr_offset(ph,HEAD_SIZE);
        }
        size_t extendsize = asize > CHUNKSIZE ? asize : CHUNKSIZE;
		ph = crt_extend_heap(ph, extendsize);
	}
    
    return NULL;
}
