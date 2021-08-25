#include "IOTBinaryProtocol.h"

#define PIN_LED_RED     9  //D9
#define PIN_LED_GREEN   5  //D5
#define PIN_LED_BLUE   10  //D10
#define HMI_REG_LED_RED    44
#define HMI_REG_LED_GREEN  45
#define HMI_REG_LED_BLUE   46

#define PIN_OPTO    4  //A4
#define PIN_VR      5  //A5
#define PIN_BUZZER  3  //D3
#define PIN_SWITCH  4  //D4
#define HMI_REG_OPTO    53
#define HMI_REG_VR      51
#define HMI_REG_BEEP     9
#define HMI_REG_SWITCH  52

#define PIN_SERVO_PWM  6  //D6
#define PIN_SERVO_D1  12  //D12
#define PIN_SERVO_D2  11  //D11
#define PIN_SERVO_D3   8  //D8
#define PIN_SERVO_D4   7  //D7
#define HMI_REG_CLOCKWISE  47
#define HMI_REG_COUNTERWISE  48

IOTBinaryProtocol p(Serial);
int servo_clockwise = 0;
int servo_counterwise = 0;
int servo_pins[4] = {PIN_SERVO_D1, PIN_SERVO_D2, PIN_SERVO_D3, PIN_SERVO_D4};
int servo_last_idx = -1;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_SWITCH, INPUT);
    pinMode(PIN_SERVO_PWM, OUTPUT);
    pinMode(PIN_SERVO_D1, OUTPUT);
    pinMode(PIN_SERVO_D2, OUTPUT);
    pinMode(PIN_SERVO_D3, OUTPUT);
    pinMode(PIN_SERVO_D4, OUTPUT);
}

void loop() {
    unsigned long now = millis();
    if (now % 200 == 0) {
        p.sendMessage(IOTCommand::SET_REG, HMI_REG_VR, analogRead(PIN_VR));
        p.sendMessage(IOTCommand::SET_REG, HMI_REG_OPTO, analogRead(PIN_OPTO));
        int sw = digitalRead(PIN_SWITCH);
        if (sw == HIGH) {
            p.sendMessage(IOTCommand::SET_REG, HMI_REG_SWITCH, 1);
        }
        else {
            p.sendMessage(IOTCommand::SET_REG, HMI_REG_SWITCH, 0);
        }
    }
    if (now % 20 == 0) {
        if (servo_clockwise == 0 && servo_counterwise == 0) {
            for (int i=0; i<4; i++) {
              digitalWrite(servo_pins[i], LOW);
            }
            servo_last_idx = -1;
        }
        if (servo_clockwise == 1) {
            if (servo_last_idx != -1) {
                digitalWrite(servo_pins[servo_last_idx], LOW);
                servo_last_idx = (servo_last_idx+1) % 4;
                digitalWrite(servo_pins[servo_last_idx], HIGH);
            }
            else {
                servo_last_idx = 0;
                digitalWrite(servo_pins[servo_last_idx], HIGH);
            }
        }
        if (servo_counterwise == 1) {
            if (servo_last_idx != -1) {
                digitalWrite(servo_pins[servo_last_idx], LOW);
                servo_last_idx = servo_last_idx - 1;
                if (servo_last_idx == -1) servo_last_idx = 3;
                digitalWrite(servo_pins[servo_last_idx], HIGH);
            }
            else {
                servo_last_idx = 3;
                digitalWrite(servo_pins[servo_last_idx], HIGH);
            }
        }
        delay(1);
    }
}

void serialEvent(){
    bool avil = p.hasMessage();
    if (avil) {
        if (p.getMessageCommand() == IOTCommand::SET_REG) {
            if (p.getMessageRegisterID() == HMI_REG_LED_RED) {
                analogWrite(PIN_LED_RED, p.getMessageDataUnsignedChar());
            }
            if (p.getMessageRegisterID() == HMI_REG_LED_GREEN) {
                analogWrite(PIN_LED_GREEN, p.getMessageDataUnsignedChar());
            }
            if (p.getMessageRegisterID() == HMI_REG_LED_BLUE) {
                analogWrite(PIN_LED_BLUE, p.getMessageDataUnsignedChar());
            }
            if (p.getMessageRegisterID() == HMI_REG_BEEP) {
                int beep = p.getMessageDataInt();
                if (beep == 1) {
                    digitalWrite(PIN_BUZZER, HIGH);
                }
                else {
                    digitalWrite(PIN_BUZZER, LOW);
                }
            }
            if (p.getMessageRegisterID() == HMI_REG_CLOCKWISE) {
                servo_clockwise = p.getMessageDataInt();
            }
            if (p.getMessageRegisterID() == HMI_REG_COUNTERWISE) {
                servo_counterwise = p.getMessageDataInt();
            }
        }
        p.flushMessage();
    }
    else {
          IOTError error = p.getParsingError();
          if (error == IOTError::WRONG_COMMAND){
              byte error_byte = p.getErrorByte();
              Serial.print("The wrong command byte is: 0x");
              Serial.print(error_byte, HEX);
              Serial.print("\n");
         }
    }

}
