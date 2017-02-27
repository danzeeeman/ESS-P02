#include "ofApp.h"

using namespace ofxCv;
using namespace cv;
const float dyingTime = 1;
void Glow::setup(const cv::Rect& track) {
	color.setHsb(ofRandom(0, 255), 255, 255);
	cur = toOf(track).getCenter();
	smooth = cur;
}

void Glow::update(const cv::Rect& track) {
	cur = toOf(track).getCenter();
	smooth.interpolate(cur, .5);
	all.addVertex(smooth.x, smooth.y, 0.0);
}

float Glow::getTime(){
	float curTime = ofGetElapsedTimef();
	if (!dead) {
		return curTime - startedDying;
	}
	else {
		return 0;
	}

}

void Glow::kill() {
	float curTime = ofGetElapsedTimef();
	if (startedDying == 0) {
		startedDying = curTime;
	}
	else if (curTime - startedDying > dyingTime) {
		dead = true;
	}
}

void Glow::draw() {
	ofPushStyle();
	float size = 16;
	ofSetColor(255);
	if (startedDying) {
		ofSetColor(ofColor::red);
		size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
	}
	ofNoFill();
	ofDrawCircle(cur, size);
	ofSetColor(color);
	all.draw();
	ofSetColor(255);
	ofDrawBitmapString(ofToString(label), cur);
	ofPopStyle();
}

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofBackground(0);

	grabber.setDeviceID(1);
	grabber.setup(640, 480);

	osc.add(ipAddress.set("IP", "127.0.0.1"));
	osc.add(port.set("PORT", 7777, 7000, 8000));
	osc.add(resetOSC.set("Reset", false));

	showLabels = true;

	cv.setName("CV Settings");
	cv.add(mThreshold.set("Threshold", 10, 0, 255));
	cv.add(bInvert.set("Invert", true));
	cv.add(bTrackHs.set("Track Hue/Saturation", false));
	cv.add(mTargetColorOne.set("ColorOne", ofColor(0, 0, 0)));
	cv.add(mTargetColorTwo.set("ColorTwo", ofColor(0, 0, 0)));
	cv.add(mTargetColorThree.set("ColorThree", ofColor(0, 0, 0)));
	cv.add(mPersistence.set("Persistence", 15, 5, 600));
	cv.add(mMaxDistance.set("Max Distance", 10, 0, 500));
	cv.add(mMaxArea.set("Max Area", 10, 1, 640 * 480));
	cv.add(mMinArea.set("Min Area", 1, 1, 640));
	gui.setup(cv);
	gui.add(osc);

	gui.loadFromFile("settings.xml");

	// wait for half a frame before forgetting something
	mTrackerOne.setPersistence(mPersistence);
	mTrackerTwo.setPersistence(mPersistence);
	mTrackerThree.setPersistence(mPersistence);
	// an object can move up to 50 pixels per frame
	mTrackerOne.setMaximumDistance(mMaxDistance);
	mTrackerTwo.setMaximumDistance(mMaxDistance);
	mTrackerThree.setMaximumDistance(mMaxDistance);

	sender.setup(ipAddress.get(), port.get());
	

	contourFinderOne.setUseTargetColor(true);
	contourFinderTwo.setUseTargetColor(true);
	contourFinderThree.setUseTargetColor(true);


	gui.setPosition(grabber.getWidth(), 0);
	ofSetWindowShape(grabber.getWidth()+gui.getWidth(), gui.getHeight());
}

