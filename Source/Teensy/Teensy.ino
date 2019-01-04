#include <ADC.h>

typedef unsigned int uint;
typedef unsigned long ulong;

// Pin I/O
const int NOISE_SOURCE = A9; //TODO set pin, I think A0-A9 is valid


//Internal tuning
const uint BUF_LENGTH = 128; // How large is each ping-pong buffer
const uint RAW_BUF_LENGTH = BUF_LENGTH * 8 * 2;
const uint BUF_TX_TRIG = 100 * 8 * 2;  // When the buffer has this many bytes in it, switch to the other buffer, and transmit now.

const ulong PING_INTERVAL = 100;
const uint PING_PIN = 13; //LED pin
ulong last_ping = 0;

//Global variables

byte bufferA[RAW_BUF_LENGTH];
byte bufferB[RAW_BUF_LENGTH];
uint bufptrA = 0;
uint bufptrB = 0;
byte usebufA = 1; // usebufA is used as a boolean

ADC *adc = new ADC();

void setup()
{
  pinMode(PING_PIN, OUTPUT);

  for (uint i=0; i < NVIC_NUM_INTERRUPTS; i++) NVIC_SET_PRIORITY(i, 128);  //The Teensy LC only uses interrupt priorities of 0, 64, 128, 192, 255
  NVIC_SET_PRIORITY(IRQ_ADC0,64);  // The ADC ISR must have a higher priority (lower number) than the USB ISR

  Serial.begin(9600); // USB is always 12 Mbit/sec

  ///// ADC0 ////
  // reference can be ADC_REF_3V3, ADC_REF_1V2 (not for Teensy LC) or ADC_REF_EXT.
  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_0);

  adc->setAveraging(1); // set number of averages
  adc->setResolution(16); // set bits of resolution

  // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED_16BITS, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
  // see the documentation for more information
  // additionally the conversion speed can also be ADC_ADACK_2_4, ADC_ADACK_4_0, ADC_ADACK_5_2 and ADC_ADACK_6_2,
  // where the numbers are the frequency of the ADC clock in MHz and are independent on the bus speed.
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);

  // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed

  // always call the compare functions after changing the resolution!
  //adc->enableCompare(1.0/3.3*adc->getMaxValue(ADC_0), 0, ADC_0); // measurement will be ready if value < 1.0V
  //adc->enableCompareRange(1.0*adc->getMaxValue(ADC_0)/3.3, 2.0*adc->getMaxValue(ADC_0)/3.3, 0, 1, ADC_0); // ready if value lies out of [1.0,2.0] V
  //  In this case, we only want the raw byte vales from the ADC.  No need for scaling.

  adc->startContinuous(NOISE_SOURCE, ADC_0);
  adc->enableInterrupts(ADC_0);
}


void loop()
{
  byte *in_buff;
  uint in_buff_len;

  if(bufptrA > BUF_TX_TRIG && usebufA == 1)
  {
    __disable_irq();
    usebufA = 0;
    bufptrB = 0;
    __enable_irq();
    in_buff = bufferA;
    in_buff_len = bufptrA;
  }
  else if(bufptrB > BUF_TX_TRIG && usebufA == 0)
  {
    __disable_irq();
    usebufA = 1;
    bufptrA = 0;
    __enable_irq();
    in_buff = bufferB;
    in_buff_len = bufptrB;
  }
  else
    return;

  // Accumulate 1 byte
  byte b_acc = 0;
  byte b_size = 0;
  // Accumulate bytes for output
  byte out_buff[BUF_LENGTH];
  uint out_pos = 0;
  // Iterator position in in_buff
  uint in_pos = 0;

  //Will iterate by 2's
  while(in_pos < in_buff_len - 1)
  {
    //Get two bits
    byte s1 = in_buff[in_pos++];
    byte s2 = in_buff[in_pos++];

    //Whitening
    if(s1 == s2)
      continue;

    //Heartbeat
    ulong t = millis();
    if(t - last_ping  > PING_INTERVAL)
    {
      last_ping = t;
      if(s1)
        digitalWriteFast(PING_PIN, HIGH);
      else
        digitalWriteFast(PING_PIN, LOW);
      continue;
    }

    //Accumulate into byte
    b_acc = b_acc << 1;
    b_acc += s1;
    b_size++;

    //Is a base64 char?
    if(b_size != 6)
      continue;

    //Place full byte in output buffer
    if(b_acc < 10)
        b_acc += '0';
    else if(b_acc < 10+ 26)
        b_acc += 'A' -10;
    else if(b_acc < 10+26+ 26)
        b_acc += 'a' -10-26;
    else if(b_acc < 10+26+26+ 1)
        b_acc = '/';
    else
        b_acc = '+';
        
    out_buff[out_pos++] = b_acc;
    b_acc = 0;
    b_size = 0;

  }

  Serial.write(out_buff, out_pos);
  //for(uint i=0; i < out_pos; i++) Serial.println(out_buff[i]);

}


void adc0_isr(void) {
  __disable_irq();
  byte v = 1 & adc->analogReadContinuous(ADC_0);
  if(usebufA == 1 && bufptrA < RAW_BUF_LENGTH)
    bufferA[bufptrA++] = v;
  if(usebufA == 0 && bufptrB < RAW_BUF_LENGTH)
    bufferB[bufptrB++] = v;
  __enable_irq();
}