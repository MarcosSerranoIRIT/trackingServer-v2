#include "ofApp.h"

void ofApp::setup() {
	ofSetFrameRate(25);
	frameRate = 0;
	ofSetVerticalSync(true);
	ofDisableNormalizedTexCoords();
	debug_mode = true;
	pp.EnableGesture();

	findHue = 100;
	findDepth = 0;

	wcolor=640; 
	hcolor= 480;

	w=320; 
	h=240;
	pp.EnableImage(PXCImage::COLOR_FORMAT_RGB24,wcolor,hcolor);
	pp.EnableImage(PXCImage::COLOR_FORMAT_DEPTH,w,h);

    if (!pp.Init()) return;

	// Allocate Depth image
	if (pp.QueryImageSize(PXCImage::COLOR_FORMAT_DEPTH,w,h)) {
		depthImg.allocate(w, h);
        labelMap=new unsigned char[w*h];
    }

	// Allocate Color Image
	if (pp.QueryImageSize(PXCImage::COLOR_FORMAT_RGB24,wcolor,hcolor)) {
		//largeColorImg.allocate(wcolor, hcolor);
		colorImg.allocate(w, h);
        labelMap=new unsigned char[w*h];
    }

	//reserve memory for cv images
   // rgb.allocate(w, h);
    hsv.allocate(w, h);
    hue.allocate(w, h);
    sat.allocate(w, h);
    bri.allocate(w, h);

    filtered.allocate(w, h);
	depthFiltered.allocate(w, h);

	gray.allocate(w, h);
    canny.allocate(w, h);

	millisDraw = ofGetElapsedTimeMillis();
	millisColorProcessing = ofGetElapsedTimeMillis();
	millisDepthProcessing	= ofGetElapsedTimeMillis();
	millisColorConversion = ofGetElapsedTimeMillis();
	millisDepthConversion	= ofGetElapsedTimeMillis();

	//old OF default is 96 - but this results in fonts looking larger than in other programs. 
	ofTrueTypeFont::setGlobalDpi(72);

	verdana14.loadFont("verdana.ttf", 14, true, true);
	verdana14.setLineHeight(18.0f);
	verdana14.setLetterSpacing(1.037);

	verdana30.loadFont("verdana.ttf", 30, true, true);
	verdana30.setLineHeight(34.0f);
	verdana30.setLetterSpacing(1.035);

	// open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);

	finger1.x = 0;
	finger1.y = 0;

	// INIT GUI
	// debugButton.addListener(this,&ofApp::debugButtonPressed);

	gui.setup(); // most of the time you don't need a name
	gui.add(debugButton.setup("debug", true));
	gui.add(drawButton.setup("draw all", true));
	
}

