#include "ofApp.h"


void ofApp::setup() {

    ofSetFrameRate(30);
  
    ofSoundStreamSettings settings;

    // if you want to set the device id to be different than the default
    auto devices = soundStream.getDeviceList();
    //settings.device = devices[2];

    // you can also get devices for an specific api
    // auto devices = soundStream.getDevicesByApi(ofSoundDevice::Api::PULSE);
    // settings.device = devices[0];

    // or get the default device for an specific api:
    // settings.api = ofSoundDevice::Api::PULSE;

    // or by name
   // auto devices = soundStream.getMatchingDevices("default");
    //if(!devices.empty()){
     //   settings.setInDevice(devices[0]);
   // }

    settings.setInDevice(devices[0]);
    settings.setInListener(this);
    settings.sampleRate = 44100;
    settings.numOutputChannels = 0;
    settings.numInputChannels = 1;
    settings.bufferSize = 1024;
    soundStream.setup(settings);
//    m_Grabber.setup(1920,1080);
    m_Grabber.setup(1280,720);
    mCapFbo.allocate( m_Grabber.getWidth(), m_Grabber.getHeight(), GL_RGBA );
//    mCapFbo.allocate( m_Grabber.getWidth(), m_Grabber.getHeight(), GL_RGB );
    //mCapFbo.allocate( 1280,720, GL_RGB );

    //setup(bool recordVideo, bool recordAudio, glm::vec2 videoSize, float fps, unsigned int bitrate, const std::string &ffmpegPath)
    m_Recorder.setup(true, false, glm::vec2(m_Grabber.getWidth(), m_Grabber.getHeight()) );
    m_Recorder.setAudioConfig(1024,44100);
    m_Recorder.setOverWrite(true);
    
    
    #if defined(TARGET_OSX)
    m_Recorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg"));
    #elif defined(TARGET_WIN32)
    m_Recorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe"));
    #endif
    /**
     * You can also use the following methods to crop the output file
     *     m_Recorder.addAdditionalOutputArgument("-vf \"crop=300:300:0:0\"");
     * Or you can change the output codec
     *     m_Recorder.setVideCodec("libx264");
     **/

    isRecordingVideo = false;
    isRecordingAudio = false;

    //libx264
    ofLog()<<"getVideoCodec "<<m_Recorder.getVideoCodec();
    audioFPS = 0.0f;
    audioCounter    = 0;
    bufferCounter    = 0;
    lastAudioTimeReset = ofGetElapsedTimeMillis();
}

void ofApp::update() {
     m_Grabber.update();
}

void ofApp::draw() {
    
    
    mCapFbo.begin(); {
        ofClear(0,0,0, 0);
        //        ofBackground(0);
        ofSetColor( 255 );
        m_Grabber.draw(100, 0, mCapFbo.getWidth(), mCapFbo.getHeight() );
        
        ofSetColor( 255,127 );
        ofDrawCircle(mouseX,mouseY, 30);
        // ofSetColor(ofColor::green);
        //  if( m_Recorder.isRecording() ) ofSetColor( ofColor::red );
        //   ofDrawCircle( mCapFbo.getWidth()/2, mCapFbo.getHeight()/2, ((sin( ofGetElapsedTimef() * 6.) * 0.5 + 0.5) + 0.5) * 100 + 20);
        
    } mCapFbo.end();
    
    if( m_Recorder.isRecording() ) {
        if(isRecordingVideo){
            // ofxFastFboReader can be used to speed this up :)
            ofPixels mPix;
            mCapFbo.readToPixels( mPix );
//            mPix.setImageType(OF_IMAGE_COLOR);
            if( mPix.getWidth() > 0 && mPix.getHeight() > 0 ) {
//                ofLog()<<"mCapFbo "<<mCapFbo.getWidth()<<"x"<<mCapFbo.getHeight()<<" , "<<mPix.getWidth()<<"x"<<mPix.getHeight();
                m_Recorder.addFrame( mPix );
            }
        }
    }
    ofSetColor(255,0,0);
    ofDrawRectangle(5, 5, 300, 200);
    ofSetColor( 255 );
    mCapFbo.draw(0,0);
    
    ofSetColor(255,255,220);
    waveform.draw();
    
    
    if(isRecordingVideo){
        ofDrawBitmapStringHighlight(std::to_string(m_Recorder.getRecordedDuration()), 40, 45);
    }else if(isRecordingAudio){
        ofDrawBitmapStringHighlight(std::to_string(m_Recorder.getRecordedAudioDuration(audioFPS)), 40, 45);
    }else{
        ofDrawBitmapStringHighlight(std::to_string(m_Recorder.getRecordedDuration()), 40, 45);
    }
    ofDrawBitmapStringHighlight("FPS: " + std::to_string(ofGetFrameRate()), 10, 16);
    ofDrawBitmapStringHighlight("AFPS: " + std::to_string(m_Recorder.getFps()), 140, 16);
    
    ofPushStyle(); {
        if (m_Recorder.isPaused() && m_Recorder.isRecording()) {
            ofSetColor(ofColor::yellow);
        }
        else if (m_Recorder.isRecording()) {
            ofSetColor(ofColor::red);
        }
        else {
            ofSetColor(ofColor::green);
        }
        ofDrawCircle(ofPoint(10, 40), 10);

        // Draw the information
        ofSetColor(ofColor::green);
        ofDrawBitmapStringHighlight("Press spacebar to toggle video record."
                                    "\nPress (t) to save thumbnail."
                                    "\nPress (a) to save audio stream."
                                    "\nPress (s) to start rtp video streaming.",
                                    10, ofGetHeight() - 200 );
    }
    ofPopStyle();
}

