// Arduino nano
// Sends random data through serial line,
// for use in SBC or VMs
//
// Use the following guides to understand and seed /dev/random
// https://security.stackexchange.com/a/69433
// https://www.certdepot.net/rhel7-get-started-random-number-generator/

const int source[] = {A0, A1, A2, A3, A4, A5, A6, A7};
const int size = sizeof(source)/sizeof(int);

const int status_pin = 13;

const unsigned long refresh = 100;
unsigned long last = 0;

void setup() {
    Serial.begin(9600);
    pinMode(status_pin, OUTPUT);
    digitalWrite(status_pin, LOW);
}

byte accumulate = 0;
char out = 0;

void loop() {
  
    byte s1 = get();
    byte s2 = get();
    
    if(s1 == s2) return;
    
    if( millis() - refresh > last ) {
        digitalWrite(status_pin, s1);
        last = millis();
        return;
    }
    
    out = out << 1;
    out += s1;
    accumulate++;
    
    if(accumulate == 8) {
        Serial.print(out);
        out = 0;
        accumulate = 0;
    }
    
}


byte get(){
    byte e = 0;
    for(byte i = 0; i < size; i++)
        e ^= 1 & analogRead(source[i]);
    return e;
}
