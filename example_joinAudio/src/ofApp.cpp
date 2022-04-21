#include "ofApp.h"


void ofApp::setup() {

  
    ofSoundStreamSettings settings;

    // if you want to set the device id to be different than the default
    auto devices = soundStream.getDeviceList();
//    settings.device = devices[0];

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

    settings.setInDevice(devices[1]);
    settings.setInListener(this);
    settings.sampleRate = 44100;
    settings.numOutputChannels = 0;
    settings.numInputChannels = 1;
    settings.bufferSize = 1024;
    soundStream.setup(settings);
    
    m_Grabber.setDeviceID(0);
    m_Grabber.setup(1280,720);
    mCapFbo.allocate( m_Grabber.getWidth(), m_Grabber.getHeight(), GL_RGB );
    //mCapFbo.allocate( 1280,720, GL_RGB );

    //setup(bool recordVideo, bool recordAudio, glm::vec2 videoSize, float fps, unsigned int bitrate, const std::string &ffmpegPath)
    m_videoRecorder.setup(true, false, glm::vec2(m_Grabber.getWidth(), m_Grabber.getHeight()) );
//    m_videoRecorder.setAudioConfig(1024,44100);
    m_videoRecorder.setOverWrite(true);
    
    m_audioRecorder.setup(true, false, glm::vec2(m_Grabber.getWidth(), m_Grabber.getHeight()) );
    m_audioRecorder.setAudioConfig(1024,44100);
    m_audioRecorder.setOverWrite(true);

    
    #if defined(TARGET_OSX)
    m_videoRecorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg"));
    m_audioRecorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg"));
    #elif defined(TARGET_WIN32)
    m_videoRecorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe"));
    m_audioRecorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe"));
    #endif
    /**
     * You can also use the following methods to crop the output file
     *     m_Recorder.addAdditionalOutputArgument("-vf \"crop=300:300:0:0\"");
     * Or you can change the output codec
     *     m_Recorder.setVideCodec("libx264");
     **/

    ofAddListener(m_videoRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);

    isRecordingVideo = false;
    isRecordingAudio = false;

    //libx264
    ofLog()<<"getVideoCodec "<<m_videoRecorder.getVideoCodec();
    audioFPS = 0.0f;
    audioCounter    = 0;
    bufferCounter	= 0;
    lastAudioTimeReset = ofGetElapsedTimeMillis();
}

void ofApp::update() {
    if(m_Grabber.isInitialized()) {
//        ofLog()<<"m_Grabber.update()";
       m_Grabber.update();
    }
    
    if(bJoin == true && ofGetElapsedTimef() - joinTimer > 1){
        bJoin = false;
        m_videoRecorder.setOutputPath( ofToDataPath("test_joined.mov", true ));
        bool success = m_videoRecorder.joinVideoAudio(ofToDataPath("test.mov", true ),ofToDataPath("test.mp3", true ));
        if(success){
            ofLog()<<"yes stop";
            m_videoRecorder.stop(false);
        } else{
            ofLog()<<"not stop";
        }
    }
}

