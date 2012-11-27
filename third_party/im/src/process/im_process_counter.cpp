/** \file
 * \brief Processing Counter
 *
 * See Copyright Notice in im_lib.h
 */

#include "im_process_counter.h"

#include <stdlib.h>
#include <memory.h>


int im_process_mincount = 250000;   /* 500*500 image size */

int imProcessOpenMPSetMinCount(int min_count)
{
  int old_imin_count = im_process_mincount;
  im_process_mincount = min_count;
  return old_imin_count;
}

int imProcessOpenMPSetNumThreads(int count)
{
#ifdef _OPENMP
  int old_count = omp_get_num_threads();
  omp_set_num_threads(count);
  return old_count;
#else
  (void)count;
  return 1;
#endif
}

#ifdef _OPENMP

int imCounterBegin_OMP(const char* title)
{
  if (!imCounterHasCallback()) 
    return -1;

  int counter = imCounterBegin(title);
  omp_lock_t* lck = new omp_lock_t;
  omp_init_lock(lck);
  imCounterSetUserData(counter, (void*)lck);
  return counter;
}

void imCounterEnd_OMP(int counter)
{
  if (counter == -1 || !imCounterHasCallback()) 
    return;

  omp_lock_t* lck = (omp_lock_t*)imCounterGetUserData(counter);
  omp_destroy_lock(lck);
  delete lck;
  imCounterSetUserData(counter, NULL);
  imCounterEnd(counter);
}

int imCounterInc_OMP(int counter)
{
  int processing;

  if (counter == -1 || !imCounterHasCallback()) 
    return 1;

  omp_lock_t* lck = (omp_lock_t*)imCounterGetUserData(counter);
  omp_set_lock(lck);
  processing = imCounterInc(counter);
  omp_unset_lock(lck);
    
  return processing;
}

#endif
