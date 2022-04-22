#include "ofxFFmpegRecorder.h"
// openFrameworks
#include "ofLog.h"
#include "ofVideoGrabber.h"
#include "ofSoundStream.h"

// Logging macros
#define LOG_ERROR(message) ofLogError("") << __FUNCTION__ << ":" << __LINE__ << ": " << message
#define LOG_WARNING(message) ofLogWarning("") << __FUNCTION__ << ":" << __LINE__ << ": " << message
#define LOG_NOTICE(message) ofLogNotice("") << __FUNCTION__ << ":" << __LINE__ << ": " << message

ofxFFmpegRecorder::ofxFFmpegRecorder()
: m_FFmpegPath("ffmpeg")
, m_OutputPath("")
, m_IsRecordVideo(false)
, m_IsRecordAudio(false)
, m_IsOverWrite(false)
, m_IsPaused(false)
, m_VideoSize(0, 0)
, m_BitRate(2000)
, m_AddedVideoFrames(0)
, m_AddedAudioFrames(0)
, m_Fps(30.f)
, m_bufferSize(1024)
, m_sampleRate(44100)
, m_CaptureDuration(0.f)
, m_TotalPauseDuration(0.f)
, m_DefaultVideoDevice()
, m_DefaultAudioDevice()
, m_VideCodec("mpeg4")
//, m_VideOutCodec("mpeg4")
, m_AudioCodec("libmp3lame")
, m_CustomRecordingFile(nullptr)
, m_DefaultRecordingFile(nullptr)
, initialized(false)
{
    
}

ofxFFmpegRecorder::~ofxFFmpegRecorder()
{
    stop();
}

void ofxFFmpegRecorder::setup(bool recordVideo, bool recordAudio, glm::vec2 videoSize, float fps, unsigned int bitrate, const std::string &ffmpegPath)
{
    m_IsRecordVideo = recordVideo;
    m_IsRecordAudio = recordAudio;
    m_VideoSize = videoSize;

    m_Fps = fps;
    m_BitRate = bitrate;

    if (ffmpegPath.length() > 0) {
        m_FFmpegPath = ffmpegPath;
    }
}

void ofxFFmpegRecorder::setAudioConfig(int bufferSize, int sampleRate){
    m_bufferSize = bufferSize;
    m_sampleRate = sampleRate;
}

bool ofxFFmpegRecorder::isRecordVideo() const
{
    return m_IsRecordVideo;
}

void ofxFFmpegRecorder::setRecordVideo(bool record)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_IsRecordVideo = record;
}

bool ofxFFmpegRecorder::isRecordAudio() const
{
    return m_IsRecordAudio;
}

void ofxFFmpegRecorder::setRecordAudio(bool record)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_IsRecordAudio = record;
}

std::string ofxFFmpegRecorder::getFFmpegPath() const
{
    return m_FFmpegPath;
}

void ofxFFmpegRecorder::setFFmpegPath(const std::string &path)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_FFmpegPath = path;
}

void ofxFFmpegRecorder::setFFmpegPathToAddonsPath() {
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }
    std::string systemFolder = "vs";
    #if defined(TARGET_OSX)
    systemFolder = "osx";
    #endif
    m_FFmpegPath = ofToDataPath("../../../../../addons/ofxFFmpegRecorder/libs/ffmpeg/lib/"+systemFolder+"/ffmpeg", true);
}

float ofxFFmpegRecorder::getCaptureDuration() const
{
    return m_CaptureDuration;
}

void ofxFFmpegRecorder::setCaptureDuration(float duration)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_CaptureDuration = duration;
}

ofVideoDevice ofxFFmpegRecorder::getDefaultVideoDevice() const
{
    return m_DefaultVideoDevice;
}

void ofxFFmpegRecorder::setDefaultVideoDevice(const ofVideoDevice &device)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_DefaultVideoDevice = device;
}

ofSoundDevice ofxFFmpegRecorder::getDefaultAudioDevice() const
{
    return m_DefaultAudioDevice;
}

void ofxFFmpegRecorder::setDefaultAudioDevice(const ofSoundDevice &device)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_DefaultAudioDevice = device;
}

std::string ofxFFmpegRecorder::getOutputPath() const
{
    return m_OutputPath;
}

