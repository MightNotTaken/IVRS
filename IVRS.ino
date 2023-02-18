/*Source: https://circuitdigest.com/microcontroller-projects/esp32-based-audio-player*/
#include "GSM.h"
#include "DTMF.h"
#include "MP3.h"

#define SERIAL_BAUDRATE             115200L

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  if (GSM::begin()) {
    Serial.println("GSM initialized");
    delay(5000);
    GSM::enableDTMF();
  }

  if (MP3::begin()) {
    Serial.println("MP3 initialized");
  } else {
    Serial.println("Error in initializing MP3");
  }
}

String filename = "";
void loop() {
  GSM::listen();
  GSM::process();
  if (GSM::incomingCall()) {
    Serial.print("Incomming call from ");
    Serial.println(GSM::getCallerPhoneNumber());
    GSM::attendCall();
    MP3::play("welcome");
  }
  if (DTMF.available()) {
    Serial.println(String("Dtmf: ") + DTMF.read());
    if (!MP3::play(DTMF.read())) {
      DTMF.goBack();
      MP3::play("invalid");
    }
  }
  MP3::update();
  switch (MP3::currentFile.completed()) {
    case FileType::REDIRECT: GSM::redirect("+918082297037");
      break;
    case FileType::TERMINATE: GSM::disconnectCall();
      break;
  }
}