void ofApp::update() {

    if (!pp.AcquireFrame(false)) return;
    if (!labelMap) return;
  
	// ************* FINGER TRACKING ************
	PXCGesture::GeoNode hand_data[5];
	if (pp.QueryGesture()->QueryNodeData((pxcUID) 0,  PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY| PXCGesture::GeoNode::LABEL_HAND_FINGERTIP, (pxcU32) 5,  hand_data)>=PXC_STATUS_NO_ERROR) {
      
	  cout << hand_data[0].positionImage.x << "; " << hand_data[0].positionImage.y << "\n;";

		finger1.x = hand_data[0].positionImage.x;
		finger1.y = hand_data[0].positionImage.y;
    }

	// ************* DEPTH IMAGE ************
	int millisDepth = ofGetElapsedTimeMillis();
	// Get depth image
	PXCImage *depthImage = pp.QueryImage(PXCImage::IMAGE_TYPE_DEPTH);
         PXCImage::ImageData depthData;
		 if (depthImage->AcquireAccess(PXCImage::ACCESS_READ,PXCImage::COLOR_FORMAT_DEPTH,&depthData)>=PXC_STATUS_NO_ERROR) {
			// Convert 16 bit pixel image in 8 bit pixel image
			//cv::Mat inputImage(depthImg.getHeight(), depthImg.getWidth(), CV_16UC1, depthData.planes[0], depthData.pitches[0]);
				
			//filter image based on the depth value were looking for
			#pragma omp for
			for (int i=0; i<w*h; i++) {
				pxcU16 pixel = ((pxcU16*) depthData.planes[0])[i];
				//cout << "Pixel "<< i << " depth is " << pixel << "\n;";
				depthFiltered.getPixels()[i] = ofInRange((float) pixel,0, 400) ? 255 : 0;
			}
			depthFiltered.flagImageChanged();
			
			//run the contour finder on the filtered image to find blobs with a certain hue
			depthContours.findContours(depthFiltered, 20, w*h/2, 1, false);
			
            depthImage->ReleaseAccess(&depthData);
        }
	millisDepthConversion = ofGetElapsedTimeMillis() - millisDepth;

	// ************* COLOR IMAGE ************
	int millisColor = ofGetElapsedTimeMillis();
	//ofxCvColorImage largeColorImg;
	largeColorImg.allocate(wcolor, hcolor);
	// Get color image
	PXCImage *colorImage = pp.QueryImage(PXCImage::IMAGE_TYPE_COLOR);
         PXCImage::ImageData colorData;
        if (colorImage->AcquireAccess(PXCImage::ACCESS_READ,PXCImage::COLOR_FORMAT_RGB24,&colorData)>=PXC_STATUS_NO_ERROR) {
			largeColorImg.setFromPixels(colorData.planes[0], largeColorImg.getWidth(), largeColorImg.getHeight());
			largeColorImg.resize(w,h);
			colorImg = largeColorImg;	
            colorImage->ReleaseAccess(&colorData);
        }
    pp.ReleaseFrame();
	millisColorConversion = ofGetElapsedTimeMillis() - millisColor;

		// OPENCV COLOR TRACKING
		millisColor = ofGetElapsedTimeMillis();
        hsv = colorImg; //duplicate rgb
        hsv.convertRgbToHsv();  //convert to hsb
        //store the three channels as grayscale images
        hsv.convertToGrayscalePlanarImages(hue, sat, bri);
		//hue.flagImageChanged();
		memset(filtered.getPixels(), 0, w*h);
        //filter image based on the hue value were looking for
		#pragma omp for
		 for (int i=0; i<w*h; i++) {
			 if (isPixelInDepthBoundingRect(i)){
				filtered.getPixels()[i] = ofInRange(hue.getPixels()[i],findHue-5,findHue+5) ? 255 : 0;
			 } 
        }
        filtered.flagImageChanged();
        //run the contour finder on the filtered image to find blobs
		contours.findContours(filtered, 20, w*h/2, 1, false);
		millisColorProcessing = ofGetElapsedTimeMillis() - millisColor;

}

void ofApp::draw() {
	 ofBackground(0,0,128);
	 ofSetColor(255);

	// Calculate framerate
	timeBetweenFrames = ofGetElapsedTimeMillis() - millisDraw;
	millisDraw = ofGetElapsedTimeMillis();
	frameRate = 0;
	if (timeBetweenFrames > 0)
	 frameRate = 1000/timeBetweenFrames;
	
	//draw color images
	colorImg.draw(0, 0); // 320 x 240
	verdana14.drawString("RGB 320 x 240", 10, 260);

	// Only draw all images if needed
	if (drawButton) {
		hue.draw(325,0);
		verdana14.drawString("HUE image", 335, 260);
		filtered.draw(650,0);
		contours.draw(650,0);
		verdana14.drawString("Filtered HUE + Depth", 660, 260);
		bri.draw(980,0);
		verdana14.drawString("Bright", 990, 260);
		sat.draw(1310,0);
		verdana14.drawString("Saturation", 1320, 260);
		//canny.draw(650,0);
  
		// draw depth images
		depthImg.draw(0, 390);
		verdana14.drawString("DEPTH raw", 10, 650);
		depthContours.draw(0,390);
		depthFiltered.draw(340, 390);
		verdana14.drawString("DEPTH Filtered", 350, 650);
	}

	// Draw all tracking data (blobs, etc..)
	drawTrackingData();

	drawGui();

	// Draw text with processing times and framerate
	drawTimes();
}

