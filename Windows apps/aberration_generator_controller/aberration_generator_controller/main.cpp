/*CREATED BY: Luke Ender
 * ORGANIZATION: Univeristy of Arizona, College of Optical Sciences
 * DATE: Summer 2016
 * PURPOSE: This program runs a simple tcp client which recieves sensor data from an android cell phone via a VPS, processes it, and sends the
 *          coresponding servo angle data via serial to an Arduino Uno which then moves a uFactory uArm's servos to those angles.
 * REFERENCES: This program uses serial interfacing code from (http://playground.arduino.cc/Interfacing/CPPWindows)
                and tcp server code from (http://www.csee.usf.edu/~christen/tools/tcpClient.c)
*/

#define VPS //define this to be "VPS" to get sensor data relayed from the VPS,
			//and "LOCAL" to get it directly from the phone on a local network.

#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <sstream>
#include "SerialClass.h"
#ifdef LOCAL
	#include "tcpserver_local.h"
#endif
#ifdef VPS
	#include "tcpserver_VPS.h"
#endif

using namespace std;
double armAngleRange[2] = {70,15}; // Store the degrees of freedom of each servo
double angle[2] = { armAngleRange[0]/2, armAngleRange[2]/2 }; //motors start at the middle of their ranges of motion 
float gyro_data[2];
float acc_data[3];
float inter = 0;
unsigned int nbBytes = 2; //number of bytes to send per transfer (one per motor, though currently only two are used)
char bytesToSend[2]; //stores char values to be sent to arduino via serial
float velocity[3] = {0,0,0};//set initial conditions for velocity to zero
#pragma comment(lib, "ws2_32.lib") //Winsock Library
Serial* SP = new Serial("\\\\.\\COM3"); //THIS MUST BE CHANGED TO WHEREEVER THE ARDUINO IS CONNECTED. may be viewed in the arduino IDE.

void sendData(){
	if (SP->IsConnected()) {
		SP->WriteData(bytesToSend, nbBytes);
	}
}

void sendData_Gyro(){
    for (int i = 0; i < 2; i++){;
            gyro_data[i] *= 180/3.1416; // This converts our input gyroscope data, in rads/s, to deg/s
            angle[i] += gyro_data[i]*.062; // Integrate the deg/s data to give us the change in degrees (62 ms was measured to be the delay between 
            if (angle[i] > armAngleRange[i]) // this if / else if statement keeps the angle data within the given range
                angle[i] = armAngleRange[i];
            else if (angle[i] < 0)
                angle[i] = 0;
            bytesToSend[i] = angle[i];
    }
    sendData();
    
}


void ArdSerial(char* buffer) {
	string str(buffer); // turn the char* buffer into type string
	stringstream ss(str);
	for (int i = -1; i < 2; i++) { // this loop reads the gyroscope and accelarometer data into their correspoding arrays,
											 // separating at a comma, as the incoming data is formatted as a six-row CSV (see the Android project)
		ss >> gyro_data[i];
		if ((ss.peek() == ','))
			ss.ignore();
	}
																  //if a gyroscope is not present on the phone, so if we find this to 																  //be the case, we use accelarometer data instead.
	sendData_Gyro();
}
 
int main(){
    while (true){
        int result = tcpserver();
        if (result == -1) 
			while (true) {
				printf("Something went wrong connecting to server. Attempt reconnect? y/n \n");
				char c;
				cin >> c;
				if (c == 'n')
					return(0);
				else if (c == 'y')
					break;
			}

    }
}

//EVERYTHING BELOW HERE IS CODE WRITTEN FOR CONTROLLING ARM USING ACCELAROMETER DATA. THIS WAS FOUND
//TO BE INEFFECTIVE, BUT IT WILL BE LEFT HERE IN CASE SOMEONE WANTS TO DO SOMETHING WITH IT.
/* void sendData_Accel(){
    if ((acc_data[0] < 1) && (acc_data[0] > -1)){ //This filters out small values in a attempt to cut out noise. If small
												  //values detected, simply integrate the current velocity to get displacement.
        angle[0] += velocity[0]*(.062/.18)*10;
    }
    else{
    velocity[0] += acc_data[0]*.062; //Integrate accelarometer data to get change in velocity
    angle[0] += velocity[0]*(.062/.18)*10;// integrate resulting velocity to get change in angle
    }
   // printf("%f\n",velocity[0]);
    if (angle[0] > armAngleRange[0]) // this if / else if statement keeps the angle data within the given range
        angle[0] = armAngleRange[0];
    else if (angle[0] < 0)
        angle[0] = 0;
    bytesToSend[0] = angle[0];
    sendData();
	}
	
	*/