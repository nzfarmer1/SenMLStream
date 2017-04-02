#ifndef FIXEDQUEUE_H
#define FIXEDQUEUE_H

#include "../arduino-serial/Stream.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>


template <size_t BUFSIZE>
class FixedQueue{

private:
    uint8_t buf[BUFSIZE];
    uint8_t *pIn, *pOut, *pEnd;
    uint8_t full;

    void _reset() {
        pIn = pOut = buf;       // init to any slot in buffer
        pEnd = &buf[BUFSIZE];   // past last valid slot in buffer
        full = 0;               // buffer is empty
    }

public:
    FixedQueue() {
        _reset();
    }
    
    void clear() {
        _reset();
    }

    bool isEmpty() {
        return this->count() ==0;
    }

    bool available() {
        return BUFSIZE -  this->count();
    }

    bool isFull() {
        return full ==1;
    }

    int count() {
        if (full)
            return BUFSIZE;
        return (pIn - pOut) >= 0 ? (pIn - pOut) : BUFSIZE + (pIn - pOut);
    }
    
    // add uint8_t 'c' to buffer
    bool enqueue(const uint8_t c) {
        if (pIn == pOut  &&  full)
            return 0;           // buffer overrun
    
        *pIn++ = c;             // insert c into buffer
        if (pIn >= pEnd)        // end of circular buffer?
            pIn = buf;          // wrap around
    
        if (pIn == pOut)        // did we run into the output ptr?
            full = 1;           // can't add any more data into buffer
        return 1;               // all OK
    }
    
    // get a uint8_t from circular buffer
    uint8_t dequeue() {
        uint8_t pc;
        if (pIn == pOut  &&  !full)
            return 0;           // buffer empty  FAIL
    
        pc = *pOut++;              // pick up next uint8_t to be returned
        if (pOut >= pEnd)       // end of circular buffer?
            pOut = buf;         // wrap around
    
        full = 0;               // there is at least 1 slot
        return pc;               // *pc has the data to be returned
    }

    uint8_t peek() {
        if (pIn == pOut  &&  !full)
            return 0;           // buffer empty  FAIL
    
        return *pOut;              // pick up next uint8_t to be returned
    }
};

#endif
