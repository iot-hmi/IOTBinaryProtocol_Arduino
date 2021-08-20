#include "IOTBinaryProtocol.h"

IOTBinaryProtocol::IOTBinaryProtocol(Stream& stream)
    : stream(stream),
      state(IOTState::IDLE) {}

void IOTBinaryProtocol::sendMessageHeader(IOTCommand ctype, uint16_t rid)
{
    byte head[4];
    head[0] = 0x11;
    head[1] = (byte)ctype;
    head[2] = rid & 0xFF;
    head[3] = (rid >> 8) & 0xFF;
    this->stream.write(head, 4);
}

void IOTBinaryProtocol::sendPayloadHeader(IOTData dtype, uint16_t dsize)
{
    byte head[3];
    head[0] = (byte)dtype;
    head[1] = dsize & 0xFF;
    head[2] = (dsize >> 8) & 0xFF;
    this->stream.write(head, 3);
}

void IOTBinaryProtocol::sendIntMessage(IOTCommand ctype, uint16_t rid, IOTData dtype, uint16_t dsize, uint64_t value)
{
    this->sendMessageHeader(ctype, rid);
    if (ctype != IOTCommand::GET_REG) {
        this->sendPayloadHeader(dtype, dsize);
        byte data[dsize];
        memcpy(data, &value, dsize);
        this->stream.write(data, dsize);
    }
}

void IOTBinaryProtocol::sendFloatMessage(IOTCommand ctype, uint16_t rid, uint16_t dsize, byte* fbytes)
{
    this->sendMessageHeader(ctype, rid);
    if (ctype != IOTCommand::GET_REG) {
        this->sendPayloadHeader(IOTData::FLOAT, dsize);
        this->stream.write(fbytes, dsize);
    }
}

