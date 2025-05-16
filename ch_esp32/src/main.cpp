#include <Arduino.h>
#include <main.h>
#include "R200.h"
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h> //JSON
#include <LiquidCrystal_I2C.h>

#include <Wire.h>
//#include <U8g2lib.h>
#include <U8g2lib.h>
#include <Ampel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <stdio.h>
#include "soc/rtc_wdt.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#define oled_scl 15
#define oled_sda 4
#define oled_rst 16
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ oled_scl, /* data=*/ oled_sda, /* reset=*/ oled_rst); 

Ampel ampel(13, 12, 14);
bool RaceIsRunning = false;

unsigned long TimeRaceStarted = 0;

int  u_id=0;

void startup();
void stopRace();
void startRace();
void reportTags();

void syncTime(TimerHandle_t xTimer) ;

WiFiServer wifiServer(8080);
WiFiClient client;

const char* ssid = "SMEK01";
const char* password =  "********";

#define CMD_SIZE       64 

// Define Queue handle
QueueHandle_t QueueHandle;
#define MAX_CARS  10
#define QUE_SIZE  MAX_CARS*4

#define ZROUND_SYNC_TIMER 5000
#define RFID_LAST_SEEN 2000

typedef struct TagRecord
{
  uint8_t car;
  unsigned long timeLastSeen;
} TAG_RECORD;

TagRecord TagsSeen[MAX_CARS]; // where we store transponders seen during race


//const int QueueElementSize = 10;

uint8_t _Buffer[RX_BUFFER_LENGTH] = {0};
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
TaskHandle_t Task4;

R200 rfid;



#include <zrount_js.h>
String processor(const String& var)
{
  //Serial.println(var);
  /*
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
    */
  return String();
}


