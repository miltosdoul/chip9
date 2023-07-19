#include <iostream>
using namespace std;

class EmptyQueueException : public std::exception {
public:
    const char* what() const throw() {
        return "EmptyQueueException: Queue is empty, cannot dequeue";
    }
};