void IOTBinaryProtocol::sendStringMessage(IOTCommand ctype, uint16_t rid, uint16_t dsize, const char* str)
{
    this->sendMessageHeader(ctype, rid);
    if (ctype == IOTCommand::EXEC) {
        dsize += 1;
    }
    if (ctype != IOTCommand::GET_REG) {
        this->sendPayloadHeader(IOTData::STRING, dsize);
        this->stream.print(str);
    }
    if (ctype == IOTCommand::EXEC) {
        this->stream.write('\n');
    }
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, bool data)
{
    this->sendMessageHeader(type, register_id);
    this->sendPayloadHeader(IOTData::BOOL, 1);
    if (data == true) {
        this->stream.write(1);
    }
    else {
        this->stream.write((byte)0);
    }
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, char data)
{
    this->sendIntMessage(type, register_id, IOTData::INT, 1, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, unsigned char data)
{
    this->sendIntMessage(type, register_id, IOTData::UINT, 1, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, short data)
{
    uint16_t dsize = sizeof(short);
    this->sendIntMessage(type, register_id, IOTData::INT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, unsigned short data)
{
    uint16_t dsize = sizeof(unsigned short);
    this->sendIntMessage(type, register_id, IOTData::UINT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, int data)
{
    uint16_t dsize = sizeof(int);
    this->sendIntMessage(type, register_id, IOTData::INT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, unsigned int data)
{
    uint16_t dsize = sizeof(unsigned int);
    this->sendIntMessage(type, register_id, IOTData::UINT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, long data)
{
    uint16_t dsize = sizeof(long);
    this->sendIntMessage(type, register_id, IOTData::INT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, unsigned long data)
{
    uint16_t dsize = sizeof(unsigned long);
    this->sendIntMessage(type, register_id, IOTData::UINT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, long long data)
{
    uint16_t dsize = sizeof(long long);
    this->sendIntMessage(type, register_id, IOTData::INT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, unsigned long long data)
{
    uint16_t dsize = sizeof(unsigned long long);
    this->sendIntMessage(type, register_id, IOTData::UINT, dsize, data);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, float data)
{
    uint16_t dsize = sizeof(float);
    byte* fbytes;
    fbytes = (byte*)&data;
    this->sendFloatMessage(type, register_id, dsize, fbytes);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, double data)
{
    uint16_t dsize = sizeof(double);
    byte* fbytes;
    fbytes = (byte*)&data;
    this->sendFloatMessage(type, register_id, dsize, fbytes);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, const char* str)
{
    uint16_t len = strlen(str);
    this->sendStringMessage(type, register_id, len, str);
}

void IOTBinaryProtocol::sendMessage(IOTCommand type, uint16_t register_id, const String& string)
{
    uint16_t len = string.length();
    this->sendStringMessage(type, register_id, len, string.c_str());
}

bool IOTBinaryProtocol::flushMessage()
{
    if (this->state == IOTState::W_FLUSH) {
        this->state = IOTState::IDLE;
        return true;
    }
    else {
        return false;
    }   
}

bool IOTBinaryProtocol::hasMessage()
{
    if (this->state == IOTState::W_FLUSH) {
        return true;
    }

    if (this->stream.available() == 0) {
        return false;
    }

    this->error = IOTError::NO_ERROR;
    this->error_byte = 0;

    while (this->stream.available() > 0) {
        byte in_byte = this->stream.read();
        switch (this->state) {
            case IOTState::IDLE:
                if (in_byte == 0x11) {
                    this->state = IOTState::W_CMD;
                }
                else {
                    this->error = IOTError::NOT_LEAD_BYTE;
                    this->error_byte = in_byte;
                }
                break;
            case IOTState::W_CMD:
                if (in_byte == 0x0 || in_byte == 0x1 || in_byte == 0x2) {
                    this->msg_cmd = (IOTCommand)in_byte;
                    this->state = IOTState::W_RID_LOW;
                    this->msg_reg_id = 0;
                }
                else {
                    this->error = IOTError::WRONG_COMMAND;
                    this->error_byte = in_byte;
                    this->state = IOTState::IDLE;
                }
                break;
            case IOTState::W_RID_LOW:
                this->msg_reg_id |= in_byte;
                this->state = IOTState::W_RID_HIGH;
                break;
            case IOTState::W_RID_HIGH:
                this->msg_reg_id |= (in_byte << 8);
                if (this->msg_cmd == IOTCommand::GET_REG) {
                    this->state = IOTState::W_FLUSH;
                    return true;
                }
                else {
                    this->state = IOTState::W_DATA_TYPE;
                }
                break;
            case IOTState::W_DATA_TYPE:
                if (in_byte >= 0x0 && in_byte <= 0x4) {
                    this->msg_data_type = (IOTData)in_byte;
                    this->state = IOTState::W_LEN_LOW;
                    this->msg_data_len = 0;
                }
                else {
                    this->error = IOTError::WRONG_DATA_TYPE;
                    this->error_byte = in_byte;
                    this->state = IOTState::IDLE;
                }
                break;
            case IOTState::W_LEN_LOW:
                this->msg_data_len |= in_byte;
                this->state = IOTState::W_LEN_HIGH;
                break;
            case IOTState::W_LEN_HIGH:
                this->msg_data_len |= (in_byte << 8);
                this->data_count = this->msg_data_len;
                free(this->msg_data);
                this->msg_data = (byte*)malloc(this->data_count);
                this->state = IOTState::W_DATA;
                break;
            case IOTState::W_DATA:
                this->msg_data[this->msg_data_len-this->data_count] = in_byte;
                this->data_count -= 1;
                if (this->data_count == 0) {
                    this->state = IOTState::W_FLUSH;
                    return true;
                }
                break;
            case IOTState::W_FLUSH:
                return true;
            default:
                this->state = IOTState::IDLE;
                return false;
        }
    }
    return false;
}

uint16_t IOTBinaryProtocol::fillMessageDataToBuf(byte* buf, uint16_t buf_size)
{
    if (this->state != IOTState::W_FLUSH) {
        return 0;
    }

    uint16_t fill_size = this->msg_data_len;
    if (fill_size > buf_size) {
        fill_size = buf_size;
    }
    memcpy(buf, this->msg_data, fill_size);
    return fill_size;
}

IOTCommand IOTBinaryProtocol::getMessageCommand()
{
    return this->msg_cmd;
}

uint16_t IOTBinaryProtocol::getMessageRegisterID()
{
    return this->msg_reg_id;
}

IOTData IOTBinaryProtocol::getMessageDataType()
{
    return this->msg_data_type;
}

uint16_t IOTBinaryProtocol::getMessageDataSize()
{
    return this->msg_data_len;
}

bool IOTBinaryProtocol::getMessageDataBool()
{
    return (bool)this->msg_data[0];
}

char IOTBinaryProtocol::getMessageDataChar()
{
    return (char)this->msg_data[0];
}

unsigned char IOTBinaryProtocol::getMessageDataUnsignedChar()
{
    return (unsigned char)this->msg_data[0];
}

short IOTBinaryProtocol::getMessageDataShort()
{
    short val = 0;
    byte len = sizeof(short);
    memcpy(&val, this->msg_data, len);
    return val;
}

unsigned short IOTBinaryProtocol::getMessageDataUnsignedShort()
{
    unsigned short val = 0;
    byte len = sizeof(unsigned short);
    memcpy(&val, this->msg_data, len);
    return val;
}

int IOTBinaryProtocol::getMessageDataInt()
{
    int val = 0;
    byte len = sizeof(int);
    memcpy(&val, this->msg_data, len);
    return val;
}

unsigned int IOTBinaryProtocol::getMessageDataUnsignedInt()
{
    unsigned int val = 0;
    byte len = sizeof(unsigned int);
    memcpy(&val, this->msg_data, len);
    return val;
}

long IOTBinaryProtocol::getMessageDataLong()
{
    long val = 0;
    byte len = sizeof(long);
    memcpy(&val, this->msg_data, len);
    return val;
}

unsigned long IOTBinaryProtocol::getMessageDataUnsignedLong()
{
    unsigned long val = 0;
    byte len = sizeof(unsigned long);
    memcpy(&val, this->msg_data, len);
    return val;
}

long long IOTBinaryProtocol::getMessageDataLongLong()
{
    long long val = 0;
    byte len = sizeof(long long);
    memcpy(&val, this->msg_data, len);
    return val;
}

unsigned long long IOTBinaryProtocol::getMessageDataUnsignedLongLong()
{
    unsigned long long val = 0;
    byte len = sizeof(unsigned long long);
    memcpy(&val, this->msg_data, len);
    return val;
}

float IOTBinaryProtocol::getMessageDataFloat()
{
    float val = 0;
    byte len = sizeof(float);
    memcpy(&val, this->msg_data, len);
    return val;
}

double IOTBinaryProtocol::getMessageDataDouble()
{
    double val = 0;
    byte len = sizeof(double);
    memcpy(&val, this->msg_data, len);
    return val;
}

IOTError IOTBinaryProtocol::getParsingError()
{
    return this->error;
}

byte IOTBinaryProtocol::getErrorByte()
{
    return this->error_byte;
}