void notifyClients() 
{
  //ws.textAll(String("XXX"));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) 
{
  
  MESSAGE_T message;
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
  {
    data[len] = 0;

    const size_t capacity = JSON_ARRAY_SIZE(15) + JSON_OBJECT_SIZE(1) + 15*JSON_OBJECT_SIZE(3) + 1394;
    DynamicJsonDocument doc(capacity);

    //JsonObject& root = jsonBuffer.parseObject(data);
    deserializeJson(doc, data);
    
    JsonArray ampelArray = doc["ampeln"];
    for (JsonObject ampeljson : ampelArray) 
    {        
      ampel.set_light(ampeljson["color"],ampeljson["nr"],ampeljson["status"]);      
    }    
    
    JsonArray cmdArray = doc["commandos"];
    for (JsonObject cmd : cmdArray) 
    {
      //1 ZRound Connect
      //2 Start RAce
      //3 Stop Race
      //iNextCmd=cmd["cmd"];
        message.uiSender=MSG_Sender_TCP80;
        message.ul_time= millis();
        message.car=0;

        message.uiCmd=cmd["cmd"];        
        int ret = xQueueSend(QueueHandle, (void *)&message, 0);

    }
    notifyClients();
  }
}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) 
{
  switch (type) {
    case WS_EVT_CONNECT:
      //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() 
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}


void Task4code( void * pvParameters )
{
  char szBuffer[CMD_SIZE];
  int BytesCount = -1;
  
  u8x8.printf("%s %02d\n",pcTaskGetName(NULL),xPortGetCoreID());  


  while(true)
  {
    
    
      if(Serial.available())
      {
        BytesCount =  Serial.readBytesUntil('\n',szBuffer,CMD_SIZE-1);  
        if (BytesCount  > 0) 
        {
          szBuffer[BytesCount]='\0';
        }
        else
        {
          szBuffer[0]='\0';
        } 
        
        ampel.set_ampel(szBuffer);
      }


    rtc_wdt_feed();
    delay(1);
  }
}




void Task3code( void * pvParameters )
{
  char szTCPmsg[255];
  MESSAGE_T message;
  
  u8x8.printf("%s %02d\n",pcTaskGetName(NULL),xPortGetCoreID());  
  while(true)
  {
    client = wifiServer.available();
    while (client) 
    {
      while (client.connected()) 
      {  
        while(client.available())
        {
          memset(szTCPmsg,0x00,255);
          client.readBytesUntil('&',szTCPmsg,4);
          message.uiSender=MSG_Sender_TCP8080;
          message.ul_time= millis();
          message.car=0;

          switch (szTCPmsg[1])
          {
            case 'C':
              message.uiCmd=CMD_ZROUND_STARTUP;        
              break;
            case 'I':
              message.uiCmd=CMD_ZROUND_START_RACE;
              break;
            case 'F':
              message.uiCmd=CMD_ZROUND_STOP_RACE;
              break;
          }
          int ret = xQueueSend(QueueHandle, (void *)&message, 0);
        }   
      }   
    }
    rtc_wdt_feed();
    delay(1);
  }
}


void Task1code( void * pvParameters )
{
  MESSAGE_T message;
  int uid;
  int ui_bytesget=0;
  
  u8x8.printf("%s %02d\n",pcTaskGetName(NULL),xPortGetCoreID());  
  while(true)
  {
      if(RaceIsRunning)
      {
          
        if ((ui_bytesget=rfid.GetMultiPollInstruction(_Buffer ))>18)
        {
          message.car=_Buffer[18];
          message.uiSender=MSG_Sender_RFID;
          message.ul_time= millis();
          message.uiCmd=CMD_RFID_REPORT_TAG;
          
          int ret = xQueueSend(QueueHandle, (void *)&message, 0);
          if (ret == errQUEUE_FULL) 
          {
              // Since we are checking uxQueueSpacesAvailable this should not occur, however if more than one task should
              //   write into the same queue it can fill-up between the test and actual send attempt
              Serial.println("The `TaskReadFromSerial` was unable to send data into the Queue");

          }  // Queue send check
        }
      }
                
      rtc_wdt_feed();
      delay(1);
      
  }
}

//MAIN TASK
void Task2code( void * pvParameters )
{

  u8x8.printf("%s %02d\n",pcTaskGetName(NULL),xPortGetCoreID());  
  const char *  szSender[] = {"MSG_Sender_RFID","MSG_Sender_TCP8080","MSG_Sender_TCP80"};
  const char *  szCmd[] = {"CMD_ZROUND_STARTUP","CMD_ZROUND_START_RACE","CMD_ZROUND_STOP_RACE","CMD_RFID_REPORT_TAG"};

    // Create an array of function pointers
  void (*functions[])(void) = {startup, startRace, stopRace,reportTags};

  MESSAGE_T message;
  for (;;)
  {  // A Task shall never return or exit.
    // One approach would be to poll the function (uxQueueMessagesWaiting(QueueHandle) and call delay if nothing is waiting.
    // The other approach is to use infinite time to wait defined by constant `portMAX_DELAY`:
    if (QueueHandle != NULL) 
    {  // Sanity check just to make sure the queue actually exists
      int ret = xQueueReceive(QueueHandle, &message, portMAX_DELAY);
      if (ret == pdPASS) 
      {
        /*
        u8x8.printf("%02lu:%02lu.%03lu|%s|%s|%02x|\n",  
                              message.ul_time/60000,
                              (message.ul_time%60000)/1000,
                              message.ul_time%1000,
                              szSender[message.uiSender-1],
                              szCmd[message.uiCmd-1],                              
                              message.car);
        */
        //u8x8.printf("%03d %d\n",message.car,message.ul_time);  

        u_id=message.car;  
        functions[message.uiCmd-1]();  
        u_id=0;
        // The item is queued by copy, not by reference, so lets free the buffer after use.
      } 
      else 
      {
        if(ret == pdFALSE) 
        {
          Serial.println("The `TaskWriteToSerial` was unable to receive data from the Queue");
        }
      }

    }  // Sanity check
      rtc_wdt_feed();
      delay(1);
  }  // Infinite loop
}

void setup() 
{
  Serial.begin(9600); 

  u8x8.begin();
  u8x8.setPowerSave(0);
/*
  u8x8.setCursor(0,0);  
  u8x8.setFont(u8x8_font_torussansbold8_u );
  u8x8.setCursor(2,1);  
  u8x8.printf(" (C) 2025");
  u8x8.setCursor(2,2);  
  u8x8.printf("  by ");
  u8x8.setCursor(2,3);  
  u8x8.printf(" JPS ");
  u8x8.setCursor(2,4);  
  u8x8.printf("  & ");
  u8x8.setCursor(2,5);  
  u8x8.printf(" W.S.SW");


  delay(3000);
  */
  u8x8.clearDisplay();
  u8x8.home();
  u8x8.setFont(u8x8_font_5x7_f);



  rtc_wdt_protect_off();    // Turns off the automatic wdt service
  rtc_wdt_enable();         // Turn it on manually
  rtc_wdt_set_time(RTC_WDT_STAGE0, 20000);  // Define how long you desire to let dog wait.

  QueueHandle = xQueueCreate(QUE_SIZE, sizeof( MESSAGE_T));

  // Check if the queue was successfully created
  if (QueueHandle == NULL)
   {
    Serial.println("Queue could not be created. Halt.");
    while (1) 
    {
      delay(1000);  // Halt at this point as is not possible to continue
    }
  }
  u8x8.printf("SSID: %s",ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(5000);
    u8x8.printf(".");
    
  }
  u8x8.clearDisplay();
  u8x8.home();
  u8x8.printf("%s\n",WiFi.localIP().toString());
  
 
  wifiServer.begin();
  wifiServer.setNoDelay(true);

  rfid.begin(&Serial2, 115200, 5, 18);

  //delay(1000);
  
  //rfid.setMultiplePollingMode(false);
 
  rfid.SetTransmitPower(4000);
  rfid.SetReceiverDemodulatorParameters_Mixer_G(0x08,0x06,0x0100);
  rfid.SetQueryParameters(	0x1020);
  
  rfid.SetBuzzerOnOff(false);
//Serial.printf("WORK AREA %d\n",rfid.GetWorkArea());
  rfid.SetModuleIdleSleepTime (0);

  u8x8.printf("%02x %04x %04x %01x\n",rfid.GetReceiverDemodulatorParameters_Mixer_G(),rfid.AcquireTransmitPower(),rfid.GetQueryParameters(),rfid.GetWorkArea());
  //rfid.setMultiplePollingMode(true);  
  //delay(500);
  
  ampel.setAllLow();
  ampel.set_light(COLOR_BLUE,0,SET_COLOR_ADD);
  ampel.set_light(COLOR_RED,1,SET_COLOR_ADD);
  ampel.set_light(COLOR_BLUE,2,SET_COLOR_ADD);
  ampel.set_light(COLOR_RED,3,SET_COLOR_ADD);
  ampel.set_light(COLOR_BLUE,4,SET_COLOR_ADD);
  
  

//create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "RFID",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "MAIN",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 

    
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task3code,   /* Task function. */
                    "8080",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &Task3,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 
  xTaskCreatePinnedToCore(
                    Task4code,   /* Task function. */
                    "V24 ",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &Task4,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 


// Create a timer with a period of 1000 ms (1 second)
  TimerHandle_t xTimer = xTimerCreate(
      "MyTimer",           // Timer name
      pdMS_TO_TICKS(ZROUND_SYNC_TIMER), // Timer period in ticks
      pdTRUE,              // Auto-reload
      (void *)0,           // Timer ID
      syncTime       // Callback function
  );

  // Check if the timer was created successfully
  if (xTimer != NULL) 
  {
      // Start the timer with a block time of 0 ticks
      if (xTimerStart(xTimer, 0) != pdPASS) 
      {
          u8x8.printf("Failed to start timer\n");
      }
  } 
  else 
  {
      u8x8.printf("Failed to create timer\n");
  }
  
  //WEB SERVER 
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start WEB server
  server.begin();
  

}



void loop() 
{
  ws.cleanupClients();  

  rtc_wdt_feed();
  vTaskDelay(500);
}

//=====[ startup ]==============================================================
void startup()
{
    //Serial.println("STARTUP FUCTIION");
    u8x8.clearLine(6);
    u8x8.setCursor(0,6);
    ampel.set_all_blue_on();
    u8x8.printf("ZRound connected\n");
}

//=====[ startRace ]============================================================
void startRace()
{
  // record time race started
  TimeRaceStarted = millis();
  // set race running flag
  RaceIsRunning = true;
  u8x8.clearLine(6);
  u8x8.setCursor(0,6);
  u8x8.printf("RACE IS RUNNING\n");
  ampel.set_all_green_on();
  //GHOST CAR STARTEN 
  rfid.setMultiplePollingMode(true);    
  for(int i=0;i<MAX_CARS;i++)
  {
    TagsSeen[i].car=0;
    TagsSeen[i].timeLastSeen=0L;
  }
}

//=====[ stopRace ]=============================================================
void stopRace()
{
  TimeRaceStarted = 0;
  RaceIsRunning = false;
    u8x8.clearLine(6);
  u8x8.setCursor(0,6);
  u8x8.printf("RACE IS STOPED\n");
  ampel.set_all_blue_on();
  rfid.setMultiplePollingMode(false);  
}

//=====[ reportTags ]===========================================================
void reportTags()
{
  int i=0; 
  if(RaceIsRunning)
  {
    unsigned long ulTime=millis();
    for(i=0;i<MAX_CARS;i++)
    {
      if(TagsSeen[i].car==u_id ||
      !TagsSeen[i].car)
      {
          TagsSeen[i].car=u_id;
          break;
      }
    }
  
    //letzesmal + 5 sek ist kleier als jetzt 
    if(TagsSeen[i].timeLastSeen+RFID_LAST_SEEN < ulTime)
    {
      client.printf("%L%1x,%x&",u_id,ulTime-TimeRaceStarted);
      /*
      client.printf("%1x",u_id);
      client.print(',');
      client.print(ulTime-TimeRaceStarted , HEX);
      client.print("&");
     */
      TagsSeen[i].timeLastSeen=ulTime;

      char szText[255];
      //u8x8.printf("%02lu:%02lu %03lu", currentRaceTime/60000, (currentRaceTime% 60000) / 1000, currentRaceTime %1000);
      sprintf(szText,"Index %02d CAR 0x%02x Time %02lu:%02lu %03lu Zround register car %d %d ms",i,u_id,(ulTime-TimeRaceStarted)/60000, ((ulTime-TimeRaceStarted)% 60000) / 1000, (ulTime-TimeRaceStarted) %1000,u_id,ulTime-TimeRaceStarted);
      ws.textAll(szText);      
    }
  }
}
    

void syncTime(TimerHandle_t xTimer) 
{
  if(!RaceIsRunning)
    return;

  unsigned long currentRaceTime = millis() - TimeRaceStarted;
   u8x8.clearLine(7);
  u8x8.setCursor(0,7);
  u8x8.printf("%02lu:%02lu %03lu", currentRaceTime/60000, (currentRaceTime% 60000) / 1000, currentRaceTime %1000);

  client.print ("T");
  client.print(currentRaceTime, HEX);
  client.print("&");

}

