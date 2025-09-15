/******************************************************************************* 
* @Ferrero                  ╔═══╦╗─╔╦═══╦═══╦═══╗               (c) 15.09.2025 *
*                           ║╔══╣║─║║╔══╣╔═╗║╔═╗║                     v1.0.0   *
*                           ║╚══╣╚═╝║╚══╣║─║║╚═╝║                              *
*                           ║╔══╣╔═╗║╔══╣╚═╝║╔══╝                              *
*                           ║╚══╣║─║║╚══╣╔═╗║║                                 *
*                           ╚═══╩╝─╚╩═══╩╝─╚╩╝                                 *
*******************************************************************************/
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "eheap.h"


/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
/*******************************************************************************
 * Local types definitions
 ******************************************************************************/
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint8_t eheap[EHEAP_SIZE] = {0};
static eheap_free_block_t* eheap_free_list = NULL;
static eheap_stats_t eheap_stats = {0};
//static eheap_mutex_t eheap_mutex;       // Platform-specific mutex

/*******************************************************************************
 * Local function prototypes
 ******************************************************************************/
static void eheap_lock(void);
static void eheap_unlock(void);
static void eheap_update_stats(void);
bool eheap_validate_ptr(void* ptr);
static void eheap_defragment(void);

/*******************************************************************************
 * Function implementation
 ******************************************************************************/
/*******************************************************************************
 ** \brief  Initialize heap mutex (platform specific)
 ** \param  None
 ** \retval None
 ******************************************************************************/
static void eheap_init_mutex(void)
{
  // Platform-specific mutex initialization
  // Example: pthread_mutex_init(&eheap_mutex, NULL);
  // Or: InitializeCriticalSection(&eheap_mutex);
}

/*******************************************************************************
 ** \brief  Lock heap mutex
 ** \param  None
 ** \retval None
 ******************************************************************************/
static void eheap_lock(void)
{
  // Platform-specific mutex lock
  // pthread_mutex_lock(&eheap_mutex);
  // Or: EnterCriticalSection(&eheap_mutex);
}

/*******************************************************************************
 ** \brief  Unlock heap mutex
 ** \param  None
 ** \retval None
 ******************************************************************************/
static void eheap_unlock(void)
{
  // Platform-specific mutex unlock
  // pthread_mutex_unlock(&eheap_mutex);
  // Or: LeaveCriticalSection(&eheap_mutex);
}

/*******************************************************************************
 ** \brief  Align size to EHEAP_ALIGNMENT
 ** \param  None
 ** \retval None
 ******************************************************************************/
static size_t eheap_align_up(size_t size)
{
  return (size + EHEAP_ALIGNMENT - 1) & ~(EHEAP_ALIGNMENT - 1);
}

/*******************************************************************************
 ** \brief  Update heap statistics
 ** \param  None
 ** \retval None
 ******************************************************************************/
static void eheap_update_stats(void)
{
  size_t free_memory = 0;
  size_t largest_block = 0;
  size_t free_blocks_count = 0;
  eheap_free_block_t* current = eheap_free_list;
  while (current) 
  {
    free_memory += current->size;
    free_blocks_count++;
    if (current->size > largest_block) largest_block = current->size;
    current = current->next;
  }
  eheap_stats.current_usage = EHEAP_SIZE -free_memory;
  eheap_stats.largest_free_block = largest_block;
  if (eheap_stats.current_usage > eheap_stats.peak_usage) eheap_stats.peak_usage = eheap_stats.current_usage;
  if (free_blocks_count > 1) eheap_stats.fragmentation = (free_blocks_count * 100) / (EHEAP_SIZE / sizeof(eheap_free_block_t));
  else                       eheap_stats.fragmentation = 0;
}

/*******************************************************************************
 ** \brief  Defragment heap by merging adjacent free blocks
 ** \param  None
 ** \retval None
 ******************************************************************************/
static void eheap_defragment(void)
{
  eheap_free_block_t* current = eheap_free_list;
  while (current && current->next) 
  {
    uint8_t* current_end = (uint8_t*)current + current->size;
    if (current_end == (uint8_t*)current->next) 
    {
      current->size += current->next->size;
      current->next = current->next->next;
    } 
    else 
    {
      current = current->next;
    }
  }
}

