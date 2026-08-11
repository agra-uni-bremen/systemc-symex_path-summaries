#include <stdlib.h>

int   ptc_exec(pthread_t *t, const pthread_attr_t *z, void *(*routine)(void *), void *arg);
int   pthread_create(pthread_t *t, const pthread_attr_t *z, void *(*routine)(void *), void *arg);


int   pthread_cond_destroy(pthread_cond_t *t);
int   pthread_cond_init(pthread_cond_t *t, const pthread_condattr_t *z);
//int   pthread_cond_signal(pthread_cond_t *t);
//int   pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int   pthread_mutex_destroy(pthread_mutex_t *t);
int   pthread_mutex_init(pthread_mutex_t *t, const pthread_mutexattr_t *z);
int   pthread_mutex_lock(pthread_mutex_t *t);
int   pthread_mutex_trylock(pthread_mutex_t *);
int   pthread_mutex_unlock(pthread_mutex_t *t);

int   pthread_attr_destroy(pthread_attr_t *t);
int   pthread_attr_init(pthread_attr_t *t);

int   pthread_attr_setdetachstate(pthread_attr_t *t, int z);
int   pthread_attr_setstacksize(pthread_attr_t *t , size_t z);
