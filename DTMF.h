#ifndef DTMF_H__
#define DTMF_H__
class DTMF {
  String value;
  bool __available;
public:

  void handle(char dtmf) {
    if (dtmf == '*') {
      if (this->value.length()) {
        this->value = this->value.substring(0, this->value.length() - 1);
      }
    } else if (dtmf == '#') {
      this->value = "";
    } else {
      this->value += dtmf;
    }
    this->__available = true;
  }

  void goBack() {
    if (this->value.length()) {
      this->value = this->value.substring(0, this->value.length() - 1);
    }
  }

  bool available() {
    return this->__available;
  }

  String read() {
    this->__available = false;
    return this->value;
  }

  void reset() {
    this->__available = false;
    this->value = "";
  }

} DTMF;
#endif