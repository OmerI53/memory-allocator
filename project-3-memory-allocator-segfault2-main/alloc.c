#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define USED 1
#define NOT_USED 0
#define BATCH_SIZE 32

// Data Structures
typedef struct mem_block {
    size_t size;
    int is_used;
    void *address;
    struct mem_block *next;
    size_t batch_size;
} mem_block;

static mem_block *mem_list = NULL;

/*
Search the allocated memory list
*/
void merge_memory(mem_block *block){
    size_t avaible_size = 0;
    size_t batch_size = block->batch_size;
    mem_block *current = block;
    while (current != NULL && current->is_used == NOT_USED && current->batch_size == batch_size)
    {
        avaible_size += current->size;
        current = current->next;
    }
    block->size = avaible_size;
}


void split_memory(size_t size, mem_block *block){
    //enough space to put a new memory block, multiple of long
    if(sizeof(mem_block) + 8 <= (block->size - size)){
        mem_block *new_block = block->address + size; 
        
        new_block->next = block->next;
        block->next = new_block;

        new_block->is_used = NOT_USED;
        new_block->size = block->size - size - sizeof(mem_block);
        new_block->address = block->address + size + sizeof(mem_block);
        new_block->batch_size = size;
        merge_memory(block);
    }else{
        return;
    }


}

void *find_address(size_t size){

    mem_block *current = mem_list;
    while (current != NULL)
    {
        if(current->is_used == NOT_USED && size == current->batch_size && current->size >= size){
            current->is_used = USED;
            split_memory(size,current);
            return current->address;
        }
        current = current->next;
    }
    return NULL;
}

mem_block *find_block(void *ptr){
    mem_block *current = mem_list;
    while (current != NULL)
    {
        if(current->address == ptr){
            return current; 
        }
        current = current->next;
    }

    return NULL;
}

/*
Manipultate allocated memory list
*/
mem_block* add_to_mem_list(size_t size, void* address){

    if(mem_list == NULL){
        mem_block *new_block = address;
        new_block->size = size;
        new_block->is_used = USED;
        new_block->next = NULL;
        new_block->address = (char*)address + sizeof(mem_block);
        new_block->batch_size = size;

        mem_list = new_block;
        return new_block;
    }else{
        mem_block *current = mem_list;
        mem_block *prev = mem_list;

        while (current != NULL)
        {
            prev = current;
            current = current->next;
        }
        
        mem_block *new_block = address;
        new_block->size = size;
        new_block->is_used = USED;
        new_block->next = NULL;
        new_block->address = (char*)address + sizeof(mem_block);
        new_block->batch_size = size;

        prev->next = new_block;
        return new_block;
    }
}

mem_block* batch_allocate(size_t size){
    
    size_t inc_size = size*BATCH_SIZE + BATCH_SIZE*sizeof(mem_block);
    
    void *address = sbrk(inc_size);
    mem_block *mem_info = add_to_mem_list(size, address);
    
    size_t batch_size = (BATCH_SIZE-1)*(size+ sizeof(mem_block));
    void *batch_address = (char*)address + size + sizeof(mem_block); 
    mem_block *batch_block = add_to_mem_list(batch_size,batch_address);
    batch_block->is_used = NOT_USED;
    batch_block->batch_size = size;

    return mem_info;
}



void *kumalloc(size_t size)
{   

    if(size == 0){
        return NULL;
    }

    int alignment = size%8;
    if(alignment != 0){
        size = ((size/8)+1)*8;
    }
    //printf("%zu-",size);
    void *existing = find_address(size);
    if(existing != NULL){
        return existing;
    }

    mem_block *mem_info = batch_allocate(size);

    return mem_info->address;

}

void *kucalloc(size_t nmemb, size_t size)
{
    if(size == 0 || nmemb == 0){
        return NULL;
    }
    void *mem = kumalloc(nmemb*size);
    memset(mem,0,nmemb*size);
    return mem;

}

void kufree(void *ptr)
{
    mem_block *ptr_block = find_block(ptr);
    if(ptr_block == NULL){
        return;
    }
    ptr_block->is_used = NOT_USED;
    merge_memory(ptr_block);
    
}

void *kurealloc(void *ptr, size_t size)
{
    if (size == 0 && ptr != NULL) {
        kufree(ptr);
        return NULL;
    } else if (ptr == NULL) {
        void *mem = kumalloc(size);
        return mem;
    } else {
        mem_block *old_block = find_block(ptr);
        if (old_block->size >= size) {
            // If the new size is smaller or equal, return the original pointer
            return ptr;
        } else {
            void *newlocation = kumalloc(size);
            if (newlocation != NULL) {
                memcpy(newlocation, ptr, old_block->size);
                kufree(ptr);
                return newlocation;
            }
        }
    }
    
    return NULL;
}

/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */
#if 0
void *malloc(size_t size) { return kumalloc(size); }
void *calloc(size_t nmemb, size_t size) { return kucalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return kurealloc(ptr, size); }
void free(void *ptr) { kufree(ptr); }
#endif
