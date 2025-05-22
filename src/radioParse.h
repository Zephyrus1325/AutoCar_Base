#include <Arduino.h>
#include "radioEnum.h"

extern uint8_t transmitRole;

void sendChunk(chunk_data* chunk){
    Serial.write(0xAA);
    Serial.write(0x00);
    Serial.write(chunk->subdivision);

    int16_t posX = (int16_t) chunk->position.x;
    int16_t posY = (int16_t) chunk->position.y;
    Serial.write(posX & 0x00ff);
    Serial.write(posX >> 8);
    Serial.write(posY & 0x00ff);
    Serial.write(posY >> 8);

    for(int i = 0; i < CHUNK_RADIO_SIZE; i++){
        Serial.write(chunk->data[i]);
    }
}

void readCommand(command* comm){
    //Serial.println(comm->command_type);
    switch(comm->command_type){
        case END_OF_PACKET:
            //Serial.println("RECEIVED EOP");
            transmitRole = 1;
            break;
        default:
            break;

    }
}