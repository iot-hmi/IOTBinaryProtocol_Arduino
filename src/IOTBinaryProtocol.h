#ifndef IOT_BINARY_PROTOCOL_H
#define IOT_BINARY_PROTOCOL_H

#include <Arduino.h>

enum class IOTState
{
    IDLE, W_CMD, W_RID_LOW, W_RID_HIGH, W_DATA_TYPE, W_LEN_LOW, W_LEN_HIGH, W_DATA, W_FLUSH
};

enum class IOTError
{
    NO_ERROR, NOT_LEAD_BYTE, WRONG_COMMAND, WRONG_DATA_TYPE
};

enum class IOTCommand
{
    SET_REG = 0,
    GET_REG,
    EXEC
};

enum class IOTData
{
    BOOL = 0,
    STRING,
    INT,
    UINT,
    FLOAT
};

class IOTBinaryProtocol
{
    public:
        IOTBinaryProtocol(Stream& stream);
        void sendMessage(IOTCommand type, uint16_t register_id, bool data);
        void sendMessage(IOTCommand type, uint16_t register_id, char data);
        void sendMessage(IOTCommand type, uint16_t register_id, unsigned char data);
        void sendMessage(IOTCommand type, uint16_t register_id, short data);
        void sendMessage(IOTCommand type, uint16_t register_id, unsigned short data);
        void sendMessage(IOTCommand type, uint16_t register_id, int data);
        void sendMessage(IOTCommand type, uint16_t register_id, unsigned int data);
        void sendMessage(IOTCommand type, uint16_t register_id, long data);
        void sendMessage(IOTCommand type, uint16_t register_id, unsigned long data);
        void sendMessage(IOTCommand type, uint16_t register_id, long long data);
        void sendMessage(IOTCommand type, uint16_t register_id, unsigned long long data);
        void sendMessage(IOTCommand type, uint16_t register_id, float data);
        void sendMessage(IOTCommand type, uint16_t register_id, double data);
        void sendMessage(IOTCommand type, uint16_t register_id, const String& string);
        void sendMessage(IOTCommand type, uint16_t register_id, const char* str);
        bool hasMessage();
        bool flushMessage();
        IOTCommand getMessageCommand();
        uint16_t getMessageRegisterID();
        IOTData getMessageDataType();
        uint16_t getMessageDataSize();
        bool getMessageDataBool();
        char getMessageDataChar();
        short getMessageDataShort();
        int getMessageDataInt();
        long getMessageDataLong();
        long long getMessageDataLongLong();
        unsigned char getMessageDataUnsignedChar();
        unsigned short getMessageDataUnsignedShort();
        unsigned int getMessageDataUnsignedInt();
        unsigned long getMessageDataUnsignedLong();
        unsigned long long getMessageDataUnsignedLongLong();
        float getMessageDataFloat();
        double getMessageDataDouble();
        uint16_t fillMessageDataToBuf(byte* buf, uint16_t buf_size);
        IOTError getParsingError();
        byte getErrorByte();
    private:
        Stream& stream;
        IOTCommand msg_cmd;
        uint16_t msg_reg_id;
        IOTData msg_data_type;
        uint16_t msg_data_len;
        uint16_t data_count;
        byte* msg_data;
        IOTState state;
        IOTError error;
        byte error_byte;

        void sendMessageHeader(IOTCommand ctype, uint16_t rid);
        void sendPayloadHeader(IOTData dtype, uint16_t dsize);
        void sendIntMessage(IOTCommand ctype, uint16_t rid, IOTData dtype, uint16_t dsize, uint64_t value);
        void sendFloatMessage(IOTCommand ctype, uint16_t rid, uint16_t dsize, byte* fbytes);
        void sendStringMessage(IOTCommand ctype, uint16_t rid, uint16_t dsize, const char* str);
};

#endif  // IOT_BINARY_PROTOCOL_H
