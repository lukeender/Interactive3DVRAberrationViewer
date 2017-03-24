"""
CREATED BY: Luke Ender
ORGANIZATION: University of Arizona, College of Optical Sciences
DATE: Summer 2016
PURPOSE: This program is designed to relay a tcp stream from a single android cell phone to a computer.
NOTES: This program is designed for use with Puthon 2.x! 3.x will require reformating.
"""

import socket
import select
import subprocess
import psutil

BUFF = 30
HOST = '0.0.0.0'
PORT = 8090
client_data = ["this should never be accessed","start","start"] #A list to store the client's data
allsock_list = [] # A list for storage of all sockets, for use with the select.select function to check witch have avalable connections/data
laptop_select = 0 #location of laptop (data destination) in clientsock_list
phone_select = 0 #" phone "

def connect(device_select):
        global num_clients
        num_clients += 1
        if device_select == 0:
                device_select = num_clients
        else:
                allsock_list.insert(device_select,allsock_list.pop())
        return device_select

def disconnect(device):
        global num_clients,phone_select,laptop_select
        num_clients -= 1
        print device, "has disconnected"
        if device == "phone":
                del allsock_list[phone_select]
                if laptop_select > phone_select:
                        laptop_select,phone_select = phone_select,laptop_select
        elif device == "laptop":
                del allsock_list[laptop_select]
                if phone_select > laptop_select:
                        laptop_select,phone_select = phone_select,laptop_select



def new_client():
                global laptop_select,phone_select,num_clients
                clientsock, addr = allsock_list[0].accept()
                allsock_list.append(clientsock)
                inputready, outputready, exceptready = select.select([clientsock],[],[])
                for clients in inputready:
                        if clients == clientsock:
                                data = clients.recv(BUFF)
                                if data[0:5] == "phone": #both the phone and server send a message containing who they are to the server upon connection
                                    phone_select = connect(phone_select)
                                    print "phone connected from ", addr[0]
                                elif data[0:6] == "laptop":
                                        laptop_select = connect(laptop_select)
                                        print "laptop connected from ", addr[0]
                                else:
                                    allsock_list.pop()
                        else:
                            allsock_list.pop()

def getdata():
        global laptop_select,phone_select, num_clients
        while 1:
                inputready, outputready, exceptready = select.select(allsock_list, [], []) #select.select takes in a list containing ports and returns a list of which have an avalable connection or some available data.
                check = 1
#               print len(inputready)
                for clients in inputready: #iterate through all ports with available data/connection
                        if (clients == allsock_list[0]) and (num_clients < 2):
                            new_client()
                        elif  clients == allsock_list[phone_select]:
                            client_data[phone_select] = clients.recv(BUFF)
                            if len(client_data[phone_select]) == 0:
                                disconnect("phone")
                                check = 0
                        elif clients == allsock_list[laptop_select]:
                                try:
                                        client_data[laptop_select] = clients.recv(BUFF)
                                        if client_data[laptop_select][0:5] == "start"
                                                subprocess.call("./ffmpeg  -protocol_whitelist file,udp,rtp -i foo.sdp  -vcodec copy -acodec copy -f flv "rtmp://104.168.29.115/hls/test" &", shell=True)
                                        elif client_data[laptop_select][0:4] == "stop"
                                                for proc in psutil.process_iter():
    # check whether the process name matches
                                                        if proc.name() == "ffmpeg":
                                                                proc.kill()
                                except socket.error,e:
                                        print e
                                        disconnect("laptop")
                                if len(client_data[laptop_select]) == 0:
                                        disconnect("laptop")
                                check = 0
                        if (check == 1) and (num_clients >= 2):
                                try:
                                        allsock_list[laptop_select].send(client_data[phone_select])
                                        #print client_data[phone_select]
                                except socket.error, e:
                                        print e
                                        disconnect("laptop")





if __name__=='__main__':
        global num_clients
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((HOST,PORT))
        s.listen(5) #Set up port (obviously)
		allsock_list.append(s)
        num_clients = 0
        getdata()