/*******************************************************************************
 ** \brief  Validate pointer before freeing
 ** \param  None
 ** \retval None
 ******************************************************************************/
bool eheap_validate_ptr(void* ptr)
{
  if(!ptr) return false;
  uint8_t* heap_start = eheap;
  uint8_t* heap_end = eheap + EHEAP_SIZE;
  uint8_t* test_ptr = (uint8_t*)ptr;
  if(test_ptr < heap_start || test_ptr >= heap_end) return false; // Check if pointer is within heap bounds
  if(((uintptr_t)ptr & (EHEAP_ALIGNMENT - 1)) != 0) return false; // Check alignment
  return true;
}

/*******************************************************************************
 ** \brief  Initialize heap
 ** \param  None
 ** \retval None
 ******************************************************************************/
void eheap_init(void)
{
  eheap_init_mutex();
  eheap_lock();
  memset(eheap, 0, EHEAP_SIZE);
  eheap_free_list = (eheap_free_block_t*)eheap;
  eheap_free_list->size = EHEAP_SIZE;
  eheap_free_list->next = NULL;
  memset(&eheap_stats, 0, sizeof(eheap_stats));
  eheap_update_stats();
  eheap_unlock();
}

/*******************************************************************************
 ** \brief  Allocate memory
 ** \param  None
 ** \retval None
 ******************************************************************************/
void* eheap_alloc(size_t size)
{
  if(size == 0 || size > EHEAP_SIZE - sizeof(eheap_free_block_t))
  {
    eheap_stats.alloc_failures++;
    return NULL;
  }
  eheap_lock();
  eheap_stats.total_allocations++;
  size = eheap_align_up(size);
  size_t total_size = size + sizeof(eheap_free_block_t);
  eheap_free_block_t** current = &eheap_free_list;
  eheap_free_block_t** best_fit = NULL;
  size_t best_fit_size = SIZE_MAX;
  while (*current) // Best-fit algorithm
  {
    if ((*current)->size >= total_size) 
    {
      if ((*current)->size < best_fit_size) // Found potential block, check if it's better fit
      {
        best_fit = current;
        best_fit_size = (*current)->size;
      }
    }
    current = &(*current)->next;
  }
  if (!best_fit) 
  {
    eheap_stats.alloc_failures++;
    eheap_unlock();
    return NULL;
  }
  eheap_free_block_t* allocated = *best_fit;
  if (allocated->size >= total_size + sizeof(eheap_free_block_t) + EHEAP_ALIGNMENT) // Check if we can split the block
  {
    eheap_free_block_t* new_free = (eheap_free_block_t*)((uint8_t*)allocated + total_size);
    new_free->size = allocated->size - total_size;
    new_free->next = allocated->next;
    *best_fit = new_free;
    allocated->size = total_size;
  } 
  else 
  {
    *best_fit = allocated->next; // Use whole block
  }
  void* user_ptr = (void*)(allocated + 1);
  memset(user_ptr, 0, size);
  eheap_update_stats();
  eheap_unlock();
  return user_ptr;
}

/*******************************************************************************
 ** \brief  Allocate and zero-initialize memory
 ** \param  None
 ** \retval None
 ******************************************************************************/
void* eheap_calloc(size_t num, size_t size)
{
  size_t total_size = num * size;
  void* ptr = eheap_alloc(total_size);
  if (ptr) memset(ptr, 0, total_size);
  return ptr;
}

/*******************************************************************************
 ** \brief  Reallocate memory
 ** \param  None
 ** \retval None
 ******************************************************************************/
