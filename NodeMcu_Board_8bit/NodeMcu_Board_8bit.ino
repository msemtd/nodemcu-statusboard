/**
 *  Based on the example StreamHTTPClient.ino
 *  Just a tester for simple status board HTTP client - connects to a simple NodeJS server to get some digital outputs
 *
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;

#include "private.h"
#ifndef PRIVATE_WIFI_AP_NAME
#error "You need to define some variables in private.h or perhaps here..."
#define PRIVATE_WIFI_AP_NAME "your_access_point"
#define PRIVATE_WIFI_AP_PASS "your_ap_password"
#define PRIVATE_NODEJS_SERVER "www.your_server.org"
#define PRIVATE_NODEJS_PORT 80
#define PRIVATE_NODEJS_PATH "/statusboards/1/nodemcutest"
#endif

bool testmode = false;

uint32_t getFreq = 10000;
uint32_t getLast = 0;

// this is our mapping from bits to pin constants
// we don't want to use D4 on the NodeMCU - that's the blue LED!
const uint8_t pmap[8] = { D1, D2, D3, 0xFF, D5, D6, D7, D8 };

#define DIAG_INPUT_MAX 128
String consoleInput = "";

void setup() {
    consoleInput.reserve(DIAG_INPUT_MAX);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    for(uint8_t i = 0; i < 8; i++)
      if(pmap[i] != 0xFF) pinMode(pmap[i], OUTPUT);
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
        set_digital_outs((t%2) ? 0xFF : 0x00);
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
    if(testmode){
        static uint8_t step = 0;
        set_digital_outs(bit(step));
        step = (++step)%8;
        //static bool tog = false;
        //set_digital_outs(tog ? 0x55 : 0xAA);
        //tog = !tog;
        delay(1000);
    } else {
        normal_mode();
    }
}

void set_digital_outs(uint8_t b) {
  Serial.printf("set_digital_outs %02X...\n", b);
  for(uint8_t i = 0; i < 8; i++)
    if(pmap[i] != 0xFF) digitalWrite(pmap[i], bitRead(b, i));
}

void normal_mode() {
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
    // TODO buffer and inspect - look for markers etc.
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
    char cmd = consoleInput[0];
    if(cmd == '?') {
        Serial.printf("\n? = HELP COMMAND\n");
        Serial.printf("\nCommands available: A, B, C (case sensitive).\n");
        // blah, blah, blah...
    } else if(cmd == 'A') {
        Serial.printf("\nAY?\n");
    } else if(cmd == 'B') {
        Serial.printf("\nBe what?\n");
    } else if(cmd == 'C') {
        Serial.printf("\nI don't see anything!\n");
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
            // get length of document (is -1 when Server sends no Content-Length header)
            int len = http.getSize();
            // create buffer for read
            uint8_t buff[128] = { 0 };
            // get tcp stream
            WiFiClient * stream = http.getStreamPtr();
            // read all data from server
            while(http.connected() && (len > 0 || len == -1)) {
                // get available data size
                size_t size = stream->available();
                if(size) {
                    // read up to 128 byte
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    // write it to Serial
                    Serial.write(buff, c);
                    if(len > 0) {
                        len -= c;
                    }
                }
                delay(1);
            }
            Serial.println();
            Serial.print("[HTTP] connection closed or file end.\n");
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}
