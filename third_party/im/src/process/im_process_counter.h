/** \file
 * \brief Processing Counter
 *
 * See Copyright Notice in im_lib.h
 */

#ifndef __IM_PROCESSING_COUNTER_H
#define __IM_PROCESSING_COUNTER_H

#include <im_counter.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#if	defined(__cplusplus)
extern "C" {
#endif

/* Used inside "pragma omp parallel for if()" */
extern int im_process_mincount;
#define IM_OMP_MINCOUNT(_c)  (_c)>im_process_mincount
#define IM_OMP_MINHEIGHT(_h) (_h)*(_h)>im_process_mincount

int imProcessOpenMPSetMinCount(int min_count);
int imProcessOpenMPSetNumThreads(int count);

#define IM_INT_PROCESSING     int processing = 1;

#ifdef _OPENMP

#define IM_BEGIN_PROCESSING   if (processing) {
#define IM_COUNT_PROCESSING   if (!imCounterInc_OMP(counter)) { processing = 0;
#define IM_END_PROCESSING     }}
#define IM_MAX_THREADS        omp_get_max_threads()
#define IM_THREAD_NUM         omp_get_thread_num()

int  imCounterBegin_OMP(const char* title);
void imCounterEnd_OMP(int counter);
int  imCounterInc_OMP(int counter);

#define imProcessCounterBegin imCounterBegin_OMP
#define imProcessCounterEnd   imCounterEnd_OMP

#else

/*
#pragma warning (disable : 4068) */ /* disable unknown pragma warnings */

#define IM_BEGIN_PROCESSING   
#define IM_COUNT_PROCESSING   if (!imCounterInc(counter)) { processing = 0; break; }
#define IM_END_PROCESSING

#define IM_MAX_THREADS 1
#define IM_THREAD_NUM  0

#define imProcessCounterBegin imCounterBegin
#define imProcessCounterEnd   imCounterEnd

#endif


#if defined(__cplusplus)
}
#endif

#endif
