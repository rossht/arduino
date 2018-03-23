/*****************************************
  Author: Ross Taylor 
  Date: 16/05/2015
 
  An efficient AC phase cutting PWM for mains lighting and motors etc...
 
  Project hardware follows the Instructables circuit design: http://www.instructables.com/id/Arduino-controlled-light-dimmer-The-circuit/
  
  The script:
  'AC_in' polls the signal and triggers the interrupt routine to reset the overflow counter for timer2, dependent on the lighting 
  level required, and triggers the TRIAC on for a short time (around 0.6 ms, also timed using timer2 and the if/else statement)
  before turning TRIAC off.
  
*******************************************/

int TRIAC = 3;        //the triac is on this pin
int AC_in = 2;        //AC signal in for polling
float brightness = 0.4;    // initialise brightness level as some value in [0,1]

float t1 = (1.0-brightness)*0.01f;    // t1 is the time interval, in seconds, between an AC zero crossing and firing the triac.
int timer_t1 = 256-(int)(t1*15625.0f);  // note that multiplying by 15625 is the same as dividing by 6.4e-5 ( = 1024/16e6 )

volatile boolean delayToggle = false;

// initialisation for example main loop:
float t = 0;

void setup(){
  pinMode(AC_in, INPUT);
  pinMode(TRIAC,OUTPUT);    //set triac output pin

  cli();//disable interrupts
  TCCR2A = 0;               // set entire TCCR0A register to 0
  TCCR2B = 0;               // same for TCCR0B
  TCNT2  = 0;               //initialize counter value to 0
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  // set a prescale of 1024
  TIMSK2 |= (1 << TOIE2);   // enable timer overflow interrupt
  
  sei();//enable interrupts
  
  attachInterrupt(0, set_t1, RISING);
}

void set_t1(){               // called on external interrupt (zero crossing)
  TCNT2 = timer_t1;
}

ISR(TIMER2_OVF_vect){       // timer2 overflow interrupt routine
  if (delayToggle == false)  {
    digitalWrite(TRIAC, HIGH);
    delayToggle = true;
    TCNT2 = 247;            // set timer for t2
  }
  else  {
    digitalWrite(TRIAC, LOW);
    delayToggle = false;
  }
}

void loop(){
  // anything can go in the main loop here, just set 'timer_t1' to some value in [0,255] to set the dimming level.
  // The following code makes a bulb fade in and out sinusoidally:
  float f = 1.0f;
  brightness = 0.5f + 0.4f*sin(2.0f*3.14159f*f*t);
  t1 = (1-brightness)*0.01f;
  timer_t1 = 256-(int)(t1*15625.f);
  delay(10);
  t = t + 0.01f;
}
