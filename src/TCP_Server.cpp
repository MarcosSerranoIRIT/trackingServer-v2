

//
//  TCP_Server.cpp
//  ofxARToolKitPlusExample
//
//  Created by Marcos Serrano on 07/11/2013.
//
//

#include "TCP_Client.h"


TCP_Client::TCP_Client(string ip_, int port_){
    
    //port = 1234;
    port = port_;
    ip = ip_;
    
    // ******** NETWORK SETUP ******* //
    //our send and recieve strings
	msgTx	= "";
	msgRx	= "";
    
	//are we connected to the server - if this fails we
	//will check every few seconds to see if the server exists
	weConnected = tcpClient.setup(ip, port);
    
	//optionally set the delimiter to something else.
    // The delimter in the client and the server have to be the same
	tcpClient.setMessageDelimiter("\n");
	
	connectTime = 0;
	deltaTime = 0;
    
	tcpClient.setVerbose(true);
    
}


//--------------------------------------------------------------
/*void TCP_Client::update(){
	
    //we are connected - lets send our text and check what we get back
	if(weConnected){
		if(tcpClient.send(msgTx)){
            
			//if data has been sent lets update our text
			string str = tcpClient.receive();
			if( str.length() > 0 ){
				msgRx = str;
                cout << "Message received: " << msgRx ;
			}
		}else if(!tcpClient.isConnected()){
			weConnected = false;
		}
	}else{
		//if we are not connected lets try and reconnect every 5 seconds
		deltaTime = ofGetElapsedTimeMillis() - connectTime;
        
		if( deltaTime > 5000 ){
			weConnected = tcpClient.setup(ip, port);
			connectTime = ofGetElapsedTimeMillis();
		}
        
	}
	
}*/

void TCP_Client::update(){
	
    //we are connected - lets send our text and check what we get back
	if(weConnected){
		if(tcpClient.send(msgTx)){
            
			//if data has been sent lets update our text
			string str = tcpClient.receive();
			if( str.length() > 0 ){
				msgRx = str;
               
			}
		}else if(!tcpClient.isConnected()){
			weConnected = false;
		}
	}else{
		//if we are not connected lets try and reconnect every 5 seconds
		deltaTime = ofGetElapsedTimeMillis() - connectTime;
        
		if( deltaTime > 5000 ){
			weConnected = tcpClient.setup(ip, port);
			connectTime = ofGetElapsedTimeMillis();
		}
        
	}
	
}



void TCP_Client::send(string msg){
    
    //we are connected - lets send our text and check what we get back
	if(weConnected){
		if(tcpClient.send(msg)){
            
			//if data has been sent lets update our text
			string str = tcpClient.receive();
			if( str.length() > 0 ){
				msgRx = str;
                //cout << "Message received: " << msgRx ;
			}
            
		}else if(!tcpClient.isConnected()){
			weConnected = false;
		}
	} else {
		//if we are not connected lets try and reconnect every 5 seconds
		deltaTime = ofGetElapsedTimeMillis() - connectTime;
        
		if( deltaTime > 5000 ){
			weConnected = tcpClient.setup(ip, port);
			connectTime = ofGetElapsedTimeMillis();
		}
        
	}

    
}


string TCP_Client::getMsgReceived(){
    string answer = msgRx;
    msgRx = "";
    
    return answer;
}