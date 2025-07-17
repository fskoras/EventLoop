#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#include "event_loop.h"


Event* createEvent(int type, void* data) {
    Event* ev = malloc(sizeof(Event));
    ev->type = type;
    ev->data = data;
    ev->next = NULL;
    return ev;
}

EventQueue* createEventQueue() {
    EventQueue* queue = malloc(sizeof(EventQueue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->thread = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    pthread_cond_init(&queue->wait_cond, NULL);
    return queue;
}

void clearEventQueue(EventQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    Event* current = queue->head;
    while (current != NULL) {
        Event* next = current->next;
        free(current);
        current = next;
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    for (int i = 0; i < MAX_EVENT_TYPES; i++) {
        queue->type_counter[i] = 0;
    }
    pthread_mutex_unlock(&queue->mutex);
}

void destroyEventQueue(EventQueue* queue) {
    clearEventQueue(queue);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    pthread_cond_destroy(&queue->wait_cond);
    free(queue);
}

void setEventCallback(EventQueue* queue, EventCallback callback) {
    queue->callback = callback;
}

Event* poll_event(EventQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->head == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    Event* ev = dequeue_unsafe(queue);
    pthread_cond_signal(&queue->wait_cond);
    pthread_mutex_unlock(&queue->mutex);
    return ev;
}

void wait_event(EventQueue* queue, int type) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->type_counter[type] == 0) {
        pthread_cond_wait(&queue->wait_cond, &queue->mutex);
    }
    pthread_mutex_unlock(&queue->mutex);
}

void push_event(EventQueue* queue, Event* event) {
    pthread_mutex_lock(&queue->mutex);
    enqueue_unsafe(queue, event);
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

void event_loop(EventQueue* queue) {
    for (;;) {
        Event* ev = poll_event(queue);
        int r = queue->callback(queue, ev);
        free(ev);
        if (r == EVENT_TERMINATE)  break;
    }
}

void* event_queue_thread(void* arg) {
    EventQueue* queue = (EventQueue*)arg;
    event_loop(queue);
    return NULL;
}

void startQueue(EventQueue* queue, EventCallback callback) {
    setEventCallback(queue, callback);
    if (pthread_create(&queue->thread, NULL, event_queue_thread, (void*)queue) != 0) {
        perror("Failed to create event loop thread");
        exit(EXIT_FAILURE);
    }
    pthread_setname_np(queue->thread, "queue");
}

void stopQueue(EventQueue* queue) {
    push_event(queue, createEvent(EVENT_TERMINATE, NULL));
    if (pthread_join(queue->thread, NULL) != 0) {
        perror("Failed to stop event loop thread");
        exit(EXIT_FAILURE);
    }
    queue->thread = 0;
    queue->callback = NULL;
}


// ----------------------------------------------------------------------------
// UNSAFE API
// ----------------------------------------------------------------------------

bool has_event_unsafe(EventQueue* queue, int type)
{
    return (bool)(queue->type_counter[type] > 0);
}

void enqueue_unsafe(EventQueue* queue, Event* event) {
    if (queue->head == NULL) {
        queue->head = event;
        queue->tail = event;
    }
    else {
        queue->tail->next = event;
        queue->tail = event;
    }
    queue->size++;
    queue->type_counter[event->type]++;
}

Event* dequeue_unsafe(EventQueue* queue) {
    Event* ev = queue->head;
    if (ev == queue->tail) {
        queue->head = NULL;
        queue->tail = NULL;
    }
    else {
        queue->head = queue->head->next;
    }
    assert(queue->size > 0);
    queue->size--;
    queue->type_counter[ev->type]--;
    return ev;
}