void ofApp::update() {
	grabber.update();
	if (grabber.isFrameNew()) {
		// wait for half a frame before forgetting something
		mTrackerOne.setPersistence(mPersistence);
		mTrackerTwo.setPersistence(mPersistence);
		mTrackerThree.setPersistence(mPersistence);
		// an object can move up to 50 pixels per frame
		mTrackerOne.setMaximumDistance(mMaxDistance);
		mTrackerTwo.setMaximumDistance(mMaxDistance);
		mTrackerThree.setMaximumDistance(mMaxDistance);

		contourFinderOne.setMinArea(mMinArea);
		contourFinderOne.setMaxArea(mMaxArea);
		contourFinderOne.getTracker().setPersistence(mPersistence);
		contourFinderOne.getTracker().setMaximumDistance(mMaxDistance);
		contourFinderOne.setTargetColor(mTargetColorOne, bTrackHs ? TRACK_COLOR_HS : TRACK_COLOR_RGB);
		contourFinderOne.setThreshold(mThreshold);
		contourFinderOne.findContours(grabber);
		mTrackerOne.track(contourFinderOne.getBoundingRects());


		contourFinderTwo.setMinArea(mMinArea);
		contourFinderTwo.setMaxArea(mMaxArea);
		contourFinderTwo.getTracker().setPersistence(mPersistence);
		contourFinderTwo.getTracker().setMaximumDistance(mMaxDistance);
		contourFinderTwo.setTargetColor(mTargetColorTwo, bTrackHs ? TRACK_COLOR_HS : TRACK_COLOR_RGB);
		contourFinderTwo.setThreshold(mThreshold);
		contourFinderTwo.findContours(grabber);
		mTrackerTwo.track(contourFinderTwo.getBoundingRects());


		contourFinderThree.setMinArea(mMinArea);
		contourFinderThree.setMaxArea(mMaxArea);
		contourFinderThree.getTracker().setPersistence(mPersistence);
		contourFinderThree.getTracker().setMaximumDistance(mMaxDistance);
		contourFinderThree.setTargetColor(mTargetColorThree, bTrackHs ? TRACK_COLOR_HS : TRACK_COLOR_RGB);
		contourFinderThree.setThreshold(mThreshold);
		contourFinderThree.findContours(grabber);
		mTrackerThree.track(contourFinderThree.getBoundingRects());

	}

	RectTracker& tracker = contourFinderOne.getTracker();
	for (int i = 0; i < contourFinderOne.size(); i++) {
		unsigned int label = contourFinderOne.getLabel(i);
		// only draw a line if this is not a new label
		if (tracker.existsPrevious(label)) {
			// use the label to pick a random color
			ofSeedRandom(label << 24);
			ofSetColor(ofColor::fromHsb(ofRandom(255), 255, 255));
			// get the tracked object (cv::Rect) at current and previous position
			const cv::Rect& previous = tracker.getPrevious(label);
			const cv::Rect& current = tracker.getCurrent(label);
			// get the centers of the rectangles
			ofVec2f previousPosition(previous.x + previous.width / 2, previous.y + previous.height / 2);
			ofVec2f currentPosition(current.x + current.width / 2, current.y + current.height / 2);

			ofxOscMessage b;
			b.setAddress("/drawing/one");
			ofVec2f velocity = toOf(contourFinderOne.getVelocity(i));
			b.addFloatArg(currentPosition.x);
			b.addFloatArg(currentPosition.y);
			b.addFloatArg(contourFinderOne.getContourArea(i));
			b.addIntArg(velocity.length());
			sender.sendMessage(b);
		}
	}

	tracker = contourFinderTwo.getTracker();
	for (int i = 0; i < contourFinderTwo.size(); i++) {
		unsigned int label = contourFinderTwo.getLabel(i);
		// only draw a line if this is not a new label
		if (tracker.existsPrevious(label)) {
			// use the label to pick a random color
			ofSeedRandom(label << 24);
			ofSetColor(ofColor::fromHsb(ofRandom(255), 255, 255));
			// get the tracked object (cv::Rect) at current and previous position
			const cv::Rect& previous = tracker.getPrevious(label);
			const cv::Rect& current = tracker.getCurrent(label);
			// get the centers of the rectangles
			ofVec2f previousPosition(previous.x + previous.width / 2, previous.y + previous.height / 2);
			ofVec2f currentPosition(current.x + current.width / 2, current.y + current.height / 2);

			ofxOscMessage b;
			b.setAddress("/drawing/two");
			ofVec2f velocity = toOf(contourFinderTwo.getVelocity(i));
			b.addFloatArg(currentPosition.x);
			b.addFloatArg(currentPosition.y);
			b.addFloatArg(contourFinderTwo.getContourArea(i));
			b.addIntArg(velocity.length());
			sender.sendMessage(b);
		}
	}

	tracker = contourFinderThree.getTracker();
	for (int i = 0; i < contourFinderThree.size(); i++) {
		unsigned int label = contourFinderThree.getLabel(i);
		// only draw a line if this is not a new label
		if (tracker.existsPrevious(label)) {
			// use the label to pick a random color
			ofSeedRandom(label << 24);
			ofSetColor(ofColor::fromHsb(ofRandom(255), 255, 255));
			// get the tracked object (cv::Rect) at current and previous position
			const cv::Rect& previous = tracker.getPrevious(label);
			const cv::Rect& current = tracker.getCurrent(label);
			// get the centers of the rectangles
			ofVec2f previousPosition(previous.x + previous.width / 2, previous.y + previous.height / 2);
			ofVec2f currentPosition(current.x + current.width / 2, current.y + current.height / 2);

			ofxOscMessage b;
			b.setAddress("/drawing/three");
			ofVec2f velocity = toOf(contourFinderThree.getVelocity(i));
			b.addFloatArg(currentPosition.x);
			b.addFloatArg(currentPosition.y);
			b.addFloatArg(contourFinderThree.getContourArea(i));
			b.addIntArg(velocity.length());
			sender.sendMessage(b);
		}
	}

	if (resetOSC) {
		resetOSC = false;
		sender.setup(ipAddress.get(), port.get());
	}
}

