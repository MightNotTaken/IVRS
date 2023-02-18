#ifndef GSM_H__
#define GSM_H__

#define NEW_LINE_CHARACTER        '\n'
#define SHOW_RESIDUAL_DATA        true
#define HIDE_RESIDUAL_DATA        false
#define TOTAL_PAYLOADS            10
#include <Arduino.h>
#include "DTMF.h"
#define SerialAT Serial2


struct Command {
  String command;
  int count;
  uint32_t timeout;
  bool read;
  Command(String command = "AT", int count = 2, uint32_t timeout =  10000, bool read = false) {
    this->set(command, count, timeout, read);
  }
  void set(String command = "AT", int count = 2, uint32_t timeout =  10000, bool read = false) {
    this->command = command;
    this->count = count;
    this->timeout = timeout;
    this->read = read;
  }
};
Command testCommand("ATE0", 2, 1000);

namespace GSM {
  String dtmfString = "";
  bool __initialized = false;
  uint32_t __timeout = 0;
  uint32_t __start = 0;
  Command currentCommand;
  int ringCount = 0;
  bool __ringing = false;
  bool __dataChanged = false;
  bool __incall = false;
  unsigned long long __phoneNumber = 0;
  
  bool timedout();
  bool begin();
  void listen();
  void listenRaw();
  bool available();
  void flush(bool = HIDE_RESIDUAL_DATA);
  bool initialized();
  void send(String);
  void send(Command);
  void sendSync(Command);
  void ltrim(String& );
  void deepProcess(String&);
  void handleDTMF(char);

  int payloadIndex = 0;
  String payloads[TOTAL_PAYLOADS];


  bool begin() {
    flush();
    SerialAT.begin(9600, SERIAL_8N1, 16, 17);
    delay(100);
  
    send(testCommand);
    while(!timedout()) {
      listen();
      if (initialized()) {
        return true;
      }
    }
    return false;
  }



  void sendSync(Command cmd) {
    send(cmd);
    while (!GSM::available()) {
      GSM::listen();
      delay(10);
    }
  }

  void send(String command) {
    flush();
    SerialAT.print(command);
    SerialAT.print("\r\n");
    __start = millis();
  }

  void send(Command cmd) {
    send(cmd.command);
    Serial.println(cmd.command);
    currentCommand.count = cmd.count;
    currentCommand.timeout = cmd.timeout;
    currentCommand.read = false;
  }


  void listenRaw() {
    while(SerialAT.available()) {
      char ch = SerialAT.read();
      Serial.println(String(int(ch)) + " : " + ch);
    }
  }

  void listen() {
    while(SerialAT.available()) {
      if (SerialAT.available() == 1) {
        delay(100);
      }
      __initialized = true;
      char ch = (char)SerialAT.read();
      payloads[payloadIndex] += ch;
      if (ch == NEW_LINE_CHARACTER) {
        ltrim(payloads[payloadIndex]);
        if (payloads[payloadIndex].length()) {
          payloadIndex ++;
          payloadIndex %= TOTAL_PAYLOADS;
        }
        currentCommand.count --;
      }
    }
  }


  void ltrim(String& t) {
    while (t[0] == ' ' || t[0] == '\n' || t[0] == 13) {
      t = t.substring(1);
    }
  }

  String getNextCommand(int index) {
    ltrim(payloads[index]);
    if (payloads[index].indexOf(NEW_LINE_CHARACTER) > -1) {
      String payload = payloads[index];
      payloads[index] = "";
      payload.trim();
      return payload;
    }
    return "";
  }
  void process() {
    int i = 0;
    do {
      String command = getNextCommand(i);
      if (!command.length()) {
        break;
      } else {
        Serial.println(command);
        deepProcess(command);
      }
      i++;
    } while (true);
    if (payloadIndex > 0) {
      payloads[0] = payloads[payloadIndex];
      payloads[payloadIndex] = "";
      payloadIndex = 0;
    }
  }

  void deepProcess(String& cmd) {
    if (cmd.indexOf("RING") > -1) {
      ringCount ++;
      if (ringCount > 3) {
        __ringing = true;
      }
    } else if (cmd.indexOf("+CLIP") > -1) {
      sscanf(cmd.c_str(), "+CLIP: \"%lld", &__phoneNumber);
      __ringing = true;
    } else if (cmd.indexOf("+DTMF") > -1) {
      char dtmf;
      sscanf(cmd.c_str(), "+DTMF: %c", &dtmf);
      DTMF.handle(dtmf);
    }
    else if (cmd.indexOf("NO CARRIER") > -1) {
      __incall = false;
    }
  }

  void enableDTMF() {
    sendSync(Command("AT+DDET=1"));
    GSM::process();
  }

  bool incomingCall() {
    return __ringing;
  }

  void attendCall() {
    __ringing = false;
    __incall = true;
    sendSync(Command("ATA"));
    DTMF.reset();
  }

  void disconnectCall() {
    __incall = false;
    DTMF.reset();
    sendSync(Command("ATH"));
  }

  void redirect(String number) {
    String redirectionCmd = "AT+CCFC=1,2,\"number\"";
    redirectionCmd.replace("number", number);
    sendSync(redirectionCmd);
  }

  String getCallerPhoneNumber() {
    char str[20];
    sprintf(str, "%lld", __phoneNumber);
    return str;
  }


  bool available() {
    if (!currentCommand.read) {
      if (currentCommand.count <= 0 || timedout()) {
        return true;
      }
    }
    return false;
  }

  void flush(bool showOutput) {
    if (SerialAT.available()) {
      while (SerialAT.available()) {
        delay(1);
        if (showOutput) {
          Serial.print((char)SerialAT.read());
        }
      }
    }
    
    payloadIndex = 0;
    for (int i=0; i<TOTAL_PAYLOADS; i++) {
      payloads[i] = "";
    }
  }

  bool initialized() {
    return __initialized;
  }
  
  bool timedout() {
    return millis() - __start > currentCommand.timeout;
  }
};

#endif
