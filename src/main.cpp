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
 
 RF24 radio(2, 4);  // nRF24L01(+) radio attached using Getting Started board
 
 RF24Network network(radio);      // Network uses that radio
 const uint16_t this_node = 00;   // Address of our node in Octal format (04, 031, etc)
 const uint16_t other_node = 01;  // Address of the other node in Octal format
 
 
 
 void setup(void) {
    Serial.begin(115200);
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
    network.begin(/*node address*/ this_node);
 }
 
 void loop(void) {
 
    network.update();  // Check the network regularly
  
    while (network.available()) {  // Is there anything ready for us?
    
        RF24NetworkHeader header;  // If so, grab it and print it out
        radioQueueData buffer;
        network.read(header, &buffer, sizeof(radioQueueData));
		log_message* msg = (log_message*) buffer.data;
		chunk_data* chunk = (chunk_data*) buffer.data;

        switch(buffer.messageType){
            case LOG_MESSAGE:
                Serial.print(msg->text);
                break;
			case CHUNK_DATA:
				//Serial.print("Chunk Pos X: ");
				//Serial.print(chunk->position.x);
				//Serial.print(" Y: ");
				//Serial.print(chunk->position.y);
				//Serial.print(" Chunk Subdivision: ");
				//Serial.println(chunk->subdivision);
				break;
			default:
				Serial.println("Received another type of data");
                
				
        }

    }
 }