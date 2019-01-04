#ifndef VOLATILE_FIFO_H
#define VOLATILE_FIFO_H

#include <Arduino.h>
#include <stdint.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 256
#endif

class VolatileFIFO {
    
    public:
    
        //Constructors
        VolatileFIFO();
        VolatileFIFO(int blocking);
        
        //Returns 1 (true) if buffer is full
        int isFull() const;
        
        //Returns 1 (true) if buffer is empty
        int isEmpty() const;
        
        //Returns used size of buffer (doesn't disable interrupts, is atomic)
        unsigned int length() const;
        
        //Write a byte into buffer
        int write(uint8_t value);
        int safe_write(uint8_t value);
        
        //Read a byte from buffer
        int read();
        int safe_read();
    
    private:
    
        void inc_f();
        unsigned int calc_e();
        
        volatile unsigned int front = 0;
        volatile unsigned int buf_len = 0;
        volatile uint8_t elems[BUFFER_SIZE];
        int block = true;
        
    
};

#endif //VOLATILE_RING_BUFFER_H