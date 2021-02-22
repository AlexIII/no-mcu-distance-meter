/**
 * No-MCU Ultrasonic Distance Meter - ROM programmer (W27C512)
 * Arduino mega2560
 */

// Select ONE of the folowing:
//#define CHIP_EARSE
#define CHIP_PROGRAM

//------- ROM connections ------

// ROM OE pin
#define ENABLE_14V_PIN  38  //14v OE pin control (HIGH - enabled)
#define ENABLE_12V_PIN  39  //12v OE pin control (HIGH - enabled)
#define ENABLE_5V_PIN   40  //5v OE pin control (HIGH - enabled)
#define DISABLE_HIGH_VOLTAGE do {digitalWrite(ENABLE_14V_PIN, LOW); digitalWrite(ENABLE_12V_PIN, LOW); digitalWrite(ENABLE_5V_PIN, LOW);} while(0)
#define OE_HIGH do {DISABLE_HIGH_VOLTAGE; digitalWrite(ENABLE_5V_PIN, HIGH);} while(0)
#define OE_LOW do {DISABLE_HIGH_VOLTAGE; digitalWrite(ENABLE_5V_PIN, LOW);} while(0)
#define PE_HIGH do {DISABLE_HIGH_VOLTAGE; digitalWrite(ENABLE_12V_PIN, HIGH);} while(0)
#define EE_HIGH do {DISABLE_HIGH_VOLTAGE; digitalWrite(ENABLE_14V_PIN, HIGH);} while(0)

// ROM CE pin
#define CE_PIN          41
#define CE_HIGH digitalWrite(CE_PIN, HIGH)
#define CE_LOW digitalWrite(CE_PIN, LOW)

// ROM A0-A7 => Arduino 22-29 (PORTA)
// ROM A8-A15 => Arduino 37-30 (yes, backwards) (PORTC)
#define SET_ADDR(addr) do{ PORTA = uint8_t(addr); PORTC = uint16_t(addr) >> 8; } while(0)
// ROM Q0-Q7 => Arduino 49-42 (yes, backwards) (PORTL)
#define GET_DATA() (DDRL = 0x00, PORTL = 0x00, PINL)
#define SET_DATA(data) do{ PORTL = data; DDRL = 0xFF; } while(0)

//------- ------

uint8_t eeRead(const uint16_t addr) {
  OE_HIGH;        //disable outputs and possible program/erase mode
  GET_DATA();     //set port to input
  CE_LOW;         //enable chip
  SET_ADDR(addr); //set address
  OE_LOW;         //enable outputs
  _delay_us(2);
  const uint8_t data = GET_DATA();  //read data
  OE_HIGH;        //disable outputs
  return data;
}

void eeProgram(const uint16_t addr, const uint8_t data) {
  CE_HIGH;        //disable chip
  SET_ADDR(addr); //set address
  PE_HIGH;        //set programming voltage
  SET_DATA(data); //set data
  _delay_us(2);

  //programming pulse
  CE_LOW;
  _delay_us(200);
  CE_HIGH;

  OE_HIGH;        //disable programming voltage
}

void eeErase() {
  CE_HIGH;        //disable chip
  SET_ADDR(0x0200U);
  EE_HIGH;        //set erasing voltage
  SET_DATA(0xFF);
  _delay_us(2);
  
  //erasing pulse
  CE_LOW;
  _delay_ms(200);
  CE_HIGH;

  OE_HIGH;        //disable erasing voltage
}

bool checkErased(const uint16_t chipSize) {
  bool res = true;
  for(uint16_t addr = 0; addr < chipSize || chipSize == 0; ++addr) {
    const uint8_t data = eeRead(addr);
    if(data != 0xFF) {
      res = false;
      Serial.print(F("Erase check error at 0x"));
      Serial.print(addr, HEX);
      Serial.print(F(" Data = "));
      Serial.print(int(data), HEX);
      Serial.print(F(" (expected 0xff)"));
      Serial.println();
      break;
    }
    if(chipSize == 0 && addr == 0xFFFF) break;
  }
  return res;
}

#define CHIP_SIZE uint16_t(0)

void setup() {
  Serial.begin(250000);
  Serial.println(F("Started."));
  
  //control
  pinMode(ENABLE_14V_PIN, OUTPUT);
  pinMode(ENABLE_12V_PIN, OUTPUT);
  pinMode(ENABLE_5V_PIN, OUTPUT);
  pinMode(CE_PIN, OUTPUT);
  CE_HIGH;   //disable chip select
  OE_HIGH;  //disable outputs

  //address
  DDRA = 0xFF;
  DDRC = 0xFF;
  
  DISABLE_HIGH_VOLTAGE;

  delay(10);

#if defined(CHIP_EARSE)
  eeErase();
  delay(10);
  const bool eraseOk = checkErased(CHIP_SIZE);
  if(eraseOk) Serial.println(F("Erase check ok."));
  else Serial.println(F("Erase check error."));
#elif defined(CHIP_PROGRAM)
  programBinToDecDecoder(true);
  Serial.println(F("Write finished."));
  const bool writeOk = programBinToDecDecoder(false);
  if(writeOk) Serial.println(F("Write check ok."));
  else Serial.println(F("Write check error."));
#else
  Serial.println(F("No operation selected."));
#endif

  Serial.println(F("Done."));

//  for(int i = 0; i < 0x100; ++i) {
//    Serial.print((int)eeRead(i), HEX);
//    Serial.print(' ');
//  }
  
  Serial.println();
}


void loop() {
}



// Binary to decimal decoder
uint8_t decimalDigitToSegmentCode(const uint8_t digit) {
  switch(digit) {
    case 0: return 0b0111111;
    case 1: return 0b0000110;
    case 2: return 0b1011011;
    case 3: return 0b1001111;
    case 4: return 0b1100110;
    case 5: return 0b1101101;
    case 6: return 0b1111101;
    case 7: return 0b0000111;
    case 8: return 0b1111111;
    case 9: return 0b1101111;
    default: return 0b1000000; // "-"
  }  
}

bool programBinToDecDecoder(const bool programm) {
  bool checkOk = true;
  uint8_t dpow = 1; //powers of 10
  for(uint8_t dp = 0; dp < 4; ++dp) { //digit place
    for(uint16_t n = 0; n < 0x100; ++n) { //for all input values 0..255
      const uint16_t addr = ((
          dp == 0b00? 0b10:
          dp == 0b01? 0b01:
          dp == 0b10? 0b00:
                      0b11
        ) << 8) | n;  // input dd vvvvvvvv
      const uint8_t digitCode = decimalDigitToSegmentCode( n == 255? 0xFF : ((n/dpow)%10) );  //7-bit output code for a digit
      const uint8_t data = (dp == 3? 0 : digitCode) | (((~addr) >> 2) & 0x80);
      if(programm) eeProgram(addr, data);
      else {
        const uint8_t rd = eeRead(addr);
        if(rd != data) {
          Serial.print(F("Write check error at 0x"));
          Serial.print(addr, HEX);
          Serial.print(F(" Read = 0x"));
          Serial.print(int(rd), HEX);
          Serial.print(F(", expected 0x"));
          Serial.print(int(data), HEX);
          Serial.println();
          checkOk = false;
          break;
        }
      }
      
    }
    if(!checkOk) break;
    dpow *= 10;
  }

  return checkOk;
}
