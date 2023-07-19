#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "../types.h"

class FIFO {
    typedef struct fifo_node {
        u16 value;
        fifo_node* next = nullptr;
    } Node;

private:
    Node* head = nullptr;
    Node* tail = nullptr;
    int size = 0;

public:
    FIFO() {}
    ~FIFO() { delete this; }

    FIFO(const FIFO& _fifo) {
        if (_fifo.size > 0) {
            Node* ptr = _fifo.head;
            Node* prev = nullptr;

            for (int i = 0; i < _fifo.size; i++) {
                Node* temp_node = (Node*) malloc(sizeof(Node));
                temp_node->value = ptr->value;

                if (i == 0) {
                    this->head = temp_node;
                }

                if (prev != nullptr) {
                    prev->next = temp_node;
                }

                if (i == _fifo.size - 1) {
                    temp_node->next = nullptr;
                    this->tail = temp_node;
                }

                ptr = ptr->next;
                prev = temp_node;
            }

            this->size = _fifo.size;
        }
    }

    void enqueue(u16 value) {
        if (!head) {
            head = (Node*) malloc(sizeof(Node));
            head->value = value;
            tail = head;
        }
        else {
            tail->next = (Node*) malloc(sizeof(Node));
            tail = tail->next;
            tail->value = value;
        }
        size++;
    }

    u16 dequeue() {
        if (!head) {
            printf("Queue is empty\n");
            return 0;
        }

        Node* temp = head;

        u16 value = head->value;
        head = head->next;

        free(temp);
        
        size--;

        return value;
    }
};