void ofxFFmpegRecorder::setOutputPath(const std::string &path)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_OutputPath = path;
}

float ofxFFmpegRecorder::getFps() const
{
    return m_Fps;
}

void ofxFFmpegRecorder::setFps(float fps)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_Fps = fps;
}

unsigned int ofxFFmpegRecorder::getBitRate() const
{
    return m_BitRate;
}

void ofxFFmpegRecorder::setBitRate(unsigned int rate)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_BitRate = rate;
}

std::string ofxFFmpegRecorder::getVideoCodec() const
{
    return m_VideCodec;
}
std::string ofxFFmpegRecorder::getVideoOutCodec() const
{
    return m_VideOutCodec;
}

void ofxFFmpegRecorder::setVideoCodec(const std::string &codec)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_VideCodec = codec;
}

void ofxFFmpegRecorder::setVideoOutCodec(const std::string &codec)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_VideOutCodec = codec;
}

float ofxFFmpegRecorder::getWidth() {
    return m_VideoSize.x;
}
void ofxFFmpegRecorder::setWidth(float aw) {
    m_VideoSize.x = aw;
}
float ofxFFmpegRecorder::getHeight() {
    return m_VideoSize.y;
}
void ofxFFmpegRecorder::setHeight(float ah) {
    m_VideoSize.y = ah;
}

bool ofxFFmpegRecorder::isPaused() const
{
    return m_IsPaused;
}

void ofxFFmpegRecorder::setPaused(bool paused)
{
    if (m_CustomRecordingFile == nullptr) {
        LOG_WARNING("Cannot pause the default webcam recording.");
    }
    else {
        if (paused && m_IsPaused == false) {
            m_PauseStartTime = std::chrono::high_resolution_clock::now();
        }
        else if (paused == false && m_IsPaused) {
            m_PauseEndTime = std::chrono::high_resolution_clock::now();
            float delta = std::chrono::duration<float>(m_PauseEndTime - m_PauseStartTime).count();
            m_TotalPauseDuration += delta;
        }

        m_IsPaused = paused;
    }
}

void ofxFFmpegRecorder::setPixelFormat(ofImageType aType)
{
    mPixFmt = "rgb24";
    pixChannelCount = 3;
    if (aType == OF_IMAGE_COLOR) {
        mPixFmt = "rgb24";
        pixChannelCount = 3;
    }
    else if (aType == OF_IMAGE_COLOR_ALPHA) {
        mPixFmt = "rgba";
        pixChannelCount = 4;
    }
    else if (aType == OF_IMAGE_GRAYSCALE) {
        mPixFmt = "gray";
        pixChannelCount = 1;
    }
    else {
        ofLogError() << "unsupported format, setting to OF_IMAGE_COLOR";
    }
}

float ofxFFmpegRecorder::getRecordedDuration() const
{
    return m_AddedVideoFrames / m_Fps;
}

float ofxFFmpegRecorder::getRecordedAudioDuration(float afps) const
{
    return m_AddedAudioFrames / afps;
}

