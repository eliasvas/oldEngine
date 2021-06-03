#ifndef THREAD_H
#define THREAD_H
#include "tools.h"
#define INFINITE_MS 4294967295

typedef struct Thread
{
    u32 handle;
    u32 id;
}Thread;

internal Thread thread_create(void *proc, void *params)
{
    Thread t = (Thread){0};
#if _WIN32
    t.handle = CreateThread(0,0,proc, params, 0, &t.id);
#else
    //TBD
#endif
    return t;
}

internal void thread_wait_end(Thread t, u32 millis)
{
#if _WIN32
    WaitForSingleObject(t.handle, millis);
#else
    //TBD
#endif
}

typedef struct Mutex
{
   u32 handle; 
}Mutex;

internal Mutex mutex_create(void)
{
    Mutex m;
#if _WIN32
    m.handle = CreateMutexA(NULL, FALSE, NULL);
#else
    //TBD
#endif
    return m;
}

internal u32 mutex_lock(Mutex m)
{

#if _WIN32
    WaitForSingleObject(m.handle, INFINITE);
    return TRUE;
#else
    //TBD
#endif
    return FALSE;
}

internal u32 mutex_unlock(Mutex m)
{
#if _WIN32
    ReleaseMutex(m.handle);
    return TRUE;
#else
    //TBD
#endif
    
    return FALSE;
}


#endif
