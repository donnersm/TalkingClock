/*************************************************************************************************
* credentials for connecting to WIFI Access point are: GeniusWeb , password: Genius              *
* Settings content:                                                                              *
* - Matrix configuration, do not change if you use the original PCB from my Tindie.store         *
*    https://www.tindie.com/products/26408/                                                      *
* - Audio I2s amplifier settings, do not change if you use the original PCB from my Tindie.store *
* - Audio files setup, files are stored on SD card                                               *
* - Other settings                                                                               *
**************************************************************************************************/


// -------------------------------------
// -------   Matrix Config   ------
// -------------------------------------
// setup below is for hardware from my tindie store
// Take note, the wiring is different fromt the original Tetris clock library
#define A_PIN         23
#define B_PIN         19
#define C_PIN         5
#define D_PIN         17
#define E_PIN         22
#define LAT_PIN       18
#define OE_PIN        15
#define CLK_PIN       16
#define R1_PIN        33
#define B2_PIN        13
#define R2_PIN        14
#define G2_PIN        12
#define G1_PIN        26
#define B1_PIN        27
#define kMatrixWidth  128

// -------------------------------------
// -------   Audio Config   ------
// -------------------------------------
// Digital I/O used for soundboard and SD
#define SD_CS         35
#define SPI_MOSI      2  
#define SPI_MISO      34
#define SPI_SCK       21
#define I2S_DOUT      25
#define I2S_BCLK      32  
#define I2S_LRC       4 
#define I2S_NUM       0  

// -------------------------------------
// -------   Audio files   ------
// -------------------------------------
// sounds definitions under buttons
static char Sound1[]= "eigenhuis.wav";
static char Sound2[]= "/bell.mp3";  // bell sound
static char Sound3[]= "/intro.mp3"; // Shoutout: The Simpsons
static char Sound4[]= "/fail.mp3"; 
static char Sound5[]= "/1500.wav";
static char Sound6[]= "/1200.mp3";
static char Sound7[]= "/Hour.wav";
static char Sound8[]= "/ugly.mp3";
static char Sound9[]= "/manager.mp3";
static char Sound10[]= "/3.wav"; // voice saying: did you just take a crap on my desk

// sounds that are predefinded to play at set times.
static char A815[]= "/815.mp3"; // sexy voice saying: Goodmorning
static char A900[]= "/900.mp3"; // Homer saying: Goodmorning son!
static char A930[]= "/930.mp3"; // Homor saying: A cup of coffee?
static char A1100[]= "/1100.wav"; // Morning coffee
static char A1200[]= "/1200.mp3"; // voice: purhaps you join me for dinner
static char A1400[]= "/1400.mp3"; // Marge saying: You may wanne have a drink
static char A1500[]= "/1500.wav"; //Voice: Give me some coffee
static char A1630[]= "/1630.mp3"; // Mr. Burns saying: See you back at the office don't be late.

// these are the times above sounds will be activated
static char T815[]= "8:15"; // diner
static char T900[]= "9:00"; // diner
static char T930[]= "9:30"; // Morning coffee
static char T1100[]= "11:00"; // Morning coffee
static char T1200[]= "12:00"; // dinner
static char T1400[]= "14:00"; // Water time
static char T1500[]= "15:00"; //afternoon coffee
static char T1630[]= "16:30"; // Morning coffee

// bell sound every Hour, minute, 15mins, 30mins 
static char MinuteSound[] = "/bell.mp3";
static char HourSound[]= "/Hour.wav"; // dinner
static char I15[]= "/bell.mp3"; // Water time
static char I30[]= "/30.wav"; //afternoon coffee
static char I45[]= "/bell.mp3"; // Morning coffee


// -------------------------------------
// -------   Other Settings   ------
// -------------------------------------

//sound related
int SoundVolume = 8; // playback volume, overules by the value stored in memory, change is on the webpage

// In between these hours sounds are muted
#define SilentFrom  21
#define SilentUntil 9

// comment if you dont want a sound every minute
//#define BeepEveryMinute 

// Debug on/off, set to 1 if you want bedug info on serial port
int   DEBUGFEEDBACK = 0 ;       


// Sets whether the clock should be 12 hour format or not.
bool twelveHourFormat = false;
#define PMAMXPOS  116   // if clockscale 2,3,4 --> PMAMXPOS 56,88,118, however, I didn't use it..it should be functional

// If this is set to false, the number will only change if the value behind it changes
// e.g. the digit representing the least significant minute will be replaced every minute,
// but the most significant number will only be replaced every 10 minutes.
// When true, all digits will be replaced every minute.
bool forceRefresh = false;


// comment out if you don't want the Memory statistics to be printed to serial port all the time.
//#define MemStats 
  
// If you change ticker lenght, you also need to change the webpage element for the tickers(max length)
#define TickerLength  101 // This is the lenght of the text tickers on top and bottom of screen