//MARK: record directly from device not through ofVideoGrabber
bool ofxFFmpegRecorder::record(float duration)
{
    if (isRecording()) {
        LOG_ERROR("A recording is already in proggress.");
        return false;
    }

    if (m_OutputPath.length() == 0) {
        LOG_ERROR("Output path is empty. Cannot record.");
        return false;
    }

    if (ofFile::doesFileExist(m_OutputPath, false) && m_IsOverWrite == false) {
        LOG_ERROR("The output file already exists and overwriting is disabled. Cannot capture video.");
        return false;
    }

    determineDefaultDevices();
    m_CaptureDuration = duration;

//    std::vector<std::string> args;
    std::string args = "";
    
#if defined(_WIN32)
    args += "-f dshow";
#elif defined(__APPLE__)
    args += "-f avfoundation";
#else
    args += "-f v4l2";
#endif // PLATFORM_CHECK
    
    args += " -y";
    
    if (m_CaptureDuration > 0.f) {
        args += " -t " + std::to_string(m_CaptureDuration); //-t duration , http://www.ffmpeg.org/ffmpeg.html#Main-options
    }
    
    args += " -framerate " + std::to_string(m_Fps); // set framerate
    args += " -video_size " + std::to_string(static_cast<unsigned int>(m_VideoSize.x)) + "x" + std::to_string(static_cast<unsigned int>(m_VideoSize.y)); // -s same as -video_size sets input resolution
    args += " -pixel_format uyvy422";
    
    std::string inputDevices;
    inputDevices += "\"";
    if (m_IsRecordVideo) {
        //        inputDevices += "video=\"" + m_DefaultVideoDevice.deviceName + "\"";
        inputDevices += ofToString(m_DefaultVideoDeviceIndex);
    }
    
    inputDevices += ":";
    
    if (m_IsRecordAudio) {
        inputDevices += ofToString(m_DefaultAudioDeviceIndex);
    }
    inputDevices += "\"";
   
    //    std::copy(m_AdditionalInputArguments.begin(), m_AdditionalInputArguments.end(), std::back_inserter(args));
    //    args = 
    args += " -i " + inputDevices;
    args += " -vcodec " + m_VideCodec;// alias for -c:v -codec:v,  https://ffmpeg.org/ffmpeg.html#Video-Options
    args += " -b:v " + std::to_string(m_BitRate) + "k";
    args += " -framerate " + std::to_string(m_Fps);
//    args += " -pix_fmt yuv420p";
    args += " -pix_fmt rgba";
    args += " " + m_OutputPath;

//    std::copy(m_AdditionalOutputArguments.begin(), m_AdditionalOutputArguments.end(), std::back_inserter(args));
    
    std::string cmd = m_FFmpegPath + " ";
    cmd += args + " ";
    ofLog()<<"cmd:"<<cmd;
    
#if defined(_WIN32)
    m_DefaultRecordingFile = _popen(cmd.c_str(), "w");
#else
    m_DefaultRecordingFile = popen(cmd.c_str(), "w");
#endif

    return true;
    
    //for better audio quality and sync record video and audio on different threads
    ///Users/stephanschulz/Downloads/ffmpeg -f avfoundation -framerate 30 -thread_queue_size 1024 -pixel_format uyvy422 -i "0:" -f avfoundation -thread_queue_size 1024 -i ":0" -c:v hap -format hap -y out2.mov -async 1
    
}

