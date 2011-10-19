#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

class testApp : public ofBaseApp{
public:
	
	void setup();
	void update();
	void draw();
	
	ofxOscReceiver receiver;
	
	vector<float> audioLevel;
};
