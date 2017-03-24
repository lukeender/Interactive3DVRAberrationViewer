// CREATED BY: Luke Ender
// ORGANIZATION: University of Arizona RiO program
// Date: Summer 2016
// PURPOSE: This program is designed to receive servo angle data 
//          via asyncronous serial from a PC computer and control the 
//          servos in a uArm with that data.
// REFERENCES: This program uses and requires the uArm library, available at https://github.com/uArm-Developer/UArmForArduino
//             This program was only tested with 1.5.10, and may or may not be compatable with later versions without revisions.
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <uarm_library.h>
char readSerial[2]; // Buffer to store incoming serial
int angles[2];
int numBytes = 2; //here we store the number of bytes to recieve
int offsets[3] = {30 ,75}; // Sometimes, the uArm registers some angles as negative. Rather than devise a method 
                              // to send negative numbers via serial using bit-shifting and other complex methods,
                              // it is easier to simply send the angles to the arduino as positive chars and  
                              // add an offset once it is recieved. This is what we are doing here.
void setup(){
Serial.begin(9600);
}

void loop(){
 if(Serial.available()>0) // This code loops forever, so that whenever serial data is available, it is read
  {
   Serial.readBytes(readSerial,numBytes); // read in three byte sequences
   for (int i = 0; i < 2; i++){
    angles[i] = readSerial[i] + offsets[i];
   }
  
   uarm.writeAngle((double)angles[0],(double)0,(double)angles[1],0);
  }
/*double angle1 = uarm.readAngle(SERVO_ROT_NUM);
double angle2 = uarm.readAngle(SERVO_LEFT_NUM);
double angle3 = uarm.readAngle(SERVO_RIGHT_NUM); /// This code it used to see what angles the arm registers itself at,
Serial.println(angle1);                          /// so as to find out what the range of motion is to be
Serial.println(angle2);
Serial.println(angle3);
delay(500);*/
}

