/*
Generates entropy/noise based on ADC LSB.
Outputs in base64 encoded lines.
Probably very very slow.

Tolchain: Arduino (1.8.5)
Board: Arduino Nano/Uno or ATmega328P based
*/

typedef unsigned int uint;
typedef unsigned long ulong;

#include "Base64Encode.h"


#define NOISE_SOURCE A0

#define PING_INTERVAL 100 //msec
#define PING_PIN 13 //LED pin
ulong last_ping = 0;

//Output
#define OUT_SIZE 76 //Base64 chars per line
#define BAUD 115200

void setup() {
    Serial.begin(BAUD);
    pinMode(PING_PIN, OUTPUT);
}


void loop() {
    
    uint lw = 0;
    while(lw < OUT_SIZE){
        byte s1 = get();
        byte s2 = get();
        
        //Whitening
        if(s1 == s2) continue;
        
        //The random bit
        byte rb = s1;
        
        //Heartbeat
        ulong t = millis();
        if(t - last_ping > PING_INTERVAL){
            last_ping = t;
            digitalWrite(PING_PIN, rb);
            continue;
        }
        
        //Build base64 char
        int res = base64_builder(rb);
        
        //Print new chars
        if(res != -1){
            lw++;
            Serial.print((char)res);
        }
        
    }
    Serial.println();
    
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

byte get(){
    return 1 & analogRead(NOISE_SOURCE);
}
