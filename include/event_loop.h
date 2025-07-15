#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <pthread.h>

typedef enum EventType {
    EVENT_TYPE_NOTIFY,
    EVENT_TYPE_TERMINATE,
    EVENT_TYPE_COUNT,
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
    EventCallback callback;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

Event* createEvent(int type, void* data);
EventQueue* createEventQueue();
void destroyEventQueue(EventQueue* queue);
void setEventCallback(EventQueue* queue, EventCallback callback);
void enqueue_unsafe(EventQueue* queue, Event* event);
Event* dequeue_unsafe(EventQueue* queue);
Event* poll_event(EventQueue* queue);
void push_event(EventQueue* queue, Event* event);
void startQueue(EventQueue* queue, EventCallback callback);
void stopQueue(EventQueue* queue);

#endif