#include "ofApp.h"
#include <algorithm>

struct Particle {
	ofVec2f pos;
	ofVec2f vel;
	float life;
	float size;
	float alpha;
};

struct BurstParticle {
	ofVec2f pos;
	ofVec2f vel;
	float life;
	float maxLife;
	float size;
};

vector<Particle> particles;
vector<BurstParticle> burstParticles;

float ofApp::getIntensity(float t) {
	if (t < 29.0) {
		return 0.2; // base
	} else if (t < 43.63) {
		return ofMap(t, 29.0, 43.63, 0.2, 1.0, true);
	} else {
		return 1.0; // drop
	}
}

// getColor definition
ofColor ofApp::getColor(float t) {
	if (t < 29.0) {
		return ofColor(100, 150, 255); // soft blue
	} else if (t < 43.63) {
		float lerp = ofMap(t, 29.0, 43.63, 0.0, 1.0, true);
		return ofColor(
			ofLerp(100, 255, lerp),
			ofLerp(150, 100, lerp),
			255); // blue → pink
	} else {
		return ofColor(255, 120, 255); // bright pink/magenta whatever
	}
}

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);
	ofSetWindowShape(1024, 768);

	difficulty = 0.5;
	circleActive = false;

	// THE SCORE
	score = 0;
	missWindow = 1.5; // auto miss if not clicked in time
	lastMissCheck = 0;

	// load sounds
	clickSound.load("click.wav");
	clickSound.setMultiPlay(true);

	missSound.load("miss.wav");
	missSound.setMultiPlay(true);

	musicTrack.load("trance.wav");
	musicTrack.setLoop(true);
	musicTrack.setVolume(0.5);
	musicTrack.play();

	spawnCircle();
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(10);

	// song time updater for particle draw
	songTime = musicTrack.getPositionMS() / 1000.0;
	ofColor currentColor = getColor(songTime);

	// draw particles with color based on song time
	for (auto & p : particles) {
		ofSetColor(currentColor, p.alpha);
		ofDrawCircle(p.pos, p.size);
	}

	// bpm for pulse
	float bpm = 132.0;
	float beat = fmod(songTime * bpm / 60.0, 1.0);
	float pulse = 1.0 + 0.2 * sin(TWO_PI * beat);

	// explosion on click + mouse follow
	for (auto & p : burstParticles) {
		float alpha = ofMap(p.life, 0, p.maxLife, 0, 255);
		ofSetColor(255, 100, 255, alpha);
		ofDrawCircle(p.pos, p.size);
	}

	// CREATE THE CLICKY CIRCLE
	if (circleActive) {
		// make it pulse to beat
		float radiusPulse = circleRadius * (0.9 + 0.1 * sin(ofGetElapsedTimef() * 10));

		// make it fancy (outer glow)
		ofSetColor(255, 100, 100, 50);
		ofDrawCircle(circlePos, radiusPulse + 10);

		// primary circle
		ofSetColor(255, 100, 100);
		ofDrawCircle(circlePos, radiusPulse);

		// more detailing
		ofSetColor(255, 200, 200, 150);
		ofDrawCircle(circlePos, radiusPulse * 0.6);
	}

	// show your score!
	ofSetColor(255, 255, 255);
	string scoreText = "Score: " + ofToString(score);
	float textWidth = scoreText.length() * 8; // 
	ofDrawBitmapString(scoreText, ofGetWidth() / 2 - textWidth / 2, 40);

	// timer bar for looks
	if (circleActive) {
		float timeRemaining = missWindow - (songTime - circleSpawnTime);
		float timeProgress = ofMap(timeRemaining, 0, missWindow, 0, 1, true);

		// more bar for looks
		ofSetColor(80, 20, 20);
		ofDrawRectangle(0, 0, ofGetWidth(), 8);

		// make color change based on time left to click (for looks)
		if (timeProgress > 0.66) {
			ofSetColor(100, 255, 100); // green = lotta time
		} else if (timeProgress > 0.33) {
			ofSetColor(255, 200, 0); // yellow = medium time
		} else {
			ofSetColor(255, 50, 50); // red = low time
		}
		ofDrawRectangle(0, 0, ofGetWidth() * timeProgress, 8);

		// more detailing cuz im wasting my time trying to make it look nice
		ofSetColor(255, 255, 255, 100);
		ofDrawRectangle(0, 0, ofGetWidth() * timeProgress, 2);
	} else {
		ofSetColor(100, 100, 100);
		ofDrawRectangle(0, 0, ofGetWidth(), 4);
	}

	// debug ui
	ofSetColor(200);
	ofDrawBitmapString("Difficulty: " + ofToString(difficulty, 2), 20, 60);
	ofDrawBitmapString("Time Elapsed: " + ofToString(songTime, 1) + "s", 20, 80);

	// controls + scoring
	ofSetColor(150);
	ofDrawBitmapString("UP/DOWN: Adjust Difficulty | R: Reset Score", 20, ofGetHeight() - 30);
	ofDrawBitmapString("Click Circle: +10 pts | Click Elsewhere/Miss: -50 pts", 20, ofGetHeight() - 15);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_UP) {
		difficulty += 0.05;
	}
	if (key == OF_KEY_DOWN) {
		difficulty -= 0.05;
	}
	if (key == 'r' || key == 'R') {
		score = 0; // reset score
	}

	difficulty = ofClamp(difficulty, 0.0, 1.0);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	if (!circleActive) return;

	float dist = ofDist(x, y, circlePos.x, circlePos.y);

	if (dist <= circleRadius) {
		// you hit! gain points
		score += 10;

		// fancy reward for hitting
		spawnExplosion(circlePos);

		// tactile hit
		clickSound.play();

		// spawn new circle on hit
		circleActive = false;
		spawnCircle();
	} else {
		// wow you suck
		handleMiss();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}

