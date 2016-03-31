/**
 * ProMicro LED driver: 8 pairs of LEDs or Bi-Colour LEDs on pairs of outputs
 */

#define MSEPRODUCT "ProMicro LED driver"
#define MSEVERSION "v1.0"
#include <ctype.h>

const int RXLED = 17;
const uint8_t d_map[16] = {2,3,4,5,6,7,8,9,
                           10,16,14,15,18,19,20,21};
static uint8_t en_map = 0xFF;
                        
#define DIAG_INPUT_MAX 128
String consoleInput = "";

static uint32_t getLast = 0;
static uint32_t getFreq = 1000;
static uint8_t testModeState = 0;
                           
void setup() {
    TX_RX_LED_INIT;
    pinMode(RXLED, OUTPUT);
    TXLED0;
    RXLED0;
    pinModeAll(OUTPUT);
    consoleInput.reserve(DIAG_INPUT_MAX);
    Serial.begin(9600);     // serial over the USB when connected
    Serial1.begin(115200, SERIAL_8E1);    // serial UART direct onto the ProMicro pins
    Serial.print("Starting " MSEPRODUCT " " MSEVERSION "\n\n");
    for (int i = 0; i < 16; i++) { // all off
        digitalWrite(d_map[i], LOW);
    }
}

bool itemOk(uint8_t item) {
    return (item >= 0 && item < 8);
}
void pinModeAll(int m) {
    for (int i = 0; i < 16; i++) {
        pinMode(d_map[i], m);
    }
}
void setItemRed(uint8_t item) {
    if(!itemOk(item)) return;
    digitalWrite(d_map[(item*2) + 1], LOW);
    digitalWrite(d_map[item*2], HIGH);
}
void setItemGreen(uint8_t item) {
    if(!itemOk(item)) return;
    digitalWrite(d_map[item*2], LOW);
    digitalWrite(d_map[(item*2) + 1], HIGH);
}
void setItemOff(uint8_t item) {
    if(!itemOk(item)) return;
    digitalWrite(d_map[item*2], LOW);
    digitalWrite(d_map[(item*2) + 1], LOW);
}
void setItems(uint8_t val) {
    for (int i = 0; i < 8; i++) {
        if(!(en_map & 1 << i)) {
            setItemOff(i);
            continue;
        }
        if(val & (1 << i))
            setItemGreen(i);
        else 
            setItemRed(i);
    }
}
void enableItems(uint8_t val) {
    en_map = val;
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
    } else if(testModeState == 1) {
        Serial.print("Yo test RED!\n");
        setItemRed(0);
        testModeState = 2;
    } else if(testModeState == 2) {
        Serial.print("Yo test GREEN!\n");
        setItemGreen(0);
        testModeState = 3;
    } else if(testModeState == 3) {
        Serial.print("Yo test OFF!\n");
        setItemOff(0);
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
        Serial.print("  B = Be\n");
        Serial.print("  C = See\n");
        Serial.print("  D = Set digital items\n");
        Serial.print("  E = Enable digital items\n");
        Serial.print("  T = test mode start\n");
        Serial.print("  S = test mode stop\n");
    } else if(cmd == 'A') {
        Serial.print("\nAY?\n");
    } else if(cmd == 'B') {
        Serial.print("\nBe what?\n");
    } else if(cmd == 'C') {
        Serial.print("\nI don't see anything!\n");
    } else if(cmd == 'D') {
        // D = Digital outputs set D<XX> where X is hex digit
        if(len == 3) {
            uint8_t val = hexdigs(consoleInput[1], consoleInput[2]);
            setItems(val);
        }
        Serial.print("\nOK\n");
    } else if(cmd == 'E') {
        // E = Enable digital outputs E<XX> where X is hex digit
        if(len == 3) {
            uint8_t val = hexdigs(consoleInput[1], consoleInput[2]);
            enableItems(val);
        }
        Serial.print("\nOK\n");
    } else if(cmd == 'T') {
        testModeState = 1;
    } else if(cmd == 'S') {
        testModeState = 0;
    } else {
        if(isalnum(cmd)) {
            Serial.print("\nI don't understand the command ");
            Serial.print(cmd);
            Serial.print(" - try '?' for help.\n");
        }
    }
}

// TODO look up a better version!
uint8_t hexdig(uint8_t c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'A' && c <= 'F') return c - 'A' + 10; 
    if(c >= 'a' && c <= 'f') return c - 'a' + 10; 
    return 0;
}

uint8_t hexdigs(uint8_t c1, uint8_t c2) {
    return (hexdig(c1) << 4) | hexdig(c2);
}


