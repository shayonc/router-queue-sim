#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>

typedef unsigned int uint;

typedef struct Packet {
    uint tick_generated;
    uint bits_served;
    struct Packet * below;
} Packet;

void Packet_clear(Packet * pack) {
    if (pack->below != NULL) {
        Packet_clear(pack->below);
    }
    free(pack);
}

typedef struct Queue {
    Packet * head;
    Packet * tail;
    uint size;
} Queue;
Queue queue;

void Queue_init() {
    queue.head = NULL;
    queue.tail = NULL;
    queue.size = 0;
}
void Queue_clear() {
    if (queue.size != 0) {
        Packet_clear(queue.head);
    }
	Queue_init();
}
void Queue_add(Packet * pack) {
    if (pack == NULL) {
        return;
    }
    if (queue.size == 0) {
        queue.head = pack;
        queue.tail = pack;
    }
    else {
        queue.tail->below = pack;
        queue.tail = pack;
    }
    (queue.size)++;
}
Packet * Queue_remove() {
    if (queue.size == 0) {
        return NULL;
    }
    Packet * ret = queue.head;
    queue.head = queue.head->below;
    (queue.size)--;
    if (queue.size == 0) {
        queue.tail = NULL;
    }
    return ret;
}
