#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxOsc.h"
//#include "ofxUVC.h"
class Glow : public ofxCv::RectFollower {
protected:
	ofColor color;
	ofVec2f cur, smooth;
	float startedDying;
	ofPolyline all;
public:
	Glow()
		:startedDying(0) {
	}
	void setup(const cv::Rect& track);
	void update(const cv::Rect& track);
	void kill();
	void draw();
	float getTime();
};

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	void exit();

	void keyPressed(int key);
	void mousePressed(int x, int y, int button);
	float threshold;
	ofVideoPlayer movie;
	ofVideoGrabber grabber;
	ofxCv::ContourFinder contourFinderOne;
	ofxCv::ContourFinder contourFinderTwo;
	ofxCv::ContourFinder contourFinderThree;
	ofxCv::RectTrackerFollower<Glow> mTrackerOne;
	ofxCv::RectTrackerFollower<Glow> mTrackerTwo;
	ofxCv::RectTrackerFollower<Glow> mTrackerThree;
	bool showLabels;

	ofParameterGroup cv;
	ofParameter<int> mThreshold;
	ofParameter<bool> bInvert;
	ofParameter<bool> bTrackHs;
	ofParameter<ofColor> mTargetColorOne;
	ofParameter<ofColor> mTargetColorTwo;
	ofParameter<ofColor> mTargetColorThree;
	ofParameter<int> mPersistence;
	ofParameter<int> mMaxDistance;
	ofParameter<int> mMinArea;
	ofParameter<int> mMaxArea;
	float focus;
	ofxPanel gui;
//	ofxUVC uvcControl;
//	vector<ofxUVCControl> controls;
	ofParameterGroup osc;
	ofParameter<string> ipAddress;
	ofParameter<int> port;
	ofParameter<bool> resetOSC;

	ofxOscSender sender;
};