void ofApp::drawTrackingData(){
	// Draw fingers position
	ofSetColor(255, 0, 0);
	 ofFill();
	ofCircle(finger1.x, finger1.y + 390, 5 );

	// Draw watch position
    ofSetColor(0, 0, 255);
	ofNoFill();

	for (int i=0; i<depthContours.nBlobs; i++) {
		depthRect = depthContours.blobs[i].boundingRect;
		ofRect(depthRect);
	 }

	ofSetColor(255, 0, 0);
	 ofFill();
    //draw red circles for found blobs
    for (int i=0; i<contours.nBlobs; i++) {
        x_watch = contours.blobs[i].centroid.x;
		y_watch = contours.blobs[i].centroid.y;

		int ind = x_watch*w + y_watch;
		z_watch = depthImg.getPixels()[ind];

		this->sendWatchPos();

		ofCircle(x_watch, y_watch, 10); // draw over rgb image
		ofCircle(x_watch, y_watch + 390, 10); // draw over depth image
    } 

	gui.draw();
}

void ofApp::drawGui(){
}

void ofApp::drawTimes() {
		// DRAW ALL THE TIMES IN MILLISECONDS
	ofSetColor(255);
	string fps = "FrameRate = " + ofToString( frameRate);
	string timeBetween = "Between Frames: " + ofToString(timeBetweenFrames);
	string depthConversion = "Depth conversion: " + ofToString(millisDepthConversion);
	string depthProcessing = "Depth processing: " + ofToString(millisDepthProcessing);
	string colorConversion = "Color conversion: " + ofToString(millisColorConversion);
	string colorProcessing = "Color Processing: " + ofToString(millisColorProcessing);
	int x_text = 1400;
	int y_text = 500;
	verdana14.drawString(fps, x_text, y_text);
	verdana14.drawString("Between Frames: " + ofToString(timeBetweenFrames), x_text, y_text + 20);
	verdana14.drawString(depthConversion, x_text,  y_text + 40);
	verdana14.drawString(depthProcessing, x_text,  y_text + 60);
	verdana14.drawString(colorConversion, x_text,  y_text + 80);
	verdana14.drawString(colorProcessing, x_text,  y_text + 100);

	// Calculate and display drawing time
	millisDrawing =  ofGetElapsedTimeMillis() - millisDraw;
	string drawingTime = "Drawing Time: " + ofToString(millisDrawing);
	verdana14.drawString(drawingTime, x_text,  y_text + 140);

	// Display smartwatch coordinates
	verdana14.drawString("SmartWatch coordinates", 1200, 500);
	verdana14.drawString("x = " + ofToString(x_watch), 1200,520);
	verdana14.drawString("y = " + ofToString(y_watch), 1200,  540);
	verdana14.drawString("z = " + ofToString(z_watch), 1200,  560);
}

bool ofApp::isPixelInDepthBoundingRect(int i){
	bool res = false;
	int x =  i % w;
	int y =  (i-x)/w;
	return depthRect.inside(ofPoint(x,y));
}

void ofApp::sendWatchPos(){
	
	ofxOscMessage m;
	m.setAddress("/watch/position");
	m.addIntArg(x_watch);
	m.addIntArg(y_watch);
	m.addIntArg(z_watch);
	sender.sendMessage(m);
}

