//A Hi - Pin 10- PB2 - OC1B
//B Hi - Pin 9 - PB1 - OC1A
//C Hi - Pin 3 - PD3 - OC2B
//A Lo - Pin 4 - PD4
//B Lo - Pin 5 - PD5
//C Lo - Pin 6 - PD6

#define A_HI_OCR OCR1B
#define B_HI_OCR OCR1A
#define C_HI_OCR OCR2B

#define A_LO_OFF  PORTD &= ~(1<<4)
#define A_LO_ON   PORTD |= (1<<4)
#define B_LO_OFF  PORTD &= ~(1<<5)
#define B_LO_ON   PORTD |= (1<<5)
#define C_LO_OFF  PORTD &= ~(1<<6)
#define C_LO_ON   PORTD |= (1<<6)

#define THROTTLE_PIN A7
#define THROTTLE_LOW 190
#define THROTTLE_HIGH 850

#define HALL_1_PIN A1
#define HALL_2_PIN A2
#define HALL_3_PIN A3

#define HALL_OVERSAMPLE 4

//uint8_t hallToMotor[8] = {255, 255, 255, 255, 255, 255, 255, 255};
uint8_t hallToMotor[8] = {255, 3, 5, 4, 1, 2, 0, 255}; 

void setup() {
  Serial.begin(115200);
  
  pinMode(10, OUTPUT); //set all PWM pins as output
  pinMode(9, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  pinMode(HALL_1_PIN, INPUT); //set the hall pins as input
  pinMode(HALL_2_PIN, INPUT);
  pinMode(HALL_3_PIN, INPUT);

  TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<WGM10);//phase correct 8 bit
  TCCR2A = (1<<COM2B1) | (1<<WGM20);
  
  TCCR1B = (1<<CS10);
  TCCR2B = (1<<CS20); //no prescaler, pwm at 32khz

  TCNT2 = 0;
  TCNT1 = 0;
  
  identifyHalls();
}

void loop() {
  uint8_t throttle = readThrottle();//only do this occasionally because its slow
  for(uint8_t i = 0; i < 200; i++)
  {  
    uint8_t hall = getHalls();
    uint8_t motorState = hallToMotor[hall];
    writePWM(motorState, throttle);
  }
}

void identifyHalls()
{
  for(uint8_t i = 0; i < 6; i++)
  {
    uint8_t prevState = (i + 1) % 6;
    for(uint8_t j = 0; j < 50; j++)
    {
      delay(10);
      writePWM(i, 20);
      delay(10);
      writePWM(prevState, 20);
    }
    hallToMotor[getHalls()] = (i + 2) % 6;
  }
  
  writePWM(0, 0);//turn phases off
  
  for(uint8_t i = 0; i < 8; i++)
  {
    Serial.print(hallToMotor[i]);
    Serial.print(", ");
  }
  Serial.println();

  //while(1);
}

void writePWM(uint8_t motorState, uint8_t dutyCycle)
{
  if(dutyCycle == 0)
    motorState = 255;//if zero throttle, turn all switches off
  
  switch(motorState){
    case 0://LOW A, HIGH B
      B_LO_OFF; C_LO_OFF; A_LO_ON;
      A_HI_OCR = 0; C_HI_OCR = 0; B_HI_OCR = dutyCycle;
      break;
    case 1://LOW A, HIGH C
      B_LO_OFF; C_LO_OFF; A_LO_ON;
      A_HI_OCR = 0; B_HI_OCR = 0; C_HI_OCR = dutyCycle;
      break;
    case 2://LOW B, HIGH C
      A_LO_OFF; C_LO_OFF; B_LO_ON;
      A_HI_OCR = 0; B_HI_OCR = 0; C_HI_OCR = dutyCycle;
      break;
    case 3://LOW B, HIGH A
      A_LO_OFF; C_LO_OFF; B_LO_ON;
      B_HI_OCR = 0; C_HI_OCR = 0; A_HI_OCR = dutyCycle;
      break;
    case 4://LOW C, HIGH A
      A_LO_OFF; B_LO_OFF; C_LO_ON;
      B_HI_OCR = 0; C_HI_OCR = 0; A_HI_OCR = dutyCycle;
      break;
    case 5://LOW C, HIGH B
      A_LO_OFF; B_LO_OFF; C_LO_ON;
      A_HI_OCR = 0; C_HI_OCR = 0; B_HI_OCR = dutyCycle;
      break;
    default://all off
      A_LO_OFF; B_LO_OFF; C_LO_OFF;
      A_HI_OCR = 0; B_HI_OCR = 0; C_HI_OCR = 0;
  }
}

uint8_t getHalls()
{
  uint8_t hallCounts[] = {0, 0, 0};
  for(uint8_t i = 0; i < HALL_OVERSAMPLE; i++) //read all the hall pins repeatedly, tally results 
  {
    hallCounts[0] += (PINC >> 1) & 0x01;
    hallCounts[1] += (PINC >> 2) & 0x01;
    hallCounts[2] += (PINC >> 3) & 0x01;
  }

  uint8_t hall = 0;
  
  if (hallCounts[0] >= HALL_OVERSAMPLE / 2) //if votes >= threshold, call that a 1
    hall |= (1<<0);
  if (hallCounts[1] >= HALL_OVERSAMPLE / 2)
    hall |= (1<<1);
  if (hallCounts[2] >= HALL_OVERSAMPLE / 2)
    hall |= (1<<2);

  return hall & 0x7;
}

/*uint8_t getHallsSimple()
{
  uint8_t hall = PINC >> 1;
  return hall & 0x7;
}*/

uint8_t readThrottle()
{
  int32_t adc = analogRead(THROTTLE_PIN); //analogRead takes about 100us
  
  adc = (adc - THROTTLE_LOW) << 8;
  adc = adc / (THROTTLE_HIGH - THROTTLE_LOW);

  if (adc > 255)
    return 255;

  if (adc < 0)
    return 0;
  
  return adc;
}

