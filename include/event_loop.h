#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <pthread.h>
#include <stdbool.h>

#define MAX_EVENT_TYPES 32

typedef enum EventType {
    EVENT_TERMINATE,  // 0 - reserved event type used for queue termination
} EventType;

typedef struct Event Event;
struct Event {
    int type;
    void* data;
    Event* next;
};

typedef struct EventQueue EventQueue;
typedef int (*EventCallback)(EventQueue* queue, Event* event);

struct EventQueue {
    Event* head;
    Event* tail;
    size_t size;
    size_t type_counter[MAX_EVENT_TYPES];
    EventCallback callback;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_cond_t wait_cond;
};

Event* createEvent(int type, void* data);
EventQueue* createEventQueue();
void clearEventQueue(EventQueue* queue);
void destroyEventQueue(EventQueue* queue);
void setEventCallback(EventQueue* queue, EventCallback callback);
Event* poll_event(EventQueue* queue);
void wait_event(EventQueue* queue, int type);
void push_event(EventQueue* queue, Event* event);
void startQueue(EventQueue* queue, EventCallback callback);
void stopQueue(EventQueue* queue);

// ----------------------------------------------------------------------------
// UNSAFE API
// ----------------------------------------------------------------------------

bool has_event_unsafe(EventQueue* queue, int type);
void enqueue_unsafe(EventQueue* queue, Event* event);
Event* dequeue_unsafe(EventQueue* queue);

#endif //!EVENT_LOOP_H_