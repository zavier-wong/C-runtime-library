//-----------------------------------------------
//  This is to support c++ new/delete operator
//
//-----------------------------------------------

#ifdef __linux
#if (__SIZEOF_POINTER__ == 4)
typedef unsigned int size_t;
#else
typedef unsigned long int size_t;
#endif
#endif

extern "C" void* malloc(unsigned long);
extern "C" void free(void* );

void* operator new(size_t n){
    return malloc(n);
}

void operator delete(void* p){
    free(p);
}

void operator delete(void* p,size_t n){
    operator delete(p);
}

void* operator new[](size_t n){
    return malloc(n);
}

void operator delete[](void* p){
    free(p);
}

void operator delete[](void* p,size_t n){
    operator delete[](p);
}
