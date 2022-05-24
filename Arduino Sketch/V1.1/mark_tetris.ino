/*******************************************************************
  Tetris clock orignal was written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
  I used his tetris example sketch and build on that.
  I added audio --> MP3 and WAV player

 *******************************************************************/
/* to do
    Tickers are always 100 characters long  --> change to 200
    if tickers used < 100 characters...the blanks at the end are shown...this can take a lot of time
    This means I have to change the ticker end position depending on the lenght of the tickers or anticipate for the '\0'

    All is fitted for 64*128 also have to change it for other settings
    But I have no 32*64 panel..so not this time

    to implement: news feed
    to implement: add settings for color of text in setupscreen

    Version updates
    1.1 fixed access point bug. Access point name is now Geniusweb without a password
 *  */
#define VERSION     "1.1"
#include <FastLED_NeoMatrix.h> // only need this for showing the logo, install it from library manager
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h> // Include ArduinoJson Library
#include <FS.h>
#include <SPIFFS.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>  // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
#include <TetrisMatrixDraw.h>                 // https://github.com/toblum/TetrisAnimation
#include <ezTime.h>                           // https://github.com/ropg/ezTime
//https://en.wikipedia.org/wiki/List_of_tz_database_time_zones for list of time zones
#include "settings.h"
#include "logo.h"
#include "index.h"

// libaries for sound
#include "Arduino.h" // part of the esp32 framework
#include "Audio.h"   // part of the esp32 framework
#include "SD.h"      // part of the esp32 framework

// Running a web server
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);//create instance for webSocket server on port"81"
String Message = "";
boolean webserverUpdateMark = false; // flag that is set when user is connected to webserver

/***********************************************************************************************/
/***********************************************************************************************/
// Variables that are needed to function, no need to change any below
String DisplayType, Chipset;
//String Chipset;
boolean clockpolarity;
int PANEL_RES_X;     // Number of pixels wide of each INDIVIDUAL panel module.
int PANEL_RES_Y;    // Number of pixels tall of each INDIVIDUAL panel module.
int PANEL_CHAIN;    // Total number of panels chained one to another
int Soundplaycounter = 0; // needed to wait with start of tickers after sound has been played
int ClockScale, CLOCKXPOS, COLONXPOS, DATEYPOS, DATEXPOS, RockBottom;
boolean localserver = false;
int brightnessTemp = 50; // needs for spiffs init before display init
unsigned long animationDue = 0;
String lastDisplayedTime = "";
String lastDisplayedAmPm = "";
volatile bool finishedAnimating = false;
bool showColon = true;
bool displayIntro = false;      // don't change
// Set a timezone using the following list,
// don't set it here but use the configuration webserver
// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
#define TimezoneBufferlenght  80
char Time_zone[TimezoneBufferlenght] ;
#define ssidbuflength 35
#define passwordbuflength 35
#define DEBUG_BUFFER_SIZE 150
//*****ticker ***
int CYCLE_COUNT = 0;
int SCREEN_TICKERPOS = 0;
unsigned int xText = 20;
int ticker_start_point = 129;
int ticker_end_point = -600;
int ticker_current_pos_bot = 129;
int ticker_current_pos_top = 129;
int TickerYPosBot, TickerYPosTop, Ticker_speed_Top, Ticker_speed_Bot;
char TICKER_TOP[TickerLength] = "";
char TICKER_BOT[TickerLength] = "";
char SCREEN_BUFFER[TickerLength];
unsigned long oneSecondLoopDue = 0;
unsigned long TickerloopDueBot = 0;
unsigned long TickerloopDueTop = 0;
int animationDelay = 25;        // Smaller number == faster, overruled by webinterface settings
//***************
String jsonString; // Temporary storage for the JSON String
// Wifi network station credentials
// overrules in the SPIFFS settings, use the webinterface to change it
char ssid[30] = "";     // your network SSID (name)
char password[30] = ""; // your network key
/***********************************************************************************************/
/***********************************************************************************************/

MatrixPanel_I2S_DMA *dma_display = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * animationTimer = NULL;
uint16_t myBLACK = dma_display->color565(0, 0, 0);
TetrisMatrixDraw tetris(*dma_display); // Main clock
TetrisMatrixDraw tetris2(*dma_display); // The "M" of AM/PM
TetrisMatrixDraw tetris3(*dma_display); // The "P" or "A" of AM/PM

Timezone myTZ;
Audio audio;
/* ************************ Animation handler Function B. Lough ***********************/
// This method is for controlling the tetris library draw calls
void animationHandler() {
  // Not clearing the display and redrawing it when you
  // dont need to improves how the refresh rate appears
  if (!finishedAnimating) {
    dma_display->fillScreen(myBLACK);
    if (displayIntro) {
      finishedAnimating = tetris.drawText(1, 21);
    } else {
      if (twelveHourFormat) {
        // Place holders for checking are any of the tetris objects
        // currently still animating.
        bool tetris1Done = false;
        bool tetris2Done = false;
        bool tetris3Done = false;
        tetris2.scale = ClockScale / 2;
        tetris3.scale = ClockScale / 2;
        tetris1Done = tetris.drawNumbers(CLOCKXPOS, RockBottom, showColon);
        tetris2Done = tetris2.drawText(PMAMXPOS, RockBottom - 1);
        // Only draw the top letter once the bottom letter is finished.
        if (tetris2Done) {
          tetris3Done = tetris3.drawText(PMAMXPOS, RockBottom - 11); //56
        }
        finishedAnimating = tetris1Done && tetris2Done && tetris3Done;
      }
      else {
        finishedAnimating = tetris.drawNumbers(CLOCKXPOS, RockBottom, showColon);
      }
    }
    dma_display->flipDMABuffer();
  }
}

