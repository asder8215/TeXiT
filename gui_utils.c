#include "gui_utils.h"
#include <stdbool.h>
#include <stdio.h>


typedef GtkWidget* QNodeValue;
// Node for Queue.
typedef struct QNode {
    // *value* is a reference and is not owned.
    QNodeValue value;
    struct QNode* next;
} QNode;

/// A Queue implemented as a Singly Linked List.
/// If there is only 1 item `front == back`.
/// Nodes are heap allocated, so `queue_free_nodes()` must be called at the end.
typedef struct {
    QNode* front;
    QNode* back;
} Queue;


Queue queue_new() {
    Queue q = { NULL, NULL };
    return q;
}
/// Calls `free()` on every Node, but not on the Queue itself because it is assumed it is stack allocated.
/// In other words, does NOT take ownership of *q*.
void queue_free_nodes(Queue* q) {
    QNode* current = q->front;

    while (current != NULL) {
        QNode* tmp = current;
        current = current->next;
        free(current);
        current = tmp;
    }
}
// Creates a new Node and pushes it to the Queue.
void enqueue(Queue* q, QNodeValue item) {
    QNode* new_node = malloc(sizeof(QNode));
    new_node->value = item;
    new_node->next = NULL;

    if (q->back == NULL) {
        // Queue is emtpy, add the first item.
        q->front = new_node;
        q->back = new_node;
    } else {
        // Push to the back if Queue is not empty.
        q->back->next = new_node;
        q->back = new_node;
    }
}
// Removes the front Node and returns its value.
// Returns NULL if empty.
QNodeValue queue_pop(Queue* q) {
    QNode* popped = q->front;
    QNodeValue val;
    // 0 items left
    if (popped == NULL)
        return NULL;

    val = popped->value;
    // 1 item left
    if (popped == q->back) {
        q->front = NULL;
        q->back = NULL;
    }
    // > 1 items left
    else
        q->front = q->front->next;

    free(popped);
    return val;
}
bool queue_is_empty(Queue* q) {
    return q->front == NULL;
}


GtkWidget* get_widget_by_name(GtkWidget* parent, const char* name) {
    GtkWidget* rtrn = NULL;
    Queue q;

    if (parent == NULL)
        return NULL;
    
    q = queue_new();
    // Step 1: enqueue the root.
    enqueue(&q, parent);

    // Continue searching until q is empty.
    while (!queue_is_empty(&q)) {
        // Step 2: Get next from queue.
        GtkWidget* current = queue_pop(&q);
        // Step 3: Check if it matches.
        if (gtk_widget_get_name(current) == name) {
            rtrn = current;
            break;
        }
        // Step 4: If not match, enqueue its children.
        GtkWidget* next = gtk_widget_get_first_child(current);
        while (next != NULL) {
            enqueue(&q, next);
            next = gtk_widget_get_next_sibling(next);
        }
        if (queue_is_empty(&q)) {
            printf("Queue is empty\n");
        } else {
            printf("Continue in queue\n");
        }
    }

    queue_free_nodes(&q);
    return rtrn;
}

GtkWidget* widget_get_nth_child(GtkWidget* parent, unsigned int n) {
    if (parent == NULL)
        return NULL;
    
    GtkWidget* child = gtk_widget_get_first_child(parent);
    for (int i = 0; i < n; i++)
        gtk_widget_get_next_sibling(child);
    return child;
}
