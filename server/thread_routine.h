#ifndef THREAD_ROUTINE_H
#define THREAD_ROUTINE_H

#define ORPHAN    3
#define EXTINTED  2
#define ALIVE     1
#define ZOMBIE    0

void *thread_routine(void* arg);

#endif
