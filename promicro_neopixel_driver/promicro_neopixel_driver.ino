/**
 * ProMicro LED driver: 8 pairs of LEDs or Bi-Colour LEDs on pairs of outputs
 */

#define MSEPRODUCT "ProMicro NeoPixel driver"
#define MSEVERSION "v1.0"
#include <ctype.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Supporting a single NeoPixel on pin 21 which is A3, closest to VCC & GND
#define NEOPIN        21
#define NUMPIXELS      1

const int RXLED = 17;
                        
#define DIAG_INPUT_MAX 128
String consoleInput = "";

static uint32_t getLast = 0;
static uint32_t getFreq = 1000;
static uint8_t testModeState = 0;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

void setup() {
    TX_RX_LED_INIT;
    pinMode(RXLED, OUTPUT);
    TXLED0;
    RXLED0;
    consoleInput.reserve(DIAG_INPUT_MAX);
    Serial.begin(9600);     // serial over the USB when connected
    Serial1.begin(9600);    // serial UART direct onto the ProMicro pins
    Serial.print("Starting " MSEPRODUCT " " MSEVERSION "\n\n");
    pixels.begin();
    pixels.setBrightness(20); // default brightness
    pixels.setPixelColor(0, pixels.Color(255,255,0));
    pixels.show();
}

void loop() {
    static uint32_t now;
    now = millis();
    if(now - getLast >= getFreq) {
        getLast = now;
        testCrank();
    }
    while(Serial.available()){
        proc_console_input(Serial.read());
    }
    while(Serial1.available()){
        int i = Serial1.read();
        Serial.print("nodemcu: ");
        Serial.print(i);
        Serial.print("\n");
        proc_console_input(i);
    }
}

void testCrank() {
    if(testModeState == 0) {
        return; 
    } else if(testModeState == 1) {
        Serial.print("Yo test RED!\n");
        pixels.setPixelColor(0, pixels.Color(255,0,0));
        pixels.show();
        testModeState++;
    } else if(testModeState == 2) {
        Serial.print("Yo test GREEN!\n");
        pixels.setPixelColor(0, pixels.Color(0,255,0));
        pixels.show();
        testModeState++;
    } else if(testModeState == 3) {
        Serial.print("Yo test BLUE!\n");
        pixels.setPixelColor(0, pixels.Color(0,0,255));
        pixels.show();
        testModeState++;
    } else if(testModeState == 4) {
        Serial.print("Yo test OFF!\n");
        pixels.setPixelColor(0, pixels.Color(0,0,0));
        pixels.show();
        testModeState = 1;
    } else {
        testModeState = 0;
    }
}

void proc_console_input(int data) {
    // input is processed upon CR or NL...
    if(data == 0x0A || data == 0x0D){
        proc_console_command();
        consoleInput = "";
        return;
    }
    if(consoleInput.length() < DIAG_INPUT_MAX){
        consoleInput += (char)data;
        return;
    }
    // would overflow: just truncate (could retain buffer but meh!)
    consoleInput = "";
}

void proc_console_command() {
    // app-specific console command input
    consoleInput.trim();
    int len = consoleInput.length();
    if(!len) 
        return;
    char cmd = toupper(consoleInput[0]);
    if(cmd == '?') {
        Serial.print("\n? = HELP COMMAND\n");
        Serial.print("Commands available: -\n");
        Serial.print("  A = Ay?\n");
        Serial.print("  B = Brightness: B<XX>, where XX is hex, e.g. BFF for max, B00 for min\n");
        Serial.print("  C = Set Colour: C<RRGGBB>, all hex, e.g. C5522CC\n");
        //Serial.print("  D = Set digital items\n");
        //Serial.print("  E = Enable digital items\n");
        Serial.print("  T = test mode start\n");
        Serial.print("  S = test mode stop\n");
    } else if(cmd == 'A') {
        Serial.print("\nAY?\n");
    } else if(cmd == 'B') {
        Serial.print("\nBrightness...\n");
        if(len != 3 || !is_hexdig(consoleInput[1]) || !is_hexdig(consoleInput[2])) {
            Serial.print("usage: B<XX>, where XX is hex, e.g. BFF for max, B00 for min\n");
        } else {
            uint8_t val = hexdigs(consoleInput[1], consoleInput[2]);
            pixels.setBrightness(val); // default brightness
            pixels.show();
            Serial.print("\nOK\n");
        }
    } else if(cmd == 'C') {
        Serial.print("\nColour...\n");
        if(len != 7 || !is_hexdig(consoleInput[1]) || !is_hexdig(consoleInput[2]) || !is_hexdig(consoleInput[3]) || !is_hexdig(consoleInput[4]) || !is_hexdig(consoleInput[5]) || !is_hexdig(consoleInput[6])) {
            Serial.print("usage: C<RRGGBB>, all hex, e.g. C5522CC\n");
        } else {
            uint8_t rr = hexdigs(consoleInput[1], consoleInput[2]);
            uint8_t gg = hexdigs(consoleInput[3], consoleInput[4]);
            uint8_t bb = hexdigs(consoleInput[5], consoleInput[6]);
            pixels.setPixelColor(0, pixels.Color(rr,gg,bb));
            pixels.show();
            Serial.print("\nOK\n");
        }            
    } else if(0 && cmd == 'D') {
        // D = Digital outputs set D<XX> where X is hex digit
        if(len == 3) {
            uint8_t val = hexdigs(consoleInput[1], consoleInput[2]);
            // setItems(val);
        }
        Serial.print("\nOK\n");
    } else if(0 && cmd == 'E') {
        // E = Enable digital outputs E<XX> where X is hex digit
        if(len == 3) {
            uint8_t val = hexdigs(consoleInput[1], consoleInput[2]);
            // enableItems(val);
        }
        Serial.print("\nOK\n");
    } else if(cmd == 'T') {
        Serial.print("\nTest mode...\n");
        testModeState = 1;
    } else if(cmd == 'S') {
        Serial.print("\nStop test mode...\n");
        testModeState = 0;
    } else {
        if(isalnum(cmd)) {
            Serial.print("\nI don't understand the command ");
            Serial.print(cmd);
            Serial.print(" - try '?' for help.\n");
        }
    }
}

uint8_t hexdig(uint8_t c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'A' && c <= 'F') return c - 'A' + 10; 
    if(c >= 'a' && c <= 'f') return c - 'a' + 10; 
    return 0;
}

uint8_t is_hexdig(uint8_t c) {
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

uint8_t hexdigs(uint8_t c1, uint8_t c2) {
    return (hexdig(c1) << 4) | hexdig(c2);
}

