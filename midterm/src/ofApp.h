#pragma once
#include "ofMain.h"
#include <vector>

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	// music & SFX
	ofSoundPlayer clickSound;
	ofSoundPlayer missSound;
	ofSoundPlayer musicTrack;

	// song pos + pulse
	float songTime;
	float bpm = 132.0;
	float beat;
	float pulse;

	// scoring system
	int score;
	float missWindow; // time window for how long to hit
	float lastMissCheck; // track when last check for misses

	// random functions
	float getIntensity(float t);
	ofColor getColor(float t);
	void spawnParticle(float intensity);
	void spawnExplosion(ofVec2f origin);

	// target circle things
	ofVec2f circlePos;
	float circleRadius;
	float difficulty; // 0.0 - 1.0 higher = more difficult
	bool circleActive;
	float circleSpawnTime; // tracks when last circle was spawned

	void spawnCircle();
	void handleMiss(); // function cuz a lot happens on miss
};
