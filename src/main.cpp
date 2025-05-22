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


uint8_t transmitRole = 0; // Always Start listening

uint64_t timeOutTimer = 0;


QueueHandle_t dataQueue;

void transmitData(){
    radioQueueData buffer;
    if(xQueueReceive(dataQueue, &buffer, 0)){
        RF24NetworkHeader header(car_address);
        //Serial.println("TX DATA");
        if(!network.write(header, &buffer, sizeof(radioQueueData))){
            xQueueSend(radioQueue, &buffer, 0);
        }
    }
}

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
    dataQueue = xQueueCreate(100, sizeof(radioQueueData));
    if(dataQueue == NULL){
        Serial.println("Data queue not created!");
    }
	radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    network.begin(90, this_node);
}
 
void loop(void) {
 
    network.update();  // Check the network regularly
    if(transmitRole){
        if(!uxQueueMessagesWaiting(dataQueue)){
            RF24NetworkHeader endheader(car_address);
            command comm{END_OF_PACKET};
            radioQueueData radioData{COMMAND};
            memcpy(&radioData.data, &comm, sizeof(command));
            transmitRole = 0;
            timeOutTimer = millis();
            while(!network.write(endheader, &radioData, sizeof(radioData))){vTaskDelay(100);}
            //Serial.println("SENT EOP");
        } else {
            transmitData();
        }
    } else {
        while (network.available()) {  // Is there anything ready for us?
            
            RF24NetworkHeader header;  // If so, grab it and print it out
            uint8_t buffer[600];
            for(int i = 0; i < sizeof(buffer); i++){
                buffer[i] = 0;
            }
            
            uint16_t messlen = network.read(header, &buffer, sizeof(buffer));
            radioQueueData* radioData = (radioQueueData*) &buffer;

            //Serial.print(buffer[0]);
            //Serial.print(" ");
            //Serial.println(messlen);


            switch(buffer[0]){
                case CHUNK_DATA:
                    sendChunk((chunk_data*) buffer);
                    break;
                case COMMAND:
                    readCommand((command*) radioData->data);
                    break;
            }
        }
    }

    if(Serial.available()){
        int com = Serial.read();
        command comm;
        switch(com){
            case 0:
                comm.command_type = STOP;
                break;
            case 1:
                comm.command_type = GO_FORWARD;
                break;
            case 2:
                comm.command_type = GO_BACK;
                break;
            case 3:
                comm.command_type = GO_LEFT;
                break;
            case 4:
                comm.command_type = GO_RIGHT;
                break;
            default:
                comm.command_type = 0xFF;
                break;
        }

        if(comm.command_type != 0xFF){
            radioQueueData buffer{COMMAND};
            memcpy(&buffer.data, &comm, sizeof(comm));

            xQueueSend(dataQueue, &buffer, 0);
        }   
        

    }
}