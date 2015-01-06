#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "cv.h"
#include "ofxOsc.h"
//#include "util_pipeline.h"
#include <util_pipeline.h>
#include "threadedRGB.h"
#include "ofxGui.h"

#define HOST "localhost"
#define PORT 12345

class ofApp : public ofBaseApp {

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
	protected:
		//void updateDebug() ;
		void drawTimes();
		void drawTrackingData();
		void drawGui();
		bool isPixelInDepthBoundingRect(int i);
		void sendWatchPos();

		void debugButtonPressed();

		int frameRate;
		int timeBetweenFrames;

        UtilPipeline   pp;
	    unsigned char  *labelMap;

		ofxCvColorImage largeColorImg;
		ofxCvColorImage colorImg;
		ofxCvGrayscaleImage depthImg;

		// Color and contour tracking
		ofxCvColorImage hsv;
		ofxCvGrayscaleImage hue, sat, bri, filtered;

		ofxCvContourFinder contours;
		ofxCvContourFinder depthContours;
		ofxCvGrayscaleImage depthFiltered;
		ofRectangle depthRect;

		ofxCvGrayscaleImage gray;
		ofxCvGrayscaleImage canny; 

		int findHue;
		int findDepth;

		pxcU32 w, h;
		pxcU32 wcolor, hcolor;

		// For framerate calculus
		int millisDraw;
		int millisDepthProcessing;
		int millisColorProcessing;
		int millisDepthConversion;
		int millisColorConversion;
		int millisColorCanny;
		int millisDrawing;

		// Fonts
		ofTrueTypeFont	verdana14;
		ofTrueTypeFont	verdana30;

		// Smartwatch coordinates
		int x_watch, y_watch, z_watch;

		ofxOscSender sender;
        ofBuffer imgAsBuffer;

		ofPoint finger1;

		bool debug_mode;
		ofxPanel gui;
		ofxToggle debugButton;
		ofxToggle drawButton;
		//ofxButton viewToggle;

		//threadedRGB processorRgb;

};