//--------------------------------------------------------------
void ofApp::spawnCircle() {
	// makes circle stay within bounds
	float margin = 50;

	circlePos.x = ofRandom(margin, ofGetWidth() - margin);
	circlePos.y = ofRandom(margin, ofGetHeight() - margin);

	// size changes depending on difficulty
	float minRadius = 10;
	float maxRadius = 60;

	circleRadius = ofMap(difficulty, 0.0, 1.0, maxRadius, minRadius);

	circleActive = true;

	// calc for miss detection
	circleSpawnTime = musicTrack.getPositionMS() / 1000.0;
}

//--------------------------------------------------------------
void ofApp::handleMiss() {
	// damn
	score -= 50;

	// really abhorrent miss sound
	missSound.play();

	// red jumpscare (doesn't work :( )
	ofColor flashColor = ofColor(255, 0, 0, 100);
	ofSetColor(flashColor);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

	// moves circle on miss
	spawnCircle();
}

//--------------------------------------------------------------
void ofApp::spawnParticle(float intensity) {
	Particle p;

	p.pos = ofVec2f(ofGetWidth() / 2, ofGetHeight() / 2);

	float angle = ofRandom(TWO_PI);
	float speed = ofRandom(2.0, 6.0) * intensity;

	p.vel = ofVec2f(cos(angle), sin(angle)) * speed;
	p.life = ofRandom(60, 120);

	p.size = ofRandom(4, 10) * intensity;
	p.alpha = ofRandom(100, 200);

	particles.push_back(p);
	//background particle system
}

//---------------------------------------------------------------
void ofApp::spawnExplosion(ofVec2f origin) {
	int count = 40;

	for (int i = 0; i < count; i++) {
		BurstParticle p;

		p.pos = origin;

		float angle = ofRandom(TWO_PI);
		float speed = ofRandom(3.0, 8.0);

		p.vel = ofVec2f(cos(angle), sin(angle)) * speed;

		p.life = 180;
		p.maxLife = 180;

		p.size = ofRandom(3, 6);

		burstParticles.push_back(p);
		//particle system for when you click on it
	}
}

//----------------------------------------------------------------
void ofApp::update() {
	songTime = musicTrack.getPositionMS() / 1000.0;

	float intensity = getIntensity(songTime);

	beat = fmod(songTime * bpm / 60.0, 1.0);
	pulse = 1.0 + 0.2 * sin(TWO_PI * beat);

	// check miss detection
	// check for active circle(s) if timer elapses
	if (circleActive && (songTime - circleSpawnTime) > missWindow) {
		handleMiss();
	}

	// spawn rate of background particles scales with intensity
	int spawnCount = ofMap(intensity, 0.2, 1.0, 1, 6, true);

	for (int i = 0; i < spawnCount; i++) {
		spawnParticle(intensity);
	}

	// update background particles
	for (auto & p : particles) {
		p.pos += p.vel;
		p.life -= 1;

		p.alpha -= 2.0;
		p.alpha = max(p.alpha, 0.0f);
	}

	// delete dead background particles
	particles.erase(
		remove_if(particles.begin(), particles.end(),
			[](Particle & p) { return p.life <= 0; }),
		particles.end());

	// makes burst particles follow mouse
	for (auto & p : burstParticles) {
		ofVec2f mouse(ofGetMouseX(), ofGetMouseY());
		ofVec2f toMouse = mouse - p.pos;

		if (toMouse.length() > 0) {
			toMouse.normalize();
			float attraction = 0.3;
			p.vel += toMouse * attraction;
		}

		p.vel *= 0.95;
		p.pos += p.vel;
		p.life -= 1;
	}

	// cleanup burst particles
	burstParticles.erase(
		std::remove_if(
			burstParticles.begin(),
			burstParticles.end(),
			[](BurstParticle & p) {
				return p.life <= 0;
			}),
		burstParticles.end());
}
//wow you reached the end
