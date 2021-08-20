IOTBinaryProtocol Library for Arduino
=====================================

This library provide easy to use functions for sending and parsing IOT binary protocol messages. You can use it with any Stream objects, like Serial and Wire, etc.

The library is tesed with UNO and IDE 1.8.15.

## Sending Messages

For example, send a message to set value of register3 to 100 every second.

```c++
#include "IOTBinaryProtocol.h"

IOTBinaryProtocol p(Serial);

void setup() {
    Serial.begin(9600);
}

void loop() {
    uint16_t register_id = 0x3;
    int val = 100;
    p.sendMessage(IOTCommand::SET_REG, register_id, val);
    delay(1000);
}
```

## Parsing Messages

For example, set value of a variable if received a SET_REG message.

```c++
#include "IOTBinaryProtocol.h"

IOTBinaryProtocol p(Serial);
int register3 = 0;

void setup() {
    Serial.begin(9600);
}

void loop() {
}

void serialEvent(){
    bool avil = p.hasMessage();
    if (avil) {
        if (p.getMessageCommand() == IOTCommand::SET_REG
            && p.getMessageRegisterID() == 0x3) {
                register3 = p.getMessageDataInt();
        }
        p.flushMessage();
    }
}
```

## Parsing Errors

We can also check parsing errors when hasMessage() returns false.

```c++
void serialEvent(){
    bool avil = p.hasMessage();
    if (avil) {
        // processing the message
    }
    else {
        IOTError error = p.getParsingError();
        if (error == IOTError::WRONG_COMMAND) {
            byte error_byte = p.getErrorByte();
            Serial.print("The wrong command byte is: 0x");
            Serial.print(error_byte, HEX);
            Serial.print("\n");
        }
    }
}

```
