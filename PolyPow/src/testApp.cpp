#include "testApp.h"

float Projectile::gravity;
float Projectile::friction;
float Projectile::explosionTime;

char maskShaderSource[] =
"uniform sampler2DRect tex;\
void main() {\
	vec2 pos = gl_TexCoord[0].st;\
	vec4 cur = texture2DRect(tex, pos);\
	if(cur.r * cur.g * cur.b == 1.) {\
		gl_FragColor = vec4(0.);\
	} else {\
		gl_FragColor = cur;\
	}\
}"; 

int otherPlayer(int id) {
	return (id + 1) % 2;
}

void testApp::maskedDraw(ofBaseHasTexture& tex) {
	ofTexture& texture = tex.getTextureReference();
	ofEnableAlphaBlending();
	maskShader.begin();
	maskShader.setUniformTexture("tex", texture, 0);
	ofSetMinMagFilters(GL_NEAREST, GL_NEAREST);
	texture.draw((int) -texture.getWidth() / 2, (int) -texture.getHeight() / 2);
	maskShader.end();
	ofDisableAlphaBlending();
}

void testApp::drawTitle(string title, float rescale) {
	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
	rescale *= panel.getValueF("titleScale");
	ofScale(rescale, rescale);
	ofDrawBitmapString(title, float(title.size()) * -4, 0);
	ofPopMatrix();
}

void testApp::setup(){
	ofSetVerticalSync(true);
	
	maskShader.setupShaderFromSource(GL_FRAGMENT_SHADER, maskShaderSource);
	maskShader.linkProgram();
	
	ken.loadMovie("ken.gif");
	ryu.loadMovie("ryu.gif");
	ken.play();
	ryu.play();
	
	curPlayer = 0;
	players.resize(2);
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	receiver.setup(8448);
	
	heartEmpty.loadImage("heart-empty.png");
	heartFull.loadImage("heart-full.png");
	
	youWin.loadSound("you-win.wav");
	
	lastDone = false;
	
	playerCount = 0;
	
	panel.setup();
	panel.addPanel("Settings");
	panel.addSlider("leftInput", 0, -1, 2);
	panel.addSlider("rightInput", 1, -1, 2);
	panel.addSlider("topInput", 0, -1, 2);
	panel.addSlider("bottomInput", 1, -1, 2);
	panel.addSlider("gravity", 20, 0, 100);
	panel.addSlider("friction", .005, 0, .01);
	panel.addSlider("explodeDistance", 100, 0, 200);
	panel.addSlider("explosionTime", 10, 0, 10);
	panel.addToggle("debug", false);
	panel.addToggle("manualMode", true);
	panel.addSlider("playerCount", 2, 0, 2, true);
	panel.addToggle("fire0", false);
	panel.addToggle("fire1", false);
	
	panel.addPanel("Positions");
	panel.addSlider("titleScale", 6.5, 1, 10);
	panel.addSlider("lifecenter", .5, 0, 1);
	panel.addSlider("lifey", -256, -512, 512);
	panel.addSlider("player0x", 0, -1, 2);
	panel.addSlider("player0y", .5, -1, 2);
	panel.addSlider("player1x", 1, -1, 2);
	panel.addSlider("player1y", .5, -1, 2);
}

void testApp::updatePosition(int player, float x, float y) {
	//cout << "incoming " << x << " " << y << endl;
	float sx = ofMap(x, panel.getValueF("leftInput"), panel.getValueF("rightInput"), 0, ofGetWidth());
	float sy = ofMap(y, panel.getValueF("topInput"), panel.getValueF("bottomInput"), 0, ofGetHeight());
	//cout << "outgoing " << sx << " " << sy << endl;
	players[player].position.set(sx, sy);
}

void testApp::fire(int player) {
	Player& me = players[player];
	//cout << "players (0: " << players[0].life << "," << players[0].lastId << "," << players[0].position << ")" << endl;
	//cout << "players (1: " << players[1].life << "," << players[1].lastId << "," << players[1].position << ")" << endl;
	if(me.life > 0) {
		Player& you = players[otherPlayer(player)];
		ofVec2f position = me.position;
		ofVec2f velocity = you.position - me.position;
		projectiles.push_back(Projectile(player, position, velocity));
	}
}

void testApp::setPlayerCount(int playerCount) {
	this->playerCount = playerCount;
	for(int i = 0; i < players.size(); i++) {
		players[i].reset();
	}
}

bool isOldProjectile(Projectile& projectile) {
	return projectile.age() > 10. || projectile.dead;
}

int testApp::getId(int realid) {
	// converts realid to 0 or 1
	int id;
	bool exists = false;
	for(int i = 0; i < players.size(); i++) {
		if(players[i].lastId == realid) {
			id = i;
			exists = true;
		}
	}
	if(!exists) {
		id = curPlayer;
		players[id].lastId = realid;
		curPlayer = otherPlayer(curPlayer);
	}
	return id;
}