/* ************************  Function to draw connecting word B. Lough ***********************/
void drawConnecting(int x = 0, int y = 0)
{
  dma_display->fillScreen(myBLACK);
  tetris.drawChar("C", x, y, tetris.tetrisCYAN);
  tetris.drawChar("o", x + 5, y, tetris.tetrisMAGENTA);
  tetris.drawChar("n", x + 11, y, tetris.tetrisYELLOW);
  tetris.drawChar("n", x + 17, y, tetris.tetrisGREEN);
  tetris.drawChar("e", x + 22, y, tetris.tetrisBLUE);
  tetris.drawChar("c", x + 27, y, tetris.tetrisRED);
  tetris.drawChar("t", x + 32, y, tetris.tetrisWHITE);
  tetris.drawChar("i", x + 37, y, tetris.tetrisMAGENTA);
  tetris.drawChar("n", x + 42, y, tetris.tetrisYELLOW);
  tetris.drawChar("g", x + 47, y, tetris.tetrisGREEN);
  dma_display->flipDMABuffer();
}

/* ************************ webserver Function ***********************/
void handleRoot() {
  String s = webpage; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}
void handleMesg() {
  handleRoot();
  Message = server.arg("Message");
}

/* ************************ Setup Function ***********************/
void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);  // Will format on the first run after failing to mount
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  checkSPIFFS();

  if (!SD.begin(SD_CS))
  {
    Serial.println(F("Error talking to SD card!"));
  }
  else Serial.println(F("SD Card ready"));
  Serial.println(F("next audio...."));
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // 0...21
  Serial.println(F("Audio Initialized..."));


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (  WiFi.waitForConnectResult() != WL_CONNECTED ) // Try to connect
  { localserver = true;
    Serial.println(F("Could not connect...so creating access point..."));
    WiFi.softAP("GeniusWeb", "");
    Serial.println (WiFi.localIP() );
  }
 // if (localserver == true)
 // {
    //WiFi.softAP("GeniusWeb", "Genius");
   // Serial.println (WiFi.localIP() );
  //}
  server.on("/", []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on("/myMesg", handleMesg);
  // now start the server

  server.begin();
  Serial.println(F(""));
  Serial.println(F("HTTP server started..."));
  Serial.println(F(""));

  webSocket.begin();  // init the Websocketserver
  webSocket.onEvent(webSocketEvent);  // init the webSocketEvent function when a websocket event occurs

  if (DisplayType.equals("64x64x1NC")) {
    Serial.println(F("valid Display setting: 64*64px  1 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 64;
    PANEL_CHAIN = 1;
    clockpolarity = false;
  }
  else if (DisplayType.equals("64x64x2NC")) {
    Serial.println(F("valid Display setting: 64*64px  2 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 64;
    PANEL_CHAIN = 2;
    clockpolarity = false;
  }
  else if (DisplayType.equals("64x32x1NC")) {
    Serial.println(F("valid Display setting: 64*32px  1 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 32;
    PANEL_CHAIN = 1;
    clockpolarity = false;
  }
  else if (DisplayType.equals("64x32x2NC")) {
    Serial.println(F("valid Display setting: 64*32px  2 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 32;
    PANEL_CHAIN = 2;
    clockpolarity = false;
  }
  else if (DisplayType.equals("64x64x1IC")) {
    Serial.println(F("valid Display setting: 64*64px  1 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 64;
    PANEL_CHAIN = 1;
    clockpolarity = true;
  }
  else if (DisplayType.equals("64x64x2IC")) {
    Serial.println(F("valid Display setting: 64*64px  2 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 64;
    PANEL_CHAIN = 2;
    clockpolarity = true;
  }
  else if (DisplayType.equals("64x32x1IC")) {
    Serial.println(F("valid Display setting: 64*32px  1 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 32;
    PANEL_CHAIN = 1;
    clockpolarity = true;
  }
  else if (DisplayType.equals("64x32x2IC")) {
    Serial.println(F("valid Display setting: 64*32px  2 panels"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 32;
    PANEL_CHAIN = 2;
    clockpolarity = true;
  }
  else {
    Serial.println(F("not valid display, assuming 1 panel of 64 x 64px with inverted clock"));
    PANEL_RES_X = 64;
    PANEL_RES_Y = 64;
    PANEL_CHAIN = 1;
    clockpolarity = true;
    RockBottom =  22; //26
    DATEYPOS = 12;
  }

  // Now adjust size of clock and locations of items, depending on display size and number
  if ((PANEL_RES_Y == 32) && (PANEL_CHAIN == 1)) {
    ClockScale = 1;
    CLOCKXPOS = 2;
    COLONXPOS = 2;
    RockBottom =  22;

    // DATEYPOS = 12;
    //DATEXPOS = 2;
    // TickerYPosBot = 55;
    // TickerYPosTop = 2;
  }
  if ((PANEL_RES_Y == 32) && (PANEL_CHAIN == 2)) {

    ClockScale = 1;
    CLOCKXPOS = 2;
    COLONXPOS = 2;
    RockBottom =  22;

    // DATEYPOS = 12;
    DATEXPOS = 2;
    // TickerYPosBot = 55;
    // TickerYPosTop = 2;
  }
  if ((PANEL_RES_Y == 64) && (PANEL_CHAIN == 1)) {
    ClockScale = 2;
    CLOCKXPOS = 2;
    COLONXPOS = 2;
    RockBottom =  52; //26
    DATEYPOS = 12;
    DATEXPOS = 2;
    TickerYPosBot = 55;
    TickerYPosTop = 2;
  }
  if ((PANEL_RES_Y == 64) && (PANEL_CHAIN == 2)) {
    ClockScale = 3;
    CLOCKXPOS = 19;
    COLONXPOS = 19;
    RockBottom =  52; //26
    DATEYPOS = 12;
    DATEXPOS = 2;
    TickerYPosBot = 55;
    TickerYPosTop = 2;
  }

  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );
  Serial.println(F(""));
  if (Chipset.equals("ICN2038S")) {
    mxconfig.driver = HUB75_I2S_CFG::ICN2038S;
    Serial.println(F("chipset changed to:ICN2038S"));
  }
  else if (Chipset.equals("FM6124")) {
    mxconfig.driver = HUB75_I2S_CFG::FM6124;
    Serial.println(F("chipset changed to:FM6124"));
  }
  else if (Chipset.equals("FM6126A")) {
    mxconfig.driver = HUB75_I2S_CFG::FM6126A;
    Serial.println(F("chipset changed to:FM6126A"));
  }
  else if (Chipset.equals("MBI5124")) {
    mxconfig.driver = HUB75_I2S_CFG::MBI5124;
    Serial.println(F("chipset changed to:MBI5124"));
  }

  mxconfig.double_buff = false;
  // May or may not be needed depending on your matrix
  // Example of what needing it looks like:
  // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216
  mxconfig.gpio.e = E_PIN;                // we MUST assign pin e to some free pin on a board to drive 64 pix height panels with 1/32 scan
  mxconfig.gpio.r1 = R1_PIN;
  mxconfig.gpio.g1 = G1_PIN;
  mxconfig.gpio.b1 = B1_PIN;
  mxconfig.gpio.r2 = R2_PIN;
  mxconfig.gpio.g2 = G2_PIN;
  mxconfig.gpio.b2 = B2_PIN;
  mxconfig.gpio.a = A_PIN;
  mxconfig.gpio.b = B_PIN;
  mxconfig.gpio.c = C_PIN;
  mxconfig.gpio.d = D_PIN;
  mxconfig.gpio.e = E_PIN;
  mxconfig.gpio.lat = LAT_PIN;
  mxconfig.gpio.oe = OE_PIN;
  mxconfig.gpio.clk = CLK_PIN;
  mxconfig.clkphase = clockpolarity; // some panels need a  reversed clockpulse I first encountered it in batch may 2021 PH3 64*64 V4.1 HX
  mxconfig.latch_blanking = 4;//4
  mxconfig.mx_height = PANEL_RES_Y;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->fillScreen(myBLACK);

  tetris.display = dma_display; // Main clock
  tetris2.display = dma_display; // The "M" of AM/PM
  tetris3.display = dma_display; // The "P" or "A" of AM/PM

  if (localserver == false) {
    Serial.println(F(""));
    Serial.println(F("\nWiFi connected\nIP address: "));
    Serial.println(WiFi.localIP());
    Serial.println(F(""));
    // "connecting"
    drawConnecting(5, 10);

    // Setup EZ Time
    setDebug(INFO);
    waitForSync();
    Serial.println("\nUTC:             " + UTC.dateTime());

    myTZ.setLocation(F(Time_zone));
    Serial.println(F(Time_zone));
    Serial.print(F("Time in your set timezone:"));
    Serial.println(myTZ.dateTime());
  }
  else {
    dma_display->fillScreen(myBLACK);
    dma_display->setTextSize(1);     // size 1 == 8 pixels high
    dma_display->setTextWrap(false);
    dma_display->setTextColor(tetris.tetrisBLUE);
    dma_display->setCursor(0, 0);
    dma_display->print(F("connect to WIFI"));
    delay(4000);
    dma_display->fillScreen(myBLACK);
    dma_display->setCursor(0, 0);
    dma_display->print(F("WIFI:"));
    dma_display->setCursor(0, 10);
    dma_display->print(F("GeniusWeb"));
    dma_display->setCursor(0, 20);
    dma_display->print(F(" no pasw:"));
    dma_display->setCursor(0, 30);
    //dma_display->print(F(""));
    dma_display->setCursor(0, 40);
    dma_display->print(F("Then Go to:"));
    dma_display->setCursor(0, 50);
    dma_display->print(F("192.168.4.1"));
    delay(4000);
  }
  if ((PANEL_RES_Y == 64) && (PANEL_RES_X == 64))drawLogo(); // only if panel is 64x64px
  delay(4000);
  finishedAnimating = false;
  displayIntro = false;
  tetris.scale = ClockScale;
  delay(200);
  dma_display->setBrightness8(brightnessTemp);
  audio.connecttoFS(SD, "/intro.mp3"); // most of the time overritten by the bell sound....
}
/* ************************ Setup Function END ***********************/


/* ************ Function to set time on matrix ***********************/
void setMatrixTime() {
  String timeString = "";
  String AmPmString = "";
  if (twelveHourFormat) {
    // Get the time in format "1:15" or 11:15 (12 hour, no leading 0)
    // Check the EZTime Github page for info on
    // time formatting
    timeString = myTZ.dateTime("g:i");
    //If the length is only 4, pad it with
    // a space at the beginning
    if (timeString.length() == 4) {
      timeString = " " + timeString;
    }
    //Get if its "AM" or "PM"
    AmPmString = myTZ.dateTime("A");
    if (lastDisplayedAmPm != AmPmString) {
      lastDisplayedAmPm = AmPmString;
      // Second character is always "M"
      // so need to parse it out
      tetris2.setText("M", forceRefresh);
      // Parse out first letter of String
      tetris3.setText(AmPmString.substring(0, 1), forceRefresh);
      HandleAlarm();
    }
  } else
  {
    // Get time in format "01:15" or "22:15"(24 hour with leading 0)
    timeString = myTZ.dateTime("H:i");
  }
  // Only update Time if its different
  if (lastDisplayedTime != timeString) {
    // Serial.println(timeString);
    lastDisplayedTime = timeString;
    tetris.setTime(timeString, forceRefresh);
    HandleAlarm();
    // Must set this to false so animation knows
    // to start again
    finishedAnimating = false;
  }
}
/* ************ End Function to set time on matrix ***********************/


/* ************ Function to handle Colon after animation ***********************/
void handleColonAfterAnimation() {
  // It will draw the colon every time, but when the colour is black it
  // should look like its clearing it.
  uint16_t colour =  showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
  // The x position that you draw the tetris animation object
  int x = twelveHourFormat ? CLOCKXPOS : 2;
  // The y position adjusted for where the blocks will fall from
  // (this could be better!)
  int y = RockBottom - (TETRIS_Y_DROP_DEFAULT * tetris.scale);
  tetris.drawColon(COLONXPOS, y, colour);
  dma_display->flipDMABuffer();
}
/* ************  End of Function to handle Colon after animation ***********************/



/* *********************** Main Loop *******************************/
void loop() {
  audio.loop();
  server.handleClient();  // webserver methode that handles all Client
  audio.loop();
  webSocket.loop(); // websocket server methode that handles all Client
  audio.loop();
  unsigned long now = millis();

  if ((now > TickerloopDueBot) && (Ticker_speed_Bot > 0)) {
    if ((finishedAnimating) && (AudioIsPlaying() == false) && (Soundplaycounter == 0))updateTickerBot();
    TickerloopDueBot = now + (100 - Ticker_speed_Bot);
  }
  audio.loop();
  if ((now > TickerloopDueTop) && (Ticker_speed_Top > 0)) {
    if ((finishedAnimating) && (AudioIsPlaying() == false) && (Soundplaycounter == 0))updateTickerTop();
    TickerloopDueTop = now + (100 - Ticker_speed_Top);
  }

  if (now > oneSecondLoopDue) {
    if ((AudioIsPlaying() == false) && (finishedAnimating) && (webserverUpdateMark == true) && (Soundplaycounter == 0)) {
      update_webpage(); // Update
    }
    if (Soundplaycounter > 0)Soundplaycounter--;

#ifdef MemStats
    // Print some memory and sketch info
    Serial.printf ( "Talking Tetris clock is running on CPU %d at %d MHz.  Free memory %d\n",
                    xPortGetCoreID(), ESP.getCpuFreqMHz(), ESP.getFreeHeap() ) ;
#endif

    // We can call this often, but it will only
    // update when it needs to
    setMatrixTime();
    showColon = !showColon;

    // To reduce flicker on the screen we stop clearing the screen
    // when the animation is finished, but we still need the colon to
    // to blink
    if (finishedAnimating) {
      // also placing the name of the day
      dma_display->setTextColor(tetris.tetrisWHITE);
      if (PANEL_CHAIN == 2) {
        dma_display->setCursor(DATEXPOS, DATEYPOS);
        dma_display->print(myTZ.dateTime("l:d:M:Y"));
      }
      else {
        dma_display->setCursor(DATEXPOS, DATEYPOS);
        dma_display->print(myTZ.dateTime("l"));
        dma_display->setCursor(DATEXPOS, DATEYPOS + 8);
        dma_display->print(myTZ.dateTime("M d"));
      }
      dma_display->flipDMABuffer();
      handleColonAfterAnimation();
    }
    oneSecondLoopDue = now + 1000;
  }
  now = millis();
  if (now > animationDue) {
    animationHandler();
    animationDue = now + animationDelay;
  };
}
/* *********************** Main Loop end *******************************/


/* *********************** Function to handle sound *******************************/
void HandleAlarm() {
  // we check markers to play sounds

  int  tr;
  tr = myTZ.hour();
  if ((tr < SilentFrom) && (tr > SilentUntil)) {
#ifdef BeepEveryMinute
    // audio.connecttoFS(SD, "/bell.mp3");
    audio.connecttoFS(SD, MinuteSound);
#endif
    if (myTZ.dateTime("i") == "15")audio.connecttoFS(SD, I15); 
    if (myTZ.dateTime("i") == "30")audio.connecttoFS(SD, I30); 
    if (myTZ.dateTime("i") == "45")audio.connecttoFS(SD, I45); 
    if (myTZ.dateTime("i") == "00")audio.connecttoFS(SD, HourSound); 
  }
  // next the important events thiss will overrule the bell sound
  if (myTZ.dateTime("H:i") == T815)audio.connecttoFS(SD, A815); // diner
  if (myTZ.dateTime("H:i") == T900)audio.connecttoFS(SD, A900); // diner
  if (myTZ.dateTime("H:i") == T930)audio.connecttoFS(SD, A930); // Morning coffee
  if (myTZ.dateTime("H:i") == T1100)audio.connecttoFS(SD, A1100); // Morning coffee
  if (myTZ.dateTime("H:i") == T1200)audio.connecttoFS(SD, A1200); // dinner
  if (myTZ.dateTime("H:i") == T1400)audio.connecttoFS(SD, A1400); // Water time
  if (myTZ.dateTime("H:i") == T1500)audio.connecttoFS(SD, A1500); //afternoon coffee
  if (myTZ.dateTime("H:i") == T1630)audio.connecttoFS(SD, A1630); // Morning coffe


}
/* *********************** Function to handle sound end*******************************/


/* *********************** Function show logo on startup*******************************/
void drawLogo(void) {
  // logo is 46 width and 54 high
  int i = 0;
  // int xyStart[2] = {(kMatrixWidth / 2) - 23, 2};
  int xyStart[2] = { 10, 2};
  dma_display->fillScreen(myBLACK);
  CRGB pix;
  for (int y = 0; y < 50; y++) {
    for (int x = 0; x < 46; x++) {
      pix = logo[i];
      i++;
      dma_display->drawPixelRGB888(x + xyStart[0], y + xyStart[1], pix.r, pix.g, pix.b);
    }
  }
  dma_display->fillRect(5, 53, kMatrixWidth - 10, 11, dma_display->color444(0, 0, 0));
  delay(1000);
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  delay(4000);
}

/* *********************** End Function show logo on startup*******************************/


//**************************************************************************************************
//                                          D B G P R I N T                                        *
//**************************************************************************************************
// Send a line of info to serial output.  Works like vsprintf(), but checks the DEBUG flag.        *
// Print only if DEBUG flag is true.  Always returns the formatted string.                         *
//**************************************************************************************************
char* dbgprint ( const char* format, ... )
{
  static char sbuf[DEBUG_BUFFER_SIZE] ;                // For debug lines
  va_list varArgs ;                                    // For variable number of params
  va_start ( varArgs, format ) ;                       // Prepare parameters
  vsnprintf ( sbuf, sizeof(sbuf), format, varArgs ) ;  // Format the message
  va_end ( varArgs ) ;                                 // End of using parameters
  if ( DEBUGFEEDBACK )                                         // DEBUG on?
  {
    Serial.print (F( "Debug: " )) ;                           // Yes, print prefix
    Serial.println ( sbuf ) ;                          // and the info
  }
  return sbuf ;                                        // Return stored string
}
/*******************************************************************/
/*
  // Replaces placeholder with stored values
  String processor(const String& var) {
  //Serial.println(var);
  if (var == "TopText") {
    return readFile(SPIFFS, "/toptext.txt");
  }
  else if (var == "BotText") {
    return readFile(SPIFFS, "/bottext.txt");
  }
  else if (var == "Time_Zone") {
    return readFile(SPIFFS, "/time_zone.txt");
  }
  return String();
  }
*/
/*************** Function to write to SPIFFS **********************************/
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println(F("- failed to open file for writing"));
    return;
  }
  if (file.print(message)) {
    Serial.println(F("- file written"));
  } else {
    Serial.println(F("- write failed"));
  }
}
/*************** Function to read from SPIFFS **********************************/
String readFile(fs::FS &fs, const char * path) {
  // Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println(F("- empty file or failed to open file"));
    return String();
  }
  // Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  // Serial.println(fileContent);
  return fileContent;
}

///**********************************

/*************** Function to see if file in SPIFFS excists **********************************/
char CheckFile(fs::FS &fs, const char * path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    return 0;
  }
  else
    return 1;
}



/*************** Function to handle web socket  **********************************/
// This function gets a call when a WebSocket event occurs
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
  int paycargo = charArrayToInt(payload, length);
  char buf[102];
  //This function is called because a webclient is active
  // That means we need to keep the webserver data updated
  webserverUpdateMark = true;
  Serial.println(F("webmarker for update set to true; aka someone is connected"));
  dbgprint("cargo: %d\n", paycargo);
  switch (type) {
    case WStype_DISCONNECTED: // enum that read status this is used for debugging.
      Serial.print(F("WS Type "));
      Serial.print(type);
      Serial.println(F(": DISCONNECTED"));
      webserverUpdateMark = false;
      Serial.println(F("webmarker for update set to false; aka nobody is connected"));
      break;
    case WStype_CONNECTED:  // Check if a WebSocket client is connected or not
      Serial.print(F("WS Type "));
      Serial.print(type);
      Serial.println(F(": CONNECTED"));
      update_webpage();// update the webpage accordingly

      break;
    case WStype_TEXT: // check responce from client
      Serial.println("***"); // the payload variable stores teh status internally
      dbgprint("lengte: %d", length);
      if (payload[0] == 'B') {
        Serial.println(F("slider brightness moves"));
        sprintf (buf, "%03i", paycargo);
        dbgprint(buf);
        writeFile(SPIFFS, "/brightness.txt", buf);
        Serial.println(F("********"));
        String testr = readFile(SPIFFS, "/brightness.txt");
        Serial.print(F("*** Your inputString for Brightness: "));
        Serial.println(testr);
        Serial.println(F("********"));
        dma_display->setBrightness8(paycargo);
      }

      if (payload[0] == 'T') {
        Serial.println(F("slider of tetrisspeed moves"));
        sprintf (buf, "%03i", paycargo);
        dbgprint(buf);
        writeFile(SPIFFS, "/tetrisspeed.txt", buf);
        animationDelay = paycargo;
      }
      if (payload[0] == 'U') {
        Serial.println(F("slider of text speed top moves"));
        sprintf (buf, "%03i", paycargo);
        dbgprint(buf);
        writeFile(SPIFFS, "/scrollspeedtop.txt", buf);
        Ticker_speed_Top = paycargo;
      }

      if (payload[0] == 'L') {
        Serial.println(F("slider of text speed bottom moves"));
        sprintf (buf, "%03i", paycargo);
        dbgprint(buf);
        writeFile(SPIFFS, "/scrollspeedbot.txt", buf);
        Ticker_speed_Bot = paycargo;
      }

      if (payload[0] == 'Z') {
        Serial.println(F("Zonebutton clicked"));
        sprintf (buf, "%s", payload);
        char *result  = buf + 1;
        Serial.println(result);
        writeFile(SPIFFS, "/timez.txt", result);
        myTZ.setLocation(F(result));
      }

      if (payload[0] == 'X') { // received time manually
        Serial.println(F("Time clicked"));
        sprintf (buf, "%s", payload);
        char *result  = buf + 1;
        Serial.println(result);

        char *parts[2]; // an array of pointers to the pieces of the above array after strtok()
        char *ptr = NULL;
        byte index = 0;
        ptr = strtok(buf + 1, ":"); // delimiters space and comma
        while (ptr != NULL)
        {
          parts[index] = ptr;
          index++;
          ptr = strtok(NULL, ":");
        }


        // nu nog string.toint om naar de clock te schrijven
        //setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr)
        setTime(String(parts[0]).toInt(), String(parts[1]).toInt(), 0, 27, 11, 1973);
      }

      if (payload[0] == 'Q') {
        Serial.println(F("TopText changed"));
        sprintf (buf, "%s", payload);
        char *result  = buf + 1;
        Serial.println(result);
        writeFile(SPIFFS, "/toptext.txt", result);
        readFile(SPIFFS, "/toptext.txt").toCharArray(TICKER_TOP, TickerLength);
      }

      if (payload[0] == 'R') {
        Serial.println(F("BotText changed"));
        sprintf (buf, "%s", payload);
        char *result  = buf + 1;
        Serial.println(result);
        writeFile(SPIFFS, "/bottext.txt", result);

        readFile(SPIFFS, "/bottext.txt").toCharArray(TICKER_BOT, TickerLength);
        //TICKER_BOT=result;
      }
      if (payload[0] == 'C') { // received time manually
        Serial.println(F("Network update clicked"));
        sprintf (buf, "%s", payload);
        char *result  = buf + 1;
        Serial.println(result);
        char *parts[2]; // an array of pointers to the pieces of the above array after strtok()
        char *ptr = NULL;
        byte index = 0;
        ptr = strtok(buf + 1, ":"); // delimiters space and comma
        while (ptr != NULL)
        {
          parts[index] = ptr;
          index++;
          ptr = strtok(NULL, ":");
        }
        writeFile(SPIFFS, "/ssid.txt", parts[0]);
        writeFile(SPIFFS, "/netpass.txt", parts[1]);

      }

      if (payload[0] == 'D') {
        Serial.println(F("slider of Volume"));
        sprintf (buf, "%03i", paycargo);
        dbgprint(buf);
        writeFile(SPIFFS, "/volume.txt", buf);
        SoundVolume = paycargo;
        audio.setVolume(SoundVolume);
      }

      if (payload[0] == 'Y') {

        Serial.println(F("Display update clicked"));
        sprintf (buf, "%s", payload);
        char *result  = buf + 1;
        Serial.println(result);
        char *parts[2]; // an array of pointers to the pieces of the above array after strtok()
        char *ptr = NULL;
        byte index = 0;
        ptr = strtok(buf + 1, ":"); // delimiters space and comma
        while (ptr != NULL)
        {
          parts[index] = ptr;
          index++;
          ptr = strtok(NULL, ":");
        }
        writeFile(SPIFFS, "/display.txt", parts[0]);
        writeFile(SPIFFS, "/chipset.txt", parts[1]);
        ESP.restart();
      }

      if (payload[0] == 'H') {
        sprintf (buf, "%s", payload);
        Serial.printf("Asked to play sound file: %s\n", buf + 1);
        if (strcmp(buf + 1, "Sound1") == 0) {
          audio.connecttoFS(SD, Sound1);
        }
        else if (strcmp(buf + 1, "Sound2") == 0) {
          audio.connecttoFS(SD, Sound2);
        }
        else if (strcmp(buf + 1, "Sound3") == 0) {
          audio.connecttoFS(SD, Sound3);
        }
        else if (strcmp(buf + 1, "Sound4") == 0) {
          audio.connecttoFS(SD, Sound4);
        }
        else if (strcmp(buf + 1, "Sound5") == 0) {
          audio.connecttoFS(SD, Sound5);
        }
        else if (strcmp(buf + 1, "Sound6") == 0) {
          audio.connecttoFS(SD, Sound6);
        }
        else if (strcmp(buf + 1, "Sound7") == 0) {
          audio.connecttoFS(SD, Sound7);
        }
        else if (strcmp(buf + 1, "Sound8") == 0) {
          audio.connecttoFS(SD, Sound8);
        }
        else if (strcmp(buf + 1, "Sound9") == 0) {
          audio.connecttoFS(SD, Sound9);
        }
        else if (strcmp(buf + 1, "Sound10") == 0) {
          audio.connecttoFS(SD, Sound10);
        }
        else
          Serial.printf("Cannot find that audio file. Check your files and names");
      }

      break;
  }
}

/******************************* Function to update the webpage to latest info **********/

void update_webpage()
{
  StaticJsonDocument<800> doc;
  // create an object
  JsonObject object = doc.to<JsonObject>();
  object["Brightness"] = readFile(SPIFFS, "/brightness.txt");
  object["Animationspeed"] = readFile(SPIFFS, "/tetrisspeed.txt");
  object["Speedtop"] = readFile(SPIFFS, "/scrollspeedtop.txt");
  object["Speedbot"] = readFile(SPIFFS, "/scrollspeedbot.txt");
  object["Tzone"] = readFile(SPIFFS, "/timez.txt");
  object["Matrix"] = readFile(SPIFFS, "/display.txt");
  object["NetSSID"] = readFile(SPIFFS, "/ssid.txt");
  object["NETPWD"] = readFile(SPIFFS, "/netpass.txt");
  object["TopTXT"] = readFile(SPIFFS, "/toptext.txt");
  object["BotTXT"] = readFile(SPIFFS, "/bottext.txt");
  object["Volume"] = readFile(SPIFFS, "/volume.txt");
  object["Display"] = readFile(SPIFFS, "/display.txt");
  object["Chipset"] = readFile(SPIFFS, "/chipset.txt");
  object["EndJson"] = "EndJSON";
  serializeJson(doc, jsonString); // serialize the object and save teh result to teh string variable.
  //  Serial.println( jsonString ); // print the string for debugging.
  webSocket.broadcastTXT(jsonString); // send the JSON object through the websocket
  jsonString = ""; // clear the String.
}

/******************* function to convert char to int *******************/
int charArrayToInt(uint8_t *arr, int length) {
  int i, value, r, flag;
  flag = 1;
  i = value = 0;
  // start at 1 because leaving out the first character
  for ( i = 1 ; i < length ; ++i) {
    // if arr contain negative number
    if ( i == 0 && arr[i] == '-' ) {
      flag = -1;
      continue;
    }
    r = arr[i] - '0';
    value = value * 10 + r;
  }
  value = value * flag;
  return value;
}

//********************* to get settings from SPIFFS or to create default onces.**************************/
void checkSPIFFS() {
  Serial.println(F("\n** The following settings are loaded from Spiffs: **"));
  Serial.println(F(""));

  // Check for Banner text top Display
  if (CheckFile(SPIFFS, "/toptext.txt") == 0) {
    Serial.println(F("toptext.txt not found, creating default one"));
    writeFile(SPIFFS, "/toptext.txt", "Hello and Welcome");
    readFile(SPIFFS, "/toptext.txt").toCharArray(TICKER_TOP, TickerLength);
  }
  else {
    readFile(SPIFFS, "/toptext.txt").toCharArray(TICKER_TOP, TickerLength);
    Serial.printf("Top Text: ");
    Serial.printf(TICKER_TOP);
    Serial.print("\n");
  }

  // Check for Banner text bottom Display
  if (CheckFile(SPIFFS, "/bottext.txt") == 0) {
    Serial.println(F("bottext.txt not found, creating default one"));
    writeFile(SPIFFS, "/bottext.txt", "Have a creative day");
    readFile(SPIFFS, "/bottext.txt").toCharArray(TICKER_BOT, TickerLength);
  }
  else {
    readFile(SPIFFS, "/bottext.txt").toCharArray(TICKER_BOT, TickerLength);
    Serial.printf("Bottom text: ");
    Serial.printf(TICKER_BOT);
    Serial.print("\n");
  }

  // Check for Brightness setting
  if (CheckFile(SPIFFS, "/brightness.txt") == 0) {
    Serial.println(F("brightness.txt not found, creating default one"));
    writeFile(SPIFFS, "/brightness.txt", "040");
  }
  else {
    brightnessTemp = readFile(SPIFFS, "/brightness.txt").toInt();
    Serial.printf("Brightness: %d\n", brightnessTemp);
  }

  // Check for Display setting
  if (CheckFile(SPIFFS, "/display.txt") == 0) {
    Serial.println(F("display.txt not found, creating default one"));
    writeFile(SPIFFS, "/display.txt", "64x64x2NC");
  }
  else {
    DisplayType = readFile(SPIFFS, "/display.txt");
    //readFile(SPIFFS, "/display.txt").toCharArray(DisplayType,10);
    Serial.print("Display type: ");
    Serial.println(DisplayType);
    // Serial.printf("\n");
  }

  // Check for tetrisspeed setting
  if (CheckFile(SPIFFS, "/tetrisspeed.txt") == 0) {
    Serial.println(F("tetrisspeed.txt not found, creating default one"));
    writeFile(SPIFFS, "/tetrisspeed.txt", "040");
  }
  else {
    animationDelay = readFile(SPIFFS, "/tetrisspeed.txt").toInt();
    Serial.printf("Animation speed clock: %d\n", animationDelay);
  }

  // Check for Scrollspeed top text
  if (CheckFile(SPIFFS, "/scrollspeedtop.txt") == 0) {
    Serial.println(F("scrollspeedtop.txt not found, creating default one"));
    writeFile(SPIFFS, "/scrollspeedtop.txt", "040");
  }
  else {
    Ticker_speed_Top = readFile(SPIFFS, "/scrollspeedtop.txt").toInt();
    Serial.printf("Scrolling Speed top text: %d\n", Ticker_speed_Top);
  }

  // Check for Scrollspeed bottom text
  if (CheckFile(SPIFFS, "/scrollspeedbot.txt") == 0) {
    Serial.println(F("scrollspeedbot.txt not found, creating default one"));
    writeFile(SPIFFS, "/scrollspeedbot.txt", "040");
  }
  else {
    Ticker_speed_Bot = readFile(SPIFFS, "/scrollspeedbot.txt").toInt();
    Serial.printf("Scrolling Speed bottom text: %d\n", Ticker_speed_Bot);
  }

  // Check for Timezone setting
  if (CheckFile(SPIFFS, "/timez.txt") == 0) {
    Serial.println(F("timez.txt not found, creating default one"));
    writeFile(SPIFFS, "/timez.txt", "Europe/Amsterdam");
  }
  else {
    //readFile(SPIFFS, "/timez.txt").toCharArray(Time_zone,readFile(SPIFFS, "/timez.txt").length()+1);
    readFile(SPIFFS, "/timez.txt").toCharArray(Time_zone, TimezoneBufferlenght);
    Serial.printf("time zone from spiffs: ");
    Serial.printf(Time_zone);
    Serial.printf("\n");
  }

  // Check for SSID Network setting
  if (CheckFile(SPIFFS, "/ssid.txt") == 0) {
    Serial.println(F("ssid.txt not found, creating default one"));
    writeFile(SPIFFS, "/ssid.txt", "emdee");
  }
  else {
    readFile(SPIFFS, "/ssid.txt").toCharArray(ssid, ssidbuflength);
    Serial.printf("ssid from spiffs: ");
    Serial.printf(ssid);
    Serial.printf("\n");
  }

  // Check for pasword setting network
  if (CheckFile(SPIFFS, "/netpass.txt") == 0) {
    Serial.println(F("netpass.txt not found, creating default one"));
    writeFile(SPIFFS, "/netpass.txt", "MARKDONNERS01");
  }
  else {
    readFile(SPIFFS, "/netpass.txt").toCharArray(password, passwordbuflength);
    Serial.printf("password from spiffs: ");
    Serial.printf(password);
    Serial.printf("\n");
  }

  // Check for pasword setting network
  if (CheckFile(SPIFFS, "/volume.txt") == 0) {
    Serial.println(F("volume.txt not found, creating default one"));
    writeFile(SPIFFS, "/volume.txt", "8");
  }
  else {
    SoundVolume = readFile(SPIFFS, "/volume.txt").toInt();
    Serial.printf("Volume: %d\n", SoundVolume);
  }
  // Check for chipset display
  if (CheckFile(SPIFFS, "/chipset.txt") == 0) {
    Serial.println(F("chipset.txt not found, creating default one"));
    writeFile(SPIFFS, "/chipset.txt", "MBI5124");
  }
  else {

    Chipset = readFile(SPIFFS, "/chipset.txt");
    Serial.print("Chipset: ");
    Serial.println(Chipset);
  }

  Serial.println(F(""));
}


/*********** function to check if audio is playing **********/
boolean AudioIsPlaying() {
  if (audio.getAudioCurrentTime() < audio.getAudioFileDuration()) {
    Soundplaycounter = 3;
    return true;
  }
  else return false;
}
/******************* Function to update ticker on bottom of display *****************/
void  updateTickerBot() {
  dma_display->setTextSize(1);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false);
  dma_display->setCursor(3, 3);
  dma_display->fillRect(0, 54, dma_display->width(), 9, dma_display->color444(0, 0, 0));
  dma_display->setTextColor(tetris.tetrisRED);
  dma_display->setCursor(ticker_current_pos_bot, TickerYPosBot);
  dma_display->print(TICKER_BOT);
  dma_display->flipDMABuffer();
  ticker_current_pos_bot--;
  if (ticker_current_pos_bot == ticker_end_point)ticker_current_pos_bot = ticker_start_point;
  Ticker_speed_Bot = readFile(SPIFFS, "/scrollspeedbot.txt").toInt(); // workaround
}
/******************* Function to update ticker on top of display *****************/
void  updateTickerTop() {
  dma_display->setTextSize(1);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false);
  dma_display->setCursor(3, 3);
  dma_display->fillRect(0, 1, dma_display->width(), 9, dma_display->color444(0, 0, 0));
  dma_display->setTextColor(tetris.tetrisBLUE);
  dma_display->setCursor(ticker_current_pos_top, TickerYPosTop);
  dma_display->print(TICKER_TOP);
  dma_display->flipDMABuffer();
  ticker_current_pos_top--;
  if (ticker_current_pos_top == ticker_end_point)ticker_current_pos_top = ticker_start_point;
  Ticker_speed_Top = readFile(SPIFFS, "/scrollspeedtop.txt").toInt(); // workaround
}
