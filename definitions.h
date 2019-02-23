// *********************************************************************
// Holds all central definitions and globals
// *********************************************************************

Ticker display_ticker;          // Ticker that triggers redraw of the screen
Ticker timer_ticker;            // Ticker that triggers one revolution in the falling animation

// NTP
int8_t timeZone = -5;            // Timezone for NTP client
int8_t minutesTimeZone = 0;     // Minutes difference for time zone

// Globals
uint8_t init_state = 0;         // Hold the current state of the initialization process
String str_display_time = "0000"; // Holds the currently displayed time as string
bool seconds_odd = false;       // True when the seconds are currently odd

// WiFiManager
WiFiManager wifiManager;        // Global WiFiManager object
#define AP_NAME "tetris_clock"
#define AP_PASS "tetromino"
#define AP_TIMEOUT 300


// Pins for LED MATRIX
#define P_LAT D0
#define P_A D1
#define P_B D2
#define P_C D8
#define P_OE D4
PxMATRIX display(32,16, P_LAT, P_OE,P_A,P_B,P_C);
