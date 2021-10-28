#include <IRremote.h>

#define ADDRESS 0x01
#define EXIT 0x065
#define HELP 0x068
#define LIST 0x06C
#define PROGRAM 8
#define RECEIVER 2
#define RETURN 0x0A
#define WRITE 0x077

bool alert = false;
bool alarm = false;
int bytes = 0;
uint8_t input[30];
int lc = 0;
int fw_version = 1;
bool learn = true;
int n = 0;
int occ[4][4]={
    {0,0},
    {0,0},
    {0,0},
    {0,0}
};
bool program_mode = false;
int samples[4] = {0,0,0,0};
long tc = 0;
long timer = 1500000;
int valid_codes[3] = {521,537,57};
int wn = 0;


IRrecv irrecv(RECEIVER);
decode_results results;



void setup() {
  
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

void loop() {
  // Incoming codes
  if (learn){
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
        wn = occ[1][0];
    } else {
        Serial.println("Equal compare: Try again");
        wn = 0;
    }
    lc=0;
    Serial.println(wn);
  }
  if (irrecv.decode(&results)){
    Serial.println(results.value);
  for (int i=0; i <= sizeof(valid_codes)/2; i++){
      if (results.value == int(valid_codes[i])){
        Serial.print("OK : ");
        Serial.println(valid_codes[i]);
        alert = true;
        digitalWrite(LED_BUILTIN,HIGH);
      }
    }
    irrecv.resume();
  } 
 
  if (alert && !alarm){
    alarm = true;
    tc = 0;
  } else if (alert && alarm){
    tc++;
    if (tc > timer){
      alarm = false;
      alert = false;
      digitalWrite(LED_BUILTIN,LOW);
    }
  }
}

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
