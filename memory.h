#ifndef __MEMORY_H
#define __MEMORY_H

void *MEM_malloc(int size);
void MEM_free(void * ptr);
void *MEM_realloc(void *ptr, int new_size);

#endif