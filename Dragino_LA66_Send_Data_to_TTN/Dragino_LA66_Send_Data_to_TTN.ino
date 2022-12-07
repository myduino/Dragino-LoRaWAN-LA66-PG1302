#include <SoftwareSerial.h>

String inputString = "";
bool stringComplete = false;

long previousMillis = millis();
long uplink_interval = 30000;

bool time_to_at_recvb = false;
bool get_LA66_data_status = false;

bool network_joined_status = false;

SoftwareSerial LA66(10, 11);

char rxbuff[128];
uint8_t rxbuff_index=0;

void setup() {
  // initialize serial
  Serial.begin(9600);

  LA66.begin(9600);
  LA66.listen();

  LA66.println("ATZ");  // Reset LA66 MCU
  
  inputString.reserve(200);
  
}

void loop() {

  if((millis() - previousMillis >= uplink_interval) && (network_joined_status == 1)){
    previousMillis = millis();
    get_LA66_data_status = false;

    int randomNumberOne = random(28, 34);
    int randomNumberTwo = random(51, 57);

    Serial.print(F("Random Number 1: "));
    Serial.print(randomNumberOne);
    Serial.print(F("Random Number 2: "));
    Serial.print(randomNumberTwo);
    
    char sensor_data_buff[128] = "\0";

    snprintf(sensor_data_buff, 128, "AT+SENDB=%d,%d,%d,%02X%02X%02X%02X", 0, 2, 4, (short)(randomNumberOne*100)>>8 & 0xFF, (short)(randomNumberOne*100) & 0xFF, (short)(randomNumberTwo*10)>>8 & 0xFF, (short)(randomNumberTwo*10) & 0xFF);
    LA66.println(sensor_data_buff);
  }

  if(time_to_at_recvb == true){
    time_to_at_recvb = false;
    get_LA66_data_status = true;
    delay(1000);
    
    LA66.println("AT+CFG");    
  }

  while (LA66.available()) {
    char inChar = (char) LA66.read();
    inputString += inChar;

    rxbuff[rxbuff_index++]=inChar;

    if(rxbuff_index>128)
    break;
    
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
      rxbuff[rxbuff_index]='\0';
      
      if(strncmp(rxbuff, "JOINED", 6) == 0){
        network_joined_status=1;
      }

      if(strncmp(rxbuff, "Dragino LA66 Device", 19) == 0){
        network_joined_status=0;
      }

      if(strncmp(rxbuff, "Run AT+RECVB=? to see detail", 28) == 0){
        time_to_at_recvb=true;
        stringComplete=false;
        inputString = "\0";
      }

      if(strncmp(rxbuff,"AT+RECVB=",9)==0){       
        stringComplete=false;
        inputString = "\0";
        Serial.print("\r\nGet downlink data(FPort & Payload) ");
        Serial.println(&rxbuff[9]);
      }
      
      rxbuff_index=0;

      if(get_LA66_data_status==true){
        stringComplete=false;
        inputString = "\0";
      }
    }
  }

  while ( Serial.available()) {
    char inChar = (char) Serial.read();
    inputString += inChar;
    if (inChar == '\n' || inChar == '\r') {
      LA66.print(inputString);
      inputString = "\0";
    }
  }

  if (stringComplete) {
    Serial.print(inputString);
    
    inputString = "\0";
    stringComplete = false;
  }
}