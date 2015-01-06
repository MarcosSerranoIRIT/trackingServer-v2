//
//  TCP_Server.h
//  ofxARToolKitPlusExample
//
//  Created by Marcos Serrano on 07/11/2013.
//
//

#ifndef __ofxARToolKitPlusExample__TCP_Client__
#define __ofxARToolKitPlusExample__TCP_Client__

#include <iostream>

#include "ofMain.h"
#include "ofxNetwork.h"

class TCP_Client : public ofBaseApp {
    
public:
    TCP_Client(string ip_, int port_);
    void send(string msg);
    void update();
    string getMsgReceived();

private:
    
    ofxTCPClient tcpClient;
    string msgTx, msgRx;
    bool weConnected;
    
    float counter;
    int connectTime;
    int deltaTime;
    
    
    int port;
    string ip;
    
};

#endif /* defined(__ofxARToolKitPlusExample__TCP_Client__) */
