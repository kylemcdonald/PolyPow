#include "testApp.h"

void testApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	receiver.setup(8448);
	ofSetCircleResolution(64);
	ofSetLineWidth(4);
	/*
	audioLevel.resize(11);
	for(int i = 0; i < audioLevel.size(); i++) {
		audioLevel[i] = (float) i / (audioLevel.size() - 1);
	}
	*/
}

void testApp::update(){
	while(receiver.hasWaitingMessages()) {
		ofxOscMessage msg;
		receiver.getNextMessage(&msg);
		if (msg.getAddress() == "/kinect/audio") {
			int n = msg.getNumArgs();
			audioLevel.resize(n);
			for(int i = 0; i < n; i++) {
				audioLevel[i] = msg.getArgAsFloat(i);
			}
		}
		ofLogVerbose() << "got " << msg.getAddress();
	}
}

void testApp::draw(){
	ofBackground(0);
	ofSetColor(255);
	
	float startAngle = -50;
	float endAngle= +50;
	float nearRadius = 64;
	float farRadius = 512;
	
	ofVec2f start(0, -nearRadius), end(0, -nearRadius);
	start.rotate(startAngle);
	end.rotate(endAngle);
	
	/*
	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
	ofCircle(0, 0, nearRadius);
	ofNoFill();
	ofBeginShape();
	ofVertex(start.x, start.y);
	for(int i = 0; i < audioLevel.size(); i++) {
		float curLevel = audioLevel[i] * mouseX;
		float radius = ofMap(curLevel, 0, 1, nearRadius, farRadius);
		ofVec2f cur(0, -radius);
		float angle = ofMap(i, 0, audioLevel.size() - 1, startAngle, endAngle);
		cur.rotate(angle);
		ofCurveVertex(cur.x, cur.y);
	}
	ofVertex(end.x, end.y);
	ofEndShape();
	*/
	
	ofBeginShape();
	ofNoFill();
	for(int i = 0; i < audioLevel.size(); i++) {
		float x = ofMap(i, 0, audioLevel.size() - 1, 0, ofGetWidth());
		float y = ofMap(audioLevel[i], 0, 1, ofGetHeight(), 0 - mouseX);
		ofVertex(x, y);
	}
	ofEndShape();
}