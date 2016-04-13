/**
 *  Based on the example StreamHTTPClient.ino
 *  Just a tester for simple status board HTTP client - connects to a simple NodeJS server to get some digital outputs
 *  The nodeboard server responds to the GET with "BOARD:<n>:<XX>" 
 *  where n is the board number and XX is 8-bit value as leading zero padded 2 digit hex.
 *  
 *  TODO: support boards other than 1 - this should be soft configurable
 *
 *  TODO: present the startup state in some manner (STARTUP, GET_CONFIG, CONNECT_AP, AP_OK, GET_NODE_START, RUNNING, LOST_CONN, etc)
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ctype.h>

#define MSEPRODUCT "NodeMCU Board 8bit"
#define MSEVERSION "v1.1"

ESP8266WiFiMulti WiFiMulti;

#include "private.h"
#ifndef PRIVATE_WIFI_AP_NAME
#error "You need to define some variables in private.h or perhaps here..."
#define PRIVATE_WIFI_AP_NAME "your_access_point"
#define PRIVATE_WIFI_AP_PASS "your_ap_password"
#define PRIVATE_NODEJS_SERVER "www.your_server.org"
#define PRIVATE_NODEJS_PORT 80
#define PRIVATE_NODEJS_PATH "/sb/1/nodemcu"
#endif

uint32_t getFreq = 10000;
uint32_t getLast = 0;

#define DIAG_INPUT_MAX 128
String consoleInput = "";

void setup() {
    consoleInput.reserve(DIAG_INPUT_MAX);
    Serial.begin(115200);   // USB UART
    Serial1.begin(9600);    // serial UART direct onto the ProMicro pins
    Serial.setDebugOutput(true);
    Serial.print("Starting " MSEPRODUCT " " MSEVERSION "\n\n");    
    Serial.println();
    // for(uint8_t i = 0; i < 8; i++)
    //   if(pmap[i] != 0xFF) pinMode(pmap[i], OUTPUT);
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
        //set_digital_outs((t%2) ? 0xFF : 0x00);
    }
    WiFiMulti.addAP(PRIVATE_WIFI_AP_NAME, PRIVATE_WIFI_AP_PASS);
#ifdef PRIVATE_WIFI_AP_NAME2
    WiFiMulti.addAP(PRIVATE_WIFI_AP_NAME2, PRIVATE_WIFI_AP_PASS2);
#endif
#ifdef PRIVATE_WIFI_AP_NAME3
    WiFiMulti.addAP(PRIVATE_WIFI_AP_NAME3, PRIVATE_WIFI_AP_PASS3);
#endif
}

void loop() {
     uint32_t now = millis();
    if(now - getLast >= getFreq) {
        getLast = now;
        getServerData();
    }
    while(Serial.available()){
      proc_console_input(Serial.read());
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
            //setItems(val);
            Serial1.println(consoleInput);
        }
        Serial.print("\nOK\n");
    } else if(cmd == 'E') {
        // E = Enable digital outputs E<XX> where X is hex digit
        if(len == 3) {
            uint8_t val = hexdigs(consoleInput[1], consoleInput[2]);
            //enableItems(val);
            Serial1.println(consoleInput);            
        }
        Serial.print("\nOK\n");
     } else {
        Serial.printf("\nI don't understand the command %c - try '?' for help.\n");
    }
}


void getServerData() {
    // wait for WiFi connection
    if((WiFiMulti.run() != WL_CONNECTED))
        return;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure server and url
    http.begin(PRIVATE_NODEJS_SERVER, PRIVATE_NODEJS_PORT, PRIVATE_NODEJS_PATH);
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                procBoard(payload);
            }
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void procBoard(const String& info) {
    Serial.println("Server says...");
    Serial.println(info);
    if(!info.startsWith("BOARD:1:") || info.length() != 10){
        Serial.println("not our board response");
        return;
    }
    // if we want some normalisation...
    // uint8_t v = hexdigs(info.charAt(8), info.charAt(9));
    // Serial1.printf("D%02X\n", v);
    Serial1.printf("D%c%c\n", info.charAt(8), info.charAt(9));
    Serial.printf("Sent D%c%c to I/O board\n", info.charAt(8), info.charAt(9));
}

uint8_t hexdig(uint8_t c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'A' && c <= 'F') return c - 'A' + 10; 
    if(c >= 'a' && c <= 'f') return c - 'a' + 10; 
    return 0;
}

uint8_t hexdigs(uint8_t c1, uint8_t c2) {
    return (hexdig(c1) << 4) | hexdig(c2);
}
