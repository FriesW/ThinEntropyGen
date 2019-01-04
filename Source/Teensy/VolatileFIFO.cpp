#include "VolatileFIFO.h"

VolatileFIFO::VolatileFIFO() {}
VolatileFIFO::VolatileFIFO(int blocking) {
    block = blocking;
}

int VolatileFIFO::isFull() const {
    return length() == BUFFER_SIZE;
}

int VolatileFIFO::isEmpty() const {
    return length() == 0;
}

unsigned int VolatileFIFO::length() const {
    return buf_len;
}

int VolatileFIFO::safe_write(uint8_t value) {
    __disable_irq();
    int out = write(value);
    __enable_irq();
    return out;
}

int VolatileFIFO::write(uint8_t value) {
    int full = buf_len == BUFFER_SIZE;
    
    if(block && full)
        return 0;
    
    elems[calc_e()] = value;
    if(full)
        inc_f();
    else
        buf_len++;
    return 1;
}

int VolatileFIFO::safe_read() {
    __disable_irq();
    int out = read();
    __enable_irq();
    return out;
}

int VolatileFIFO::read(){
    
    if(buf_len == 0)
        return -1;
    
    uint8_t out = elems[front];
    inc_f();
    buf_len--;
    return out;
}

void VolatileFIFO::inc_f() {
    front = (1 + front) % BUFFER_SIZE;
}

unsigned int VolatileFIFO::calc_e() {
    return (front + buf_len) % BUFFER_SIZE;
}