bool ofxFFmpegRecorder::startCustomRecord()
{
    //this method of recording relies on ofPixel frame being added in ofApp::draw()
    
    if (isRecording()) {
        LOG_ERROR("A recording is already in proggress.");
        return false;
    }

    if (m_OutputPath.length() == 0) {
        LOG_ERROR("Output path is empty. Cannot record.");
        return false;
    }

    if (ofFile::doesFileExist(m_OutputPath, false) && m_IsOverWrite == false) {
        LOG_ERROR("The output file already exists and overwriting is disabled. Cannot capture video.");
        return false;
    }
    
    m_AddedVideoFrames = 0;
    
    std::vector<std::string> args;
    std::copy(m_AdditionalInputArguments.begin(), m_AdditionalInputArguments.end(), std::back_inserter(args));
    
    
    //input pixel buffer / video frame settings
    args.push_back("-y"); // -y Overwrite output files without asking, https://ffmpeg.org/ffmpeg.html#Main-options
    args.push_back("-an"); //-an option drops the audio, https://ffmpeg.org/ffmpeg.html#Audio-Options
    //    args.push_back("-r " + std::to_string(m_Fps)); // set framerate
    args.push_back("-framerate " + std::to_string(m_Fps)); // set framerate
    args.push_back("-video_size " + std::to_string(static_cast<unsigned int>(m_VideoSize.x)) + "x" + std::to_string(static_cast<unsigned int>(m_VideoSize.y))); // -s same as -video_size sets input resolution
    args.push_back("-f rawvideo"); // -f video format, rawvideo since we manually add FBO pixel buffer frames
    //    args.push_back("-pix_fmt rgb24");
    args.push_back("-pix_fmt " + mPixFmt); //-pixel format, https://ffmpeg.org/ffmpeg.html#Advanced-Video-options rgb24 for GL_RGB and rgba for GL_RGBA
//    args.push_back("-vcodec " + m_VideCodec); //rawvideo");
    args.push_back("-vcodec rawvideo");
    
    //input file options, https://ffmpeg.org/ffmpeg.html#toc-Description
    //in this video capture example input file == output file 
    args.push_back("-i -");
    args.push_back("-vcodec " + m_VideCodec); //m_VideOutCodec);// alias for -c:v -codec:v,  https://ffmpeg.org/ffmpeg.html#Video-Options
//    args.push_back("-c:v hap -format hap "); 
    args.push_back("-b:v " + std::to_string(m_BitRate) + "k"); // video bitrate, 
    args.push_back("-framerate " + std::to_string(m_Fps));
//    args.push_back("-pix_fmt yuv420p"); //
//    args.push_back("-pix_fmt rgba");
   
    //m_AdditionalOutputArguments like filters or conversion to other codec like HAP
    std::copy(m_AdditionalOutputArguments.begin(), m_AdditionalOutputArguments.end(), std::back_inserter(args));
    
    /*
     args.push_back("-y");
     args.push_back("-an");
     args.push_back("-r " + std::to_string(m_Fps));
     args.push_back("-framerate " + std::to_string(m_Fps));
    args.push_back("-s " + std::to_string(static_cast<unsigned int>(m_VideoSize.x)) + "x" + std::to_string(static_cast<unsigned int>(m_VideoSize.y)));
    args.push_back("-f rawvideo");
    //args.push_back("-pix_fmt rgb24");
    args.push_back("-pix_fmt " + mPixFmt);
    args.push_back("-vcodec rawvideo");
    args.push_back("-i -");
    

    args.push_back("-vcodec " + m_VideOutCodec);
    args.push_back("-b:v " + std::to_string(m_BitRate) + "k");
    args.push_back("-r " + std::to_string(m_Fps));
    args.push_back("-pix_fmt yuv420p");
    args.push_back("-framerate " + std::to_string(m_Fps));
     */
    
    /*
     -f fmt (input/output)
     Force input or output file format. The format is normally auto detected for input files and guessed from the file extension for output files, so this option is not needed in most cases.
     
     -f rawvideo is basically a dummy setting that tells ffmpeg that your video is not in any container
     
     -vcodec rawvideo means that the video data within the container is not compressed
     However, there are many ways uncompressed video could be stored, so it is necessary to specify the -pix_fmt option
     
     specified by the -i
     reads from an arbitrary number of input "files" (which can be regular files, pipes, network streams, grabbing devices, etc.), 
     
     -y (global)
     Overwrite output files without asking.
     
     -an (input/output)
     As an input option, blocks all audio streams of a file from being filtered or being automatically selected or mapped for any output. 
     
     -s[:stream_specifier] size (input/output,per-stream)
     Set frame size.

     As an input option, this is a shortcut for the video_size private option, recognized by some demuxers for which the frame size is either not stored in the file or is configurable – e.g. raw video or video grabbers.

     As an output option, this inserts the scale video filter to the end of the corresponding filtergraph. Please use the scale filter directly to insert it at the beginning or some other place.

     The format is ‘wxh’ (default - same as source).
     
     -vcodec codec (output)
     Set the video codec. This is an alias for -codec:v
     
     */
    
    std::copy(m_AdditionalOutputArguments.begin(), m_AdditionalOutputArguments.end(), std::back_inserter(args));
    
    args.push_back(m_OutputPath);
//    args.push_back("-codecs ");

    std::string cmd = m_FFmpegPath + " ";
    for (auto arg : args) {
        cmd += arg + " ";
    }
    
    ofLog()<<"cmd: "<<cmd;
//../../../data/ffmpeg/osx/ffmpeg -y -an -r 30.000000 -framerate 30.000000 -s 1280x720 -f rawvideo -pix_fmt rgb24 -vcodec rawvideo -i - -vcodec libx264 -b:v 8000k -r 30.000000 -framerate 30.000000 /Applications/of_v0.11.0_osx_release/addons/ofxFFmpegRecorder/example/bin/data/2021-02-05-12-56-34-203.mp4 
    
#if defined(_WIN32)
    m_CustomRecordingFile = _popen(cmd.c_str(), "wb");
#else
//    m_CustomRecordingFile = _popen(cmd.c_str(), "w");
    m_CustomRecordingFile = popen( cmd.c_str(), "w" );
#endif // _WIN32

    initialized = true;
    return true;
}

