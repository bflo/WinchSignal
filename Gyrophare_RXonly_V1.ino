/*
 * LoRa E32-TTL-100
 * Write on serial to transfer a message to other device
 * https://www.mischianti.org
 *
 * E32-TTL-100----- Arduino UNO
 * M0         ----- GND
 * M1         ----- GND
 * TX         ----- PIN 2 (PullUP)
 * RX         ----- PIN 3 (PullUP & Voltage divider)
 * AUX        ----- Not connected
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 * Doc configuration : https://www.mischianti.org/2019/10/21/lora-e32-device-for-arduino-esp32-or-esp8266-library-part-2/
 */
#include "Arduino.h"
// model E32 900T30
#define E32_TTL_1W

// You can configure Channel frequency also with this define:
#define FREQUENCY_868

// PIN branchement du relais
#define RELAIS 6
#define GYRO_ON 11
#define GYRO_OFF 88

#include "LoRa_E32.h"




LoRa_E32 e32ttl100(2, 3); // e32 TX e32 RX
 
void setup() {

  // INIT RELAI //
  pinMode(RELAIS, OUTPUT);

  // INIT LORA //
  ResponseStructContainer c;
  Serial.begin(9600);
  delay(500);
 
  // Startup all pins and UART
  e32ttl100.begin();
 
  // Get Configuration 
  c = e32ttl100.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  //Serial.println(c.status.code);
  //printParameters(configuration);
  // CONFIGURATION DU MODULE EBYTE 

  configuration.ADDL = 3;
  configuration.ADDH = 0;
  configuration.CHAN = 0x04;
  
  // FEC ON pour augmenter l'anti-interférence
  configuration.OPTION.fec = FEC_1_ON; //FEC_0_OFF

  // Transmission transparent : broadcast à tt le monde qui écoute
  configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
  
  // IO drive mode: this bit is used for the module internal pull-up resistor. 
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS; //default
  
  //Puissance - Max = 30dB
  configuration.OPTION.transmissionPower = POWER_30;

  // inutile en mode transparent mais bon..
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  
  //The lower the air data rate, the longer the transmitting distance
  //data rate minimal = 0.3k
  configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;

  configuration.SPED.uartBaudRate = UART_BPS_9600; //Default
  configuration.SPED.uartParity = MODE_00_8N1; //Default

  //Send the configuration to the device
  e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  printParameters(configuration);
	c.close();

  // Send message
  Serial.println("Hi, I'm going to send message!");
  ResponseStatus rs = e32ttl100.sendMessage("Hello, world ! ici le gyrophare");
  // Check If there is some problem of successfully send
  Serial.println(rs.getResponseDescription());
}
 
void loop() {
    // RECEPTION
  if (e32ttl100.available()>1) {
      // read the String message
    ResponseContainer rc = e32ttl100.receiveMessage();
    // Is something goes wrong print error
    if (rc.status.code!=1){
        rc.status.getResponseDescription();
    }else{
        // Lecture du message reçu
        Serial.println(rc.data);
         if (rc.data.toInt() == GYRO_ON){
	        Serial.println("----------- ALLUMER  ---------------");
          digitalWrite(RELAIS, HIGH);
         }
         if(rc.data.toInt() == GYRO_OFF){
	          Serial.println("----------- ETEINDRE ---------------");
            digitalWrite(RELAIS, LOW);
         }
    }
  }  
     //EMISSION
  if (Serial.available()) {
      String input = Serial.readString();
      ResponseStatus rs = e32ttl100.sendMessage(input);
      Serial.println(rs.getResponseDescription());
  }
}



void printParameters(struct Configuration configuration) {
	Serial.println("----------------------------------------");

	Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN);Serial.print(" ");Serial.print(configuration.HEAD, DEC);Serial.print(" ");Serial.println(configuration.HEAD, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, DEC);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, DEC);
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

	Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
	Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
	Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
	Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
	Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

	Serial.println("----------------------------------------");

}



