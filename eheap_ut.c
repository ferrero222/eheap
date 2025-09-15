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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "eheap.h"

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define TEST_START() printf("%-35s", __func__);
#define TEST_PASS()  printf("[PASS]\n");
#define TEST_FAIL()  printf("[FAIL]\n"); return false;
#define TEST_SKIP()  printf("[SKIP]\n");

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static bool eheap_test_init(void);
static bool eheap_test_basic_allocation(void);
static bool eheap_test_multiple_allocations(void);
static bool eheap_test_allocation_failure(void);
static bool eheap_test_zero_allocation(void);
static bool eheap_test_calloc_initialization(void);
static bool eheap_test_realloc_expand(void);
static bool eheap_test_realloc_shrink(void);
static bool eheap_test_realloc_null_ptr(void);
static bool eheap_test_realloc_zero_size(void);
static bool eheap_test_defragmentation(void);
static bool eheap_test_fragmentation_stats(void);
static bool eheap_test_pointer_validation(void);
static bool eheap_test_validation(void);
static bool eheap_test_stats_consistency(void);
static bool eheap_test_double_free_protection(void);
static bool eheap_test_boundary_conditions(void);

/*******************************************************************************
 * Local types definitions
 ******************************************************************************/
static int test_count = 0;
static int pass_count = 0;

typedef bool (*test_func_t)();

struct test_case {
  test_func_t func;
  const char* name;
};

struct test_case test_cases[] = {
  {eheap_test_init,                   "Heap initialization"},
  {eheap_test_basic_allocation,       "Basic allocation"},
  {eheap_test_multiple_allocations,   "Multiple allocations"},
  {eheap_test_allocation_failure,     "Allocation failure"},
  {eheap_test_zero_allocation,        "Zero allocation"},
  {eheap_test_calloc_initialization,  "Calloc initialization"},
  {eheap_test_realloc_expand,         "Realloc expand"},
  {eheap_test_realloc_shrink,         "Realloc shrink"},
  {eheap_test_realloc_null_ptr,       "Realloc NULL pointer"},
  {eheap_test_realloc_zero_size,      "Realloc zero size"},
  {eheap_test_defragmentation,        "Defragmentation"},
  {eheap_test_fragmentation_stats,    "Fragmentation statistics"},
  {eheap_test_pointer_validation,     "Pointer validation"},
  {eheap_test_validation,             "Heap validation"},
  {eheap_test_stats_consistency,      "Statistics consistency"},
  {eheap_test_double_free_protection, "Double free protection"},
  {eheap_test_boundary_conditions,    "Boundary conditions"},
  {NULL,                               NULL}
};

/*******************************************************************************
 * Local function prototypes
 ******************************************************************************/
/*******************************************************************************
 * Function implementation
 ******************************************************************************/