bool ofxFFmpegRecorder::startCustomAudioRecord()
{
    if (isRecording()) {
        LOG_ERROR("A recording is already in proggress.");
        return false;
    }

    if (m_OutputPath.length() == 0) {
        LOG_ERROR("Output path is empty. Cannot record.");
        return false;
    }

    if (ofFile::doesFileExist(m_OutputPath, false) && m_IsOverWrite == false) {
        LOG_ERROR("The output file already exists and overwriting is disabled. Cannot capture audio.");
        return false;
    }

    m_AddedAudioFrames = 0;

    std::vector<std::string> args;
    std::copy(m_AdditionalInputArguments.begin(), m_AdditionalInputArguments.end(), std::back_inserter(args));

    // audio input config
    args.push_back("-y");
    args.push_back("-vn");
    args.push_back("-r " + std::to_string(m_sampleRate));
    args.push_back("-f f32le");
    //args.push_back("-acodec aac");
    args.push_back("-ac 1");
    args.push_back("-i -");


    // audio export file config
    args.push_back("-acodec " + m_AudioCodec);
    args.push_back("-f mp3");
    args.push_back("-ar " + std::to_string(m_sampleRate));
    args.push_back("-ac 1");
    args.push_back("-b:a 320k");
    std::copy(m_AdditionalOutputArguments.begin(), m_AdditionalOutputArguments.end(), std::back_inserter(args));

    args.push_back(m_OutputPath);

    std::string cmd = m_FFmpegPath + " ";
    for (auto arg : args) {
        cmd += arg + " ";
    }

#if defined(_WIN32)
    m_CustomRecordingFile = _popen(cmd.c_str(), "wb");
#else
    m_CustomRecordingFile = popen( cmd.c_str(), "w" );
#endif // _WIN32

    return true;
}

bool ofxFFmpegRecorder::startCustomStreaming()
{
    if (isRecording()) {
        LOG_ERROR("A recording is already in proggress.");
        return false;
    }

    m_AddedVideoFrames = 0;
    m_AddedAudioFrames = 0;

    std::vector<std::string> args;
    std::copy(m_AdditionalInputArguments.begin(), m_AdditionalInputArguments.end(), std::back_inserter(args));

    args.push_back("-framerate " + std::to_string(m_Fps));
    args.push_back("-s " + std::to_string(static_cast<unsigned int>(m_VideoSize.x)) + "x" + std::to_string(static_cast<unsigned int>(m_VideoSize.y)));
    args.push_back("-f rawvideo");
    args.push_back("-pix_fmt " + mPixFmt);
    args.push_back("-vcodec rawvideo");
    args.push_back("-i -");

    args.push_back("-b:v " + std::to_string(m_BitRate) + "k");
    std::copy(m_AdditionalOutputArguments.begin(), m_AdditionalOutputArguments.end(), std::back_inserter(args));

    args.push_back("-f rtp rtp://127.0.0.1:1234");

    std::string cmd = m_FFmpegPath + " ";
    for (auto arg : args) {
        cmd += arg + " ";
    }

    #if defined(_WIN32)
    m_CustomRecordingFile = _popen(cmd.c_str(), "w");
#else
    m_CustomRecordingFile = popen(cmd.c_str(), "w");
#endif

    return true;

}

size_t ofxFFmpegRecorder::addFrame(const ofPixels &pixels)
{
    if (m_IsPaused) {
        LOG_NOTICE("Recording is paused.");
        return 0;
    }

    if (m_CustomRecordingFile == nullptr) {
        LOG_ERROR("Custom recording is not in proggress. Cannot add the frame.");
        return 0;
    }

    if (pixels.isAllocated() == false) {
        LOG_ERROR("Given pixels is not allocated.");
        return 0;
    }

    size_t written = 0;

    if (m_AddedVideoFrames == 0) {
        m_Thread = std::thread(&ofxFFmpegRecorder::processFrame, this);
        m_RecordStartTime = std::chrono::high_resolution_clock::now();
    }

    HighResClock now = std::chrono::high_resolution_clock::now();
    const float recordedDuration = getRecordedDuration();
    float delta = std::chrono::duration<float>(now - m_RecordStartTime).count() - recordedDuration - m_TotalPauseDuration;
    const float framerate = 1.f / m_Fps;

    while (m_AddedVideoFrames == 0 || delta >= framerate) {
        delta -= framerate;
        m_Frames.produce(new ofPixels(pixels));
        m_AddedVideoFrames++;
    }

    return written;
}