void ofApp::draw() {
	ofSetBackgroundAuto(showLabels);
	RectTracker& tracker = contourFinderOne.getTracker();

	ofSetColor(255);
	grabber.draw(0, 0);
	ofSetColor(mTargetColorOne);
	contourFinderOne.draw();
	for (int i = 0; i < contourFinderOne.size(); i++) {
		ofPoint center = toOf(contourFinderOne.getCenter(i));
		ofPushMatrix();
		ofTranslate(center.x, center.y);
		int label = contourFinderOne.getLabel(i);
		string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
		ofDrawBitmapString(msg, 0, 0);
		ofVec2f velocity = toOf(contourFinderOne.getVelocity(i));
		ofScale(5, 5);
		ofDrawLine(0, 0, velocity.x, velocity.y);
		ofPopMatrix();
	}

	tracker = contourFinderTwo.getTracker();
	ofSetColor(mTargetColorTwo);
	contourFinderTwo.draw();

	for (int i = 0; i < contourFinderTwo.size(); i++) {
		ofPoint center = toOf(contourFinderTwo.getCenter(i));
		ofPushMatrix();
		ofTranslate(center.x, center.y);
		int label = contourFinderTwo.getLabel(i);
		string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
		ofDrawBitmapString(msg, 0, 0);
		ofVec2f velocity = toOf(contourFinderTwo.getVelocity(i));
		ofScale(5, 5);
		ofDrawLine(0, 0, velocity.x, velocity.y);
		ofPopMatrix();
	}

	tracker = contourFinderThree.getTracker();
	ofSetColor(mTargetColorThree);
	contourFinderThree.draw();
	for (int i = 0; i < contourFinderThree.size(); i++) {
		ofPoint center = toOf(contourFinderThree.getCenter(i));
		ofPushMatrix();
		ofTranslate(center.x, center.y);
		int label = contourFinderThree.getLabel(i);
		string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
		ofDrawBitmapString(msg, 0, 0);
		ofVec2f velocity = toOf(contourFinderThree.getVelocity(i));
		ofScale(5, 5);
		ofDrawLine(0, 0, velocity.x, velocity.y);
		ofPopMatrix();
	}

	vector<Glow> followers = mTrackerOne.getFollowers();
	for (int i = 0; i < followers.size(); i++) {
		followers[i].draw();
	}

	followers = mTrackerTwo.getFollowers();
	for (int i = 0; i < followers.size(); i++) {
		followers[i].draw();
	}

	followers = mTrackerThree.getFollowers();
	for (int i = 0; i < followers.size(); i++) {
		followers[i].draw();
	}
	gui.draw();
}

void ofApp::mousePressed(int x, int y, int button) {
	if (ofGetKeyPressed(' ') && ofGetKeyPressed('1')) {
		mTargetColorOne = grabber.getPixels().getColor(x, y);
	}
	if (ofGetKeyPressed(' ') && ofGetKeyPressed('2')) {
		mTargetColorTwo = grabber.getPixels().getColor(x, y);
	}
	if (ofGetKeyPressed(' ') && ofGetKeyPressed('3')) {
		mTargetColorThree = grabber.getPixels().getColor(x, y);
	}
}

void ofApp::exit() {
	gui.saveToFile("settings.xml");
}

void ofApp::keyPressed(int key) {

}