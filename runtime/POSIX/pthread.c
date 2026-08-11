#include "pthread.h"
#include "klee/klee.h"

#include <stdbool.h>

//static bool pts_thread_first = true;
//static void *(*thread)(void *);
//static void *args;
//static bool pts_is_main = false;

int   pthread_create(pthread_t *t, const pthread_attr_t *z, void *(*routine)(void *), void *arg) {
  // just a dummy so that i can find the method in my llvm pass
  // not actually executed ever (caught by special function handler)
//  klee_warning("pthread create stub that should not be called"); //SYSCDEBUG
  return 0;
}

int   ptc_exec(pthread_t *t, const pthread_attr_t *z,
                   void *(*routine)(void *), void *arg) {
//  klee_warning("pthread create! what a hack"); //SYSCDEBUG
  routine(arg);
  return 0;
}

int   pthread_cond_destroy(pthread_cond_t *t) {
  return 0;
}
int   pthread_cond_init(pthread_cond_t *t, const pthread_condattr_t *z) {
  return 0;
}
//int   pthread_cond_signal(pthread_cond_t *t) {
//  if(!thread) {
//    klee_warning("called pthread-cond-wait without pthread-create");
//    return -1;
//  }
//  klee_warning("pthread cond signal");
//  if(pts_is_main) {
//    klee_warning("pthread cond signal isMain");
//    pts_is_main = false;
//    thread(args);
//    pts_is_main = true;
//    pts_thread_first = false;
//  }
//  return 0;
//}
//int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
//  return 0;
//}

int   pthread_mutex_destroy(pthread_mutex_t *t) {
  return 0;
}
int   pthread_mutex_init(pthread_mutex_t *t, const pthread_mutexattr_t *z) {
  return 0;
}
int   pthread_mutex_lock(pthread_mutex_t *t) {
  return 0;
}
int   pthread_mutex_trylock(pthread_mutex_t *t) {
  return 0;
}
int   pthread_mutex_unlock(pthread_mutex_t *t) {
  return 0;
}

int   pthread_attr_destroy(pthread_attr_t *t) {
  return 0;
}
int   pthread_attr_init(pthread_attr_t *t) {
  return 0;
}

int   pthread_attr_setdetachstate(pthread_attr_t *t, int z) {
  return 0;
}
int   pthread_attr_setstacksize(pthread_attr_t *t , size_t z) {
  return 0;
}