size_t ofxFFmpegRecorder::addBuffer(const ofSoundBuffer &buffer, float afps){
    if (m_IsPaused) {
        LOG_NOTICE("Recording is paused.");
        return 0;
    }

    if (m_CustomRecordingFile == nullptr) {
        LOG_ERROR("Custom recording is not in proggress. Cannot add the frame.");
        return 0;
    }

    if (buffer.getBuffer().size() == 0 ) {
        LOG_ERROR("Given buffer is not allocated.");
        return 0;
    }

    size_t written = 0;

    if (m_AddedAudioFrames == 0) {
        m_Thread = std::thread(&ofxFFmpegRecorder::processBuffer, this);
        m_RecordStartTime = std::chrono::high_resolution_clock::now();
    }

    HighResClock now = std::chrono::high_resolution_clock::now();
    const float recordedDuration = getRecordedAudioDuration(afps);
    float delta = std::chrono::duration<float>(now - m_RecordStartTime).count() - recordedDuration - m_TotalPauseDuration;
    const float framerate = 1.f / m_Fps;

    while (m_AddedAudioFrames == 0 || delta >= framerate) {
        delta -= framerate;
        m_Buffers.produce(new ofSoundBuffer(buffer));
        m_AddedAudioFrames++;
    }

    return written;
}


void ofxFFmpegRecorder::stop(bool _notify)
{
    ofLog()<<"m_CustomRecordingFile "<<m_CustomRecordingFile<<" m_DefaultRecordingFile "<<m_DefaultRecordingFile;
    if (m_CustomRecordingFile) {
        #if defined(_WIN32)
        _pclose(m_CustomRecordingFile);
        #else
        pclose(m_CustomRecordingFile);
        #endif
        m_CustomRecordingFile = nullptr;
        m_AddedVideoFrames = 0;
        m_AddedAudioFrames = 0;
        joinThread();
    }
    else if (m_DefaultRecordingFile) {
        fwrite("q", sizeof(char), 1, m_DefaultRecordingFile);
        #if defined(_WIN32)
        _pclose(m_DefaultRecordingFile);
        #else
        pclose(m_DefaultRecordingFile);
        #endif
        m_DefaultRecordingFile = nullptr;
    }
    initialized = false;
    if(_notify)outputFileComplete();
}

void ofxFFmpegRecorder::cancel()
{
    if (m_CustomRecordingFile) {
        #if defined(_WIN32)
        _pclose(m_CustomRecordingFile);
        #else
        pclose(m_CustomRecordingFile);
        #endif
        m_CustomRecordingFile = nullptr;
        m_AddedVideoFrames = 0;
        m_AddedAudioFrames = 0;
        joinThread();
    }
    else if (m_DefaultRecordingFile) {
        fwrite("q", sizeof(char), 1, m_DefaultRecordingFile);
#if defined(_WIN32)
        _pclose(m_DefaultRecordingFile);
#else
        pclose(m_DefaultRecordingFile);
#endif
        m_DefaultRecordingFile = nullptr;
    }

    ofFile::removeFile(m_OutputPath, false);
    initialized = false;
}

bool ofxFFmpegRecorder::isOverWrite() const
{
    return m_IsOverWrite;
}

void ofxFFmpegRecorder::setOverWrite(bool overwrite)
{
    m_IsOverWrite = overwrite;
}

const std::vector<std::string> &ofxFFmpegRecorder::getAdditionalInputArguments() const
{
    return m_AdditionalInputArguments;
}

void ofxFFmpegRecorder::setAdditionalInputArguments(const std::vector<std::string> &args)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_AdditionalInputArguments = args;
}

void ofxFFmpegRecorder::addAdditionalInputArgument(const std::string &arg)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_AdditionalInputArguments.push_back(arg);
}

