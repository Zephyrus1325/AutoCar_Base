/**
 * Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Update 2014 - TMRh20
 */

/**
 * Simplest possible example of using RF24Network,
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */

 #include <SPI.h>
 #include <RF24.h>
 #include <RF24Network.h>
 
 #include "radioEnum.h"
 #include "radioParse.h"

 RF24 radio(2, 4);  // nRF24L01(+) radio attached using Getting Started board
 
 RF24Network network(radio);      // Network uses that radio
 const uint16_t this_node = 00;   // Address of our node in Octal format (04, 031, etc)
 const uint16_t car_address = 01;  // Address of the other node in Octal format
 
 uint64_t lastTime = 0;

 void setup(void) {
    Serial.begin(921600);
    Serial.println(F("AutoCar V3 - Base Node"));
   
    if (!radio.begin()) {
        Serial.println(F("Radio hardware not responding!"));
        while (1) {
           vTaskDelay(100);
          // hold in infinite loop
        }
    }
    radio.setChannel(90);
	radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_MIN);
    network.begin(/*node address*/ this_node);
 }
 
 void loop(void) {
 
    network.update();  // Check the network regularly

    while (network.available()) {  // Is there anything ready for us?
    
        RF24NetworkHeader header;  // If so, grab it and print it out
        uint8_t buffer[600];
        for(int i = 0; i < sizeof(buffer); i++){
            buffer[i] = 0;
        }
        uint16_t messlen = network.read(header, &buffer, sizeof(buffer));
        
        if(buffer[0] == CHUNK_DATA && messlen == sizeof(chunk_data)){
            sendChunk((chunk_data*) buffer);
        }
        if(buffer[0] == COMMAND && messlen == sizeof(command)){
            readCommand((command*) buffer);
        }

    }

    if(millis() - lastTime > 100){
        RF24NetworkHeader header(car_address);
        uint8_t buffer[100];
        network.write(header, &buffer, 32);
        lastTime = millis();
    }

    if(Serial.available()){
        int command = Serial.read();
        radioQueueData buffer;
        switch(command){
            case 0:
                buffer.messageType = STOP;
                break;
            case 1:
                buffer.messageType = GO_FORWARD;
                break;
            case 2:
                buffer.messageType = GO_BACK;
                break;
            case 3:
                buffer.messageType = GO_LEFT;
                break;
            case 4:
                buffer.messageType = GO_RIGHT;
                break;
            default:
                buffer.messageType = 0xFF;
                break;
        }

       
        RF24NetworkHeader header(car_address);
        
        //network.write(header, &buffer, sizeof(radioQueueData));

    }
    
 }