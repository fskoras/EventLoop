#include "event_loop.h"

#include <stdio.h>
#include <pthread.h>

#define PRODUCERS_COUNT 4
#define PRODUCER_INTERATIONS 5

void* producer(void* arg) {
    EventQueue* queue = (EventQueue*)arg;
    int i = 0;
    for (i = 0; i < PRODUCER_INTERATIONS; i++) {
        push_event(queue, createEvent(EVENT_TYPE_NOTIFY, (void*)i));
    }
    return NULL;
}

int processEvent(EventQueue* queue, Event* event) {
    int data = (int)event->data;
    
    if (event->type == EVENT_TYPE_NOTIFY) {
        printf("NOTIFY: %d!\n", data);
    }

    if (event->type == EVENT_TYPE_TERMINATE) {
        printf("TERMINATE Event triggered!\n");
        return 0;
    }

    return 1;
}

int main(int argc, char const *argv[])
{
    EventQueue* queue = createEventQueue();
    pthread_t producer_thread[PRODUCERS_COUNT];
    int i;
    for (i = 0; i < PRODUCERS_COUNT; i++) {
        pthread_create(&producer_thread[i], NULL, producer, (void*)queue);
        pthread_setname_np(producer_thread[i], "producer");
    }

    startQueue(queue, processEvent);
    
    for (i = 0; i < PRODUCERS_COUNT; i++) {
        pthread_join(producer_thread[i], NULL);
    }

    stopQueue(queue);
    destroyEventQueue(queue);

    printf("FIN!\n");
    return 0;
}