void ofApp::draw() {

    mCapFbo.begin(); {
        ofBackground(0);
        ofSetColor( 255 );
        if(m_Grabber.isInitialized())   m_Grabber.draw(0, 0, mCapFbo.getWidth(), mCapFbo.getHeight() );
        ofDrawCircle(mouseX,mouseY, 30);
       // ofSetColor(ofColor::green);
      //  if( m_Recorder.isRecording() ) ofSetColor( ofColor::red );
     //   ofDrawCircle( mCapFbo.getWidth()/2, mCapFbo.getHeight()/2, ((sin( ofGetElapsedTimef() * 6.) * 0.5 + 0.5) + 0.5) * 100 + 20);
     
    } mCapFbo.end();
    
    if( m_videoRecorder.isRecording() ) {
        if(isRecordingVideo){
            // ofxFastFboReader can be used to speed this up :)
            mCapFbo.readToPixels( mPix );
            if( mPix.getWidth() > 0 && mPix.getHeight() > 0 ) {
                m_videoRecorder.addFrame( mPix );
            }
        }
    }
    
    ofSetColor( 255 );
    mCapFbo.draw(0,0);

    ofSetColor(255,255,220);
    waveform.draw();


    if(isRecordingVideo){
        ofDrawBitmapStringHighlight(std::to_string(m_videoRecorder.getRecordedDuration()), 40, 45);
    }
    if(isRecordingAudio){
        ofDrawBitmapStringHighlight(std::to_string(m_audioRecorder.getRecordedAudioDuration(audioFPS)), 40, 145);
    }
//    else{
//        ofDrawBitmapStringHighlight(std::to_string(m_videoRecorder.getRecordedDuration()), 40, 45);
//    }
    ofDrawBitmapStringHighlight("FPS: " + std::to_string(ofGetFrameRate()), 10, 16);
    ofDrawBitmapStringHighlight("AFPS: " + std::to_string(ofGetFrameRate()), 140, 16);

    ofPushStyle(); {
        if (m_videoRecorder.isPaused() && m_videoRecorder.isRecording()) {
            ofSetColor(ofColor::yellow);
        }
        else if (m_videoRecorder.isRecording()) {
            ofSetColor(ofColor::red);
        }
        else {
            ofSetColor(ofColor::green);
        }
        ofDrawCircle(ofPoint(10, 40), 10);
        
        if (m_audioRecorder.isPaused() && m_audioRecorder.isRecording()) {
            ofSetColor(ofColor::yellow);
        }
        else if (m_audioRecorder.isRecording()) {
            ofSetColor(ofColor::red);
        }
        else {
            ofSetColor(ofColor::green);
        }
        ofDrawCircle(ofPoint(10, 140), 10);

        // Draw the information
        ofSetColor(ofColor::green);
        ofDrawBitmapStringHighlight("Press spacebar to toggle video record."
                                    "\nPress (t) to save thumbnail."
                                    "\nPress (a) to save audio stream."
                                    "\nPress (s) to start rtp video streaming."
                                    "\nPress (x) rec vid + audio and join.",
                                    10, ofGetHeight() - 200 );
    }
    ofPopStyle();
}

void ofApp::keyPressed(int key)
{

}

void ofApp::keyReleased(int key) {
    if (key == ' ') {
        
        if( m_videoRecorder.isRecording() ) {
            // stop
            m_videoRecorder.stop();
            isRecordingVideo = false;
        } else {
#if defined(TARGET_OSX)
            m_videoRecorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mov", true ));
#else
            m_videoRecorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".avi", true ));
#endif

            /*
             VideoCodec: libx264
             VideoBitrate: 2000k
             PixelFormat: rgb24
             OutputPixelFormat: yuv420p
             */
//            isRecordingAudio = true;
          
            m_videoRecorder.setVideoCodec("rawvideo");

//            m_videoRecorder.setVideoCodec("libx264");
            m_videoRecorder.setBitRate(8000);
            m_videoRecorder.setPixelFormat(OF_IMAGE_COLOR);
            
            isRecordingVideo = true;
            m_videoRecorder.startCustomRecord();
        }
    } else if (key == 't') {
        m_videoRecorder.saveThumbnail(0, 0, 2, ofToDataPath("thumbnail.png"), ofVec2f(0, 0), ofRectangle(0, 0, 500, 400));
    }else if(key == 's'){
        m_videoRecorder.startCustomStreaming();
    }else if(key == 'a'){
        if( m_audioRecorder.isRecording() ) {
            // stop
            m_audioRecorder.stop();
            isRecordingAudio = false;
        } else if(audioFPS != 0) {
            m_audioRecorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mp3", true ));
            isRecordingAudio = true;
            m_audioRecorder.startCustomAudioRecord();
        }
    }
    
    if(key == 'c')  m_Grabber.close();
    
    if(key == 'm'){
       // m_Grabber.close() does not seem to be enough to release ofApp camera use
        //i needed to uncomment all uses of m_Grabber in order for m_manualRecorder to not throw error
        
        m_manualRecorder.setup(true, false, glm::vec2(1280,720) );
        m_manualRecorder.setOverWrite(true);
        //        m_manualRecorder.setAudioConfig(1024,44100);
#if defined(TARGET_OSX)
        m_manualRecorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg"));
#elif defined(TARGET_WIN32)
        m_manualRecorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe"));
#endif
        if( m_manualRecorder.isRecording() ) {
            m_manualRecorder.stop();
            m_manualRecorder.setRecordVideo(false);
            m_manualRecorder.setRecordAudio(false);
//            isRecordingVideo = false;
        }else{
//            isRecordingVideo = true;
            
//            m_Grabber.close();
//            m_manualRecorder.setVideoCodec("hap -format hap ");
            m_manualRecorder.setRecordVideo(true);
            m_manualRecorder.setRecordAudio(true);
#if defined(TARGET_OSX)
            m_manualRecorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mov", true ));
#else
            m_manualRecorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".avi", true ));