/*void ofApp::updateDebug() {
    if (!pp.AcquireFrame(false)) return;
    if (!labelMap) return;
  
	// FINGER TRACKING
	PXCGesture::GeoNode hand_data[5];
	if (pp.QueryGesture()->QueryNodeData((pxcUID) 0,  PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY| PXCGesture::GeoNode::LABEL_HAND_FINGERTIP, (pxcU32) 5,  hand_data)>=PXC_STATUS_NO_ERROR) {
      
	  cout << hand_data[0].positionImage.x << "; " << hand_data[0].positionImage.y << "\n;";

		finger1.x = hand_data[0].positionImage.x;
		finger1.y = hand_data[0].positionImage.y;
    }

	int millisDepth = ofGetElapsedTimeMillis();
	// Get depth image
	PXCImage *depthImage = pp.QueryImage(PXCImage::IMAGE_TYPE_DEPTH);
         PXCImage::ImageData depthData;
		 if (depthImage->AcquireAccess(PXCImage::ACCESS_READ,PXCImage::COLOR_FORMAT_DEPTH,&depthData)>=PXC_STATUS_NO_ERROR) {
			//depthImg.setFromPixels(depthData.planes[0], depthImg.getWidth(), depthImg.getHeight());
			// Convert 16 bit pixel image in 8 bit pixel image
			cv::Mat inputImage(depthImg.getHeight(), depthImg.getWidth(), CV_16UC1, depthData.planes[0], depthData.pitches[0]);
			cv::Mat testImage;
			int Min = 10;
			int Max = 1000;
			inputImage.convertTo(testImage, CV_8UC1, 255.0/(Max-Min),-255.0/Min);
			// Create OpenCV Image
			depthImg.setFromPixels(testImage.data, depthImg.getWidth(), depthImg.getHeight());
            depthImage->ReleaseAccess(&depthData);
        }
	millisDepthConversion = ofGetElapsedTimeMillis() - millisDepth;

	int millisColor = ofGetElapsedTimeMillis();
	//ofxCvColorImage largeColorImg;
	largeColorImg.allocate(wcolor, hcolor);
	// Get color image
	PXCImage *colorImage = pp.QueryImage(PXCImage::IMAGE_TYPE_COLOR);
         PXCImage::ImageData colorData;
        if (colorImage->AcquireAccess(PXCImage::ACCESS_READ,PXCImage::COLOR_FORMAT_RGB24,&colorData)>=PXC_STATUS_NO_ERROR) {
			largeColorImg.setFromPixels(colorData.planes[0], largeColorImg.getWidth(), largeColorImg.getHeight());
			largeColorImg.resize(w,h);
			colorImg = largeColorImg;	
            colorImage->ReleaseAccess(&colorData);
        }
    pp.ReleaseFrame();
	millisColorConversion = ofGetElapsedTimeMillis() - millisColor;

		// OPENCV DEPTH TRACKING
		millisDepth = ofGetElapsedTimeMillis();
		 //filter image based on the depth value were looking for
		#pragma omp for
        for (int i=0; i<w*h; i++) {
           depthFiltered.getPixels()[i] = ofInRange(depthImg.getPixels()[i],0, 80) ? 255 : 0;
			// filtered.getPixels()[i] = ofInRange(hue.getPixels()[i],70,120) ? 255 : 0;
        }
        depthFiltered.flagImageChanged();
		 //run the contour finder on the filtered image to find blobs with a certain hue
		depthContours.findContours(depthFiltered, 20, w*h/2, 1, false);
		millisDepthProcessing = ofGetElapsedTimeMillis() - millisDepth;


		// OPENCV COLOR TRACKING
		millisColor = ofGetElapsedTimeMillis();
        hsv = colorImg; //duplicate rgb
        hsv.convertRgbToHsv();  //convert to hsb
        //store the three channels as grayscale images
        hsv.convertToGrayscalePlanarImages(hue, sat, bri);
		//hue.flagImageChanged();
		memset(filtered.getPixels(), 0, w*h);
        //filter image based on the hue value were looking for
		#pragma omp for
		 for (int i=0; i<w*h; i++) {
			 if (isPixelInDepthBoundingRect(i)){
				filtered.getPixels()[i] = ofInRange(hue.getPixels()[i],findHue-5,findHue+5) ? 255 : 0;
			 } 
        }
        filtered.flagImageChanged();
        //run the contour finder on the filtered image to find blobs
		contours.findContours(filtered, 20, w*h/2, 1, false);
		millisColorProcessing = ofGetElapsedTimeMillis() - millisColor;


		// OPENCV CANNY EDGES
		millisColor = ofGetElapsedTimeMillis();
		/*colorImg.convertToGrayscalePlanarImage(gray,1);
		int minVal = 60;   
		int maxVal = 150;   
		cvCanny(gray.getCvImage(), canny.getCvImage(), minVal,  maxVal, 3);  
		canny.flagImageChanged();  /
		millisColorCanny = ofGetElapsedTimeMillis() - millisColor;
}*/

//--------------------------------------------------------------
void ofApp::debugButtonPressed(){
	debug_mode = !debug_mode;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if ( (x < 320) && (y<240) ){
		//calculate local mouse x,y in image
		int mx = x % w;
		int my = y % h;
    
		//get hue value on mouse position
		 findHue = hue.getPixels()[my*w+mx];
	} else if ( (x > 320) && (y<240) ) {
		//calculate local mouse x,y in image
		int dx = x % w;
		int dy = (y-245) % h;
    
		//get hue value on mouse position
		 findDepth = depthImg.getPixels()[dy*w+dx];
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