void ofApp::keyPressed(int key)
{

}

void ofApp::keyReleased(int key) {
    if (key == ' ') {
        
        if( m_Recorder.isRecording() ) {
            // stop
            m_Recorder.stop();
            isRecordingVideo = false;
        } else {
#if defined(TARGET_OSX)
            m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mov", true ));
//            m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mkv", true ));
            
#else
            m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".avi", true ));
#endif

            
            m_Recorder.setVideoCodec("hap -format hap_alpha");
            m_Recorder.setBitRate(8000);
            m_Recorder.setPixelFormat(OF_IMAGE_COLOR_ALPHA);
            
         
            /*
             m_Recorder.setVideoCodec("prores_ks");
             m_Recorder.setBitRate(800);
             m_Recorder.setPixelFormat(OF_IMAGE_COLOR_ALPHA);
             */
            isRecordingVideo = true;
            m_Recorder.startCustomRecord();
        }
    } else if (key == 't') {
        m_Recorder.saveThumbnail(0, 0, 2, ofToDataPath("thumbnail.png"), ofVec2f(0, 0), ofRectangle(0, 0, 500, 400));
    }else if(key == 's'){
        m_Recorder.startCustomStreaming();
    }else if(key == 'a'){
        if( m_Recorder.isRecording() ) {
            // stop
            m_Recorder.stop();
            isRecordingAudio = false;
        } else if(audioFPS != 0) {
            m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mp3", true ));
            isRecordingAudio = true;
            m_Recorder.startCustomAudioRecord();
        }
    }
    if(key == 'c') m_Grabber.close();
    if(key == 'm'){
        
        m_Recorder.setRecordVideo(true);
        m_Recorder.setRecordAudio(true);
#if defined(TARGET_OSX)
            m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mp4", true ));
#else
            m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".avi", true ));
#endif
        m_Recorder.record(5);
    }
}

void ofApp::mouseMoved(int x, int y)
{

}

void ofApp::mouseDragged(int x, int y, int button)
{

}

void ofApp::mousePressed(int x, int y, int button)
{
    
}

void ofApp::mouseReleased(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){

    if(ofGetElapsedTimeMillis()-lastAudioTimeReset >= 1000){
        lastAudioTimeReset = ofGetElapsedTimeMillis();
        audioFPS = audioCounter;
        audioCounter = 0;
    }else{
        audioCounter++;
    }



    if(m_Recorder.isRecording() && isRecordingAudio){
        m_Recorder.addBuffer(input,audioFPS);
    }

    waveform.clear();
    for(size_t i = 0; i < input.getNumFrames(); i++) {
        float sample = input.getSample(i,0);
        float x = ofMap(i, 0, input.getNumFrames(), 0, ofGetWidth());
        float y = ofMap(sample, -1, 1, 0, ofGetHeight());
        waveform.addVertex(x, y);
    }

    bufferCounter++;

}