#endif
            m_manualRecorder.record(0); //5);
        }
    }
    
    if(key == 'x'){
        
        if( m_videoRecorder.isRecording() ) {
            // stop
            m_videoRecorder.stop();
            isRecordingVideo = false;
            
        }else{
#if defined(TARGET_OSX)
            m_videoRecorder.setOutputPath( ofToDataPath("test.mov", true ));
#else
            m_videoRecorder.setOutputPath( ofToDataPath("test.avi", true ));
#endif
            m_videoRecorder.setVideoCodec("hap");
//            m_videoRecorder.setVideoCodec("rawvideo");
//            m_videoRecorder.setVideoCodec("libx264");
            m_videoRecorder.setBitRate(8000);
//            m_videoRecorder.setPixelFormat(OF_IMAGE_COLOR);
//            m_videoRecorder.setVideoOutCodec("hap -format hap ");
            isRecordingVideo = true;
            m_videoRecorder.startCustomRecord();
        }
        
        if( m_audioRecorder.isRecording() ) {
            // stop
          
            m_audioRecorder.stop();
            isRecordingAudio = false;
            
            //TODO: calling join video + audio here seems to be too soon since process fails
//            m_videoRecorder.setOutputPath( ofToDataPath("test_joined.mov", true ));
//            m_videoRecorder.joinVideoAudio(ofToDataPath("test.mov", true ),ofToDataPath("test.mp3", true ));
            
        } else {
            m_audioRecorder.setOutputPath( ofToDataPath("test.mp3", true ));
            isRecordingAudio = true;
            m_audioRecorder.startCustomAudioRecord();
        }
    }
    
    if(key == 'j'){
        ofLog()<<"manually trigger joining with recordingComplete being send once done ";
        m_videoRecorder.setOutputPath( ofToDataPath("test_joined.mov", true ));
       bool success = m_videoRecorder.joinVideoAudio(ofToDataPath("test.mov", true ),ofToDataPath("test.mp3", true ));
        if(success){
            ofLog()<<"yes stop";
            m_videoRecorder.stop();
        } else{
            ofLog()<<"not stop";
        }
    }
    if(key == 'k'){
        ofLog()<<"manually trigger joining without recordingComplete being send ";
        m_videoRecorder.setOutputPath( ofToDataPath("test_joined.mov", true ));
       bool success = m_videoRecorder.joinVideoAudio(ofToDataPath("test.mov", true ),ofToDataPath("test.mp3", true ));
        if(success){
            ofLog()<<"yes stop";
            m_videoRecorder.stop(false);
        } else{
            ofLog()<<"not stop";
        }
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



    if(m_audioRecorder.isRecording() && isRecordingAudio){
        m_audioRecorder.addBuffer(input,audioFPS);
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

//--------------------------------------------------------------
void ofApp::recordingComplete(RecorderOutputFileCompleteEventArgs& args){
    cout << "The recoded video file is now complete." << args.fileName    <<endl;
    
    if(ofIsStringInString(args.fileName, "test_joined") == false){
        joinTimer = ofGetElapsedTimef();
        bJoin = true;
    }
    
    //TODO: calling join video + audio here seems to be too soon since process fails
//    if(ofIsStringInString(args.fileName, "test_joined") == false){
//        m_videoRecorder.setOutputPath( ofToDataPath("test_joined.mov", true ));
//        bool success = m_videoRecorder.joinVideoAudio(ofToDataPath("test.mov", true ),ofToDataPath("test.mp3", true ));
//        if(success){
//            ofLog()<<"yes stop";
//            m_videoRecorder.stop(false);
//        } else{
//            ofLog()<<"not stop";
//        }
//    }
}
