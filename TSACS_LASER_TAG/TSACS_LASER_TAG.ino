/*
###Modes of operation###
Learn - (enabled): learn new codes and write codes into memory Max 30. (disabled): normal mode.
Strict - (enabled): codes must be programmed in memory. (disabled): codes must be between 50 - 999.

*/



#include "EEPROM.h"
#include <IRremote.h>

#define CODE_COUNT_ADDRESS 0x00
#define EXIT 0x065
#define HELP 0x068
#define LIST 0x06C
#define MAX_CODES 0x0A
#define PROGRAM 8
#define RECEIVER 2
#define RETURN 0x0A
#define WRITE 0x077

int ADDRESS = CODE_COUNT_ADDRESS;
int RADDRESS = CODE_COUNT_ADDRESS;
bool alarm = false;
int bytes = 0;
byte cc = 0;
int lc = 0;
int fw_version = 1;
bool learn = false;
int n = 0;
int occ[4][2]={
    {0,0},
    {0,0},
    {0,0},
    {0,0}
};
bool program_mode = false;
int samples[4] = {0,0,0,0};
bool strict = true;
long tc = 0;
long timer = 750000;
int valid_codes[30] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0};
int wn = 0;


IRrecv irrecv(RECEIVER);
decode_results results;



void setup() {
  //EEPROM.update(0,0);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PROGRAM, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  
  digitalWrite(3, HIGH);
  digitalWrite(4, LOW);
  
  Serial.begin(9600);

  if (digitalRead(8) == LOW){
    program();
  }
  
  irrecv.enableIRIn();

  Serial.print("READY\n");
}


// Allows user to marry laser gun with target.

void learnCodes(){
   Serial.println("Current stored codes: ");
    int csc = 0;
    for (int i=0; i<=9; i++){
      RADDRESS+=2;
     csc = readCodes(RADDRESS);
      Serial.print(RADDRESS);
      Serial.print(": ");
      Serial.println(csc, DEC);
    }
    RADDRESS = CODE_COUNT_ADDRESS;
    while (1){
      if (irrecv.decode(&results)){
        Serial.print("LEARNING: ");
        Serial.println(results.value);
        samples[lc]=results.value;
        results.value=0;
        lc++;
        irrecv.resume();
      }
      if (lc == 4)
        break;
    }
       for (int i=0; i<=3; i++){
        n = samples[i];
        occ[i][0]=samples[i];
        for (int j=0; j<=3; j++){
            if (n == int(samples[j])){
                occ[i][1] += 1;
            }
        }
    }
    if (occ[0][1] > occ[1][1] || occ[0][1] > occ[2][1] || occ[0][1] > occ[3][1]){
        wn = occ[0][0];
    } else if (occ[1][1] > occ[0][1] || occ[1][1] > occ[2][1] || occ[1][1] > occ[3][1]){
        wn = occ[1][0];
    } else if (occ[2][1] > occ[3][1] || occ[2][1] > occ[0][1]){
        wn = occ[2][0];
    }else if (occ[0][0] == occ[1][0] && occ[1][0] == occ[2][0] && occ[3][0] == occ[0][0]){
        wn = occ[0][0];
    } else {
        wn = 0;
    }
    lc=0;
    Serial.println(wn);
    // EEPROM STORE
    EEPROM.get(CODE_COUNT_ADDRESS, cc);
    if (cc >= MAX_CODES){
      Serial.println("ERROR: You have reach the maximum stored valid codes!"); //TODO: Give user option to clear these.
    } else {
      Serial.print("Current code count: ");
      ADDRESS += 2;
      cc++;
      (writeCodes(ADDRESS, wn)) ? ADDRESS -= 2 : EEPROM.put(CODE_COUNT_ADDRESS, cc);
      Serial.println(cc,DEC);
    }
}


void loop() {

  // Learn new codes
  while (learn){
    learnCodes();
  }

  // Grab valid codes from memory.
  if (valid_codes[0] == 0){
    for (int i=0; sizeof(valid_codes)/2; i++){
      valid_codes[i] = readCodes(ADDRESS+=2);
      Serial.print(i);
      Serial.print(" CODES: ");
      Serial.println(valid_codes[i]);
    }
  }

  
  if (irrecv.decode(&results)){
  for (int i=0; i <= sizeof(valid_codes)/2; i++){
      if (results.value == valid_codes[i] && !alarm && strict){
        alarm = true;
        tc=0;
        digitalWrite(LED_BUILTIN,HIGH);
      } else if (results.value > 50 && results.value < 999 && !alarm){
        alarm = true;
        tc=0;
        digitalWrite(LED_BUILTIN,HIGH);
      }
    }
    irrecv.resume();
  } 

  // Alarm activated 
  if (alarm){
    tc++;
    if (tc > timer){
      alarm = false;
      digitalWrite(LED_BUILTIN,LOW);
    }
  }
}

// Serial programming mode
void program(){
  
  Serial.print("Welcome to the TSACS target machine\n(C) James Colderwood 2021 Version: ");
  Serial.println(fw_version, DEC);

  while (1) {
    if (Serial.available() > 0){
      bytes = Serial.read();
      Serial.println(bytes, HEX);
      switch (bytes){
        case EXIT:
        Serial.println("GoodBye");
        break;
        case HELP:
        Serial.print("(e) EXIT\n(h) HELP(This menu)\n(l) List valid codes\n");
        break;
        case LIST:
        Serial.println("Listing Valid Codes: ");
        for (int i=0; i < sizeof(valid_codes)/2; i++){
          Serial.println(valid_codes[i], DEC);
        }
        Serial.println("DONE");
        break;
        case RETURN:
        continue;
        case WRITE:
        Serial.println("WARNING: This will overwrite previous valid codes, you can list these codes by pressing L. If you wish to continue, please enter new valid codes");
        Serial.println("TODO: NOT YET IMPLEMENTED");
        break;
        default:
        Serial.println("UNKNOWN COMMAND");
    }
    
    }
    
  }

}

int readCodes(int address){
  
  byte b1 = EEPROM.read(address);
  byte b2 = EEPROM.read(address+1);
  
  return (b1 << 8) + b2;
  
}

int writeCodes(int address, int number){
  if (number > 65535){
    Serial.println("ERROR: Sorry this code exceeds memory size limit. Please try again.");
    return -1;
  }
    byte b1 = (number >> 8) & 0xFF;
    byte b2 = number & 0xFF;
    
    EEPROM.update(address, b1);
    EEPROM.update(address+1, b2);
    
    return 0;
}