void* eheap_realloc(void* ptr, size_t new_size)
{
  if (!ptr) return eheap_alloc(new_size);
  if (new_size == 0) { eheap_free(ptr); return NULL;}
  if (!eheap_validate_ptr(ptr)) return NULL;
  eheap_lock();
  eheap_free_block_t* old_block = ((eheap_free_block_t*)ptr) - 1;
  size_t old_size = old_block->size - sizeof(eheap_free_block_t);
  if (new_size <= old_size){ eheap_unlock(); return ptr; }
  uint8_t* block_end = (uint8_t*)old_block + old_block->size;
  eheap_free_block_t* next_block = (eheap_free_block_t*)block_end;
  if (block_end < (uint8_t*)eheap + EHEAP_SIZE && (uint8_t*)next_block < (uint8_t*)eheap + EHEAP_SIZE) 
  {
    eheap_free_block_t* current = eheap_free_list;
    while (current) 
    {
      if (current == next_block) 
      {
        size_t required_additional = eheap_align_up(new_size) - old_size;
        if (current->size >= required_additional) // Expand into next free block
        {
          old_block->size += required_additional;
          current->size -= required_additional;
          if (current->size < sizeof(eheap_free_block_t) + EHEAP_ALIGNMENT) 
          {
            old_block->size += current->size; // Take the whole remaining block
            eheap_free_block_t** prev = &eheap_free_list; // Remove current from free list
            while (*prev != current) 
            {
              prev = &(*prev)->next;
            }
            *prev = current->next;
          }
          eheap_update_stats();
          eheap_unlock();
          return ptr;
        }
        break;
      }
      current = current->next;
    }
  }
  eheap_unlock();
  void* new_ptr = eheap_alloc(new_size);
  if (new_ptr) 
  {
    memcpy(new_ptr, ptr, old_size);
    eheap_free(ptr);
  }
  return new_ptr;
}

/*******************************************************************************
 ** \brief  Free memory
 ** \param  None
 ** \retval None
 ******************************************************************************/
void eheap_free(void* ptr)
{
  if (!ptr || !eheap_validate_ptr(ptr)) return;
  eheap_lock();
  eheap_stats.total_frees++;
  eheap_free_block_t* block = ((eheap_free_block_t*)ptr) - 1;
  eheap_free_block_t* current = eheap_free_list;
  while (current) 
  {
    if (current == block) 
    {
      eheap_unlock();
      return; 
    }
    current = current->next;
  }

  if (block->size < sizeof(eheap_free_block_t) || block->size > EHEAP_SIZE) 
  {
    eheap_unlock();
    return;
  }
  eheap_free_block_t** current_ptr = &eheap_free_list;
  while (*current_ptr && *current_ptr < block) 
  {
    current_ptr = &(*current_ptr)->next;
  }
  block->next = *current_ptr;
  *current_ptr = block;
  eheap_defragment();
  eheap_update_stats();
  eheap_unlock();
}

/*******************************************************************************
 ** \brief  Get heap statistics
 ** \param  None
 ** \retval None
 ******************************************************************************/
void eheap_get_stats(eheap_stats_t* stats)
{
  if (!stats) return;
  eheap_lock();
  memcpy(stats, &eheap_stats, sizeof(eheap_stats));
  eheap_unlock();
}

/*******************************************************************************
 ** \brief  Get heap usage percentage (0-100)
 ** \param  None
 ** \retval None
 ******************************************************************************/
size_t eheap_get_usage_percent(void)
{
  eheap_lock();
  size_t percent = (eheap_stats.current_usage *100) /EHEAP_SIZE;
  eheap_unlock();
  return percent;
}

/*******************************************************************************
 ** \brief  Check if heap is valid (debug function)
 ** \param  None
 ** \retval None
 ******************************************************************************/
bool eheap_validate(void)
{
  eheap_lock();
  bool valid = true;
  size_t total_free = 0;
  eheap_free_block_t* current = eheap_free_list;
  eheap_free_block_t* prev = NULL;
  while (current) 
  {
    if ((uint8_t*)current < eheap || (uint8_t*)current + current->size > eheap + EHEAP_SIZE) // Check if block is within heap bounds
    {
      valid = false;
      break;
    }
    if (prev && prev >= current) // Check if blocks are properly ordered
    {
      valid = false;
      break;
    }
    total_free += current->size;
    prev = current;
    current = current->next;
  }
  if(valid && (total_free + eheap_stats.current_usage != EHEAP_SIZE)) valid = false;
  eheap_unlock();
  return valid;
}

/*******************************************************************************
 ** \brief  Reset heap statistics
 ** \param  None
 ** \retval None
 ******************************************************************************/
void eheap_reset_stats(void)
{
  eheap_lock();
  eheap_stats.total_allocations = 0;
  eheap_stats.total_frees = 0;
  eheap_stats.alloc_failures = 0;
  eheap_unlock();
}