/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_init(void) 
{
  TEST_START();
  eheap_init();
  eheap_stats_t stats;
  eheap_get_stats(&stats);
  assert(stats.current_usage == 0);
  assert(stats.peak_usage == 0);
  assert(stats.largest_free_block == EHEAP_SIZE);
  assert(stats.total_allocations == 0);
  assert(stats.total_frees == 0);
  assert(stats.alloc_failures == 0);
  assert(stats.fragmentation == 0);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_basic_allocation(void) 
{
  TEST_START();
  eheap_init();
  void* ptr1 = eheap_alloc(64);
  assert(ptr1 != NULL);
  assert(((uintptr_t)ptr1 % EHEAP_ALIGNMENT) == 0);
  eheap_stats_t stats;
  eheap_get_stats(&stats);
  assert(stats.current_usage > 0);
  assert(stats.total_allocations == 1);
  eheap_free(ptr1);
  eheap_get_stats(&stats);
  assert(stats.total_frees == 1);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_multiple_allocations(void) 
{
  TEST_START();
  eheap_init();
  void* ptrs[10];
  for (int i = 0; i < 10; i++) 
  {
    ptrs[i] = eheap_alloc(32);
    assert(ptrs[i] != NULL);
  }
  eheap_stats_t stats;
  eheap_get_stats(&stats);
  assert(stats.total_allocations == 10);
  assert(stats.current_usage > 300); // 10 * (32 + overhead)
  for (int i = 0; i < 10; i++) 
  {
    eheap_free(ptrs[i]);
  }
  eheap_get_stats(&stats);
  assert(stats.total_frees == 10);
  assert(stats.current_usage == 0);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_allocation_failure(void) 
{
  TEST_START();
  eheap_init();
  void* ptr = eheap_alloc(EHEAP_SIZE + 100);
  assert(ptr == NULL);
  eheap_stats_t stats;
  eheap_get_stats(&stats);
  assert(stats.alloc_failures == 1);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_zero_allocation(void) 
{
  TEST_START();
  eheap_init();
  void* ptr = eheap_alloc(0);
  assert(ptr == NULL);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_calloc_initialization(void) 
{
  TEST_START();
  eheap_init();
  int* arr = (int*)eheap_calloc(10, sizeof(int));
  assert(arr != NULL);
  for (int i = 0; i < 10; i++) 
  {
    assert(arr[i] == 0);
  }
  for (int i = 0; i < 10; i++) 
  {
    arr[i] = i + 1;
  }
  eheap_free(arr);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_realloc_expand(void) 
{
  TEST_START();
  eheap_init();
  int* arr = (int*)eheap_alloc(5 * sizeof(int));
  assert(arr != NULL);
  for (int i = 0; i < 5; i++) 
  {
    arr[i] = i + 1;
  }
  int* new_arr = (int*)eheap_realloc(arr, 10 * sizeof(int));
  assert(new_arr != NULL);
  for (int i = 0; i < 5; i++) 
  {
    assert(new_arr[i] == i + 1);
  }
  eheap_free(new_arr);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_realloc_shrink(void) 
{
  TEST_START();
  eheap_init();
  int* arr = (int*)eheap_alloc(10 * sizeof(int));
  assert(arr != NULL);
  for (int i = 0; i < 10; i++) 
  {
    arr[i] = i + 1;
  }
  int* new_arr = (int*)eheap_realloc(arr, 5 * sizeof(int));
  assert(new_arr != NULL);
  for (int i = 0; i < 5; i++) 
  {
    assert(new_arr[i] == i + 1);
  }
  eheap_free(new_arr);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_realloc_null_ptr(void) 
{
  TEST_START();
  eheap_init();
  int* ptr = (int*)eheap_realloc(NULL, 100);
  assert(ptr != NULL);
  eheap_free(ptr);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_realloc_zero_size(void) 
{
  TEST_START();
  eheap_init();
  int* ptr = (int*)eheap_alloc(100);
  assert(ptr != NULL);
  int* new_ptr = (int*)eheap_realloc(ptr, 0);
  assert(new_ptr == NULL);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_defragmentation(void) 
{
  TEST_START();
  eheap_init();
  void* ptr1 = eheap_alloc(64);
  void* ptr2 = eheap_alloc(64);
  void* ptr3 = eheap_alloc(64);
  assert(ptr1 && ptr2 && ptr3);
  eheap_free(ptr2);
  eheap_free(ptr1);
  eheap_stats_t stats_before;
  eheap_get_stats(&stats_before);
  void* ptr_large = eheap_alloc(128);
  assert(ptr_large != NULL);
  eheap_stats_t stats_after;
  eheap_get_stats(&stats_after);
  eheap_free(ptr3);
  eheap_free(ptr_large);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_fragmentation_stats(void) 
{
  TEST_START();
  eheap_init();
  void* ptr1 = eheap_alloc(100);
  void* ptr2 = eheap_alloc(100);
  void* ptr3 = eheap_alloc(100);
  eheap_free(ptr2); 
  eheap_stats_t stats;
  eheap_get_stats(&stats);
  assert(stats.fragmentation > 0);
  eheap_free(ptr1);
  eheap_free(ptr3);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_pointer_validation(void)
{
  TEST_START();
  eheap_init();
  void* ptr = eheap_alloc(100);
  assert(eheap_validate_ptr(ptr) == true);
  assert(eheap_validate_ptr(NULL) == false);
  int outside_var;
  assert(eheap_validate_ptr(&outside_var) == false);
  uint8_t* misaligned = (uint8_t*)ptr + 1;
  assert(eheap_validate_ptr(misaligned) == false);
  eheap_free(ptr);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_validation(void) 
{
  TEST_START();
  eheap_init();
  assert(eheap_validate() == true);
  void* ptr = eheap_alloc(100);
  assert(eheap_validate() == true);
  eheap_free(ptr);
  assert(eheap_validate() == true);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_stats_consistency(void) 
{
  TEST_START();
  eheap_init();
  eheap_stats_t stats;
  eheap_get_stats(&stats);
  assert(stats.current_usage + stats.largest_free_block <= EHEAP_SIZE);
  assert(stats.peak_usage >= stats.current_usage);
  assert(stats.fragmentation <= 100);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_double_free_protection(void) 
{
  TEST_START();
  eheap_init();
  void* ptr = eheap_alloc(100);
  assert(ptr != NULL);
  eheap_free(ptr);
  eheap_free(ptr); 
  assert(eheap_validate() == true);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static bool eheap_test_boundary_conditions(void) 
{
  TEST_START();
  eheap_init();
  size_t max_single_alloc = EHEAP_SIZE - sizeof(eheap_free_block_t) - EHEAP_ALIGNMENT;
  void* ptr = eheap_alloc(max_single_alloc);
  assert(ptr != NULL);
  void* ptr2 = eheap_alloc(1);
  assert(ptr2 == NULL);
  eheap_free(ptr);
  ptr2 = eheap_alloc(1);
  assert(ptr2 != NULL);
  eheap_free(ptr2);
  TEST_PASS();
  return true;
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
static void eheap_run_all_tests(void) 
{
  printf("Starting eHeap unit tests\n");
  printf("Heap size: %d bytes\n", EHEAP_SIZE);
  printf("Alignment: %d bytes\n", EHEAP_ALIGNMENT);
  printf("=========================================\n");
  for (int i = 0; test_cases[i].func != NULL; i++) 
  {
    test_count++;
    if(test_cases[i].func()) pass_count++;
  }
  printf("=========================================\n");
  printf("Results: %d/%d tests passed\n", pass_count, test_count);
  if (pass_count == test_count) printf("All tests passed successfully!\n");
  else printf("Some tests failed!\n");
}

/*******************************************************************************
 ** \brief  None
 ** \param  None
 ** \retval None
 ******************************************************************************/
int main(void) 
{
  eheap_run_all_tests();
  return (pass_count == test_count) ? 0 : 1;
}