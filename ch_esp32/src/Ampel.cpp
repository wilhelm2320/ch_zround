
#include <Arduino.h>
#include <Ampel.h>

// Constructor


void Ampel::set_light(unsigned int color,unsigned int led_pos,unsigned int todo)
{
  //uint8_t pinValues[2] ;
  //Serial.printf("color <%d> led_pos <%d> todo <%d>\n",color,led_pos,todo);

  uint8_t *ptr_pinValues ;
  unsigned long set_bit_help=0;
  //XXXXXXXXXXXXXXXXX

  /*
    // Erzeuge ein statisches JSON-Dokument
    StaticJsonDocument<200> doc;
  
    // Erstelle ein Array mit dem Namen "Ampeln"
    JsonArray AmpelArray = doc.createNestedArray("Ampeln");
  
    // Füge ein Objekt ins Array ein
    JsonObject Ampel = AmpelArray.createNestedObject();
    Ampel["color"] = color;
    Ampel["nr"] = led_pos;
    Ampel["status"] = todo;
  
    // Den JSON-String serialisieren
    String output;
    serializeJson(doc, output);
  
    ws.textAll(output);      
*/
  set_bit_help=colorMask[color];
  set_bit_help=set_bit_help<<led_pos*3;

  ptr_pinValues=sr.getAll();
  
  switch (todo)
  {
    case SET_COLOR_ADD:

    *(ptr_pinValues)|=set_bit_help;
    *(ptr_pinValues+1)|=(set_bit_help>>8);
    break;
    case SET_COLOR_CLEAR:
      *(ptr_pinValues)&=(~set_bit_help);
      *(ptr_pinValues+1)&=(~(set_bit_help>>8));

    break;
  }
  
  sr.setAll(ptr_pinValues);     

}
void Ampel::setAllLow()
{
  //SET_COLOR_CLEAR     0x02
  
  sr.setAllLow();
}

void Ampel::set_all_red_on()
{
  this->setAllLow();
  this->set_light(COLOR_RED,0,SET_COLOR_ADD);
  this->set_light(COLOR_RED,1,SET_COLOR_ADD);
  this->set_light(COLOR_RED,2,SET_COLOR_ADD);
  this->set_light(COLOR_RED,3,SET_COLOR_ADD);
  this->set_light(COLOR_RED,4,SET_COLOR_ADD);

}
void Ampel::set_all_green_on()
{
  this->setAllLow();
  this->set_light(COLOR_GREEN,0,SET_COLOR_ADD);
  this->set_light(COLOR_GREEN,1,SET_COLOR_ADD);
  this->set_light(COLOR_GREEN,2,SET_COLOR_ADD);
  this->set_light(COLOR_GREEN,3,SET_COLOR_ADD);
  this->set_light(COLOR_GREEN,4,SET_COLOR_ADD);

}
void Ampel::set_all_blue_on()
{
  this->setAllLow();
  this->set_light(COLOR_BLUE,0,SET_COLOR_ADD);
  this->set_light(COLOR_BLUE,1,SET_COLOR_ADD);
  this->set_light(COLOR_BLUE,2,SET_COLOR_ADD);
  this->set_light(COLOR_BLUE,3,SET_COLOR_ADD);
  this->set_light(COLOR_BLUE,4,SET_COLOR_ADD);

}
void Ampel::set_ampel(char * pszBuffer)
{
  char szCmd[255];

  int iLight=0;
  int iState=0;
 
  sscanf(pszBuffer,"%[^,],%d,%d\n",szCmd,&iLight,&iState);
  if(!strcmp(szCmd, "$LIGHT"))              
  {
      this->set_light(COLOR_GREEN,abs(iLight-5),SET_COLOR_CLEAR);               
      this->set_light(COLOR_BLUE,abs(iLight-5),SET_COLOR_CLEAR);               
      this->set_light(COLOR_RED,abs(iLight-5),(!iState)?SET_COLOR_CLEAR:SET_COLOR_ADD);         
  }
}