void testApp::update(){
	ken.update();
	ryu.update();

	Projectile::gravity = panel.getValueF("gravity");
	Projectile::friction = panel.getValueF("friction");
	Projectile::explosionTime = panel.getValueF("explosionTime");
	
	if(panel.getValueB("manualMode")) {
		if(panel.getValueB("fire0")) {
			panel.setValueB("fire0", false);
			fire(getId(0));
		}
		if(panel.getValueB("fire1")) {
			panel.setValueB("fire1", false);
			fire(getId(1));
		}
		if(panel.hasValueChanged(variadic("playerCount"))) {
			setPlayerCount(panel.getValueI("playerCount"));
			panel.clearAllChanged();
		}
		updatePosition(0, panel.getValueF("player0x"), panel.getValueF("player0y"));
		updatePosition(1, panel.getValueF("player1x"), panel.getValueF("player1y"));
	}
	
	while(receiver.hasWaitingMessages()) {
		ofxOscMessage msg;
		receiver.getNextMessage(&msg);
		if (msg.getAddress() == "/pew/playerPosition") {
			updatePosition(getId(msg.getArgAsInt32(0)), msg.getArgAsFloat(1), msg.getArgAsFloat(2));
		} else if(msg.getAddress() == "/pew/fire") {
			fire(getId(msg.getArgAsInt32(0)));
		} else if (msg.getAddress() == "/pew/players") {
			setPlayerCount(msg.getArgAsInt32(0));
		}
	}
	
	float explodeDistance = panel.getValueF("explodeDistance");
	float dt = ofGetLastFrameTime();
	for(int i = 0; i < projectiles.size(); i++) {
		projectiles[i].update(dt);
		for(int j = 0; j < players.size(); j++) {
			float distance = projectiles[i].position.distance(players[j].position);
			if(projectiles[i].source != j && distance < explodeDistance) {
				if(!projectiles[i].exploding) {
					projectiles[i].explode();
					players[j].ouch();
				}
			}
		}
	}
	
	ofRemove(projectiles, isOldProjectile);
}

void testApp::drawLife(int id) {
	Player& player = players[id];
	ofEnableAlphaBlending();
	if(player.life > 0) {
		ofSetColor(255, 255);
	} else {
		ofSetColor(255, 64);
	}
	ofPushMatrix();
	ofVec2f position = player.position;
	ofTranslate(ofLerp(position.x, ofGetWidth() / 2, panel.getValueF("lifecenter")), position.y + panel.getValueF("lifey"));
	int w = heartFull.getWidth();
	float scale = ofMap(player.life, 0, Player::maxLife, 1., .3);
	ofScale(scale, scale);
	
	ofPushMatrix();
	ofScale(8, 8);
	string name = "PLAYER " + ofToString(id + 1);
	ofDrawBitmapString(name, -4. * name.size(), -4);
	ofPopMatrix();
	
	ofTranslate(-w * Player::maxLife / 2., 0);
	for(int i = 0; i < Player::maxLife; i++) {
		if(i < player.life) {
			heartFull.draw(0, 0);
		} else {
			heartEmpty.draw(0, 0);
		}
		ofTranslate(w, 0);
	}
	ofPopMatrix();
	ofDisableAlphaBlending();
}

void testApp::draw(){
	ofSetMinMagFilters(GL_NEAREST, GL_NEAREST);
		
	ofBackground(0);
	ofSetColor(255);
	
	if(playerCount == 0) {
		float rescale = ofMap(sin(ofGetElapsedTimef() * 5), -1, 1, .8, 1.2);
		ofSetColor(magenta);
		drawTitle("WAITING FOR PLAYER", rescale);
	} else if(playerCount == 1) {
		ofVec2f offset(ofRandomf(), ofRandomf());
		offset *= 5;
		ofSetColor(cyan);
		ofPushMatrix();
		ofTranslate(offset.x, offset.y);
		drawTitle("AWAITING CHALLENGER");
		ofPopMatrix();
	} else if(playerCount == 2) {
		for(int i = 0; i < projectiles.size(); i++) {
			projectiles[i].draw();
		}
		
		ofSetColor(255);
		for(int i = 0; i < players.size(); i++) {
			if(players[i].life > 0) {
				ofPushMatrix();
				ofTranslate((int) players[i].position.x, (int) players[i].position.y);
				ofScale(2, 2);
				if(players[i].position.x > ofGetWidth() / 2) {
					ofScale(-1, 1);
				}
				if(i == 0) {
					maskedDraw(ryu);
				} else {
					maskedDraw(ken);
				}
				ofPopMatrix();
			}
		}
	}
	
	int winner = 0;
	if(playerCount == 2) {
		bool done = false;
		for(int i = 0; i < players.size(); i++) {
			drawLife(i);
			if(players[i].life <= 0) {
				done = true;
			} else {
				winner = i;
			}
		}
		if(done && !lastDone) {
			youWin.play();
		}
		lastDone = done;
	}
	
	if(playerCount == 2 && lastDone) {
		bool flash = sin(2. * ofGetElapsedTimef() * TWO_PI) > 0;
		ofSetColor(flash ? yellow : 0);
		drawTitle("PLAYER " + ofToString(winner + 1) + " WINS");
	}
	
	if(panel.getValueB("debug")) {
		ofSetColor(255);
		for(int i = 0; i < players.size(); i++) {
			ofCircle(players[i].position, 32);
		}
	}
}

void testApp::keyPressed(int key) {
	if(key == OF_KEY_LEFT) {
		panel.setValueB("fire0", true);
	}
	if(key == OF_KEY_RIGHT) {
		panel.setValueB("fire1", true);
	}
}