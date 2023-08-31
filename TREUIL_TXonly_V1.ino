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

//pin Interrupteur
#define INTERRUPTEUR 6

//librairie Lora
#include "LoRa_E32.h"

// messages envoyés au gyrophares
#define GYRO_ON "11"
#define GYRO_OFF "88"

//init pin du modem Lora
LoRa_E32 e32ttl100(2, 3);  // e32 TX e32 RX

//variable globale
byte PRE_ETAT_INTER = 0;
byte NEW_ETAT_INTER = 0;
unsigned long Time_Of_The_Last_Change = 0;
bool FLAG_CHANGE = false;

void setup() {
  // init du GPIO de l'interrupteur
  pinMode(INTERRUPTEUR, INPUT);  // on va lire la gpio 6 pour connaitre l'état de l'interrupteur

  ResponseStructContainer c;
  Serial.begin(9600);
  delay(500);

  // Startup all pins and UART
  e32ttl100.begin();

  // Get Configuration
  c = e32ttl100.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*)c.data;
  Serial.println(c.status.getResponseDescription());
  //Serial.println(c.status.code);
  //printParameters(configuration);
  // CONFIGURATION DU MODULE EBYTE

  configuration.ADDL = 3;
  configuration.ADDH = 0;
  configuration.CHAN = 0x04;

  // FEC ON pour augmenter l'anti-interférence
  configuration.OPTION.fec = FEC_1_ON;  //FEC_0_OFF

  // Transmission transparent : broadcast à tt le monde qui écoute
  configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;

  // IO drive mode: this bit is used for the module internal pull-up resistor.
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;  //default

  //Puissance - Max = 30dB
  configuration.OPTION.transmissionPower = POWER_30;

  // inutile en mode transparent mais bon..
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;

  //The lower the air data rate, the longer the transmitting distance
  //data rate minimal = 0.3k
  configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;

  configuration.SPED.uartBaudRate = UART_BPS_9600;  //Default
  configuration.SPED.uartParity = MODE_00_8N1;      //Default

  //Send the configuration to the device
  e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  printParameters(configuration);
  c.close();

  // Send message
  Serial.println("Hi, I'm going to send message!");
  ResponseStatus rs = e32ttl100.sendMessage("Hello, world ! ici le treuil");
  // Check If there is some problem of successfully send
  Serial.println(rs.getResponseDescription());
}

void loop() {

  //lecture de l'état de l'interrupteur
  NEW_ETAT_INTER = digitalRead(INTERRUPTEUR);
  Serial.print("état interr = ");
  Serial.print(NEW_ETAT_INTER);
  Serial.println();

  // EMISSION
  //Si l'inter a changé d'état on envoie un ordre aux gyro
  if (NEW_ETAT_INTER != PRE_ETAT_INTER) { // si changement d'état de l'inter
    Time_Of_The_Last_Change = millis();   // on prend un top
    FLAG_CHANGE = true;                   // on lève le flag
    PRE_ETAT_INTER = NEW_ETAT_INTER;
  }

// si il s'est passé + de 500ms depuis la derniere fois qu'on a détecté un chgt
// d'état de l'interrupteur
// alors on envoie un ordre
  if (FLAG_CHANGE && ((millis() - Time_Of_The_Last_Change) >= 500)) { 
    if (NEW_ETAT_INTER == 1) {
      ResponseStatus rs = e32ttl100.sendMessage(GYRO_ON);
      Serial.println(rs.getResponseDescription());
      Serial.println("------------SEND GYRO ON-----------");
      rs = e32ttl100.sendMessage("\n");
    } else {
      ResponseStatus rs = e32ttl100.sendMessage(GYRO_OFF);
      Serial.println(rs.getResponseDescription());
      Serial.println("------------SEND GYRO OFF-----------");
      rs = e32ttl100.sendMessage("\n");
    }
    FLAG_CHANGE = false;
  }
}


void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");
  Serial.print(F("HEAD : "));
  Serial.print(configuration.HEAD, BIN);
  Serial.print(" ");
  Serial.print(configuration.HEAD, DEC);
  Serial.print(" ");
  Serial.println(configuration.HEAD, HEX);
  Serial.println(F(" "));
  Serial.print(F("AddH : "));
  Serial.println(configuration.ADDH, DEC);
  Serial.print(F("AddL : "));
  Serial.println(configuration.ADDL, DEC);
  Serial.print(F("Chan : "));
  Serial.print(configuration.CHAN, DEC);
  Serial.print(" -> ");
  Serial.println(configuration.getChannelDescription());
  Serial.println(F(" "));
  Serial.print(F("SpeedParityBit     : "));
  Serial.print(configuration.SPED.uartParity, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.SPED.getUARTParityDescription());
  Serial.print(F("SpeedUARTDatte  : "));
  Serial.print(configuration.SPED.uartBaudRate, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.SPED.getUARTBaudRate());
  Serial.print(F("SpeedAirDataRate   : "));
  Serial.print(configuration.SPED.airDataRate, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.SPED.getAirDataRate());
  Serial.print(F("OptionTrans        : "));
  Serial.print(configuration.OPTION.fixedTransmission, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.OPTION.getFixedTransmissionDescription());
  Serial.print(F("OptionPullup       : "));
  Serial.print(configuration.OPTION.ioDriveMode, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.OPTION.getIODroveModeDescription());
  Serial.print(F("OptionWakeup       : "));
  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
  Serial.print(F("OptionFEC          : "));
  Serial.print(configuration.OPTION.fec, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.OPTION.getFECDescription());
  Serial.print(F("OptionPower        : "));
  Serial.print(configuration.OPTION.transmissionPower, BIN);
  Serial.print(" -> ");
  Serial.println(configuration.OPTION.getTransmissionPowerDescription());
  Serial.println("----------------------------------------");
}
