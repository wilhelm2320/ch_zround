#ifndef Ampel_h
#define Ampel_h

#include <stdint.h>
#include <Arduino.h>
#include <ShiftRegister74HC595.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define COLOR_BLUE    0
#define COLOR_GREEN   1
#define COLOR_RED     2
#define SET_COLOR_ADD       0x01
#define SET_COLOR_CLEAR     0x02

class Ampel 
{

    public:
    Ampel(int dataPin, int clockPin, int latchPin)
        : sr(dataPin, clockPin, latchPin) {}
    
    void set_light(unsigned int color,unsigned int led_pos,unsigned int todo);
    void setAllLow();
    void set_all_blue_on();
    void set_all_green_on();
    void set_all_red_on();
    void set_ampel(char * pszBuffer);
    private:
    uint8_t *ptr_pinValues ;
    uint8_t colorMask[3] = { (1 << (COLOR_BLUE+1)), (1 << (COLOR_GREEN+1)),(1 << (COLOR_RED+1)) }; 

    ShiftRegister74HC595<2> sr;
    //ShiftRegister74HC595<2> sr(13, 12, 14);
};

#endif