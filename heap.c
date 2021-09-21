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

#define HEAD_SIZE (sizeof(heap_header))
#define addr_offset(addr,offset) ((char*)addr+offset)

//heap block list
static heap_header* heap_list=NULL;

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

//initialize the heap 
int crt_heap_init(){
    //100MB heap size
    int heap_size=1024 * 1024 * 100;
    void* base=NULL;
#ifdef WIN32
    base=VirtualAlloc(0,heap_size,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
    if(base == NULL)
        return 1;
#else
    base=sbrk(heap_size);
    if(!base)
        return 1;
#endif
    heap_list=(heap_header*)base;
    heap_list->type=BLOCK_FREE;
    heap_list->size=heap_size;
    heap_list->prev=NULL;
    heap_list->next=NULL;
    return 0;
}

//When reclaiming memory, merge the free blocks before and after
void free(void *p){
    if(!p)
        return;
    heap_header* ph=(heap_header*)addr_offset(p,-HEAD_SIZE);
    if(ph->type!=BLOCK_USED)
        return;
    ph->type=BLOCK_FREE;
    ph->next->type;
    if(ph->next != NULL && ph->next->type == BLOCK_FREE){
        ph->size+=ph->next->size;
        ph->next=ph->next->next;
    }
    if(ph->prev != NULL && ph->prev->type == BLOCK_FREE){
        ph=ph->prev;
        ph->size+=ph->next->size;
        ph->next=ph->next->next;
    }
}

//If the request size is larger than the free block, the free block is cut
void* malloc(size_t size){
    if(size==0)
        return NULL;
    heap_header* ph=heap_list;
    for(;ph;ph=ph->next){
        if(ph->type == BLOCK_USED){
            continue;
        }
        if(ph->size > size+HEAD_SIZE && ph->size < size+HEAD_SIZE*2 ){
            ph->type=BLOCK_USED;
            return addr_offset(ph,HEAD_SIZE);
        } 
        if(ph->size > size+HEAD_SIZE*2){
            ph->type=BLOCK_USED; 
            heap_header* tmp=addr_offset(ph,HEAD_SIZE+size);
            tmp->type=BLOCK_FREE;
            tmp->next=ph->next;
            tmp->prev=ph;
            tmp->size=ph->size-size-HEAD_SIZE;
            ph->next=tmp;
            ph->size=HEAD_SIZE+size;
            return addr_offset(ph,HEAD_SIZE);
        }
    }
    return NULL;
}
