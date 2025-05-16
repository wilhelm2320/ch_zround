#ifndef MAIN_h
#define MAIN_h

// Generate additional debug information to the serial connection when defined
// #define DEBUG

#include <stdint.h>
#include <Arduino.h>

#define MAX_LINE_LENGTH (64)

enum MSG_Sender : uint8_t 
{
    MSG_Sender_RFID = 0x01,
    MSG_Sender_TCP8080 = 0x02,
    MSG_Sender_TCP80 = 0x03
  };
enum ZROUND_CMD : uint8_t 
{
    CMD_ZROUND_STARTUP = 0x01,
    CMD_ZROUND_START_RACE = 0x02,
    CMD_ZROUND_STOP_RACE = 0x03,
    CMD_RFID_REPORT_TAG = 0x04

  };

  

// Define Queue handle


typedef struct 
{
  unsigned long ul_time;      
  uint8_t uiSender;
  uint8_t uiCmd;
  
  uint8_t car;
  //char line[MAX_LINE_LENGTH];
  
} MESSAGE_T;




#endif