void ofxFFmpegRecorder::clearAdditionalInputArguments()
{
    m_AdditionalInputArguments.clear();
}

const std::vector<std::string> &ofxFFmpegRecorder::getAdditionalOutputArguments() const
{
    return m_AdditionalOutputArguments;
}

void ofxFFmpegRecorder::setAdditionalOutputArguments(const std::vector<std::string> &args)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_AdditionalOutputArguments = args;
}

void ofxFFmpegRecorder::addAdditionalOutputArgument(const std::string &arg)
{
    if (isRecording()) {
        LOG_NOTICE("A recording is in proggress. The change will take effect for the next recording session.");
    }

    m_AdditionalOutputArguments.push_back(arg);
}

void ofxFFmpegRecorder::clearAdditionalOutputArguments()
{
    m_AdditionalOutputArguments.clear();
}

void ofxFFmpegRecorder::clearAdditionalArguments()
{
    m_AdditionalInputArguments.clear();
    m_AdditionalOutputArguments.clear();
}

bool ofxFFmpegRecorder::isRecording() const
{
    return m_DefaultRecordingFile != nullptr || m_CustomRecordingFile != nullptr;
}

bool ofxFFmpegRecorder::isRecordingCustom() const
{
    return m_CustomRecordingFile != nullptr;
}

bool ofxFFmpegRecorder::isRecordingDefault() const
{
    return m_DefaultRecordingFile != nullptr;
}


void ofxFFmpegRecorder::saveThumbnail(const unsigned int &hour, const unsigned int &minute, const float &second, const std::string &output, glm::vec2 size,
                                      ofRectangle crop, std::string videoFilePath)
{
    if (videoFilePath.length() == 0) {
        if (isRecording()) {
            LOG_ERROR("Cannot use the default video file because a recording is already in proggress.");
            return;
        }

        videoFilePath = m_OutputPath;

        if (ofFile::doesFileExist(m_OutputPath, false) == false) {
            LOG_ERROR("The video file (" + videoFilePath + " does not exist!");
            return;
        }
    }

    const std::string time = std::to_string(hour) + ":" + std::to_string(minute) + ":" + std::to_string(second);
    std::vector<std::string> args;

    args.push_back("-i " + videoFilePath);
    args.push_back("-ss " + time);
    args.push_back("-vframes 1");

    std::string vfString = "\"";
    if (size.x > 0 && size.y > 0) {
        vfString += "scale=" + std::to_string(size.x) + ":" + std::to_string(size.y);
    }

    if (crop.isZero() == false) {
        // Decide if a comma is required.
        if (vfString.length() > 1) {
            vfString += ",";
        }

        vfString += "crop=" + std::to_string(crop.getWidth()) + ":" + std::to_string(crop.getHeight());
        if (crop.getX() > 0 && crop.getY() > 0) {
            vfString += ":" + std::to_string(crop.getX()) + ":" + std::to_string(crop.getY());
        }
    }

    vfString += "\"";

    if (vfString.length() > 2) {
        args.push_back("-vf " + vfString);
    }

    args.push_back(output);

    std::string cmd = m_FFmpegPath + " ";
    for (auto arg : args) {
        cmd += arg + " ";
    }
    
#if defined(_WIN32)
    FILE *file = _popen(cmd.c_str(), "w");
    _pclose(file);
#else
    FILE *file = popen(cmd.c_str(), "w");
    pclose(file);
#endif
    
}

void ofxFFmpegRecorder::determineDefaultDevices()
{
    ofLog()<<"determineDefaultDevices()";
    
    if (m_IsRecordVideo && m_DefaultVideoDevice.deviceName.length() == 0) {
        ofVideoGrabber videoGrabber;
        const std::vector<ofVideoDevice> devices = videoGrabber.listDevices();
        int temp_index = 0;
        for (const ofVideoDevice &device : devices) {
            ofLog()<<device.deviceName;
            if (device.bAvailable) {
                m_DefaultVideoDevice = device;
                m_DefaultVideoDeviceIndex = temp_index;
                break;
            }
            temp_index++;
        }
        videoGrabber.close();
    }

    if (m_IsRecordAudio && m_DefaultAudioDevice.isDefaultInput == false) {
        ofSoundStream soundStream;
        const std::vector<ofSoundDevice> audoDevices = soundStream.getDeviceList();
        int temp_index = 0;
        for (const ofSoundDevice &device : audoDevices) {
            ofLog()<<device.name;
            if (device.isDefaultInput) {
                m_DefaultAudioDevice = device;
                m_DefaultAudioDeviceIndex = temp_index;
                break;
            }
            temp_index++;
        }
        soundStream.close();
    }
}

