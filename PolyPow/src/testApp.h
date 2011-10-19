#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxAutoControlPanel.h"

static ofColor cyan(0, 174, 239), magenta(236, 0, 140), yellow(255, 242, 0);

class Player {
public:
	static const int maxLife = 8;
	ofVec2f position;
	float life;
	int lastId;
	Player() {
		reset();
	}
	void reset() {
		life = maxLife;
	}
	void ouch() {
		life -= .5;
		life = MAX(life, 0);
	}
};

class Projectile {
public:
	static float gravity, friction, explosionTime;
	ofVec2f position;
	ofVec2f velocity;
	float rotation;
	float rotationVelocity;
	ofMesh mesh;
	int source;
	bool exploding;
	float birth;
	float radius;
	bool dead;
	
	Projectile(int source, ofVec2f position, ofVec2f velocity)
	:source(source)
	,position(position)
	,velocity(velocity) {
		exploding = false;
		dead = false;
		birth = ofGetElapsedTimef();
		this->velocity.y += ofRandomf() * 100;
		int sides = ofRandom(3, 9);
		radius = ofRandom(2, 48);
		rotationVelocity = ofRandom(-90, 90);
		mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
		for(int i = 0; i < sides; i++) {
			ofVec2f cur(0, radius);
			cur.rotateRad(ofMap(i, 0, sides, 0, TWO_PI));
			mesh.addVertex(cur);
		}
	}
	void update(float dt) {
		if(!exploding) {
			ofVec2f force(0, gravity);
			velocity += force * dt;
			velocity *= (1. - friction);
			position += velocity * dt;
		}
		rotationVelocity *= (1. - friction);
		rotation += rotationVelocity * dt;
	}
	void explode() {
		exploding = true;
		birth = ofGetElapsedTimef();
	}
	float getScale() {
		return exploding ? ofMap(age(), 0, explosionTime, 0, ofGetHeight()) : 1;
	}
	float age() {
		return ofGetElapsedTimef() - birth;
	}
	void draw() {
		ofPushMatrix();
		ofTranslate(position.x, position.y);
		float theta = atan2(velocity.y, velocity.x);
		ofRotate(ofRadToDeg(theta) + rotation);
		if(exploding) {
			float shake = 30;
			ofTranslate(ofRandomf() * shake, ofRandomf() * shake);
			float scale = getScale();
			ofScale(scale, scale);
			if(ofGetFrameNum() % 2 == 0) {
				mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
			} else {
				mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
			}
			if(scale * radius > ofGetWidth()) {
				dead = true;
			}
		}
		ofSetLineWidth(8);
		ofSetColor(source ? magenta : yellow);
		mesh.draw();
		ofSetLineWidth(2);
		ofSetColor(255);
		mesh.draw();
		ofPopMatrix();
	}
};

class testApp : public ofBaseApp{
public:
	
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	int getId(int realid);
	void maskedDraw(ofBaseHasTexture& tex);
	void drawTitle(string title, float rescale = 1);
	void setPlayerCount(int playerCount);
	void updatePosition(int player, float x, float y);
	void fire(int player); 
	void drawLife(int i);
	
	ofShader maskShader;
	
	ofxOscReceiver receiver;
	ofxAutoControlPanel panel;
	int playerCount;
	
	ofImage heartEmpty, heartFull;
	
	int curPlayer;
	vector<Player> players;
	vector<Projectile> projectiles;
	
	ofSoundPlayer youWin;
	bool lastDone;
	
	ofVideoPlayer ken, ryu;
};
