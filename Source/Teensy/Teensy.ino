typedef unsigned int uint;
typedef unsigned long ulong;

#include <ADC.h>
#include "VolatileFIFO.h"
#include "Base64Encode.h"

#define DEBUG

#define NOISE_SOURCE A9 //TODO set pin, I think A0-A9 is valid

//Status / alive LED
#define PING_INTERVAL 100 //msec
#define PING_PIN 13 //LED pin
elapsedMillis last_ping = 0;

//Character output buffer
#define OUT_SIZE 76
char out_buff[OUT_SIZE+1];
uint out_p = 0;

VolatileFIFO *buffer = new VolatileFIFO();
ADC *adc = new ADC();

#ifdef DEBUG
    elapsedMillis debug_window;
    ulong debug_buff_reads;
    ulong debug_data_transfer;
    bool debug_w_empty;
    bool debug_w_full;
    #define DEBUG_RESET debug_w_full = debug_w_empty = 0; debug_data_transfer = debug_buff_reads = debug_window = 0
#endif

void setup()
{
    #ifdef DEBUG
    DEBUG_RESET;
    #endif
    
    //Finish initialization
    out_buff[OUT_SIZE] = '\n';
    
    //Debug led as output
    pinMode(PING_PIN, OUTPUT);

    //ADC_REF_3V3, ADC_REF_EXT
    adc->setReference(ADC_REFERENCE::REF_3V3, ADC_0);

    adc->setAveraging(1); //set number of averages
    adc->setResolution(16); //max 16 bits

    //VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED, VERY_HIGH_SPEED
    //Additional options: https://github.com/pedvide/ADC/blob/master/ADC.h#L128
    adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
    adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);

    adc->startContinuous(NOISE_SOURCE, ADC_0);
    adc->enableInterrupts(ADC_0);
}


void loop()
{
    
    #ifdef DEBUG
    if(debug_window > 1000){
        Serial.println("\nDBP");
        Serial.println(debug_buff_reads);
        Serial.println(debug_data_transfer);
        Serial.println(debug_w_empty);
        Serial.println(debug_w_full);
        DEBUG_RESET;
    }
    if(buffer->isEmpty())
        debug_w_empty = true;
    if(buffer->isFull())
        debug_w_full = true;
    #endif
    
    if(buffer->length() == 0){
        return;
    }
    int res = buffer->safe_read();
    if(res == -1) 
        return;
    byte d = res;
    
    #ifdef DEBUG
    debug_buff_reads++;
    #endif
    
    
    for(uint i = 0; i < 4; i++){
        
        //Unpack & shift
        byte s1 = d & 1;
        byte s2 = (d >> 1) & 1;
        d = d >> 2;
        
        //Whitening
        if(s1 == s2)
            continue;
        
        //The random bit
        byte rb = s1;
        
        //Heartbeat
        if(last_ping > PING_INTERVAL){
            last_ping = 0;
            if(rb)
                digitalWriteFast(PING_PIN, HIGH);
            else
                digitalWriteFast(PING_PIN, LOW);
            continue;
        }
        
        //Build base64 char
        int res = base64_builder(rb);
        //If new char, then put in out buffer
        if(res != -1)
            out_buff[out_p++] = res;
        
        //If a line, then dump
        if(out_p == OUT_SIZE) {
            #ifdef DEBUG
            debug_data_transfer += OUT_SIZE;
            #else
            Serial.write(out_buff, OUT_SIZE+1);
            #endif
            out_p = 0;
        }
    }

}

int base64_builder(byte next_bit){
    static byte accum = 0;
    static byte a_len = 0;
    
    int out = -1;
    
    accum = accum << 1;
    accum += next_bit;
    a_len++;
    
    if(a_len == 6){
        out = base64enc(accum);
        accum = 0;
        a_len = 0;
    }
    
    return out;
}


void adc0_isr(void) {
    static byte accum = 0;
    static byte a_len = 0;
    
    noInterrupts();
    
    byte v = 1 & adc->analogReadContinuous(ADC_0);
    
    accum = accum << 1;
    accum += v;
    a_len++;
    
    if(a_len == 8){
        buffer->write(accum);
        accum = 0;
        a_len = 0;
    }
    
    interrupts();
}