void ofxFFmpegRecorder::processFrame()
{
 
    while (isRecording()) {
        ofPixels *pixels = nullptr;
        if (m_Frames.consume(pixels) && pixels) {
            const unsigned char *data = pixels->getData();
            const size_t dataLength = m_VideoSize.x * m_VideoSize.y * pixChannelCount;
            const size_t written = fwrite(data, sizeof(char), dataLength, m_CustomRecordingFile);
            if (written <= 0) {
                LOG_WARNING("Cannot write the frame.");
            }

            pixels->clear();
            delete pixels;
        }
    }
}

void ofxFFmpegRecorder::processBuffer()
{
    while (isRecording()) {
        ofSoundBuffer *buffer = nullptr;
        if (m_Buffers.consume(buffer) && buffer) {
            //const float *data = buffer->getBuffer().data();
            //const size_t dataLength = buffer->getBuffer().size();
            const size_t written = fwrite(&buffer->getBuffer()[0], sizeof(float), buffer->getBuffer().size(), m_CustomRecordingFile);
            if (written <= 0) {
                LOG_WARNING("Cannot write the buffer.");
            }
            buffer->clear();
            delete buffer;
        }
    }
}

void ofxFFmpegRecorder::joinThread()
{
    if (m_Thread.joinable()) {
        m_Thread.join();
    }
}

bool ofxFFmpegRecorder::joinVideoAudio(std::string _videoFilePath, std::string _audioFilepath){
    
    ofLog()<<"joinVideoAudio()";
    
    //ffmpeg -i video.mp4 -i audio.wav -c copy output.mkv
    
    if (m_OutputPath.length() == 0) {
        LOG_ERROR("Output path is empty. Cannot record.");
        return false;
    }

    if (ofFile::doesFileExist(m_OutputPath, false) && m_IsOverWrite == false) {
        LOG_ERROR("The output file already exists and overwriting is disabled. Cannot capture video.");
        return false;
    }
    
    std::string args = "";
    args += " -y";
    args += " -i " + _videoFilePath;
    args += " -i " + _audioFilepath;
    args += " -c copy "+ m_OutputPath;
    
    std::string cmd = m_FFmpegPath + " ";
    //    for (auto arg : args) {
    //        cmd += arg + " ";
    //    }
    cmd += args + " ";
    ofLog()<<"cmd:"<<cmd;
    
//#if defined(_WIN32)
//    m_DefaultRecordingFile = _popen(cmd.c_str(), "w");
//#else
//    m_DefaultRecordingFile = popen(cmd.c_str(), "w");
//#endif
#if defined(_WIN32)
    m_CustomRecordingFile = _popen(cmd.c_str(), "wb");
#else
//    m_CustomRecordingFile = _popen(cmd.c_str(), "w");
    m_CustomRecordingFile = popen( cmd.c_str(), "w" );
#endif // _WIN32
    
    return true;
}

//--------------------------------------------------------------
void ofxFFmpegRecorder::outputFileComplete()
{
    // at this point all data that ffmpeg wants should have been consumed
    // one of the threads may still be trying to write a frame,
    // but once close() gets called they will exit the non_blocking write loop
    // and hopefully close successfully

//    bIsInitialized = false;
//
//    if (bRecordVideo) {
//        videoThread.close();
//    }
//    if (bRecordAudio) {
//        audioThread.close();
//    }

//    retirePipeNumber(pipeNumber);

//    ffmpegThread.waitForThread();
    // TODO: kill ffmpeg process if its taking too long to close for whatever reason.

    // Notify the listeners.
    RecorderOutputFileCompleteEventArgs args;
    args.fileName = m_OutputPath;
    ofNotifyEvent(outputFileCompleteEvent, args);
}
