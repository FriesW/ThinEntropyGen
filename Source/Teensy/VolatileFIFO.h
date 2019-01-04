#ifndef VOLATILE_FIFO_H
#define VOLATILE_FIFO_H

#include <Arduino.h>
#include <stdint.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 256
#endif

class VolatileFIFO {
    
    public:
    
        //Constructor
        VolatileFIFO();
        
        VolatileFIFO(int blocking);
        
        //Destructor
        virtual ~VolatileFIFO(); //TODO is this needed?
        
        //Returns 1 (true) if buffer is full
        int safe_isFull() const;
        int isFull() const;
        
        //Returns 1 (true) if buffer is empty
        int safe_isEmpty() const;
        int isEmpty() const;
        
        //Returns used size of buffer
        unsigned int length() const;
        
        //Returns used size of buffer without disabling interrupts
        unsigned int est_length() const;
        
        //Write a byte into buffer
        int write(uint8_t value);
        int safe_write(uint8_t value);
        
        //Read a byte from buffer
        int read();
        int safe_read();
        
    //protected: //TODO is this needed?
    private:
    
        void inc_f();
        unsigned int calc_e();
        
        volatile unsigned int front = 0;
        volatile unsigned int buf_len = 0;
        volatile uint8_t elems[BUFFER_SIZE];
        int block = true;
        
    
};

#endif //VOLATILE_RING_BUFFER_H