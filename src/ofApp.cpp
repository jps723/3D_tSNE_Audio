#include "ofApp.h"
#include "ofJson.h"

//--------------------------------------------------------------
void ofApp::setup(){
    //camera setup for 3d
    //ofEnableDepthTest();
    ofSetVerticalSync(true);
    cam.setDistance(1000);    
    
    // this should point to the json file containing your audio files and tsne coordinates
    // follow instructions in ml4a.github.io/guides/AudioTSNEViewer/
    tsnePath = ofToDataPath("tsne-3d-284SoundsFromMars.json");
    load(tsnePath);
    //adds button for loading the model
    bLoad.addListener(this, &ofApp::eLoad);
    //GUI setup code
    gui.setup();
    gui.setName("Audio t-SNE");
    gui.add(maxDuration.set("maxDuration", 1.0, 0.1, 2.0));
    gui.add(mouseRadius.set("mouseRadius", 250, 100, 500));
    gui.add(pauseLength.set("pauseLength", 2.0, 0.2, 5.0));
    gui.add(bLoad.setup("Load model"));
    
    
}

//--------------------------------------------------------------
void ofApp::eLoad() {
    //Check for loading an external model
    ofFileDialogResult result = ofSystemLoadDialog("Which xml file to load?", true);
    if (result.bSuccess) {
        load(result.filePath);
    }
}

//--------------------------------------------------------------
void ofApp::load(string filename) {
    tsnePath = filename;
    int counter = 0;
    ofJson js;
    ofFile file(tsnePath);
    parsingSuccessful = file.exists();
    
    if (!parsingSuccessful) {
        ofLog(OF_LOG_ERROR) << "parsing not successful";
        return;
    }
    sounds.clear();
    //parses the json file into filepath and x,y,z coordinates
    file >> js;
    for (auto & entry: js) {
        if(!entry.empty()) {
            string samplePath = entry["path"];
            float x = entry["point"][0];
            float y = entry["point"][1];
            float z = entry["point"][2];
            
            //creates a new AudioClip and maps the sounds to the given path and x,y,z coordinates
            AudioClip newSound;
            newSound.sound.load(samplePath);
            newSound.point.setPosition(x, y, z);
            //sets time to 0
            newSound.t = 0;
            newSound.filename = samplePath;
            
            //pushes the sounds into an array
            sounds.push_back(newSound);
            if(counter == 1018){
                std::cout << samplePath;
            }
            counter++;
        }
    }

}

//--------------------------------------------------------------
void ofApp::update(){
    if (!parsingSuccessful) {
        return;
    }
    //Stop the sound if it is playing and the sound has reached it's maximum duration
    for (int i=0; i<sounds.size(); i++) {
        if (sounds[i].sound.isPlaying() && sounds[i].sound.getPositionMS() > maxDuration*1000) {
            sounds[i].sound.stop();
        }
    }
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    //ofEnableDepthTest();
    ofBackgroundGradient(ofColor(100, 200, 0), ofColor(200, 100, 0));
    if (!parsingSuccessful) {
        ofDrawBitmapString("Could not find file "+tsnePath+"\nSee the instructions for how to create one.", 50, 50);
        return;
    }
    //set the color to green if the sound is playing/the object is selected, otherwise set color to red
    for (int i=0; i<sounds.size(); i++) {
        if (sounds[i].sound.isPlaying()) {
            ofSetColor(0, 255, 0, 180);
        }
        else {
            ofSetColor(255, 0, 0, 180);
        }
        //begin camera and draw a box at the x,y,z coords for each sound in the json
        cam.begin();
        ofDrawBox(ofGetWidth() * sounds[i].point.getX(), ofGetHeight() * sounds[i].point.getY(), ofGetWidth() * sounds[i].point.getZ(), 8);
        
        cam.end();
        //draw black circles in the 2d arrangement.
        //ofSetColor(0);
        //ofDrawCircle(ofGetWidth() * sounds[i].point.getX(), ofGetHeight() * sounds[i].point.getY(), 4);
    }
    int n = sounds.size();
    ofVec2f nearestVertex;
    ofVec2f mouse(mouseX, mouseY);
    for(int i = 0; i < n; i++) {
        glm::vec3 glmMouse;
        glmMouse = glm::vec3(ofGetWidth() * sounds[i].point.getX(),ofGetHeight() * sounds[i].point.getY(),ofGetWidth() * sounds[i].point.getZ());
        ofVec3f cur = cam.worldToScreen(glmMouse);
        float distance = cur.distance(mouse);
        if(i == 0 || distance < nearestDistance) {
            nearestDistance = distance;
            nearestVertex = cur;
            nearestIndex = i;
        }
    }
    
    ofSetColor(ofColor::gray);
    ofDrawLine(nearestVertex, mouse);
    
    ofNoFill();
    ofSetColor(ofColor::yellow);
    ofSetLineWidth(2);
    ofDrawCircle(nearestVertex, 4);
    ofSetLineWidth(1);
    
    ofVec2f offset(10, -10);
    ofDrawBitmapStringHighlight(ofToString(nearestIndex), mouse + offset);
//    std::cout << "nearest vertex: " << nearestVertex << endl;
//    std::cout << "nearest distance: " << nearestDistance << endl;
//    std::cout << "cur: " << ofFile::cur << endl;
    ofDisableDepthTest();
    gui.draw();
    ofEnableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    //for all sounds in the array, measure the distance to the mouse and only trigger if the mouse is within the mouseRadius
    //variable and the sound isn't already playing
//    for (int i=0; i<sounds.size(); i++) {
//        float distanceToMouse = ofDistSquared(x, y, z, ofGetWidth() * sounds[i].point.getX(), ofGetHeight() * sounds[i].point.getY(), ofGetWidth() * sounds[i].point.getZ());
//        if (distanceToMouse < mouseRadius && !sounds[i].sound.isPlaying() && (ofGetElapsedTimef() - sounds[i].t > pauseLength)) {
//            sounds[i].t = ofGetElapsedTimef();
//            sounds[i].sound.play();
//        }
//    }
    
    if (nearestIndex < mouseRadius &&
        !sounds[nearestIndex].sound.isPlaying() &&
        (ofGetElapsedTimef() - sounds[nearestIndex].t > pauseLength)) {
//        if(nearestDistance < 10){
        sounds[nearestIndex].t = ofGetElapsedTimef();
        sounds[nearestIndex].sound.play();
        std::cout << "Sample Path: " << sounds[nearestIndex].filename << endl;
//        }else{
//            std::cout << nearestDistance << endl;
//        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
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
