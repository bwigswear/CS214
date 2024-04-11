#ifndef MYMALLOC_H_
#define MYMALLOC_H_

void myinit(int allocAlg);
void* mymalloc(size_t size);
void myfree(void* ptr);
void* myrealloc(void* ptr, size_t size);
void mycleanup();

#endif