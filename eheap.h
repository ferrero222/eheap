/******************************************************************************* 
* @Ferrero                  ╔═══╦╗─╔╦═══╦═══╦═══╗               (c) 15.09.2025 *
*                           ║╔══╣║─║║╔══╣╔═╗║╔═╗║                     v1.0.0   *
*                           ║╚══╣╚═╝║╚══╣║─║║╚═╝║                              *
*                           ║╔══╣╔═╗║╔══╣╚═╝║╔══╝                              *
*                           ║╚══╣║─║║╚══╣╔═╗║║                                 *
*                           ╚═══╩╝─╚╩═══╩╝─╚╩╝                                 *
*******************************************************************************/
#ifndef __EHEAP_H
#define __EHEAP_H

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdbool.h>

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EHEAP_SIZE         2048
#define EHEAP_ALIGNMENT    8

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
typedef struct {
  size_t total_allocations;
  size_t total_frees;
  size_t alloc_failures;
  size_t peak_usage;
  size_t current_usage;
  size_t fragmentation;
  size_t largest_free_block;
} eheap_stats_t;

typedef struct eheap_free_block_t {
  size_t size;                       // block size including header
  struct eheap_free_block_t* next;   // ptr to next free block
} eheap_free_block_t;

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
void eheap_init(void);
void* eheap_alloc(size_t size);
void* eheap_calloc(size_t num, size_t size);
void* eheap_realloc(void* ptr, size_t new_size);
void eheap_free(void* ptr);
void eheap_get_stats(eheap_stats_t* stats);
size_t eheap_get_usage_percent(void);
bool eheap_validate(void);
void eheap_reset_stats(void);
bool eheap_validate_ptr(void* ptr);

#endif //__EHEAP_H
