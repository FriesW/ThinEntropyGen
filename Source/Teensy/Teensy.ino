typedef unsigned int uint;
typedef unsigned long ulong;

#include <ADC.h>
#include "VolatileFIFO.h"
#include "Base64Encode.h"

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

void setup()
{
    //Finish initialization
    out_buff[OUT_SIZE] = '\n';
    
    //Actual setup
    
    pinMode(PING_PIN, OUTPUT);

    for (uint i=0; i < NVIC_NUM_INTERRUPTS; i++) NVIC_SET_PRIORITY(i, 128);  //The Teensy LC only uses interrupt priorities of 0, 64, 128, 192, 255
    NVIC_SET_PRIORITY(IRQ_ADC0,64);  // The ADC ISR must have a higher priority (lower number) than the USB ISR

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
    if(buffer->est_length() == 0)
        return;
    int res = buffer->safe_read();
    if(res == 0)
        return;
    byte d = res;
    
    
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
            Serial.write(out_buff, OUT_SIZE+1);
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