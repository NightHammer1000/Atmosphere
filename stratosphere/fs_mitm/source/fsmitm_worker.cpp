#include <switch.h>
#include <stratosphere.hpp>
#include "fsmitm_worker.hpp"

static SystemEvent *g_new_waitable_event = NULL;

static HosMutex g_new_waitable_mutex;
static HosSemaphore g_sema_new_waitable_finish;

static WaitableManager *g_worker_waiter = NULL;

Result FsMitMWorker::AddWaitableCallback(void *arg, Handle *handles, size_t num_handles, u64 timeout) {
    (void)arg;
    svcClearEvent(handles[0]);
    g_sema_new_waitable_finish.Signal();
    return 0;
}

void FsMitMWorker::AddWaitable(IWaitable *waitable) {
    g_worker_waiter->add_waitable(waitable);
    g_new_waitable_mutex.Lock();
    g_new_waitable_event->signal_event();
    g_sema_new_waitable_finish.Wait();
    g_new_waitable_mutex.Unlock();
}

void FsMitMWorker::Main(void *arg) {
    /* Initialize waitable event. */
    g_new_waitable_event = new SystemEvent(NULL, &FsMitMWorker::AddWaitableCallback);

    /* Make a new waitable manager. */
    g_worker_waiter = new WaitableManager(U64_MAX);
    g_worker_waiter->add_waitable(g_new_waitable_event);
    
    /* Service processes. */
    g_worker_waiter->process();
    
    delete g_worker_waiter;
}
