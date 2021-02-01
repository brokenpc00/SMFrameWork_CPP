//
//  SMAudioPlayer.cpp
//  IPCT
//
//  Created by SteveKim on 2020/02/21.
//



#include "SMAudioPlayer.h"
#include "SMAudioDecoder.h"
#include "AudioFrame.h"

#define alCall(funcCall) {funcCall; SMAudioPlayer::CheckError(__FILE__, __LINE__, #funcCall) ? abort() : ((void)0); }


unsigned int __idIndex = 0;

SMAudioPlayer::SMAudioPlayer() : _updateDeltaTime(0.033f)
, _streamingSource(false)
, _fmt(0)
, _channels(0)
, _bit_rate(0)
, _samples(0)
, _isDestroyed(false)
, _loop(false)
, _id(++__idIndex)
, _device(nullptr)
, _bufferID(0)
{
    memset(_bufferIds, 0, sizeof(_bufferIds));

    _device = alcOpenDevice(NULL);
    if (!_device) {
        CCLOG("[[[[[ OpenAL Device open error");
    }

    _enumeration= alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");

    if (_enumeration == AL_FALSE) {
        CCLOG("[[[[[ enumeration not supported");
    } else {
        CCLOG("[[[[[ enumeration supported");
    }

    listingAudioDevice(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
}

SMAudioPlayer::~SMAudioPlayer()
{
    CCLOG("asdf");
    destroy();
    
    if (_streamingSource) {
        alDeleteBuffers(QUEUE_BUFFER_NUM, _bufferIds);
    }
}

void SMAudioPlayer::listingAudioDevice(const ALCchar* devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;

    CCLOG("[[[[[ Devices list : ");
    CCLOG("----------");
    while (device && *device != '\0' && next && *next != '\0') {
        CCLOG("[[[[[ %s", device);
            len = strlen(device);
            device += (len + 1);
            next += (len + 2);
    }
    CCLOG("----------");
}

void SMAudioPlayer::destroy()
{
    if (_isDestroyed) {
        return;
    }
    
    _isDestroyed = true;
    do {
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
        ALint sourceState;
        ALint bufferProcessed = 0;
        alGetSourcei(_alSourceID, AL_SOURCE_STATE, &sourceState);
        if (sourceState==AL_PLAYING) {
            alGetSourcei(_alSourceID, AL_BUFFERS_PROCESSED, &bufferProcessed);
            while (bufferProcessed < QUEUE_BUFFER_NUM) {
                std::this_thread::sleep_for(std::chrono::microseconds(2));
                alGetSourcei(_alSourceID, AL_BUFFERS_PROCESSED, &bufferProcessed);
            }
            alSourceUnqueueBuffers(_alSourceID, QUEUE_BUFFER_NUM, _bufferIds);
        }
#endif
    } while(false);
    
    alSourceStop(_alSourceID);
    alSourcei(_alSourceID, AL_BUFFER, 0);
    
    
    _ready = false;
}

bool SMAudioPlayer::initAudio(std::string audioUrl)
{
    ALCenum error;
    _context = alcCreateContext(_device, NULL);
    if (!alcMakeContextCurrent(_context)) {
        error = alGetError();
        CCLOG("[[[[[ context make failed");
    }
    
    
    alCall(alGenSources(1, &_alSourceID));
    alCall(alSpeedOfSound(1.0f));
    alCall(alDopplerVelocity(1.0f));
    alCall(alDopplerFactor(1.0f));
    alCall(alSourcef(_alSourceID, AL_PITCH, 1.0f));
    alCall(alSourcef(_alSourceID, AL_GAIN, 1.0f));
    alCall(alSourcei(_alSourceID, AL_LOOPING, AL_FALSE));
//    alCall(alSourcef(_alSourceID, AL_SOURCE_TYPE, AL_STREAMING));

    
    
    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

    alListener3f(AL_POSITION, 0, 0, 1.0f);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_POSITION %x", error);
    }
    alListener3f(AL_VELOCITY, 0, 0, 0);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_VELOCITY %x", error);
    }
    alListenerfv(AL_ORIENTATION, listenerOri);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_ORIENTATION %x", error);
    }

    alGenSources((ALuint)1, &_alSourceID);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error Gen source %x", error);
    }

    alSourcef(_alSourceID, AL_PITCH, 1);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_PITCH %x", error);
    }
    alSourcef(_alSourceID, AL_GAIN, 1);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_GAIN %x", error);
    }
    alSource3f(_alSourceID, AL_POSITION, 0, 0, 0);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_POSITION %x", error);
    }
    alSource3f(_alSourceID, AL_VELOCITY, 0, 0, 0);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_VELOCITY %x", error);
    }
    alSourcei(_alSourceID, AL_LOOPING, AL_FALSE);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error AL_LOOPING %x", error);
    }

//    alGenBuffers((ALuint)1, &_bufferID);
//    error = alGetError();
//    if (error != AL_NO_ERROR) {
//        CCLOG("[[[[[ Error Gen Buffers %x", error);
//    }
//
//    alSourcei(_alSourceID, AL_BUFFER, _bufferID);
//    error = alGetError();
//    if (error != AL_NO_ERROR) {
//        CCLOG("[[[[[ Error binding Buffers %x", error);
//    }

//    alGenBuffers(QUEUE_BUFFER_NUM, _bufferIds);
//    error = alGetError();
//    if (error != AL_NO_ERROR) {
//        CCLOG("[[[[[ Error Gen Buffers %x", error);
//    }
    
//    for (int i=0; i<QUEUE_BUFFER_NUM; i++) {
//        alSourcei(_alSourceID, AL_BUFFER, _bufferIds[i]);
//        error = alGetError();
//        if (error != AL_NO_ERROR) {
//            CCLOG("[[[[[ Error binding Buffers %x", error);
//        }
//    }
    
//    std::string soundFile = cocos2d::FileUtils::getInstance()->fullPathForFilename("sound/effect1.wav");
//    if (!cocos2d::FileUtils::getInstance()->isFileExist(soundFile)) {
//        CCLOG("[[[[[ not exist sound file");
//    }
//
    
//    CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect(soundFile.c_str());
//    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(soundFile.c_str(), true);
    
    initAudioDecoder(audioUrl);
    
    return true;
}

void SMAudioPlayer::startAudioFn()
{
//    alSourcePlay(_alSourceID);
    
//    alGetSourcei(_alSourceID, AL_SOURCE_STATE, &_source_state);
//    ALCenum error = alGetError();
//    if (error != AL_NO_ERROR) {
//        CCLOG("[[[[[ Error AL_SOURCE_STATE %x", error);
//    }
    
    if (!isScheduled(schedule_selector(SMAudioPlayer::audioBufferPeek))) {
        // decoder start
        playAudio();
        // play timer
        schedule(schedule_selector(SMAudioPlayer::audioBufferPeek), _updateDeltaTime);
    }
}

void SMAudioPlayer::stopAudioFn()
{
    alSourceStop(_alSourceID);
    
    if (isScheduled(schedule_selector(SMAudioPlayer::audioBufferPeek))) {
        // decoder stop
        stopAudio();
        // stop timer
        unschedule(schedule_selector(SMAudioPlayer::audioBufferPeek));
    }

    
}

//void SMAudioPlayer::setData(uint8_t* buf, int size)
//{
//
//}

void SMAudioPlayer::clearAudio()
{
    alDeleteSources(1, &_alSourceID);
    alDeleteBuffers(1, &_bufferID);
    for (int i=1; i<=QUEUE_BUFFER_NUM; i++) {
        alDeleteBuffers(i, &_bufferIds[i]);
    }
    _device = alcGetContextsDevice(_context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(_context);
    alcCloseDevice(_device);
}

void SMAudioPlayer::onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime)
{
    if (success) {
        _samples=samples;_bit_rate=rate;_fmt=fmt;
        
        startAudioFn();
    }
}

ALenum SMAudioPlayer::getFormat(short channels, short bitsPerChannel)
{
    
//    return channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    bool stereo = (channels > 1);

    switch (bitsPerChannel) {
    case 16:
            return stereo ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    case 8:
            return stereo ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
    default: return -1;
    }
}

void SMAudioPlayer::openAudioFromQueue(uint8_t *buf, int size)
{
    std::lock_guard<std::mutex> guard(_audioQueueMutex);

    updateQueueBuffer();
    
    ALuint bufferID = 0;
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, _format, buf, size, _bit_rate);
    alSourceQueueBuffers(_alSourceID, 1, &bufferID);
    ALint state;
    alGetSourcei(_alSourceID, AL_SOURCE_STATE, &state);
    if (state!=AL_PLAYING) {
        alSourcePlay(_alSourceID);
    }
}

bool SMAudioPlayer::updateQueueBuffer()
{
    ALint state;
    int processed, queued;
    
    alGetSourcei(_alSourceID, AL_SOURCE_STATE, &state);
    
    if (state==AL_STOPPED) {
        return false;
    }
    
    alGetSourcei(_alSourceID, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(_alSourceID, AL_BUFFERS_QUEUED, &queued);
    
    while (processed--) {
        alSourceUnqueueBuffers(_alSourceID, 1, &_bufferID);
        alDeleteBuffers(1, &_bufferID);
    }
    
    return true;

}

void SMAudioPlayer::audioBufferPeek(float dt)
{
    // get audio data... & play
    auto frame = getAudioFrame();
    if (frame == nullptr) {
        // 디코딩된 프레임 없음
        return;
    }
    ALCenum error;

    uint8_t * audio_buf = frame->getAudioBuffer();
    int audio_buf_size = frame->getAudioSize();
    _format = getFormat(frame->getChannels(), frame->getBitsPerChannel());
    _bit_rate = frame->getSampleRate();
    
    openAudioFromQueue(audio_buf, audio_buf_size);
    
    recycleAudioFrame(frame);
    return;

    if (0) {
        EnqueueBuffer(audio_buf, audio_buf_size, frame->getBitsPerChannel());
        
    } else {
        int numBuffedsProcessed = 0;
//        alGetSourcei(_alSourceID, AL_BUFFERS_PROCESSED, &numBuffedsProcessed);
//        if (numBuffedsProcessed<=0) {
//            recycleAudioFrame(frame);
//            return;
////        }
//
//        static bool once = false;
//        if (!once) {
//            once = true;
//
//            for (int i=0; i<QUEUE_BUFFER_NUM; i++) {
//                alCall(alBufferData(_bufferIds[i], _format, audio_buf, audio_buf_size*frame->getFormat()/8, frame->getSampleRate()));
//            }
//
//            alCall(alSourceQueueBuffers(_alSourceID, QUEUE_BUFFER_NUM, _bufferIds));
//            alCall(alSourcePlay(_alSourceID));
//        }
//
//        alCall(alGetSourcei(_alSourceID, AL_BUFFERS_PROCESSED, &numBuffedsProcessed));
//        if (numBuffedsProcessed>=0) {
//            recycleAudioFrame(frame);
//            return;
//        }
        
        static bool once = false;
        if (!once) {
            once = true;
            alCall(alSourceUnqueueBuffers(_alSourceID, 1, &_bufferID));
            alCall(alBufferData(_bufferID, _format, audio_buf, audio_buf_size*frame->getFormat()/8, frame->getSampleRate()));
            alCall(alSourceQueueBuffers(_alSourceID, 1, &_bufferID));
            alCall(alSourcePlay(_alSourceID));
        }
        
        
        alCall(alGetSourcei(_alSourceID, AL_BUFFERS_PROCESSED, &numBuffedsProcessed));
        if (numBuffedsProcessed>=0) {
            recycleAudioFrame(frame);
            return;
        }
        alCall(alSourceUnqueueBuffers(_alSourceID, 1, &_bufferID));
        alCall(alBufferData(_bufferID, _format, audio_buf, audio_buf_size*frame->getFormat()/8, frame->getSampleRate()));
        alCall(alSourceQueueBuffers(_alSourceID, 1, &_bufferID));
        ALint  playStatus;
        alCall(alGetSourcei(_alSourceID,  AL_SOURCE_STATE,  &playStatus));
        if(playStatus!=  AL_PLAYING) {
            alCall(alSourcePlay(_alSourceID));
        }

    }
    recycleAudioFrame(frame);
    return;

//    alCall(alSourceUnqueueBuffers(_alSourceID, 1, &_bufferID));
//    ALCenum error = alGetError();
//    if (error != AL_NO_ERROR) {
//        CCLOG("[[[[[ Error alBufferData : %s", openALErrorString(error).c_str());
//    }

    
    // format 4352
    // freq 11025s
    int freq = frame->getSampleRate();
//    alCall(alBufferData(_bufferID, _format, audio_buf, audio_buf_size, freq));
    alBufferData(_bufferID, _format, audio_buf, audio_buf_size*2, freq);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error alBufferData : %s, bufID : %d, format : %d, size : %d, freq : %d", openALErrorString(error).c_str(), _bufferID, _format, audio_buf_size, freq);
    }
    
    alSourcePlay(_alSourceID);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        CCLOG("[[[[[ Error alSourcePlay : %s", openALErrorString(error).c_str());
    }
//    alSourcei(_alSourceID, AL_BUFFER, _bufferID);
//    error = alGetError();
//    if (error != AL_NO_ERROR) {
//        CCLOG("[[[[[ Error Gen Buffers %x", error);
//    }
    
//    alSourcePlay(_alSourceID);
    

//    CCLOG("[[[[[ audio playing >>> size : %d, bitrate : %d, format : %d, samples : %d, channels : %d", audio_buf_size, bit_rate, format, samples, channels);
    
//    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(<#const char *filePath#>)

    recycleAudioFrame(frame);
}

std::string SMAudioPlayer::openALErrorString(int error)
{
    switch (error) {
        case AL_NO_ERROR: return "AL_NO_ERROR";
//        case AL_INVALID_DEVICE: return "AL_INVALID_DEVICE";
//        case AL_INVALID_CONTEXT: return "AL_INVALID_CONTEXT";
        case AL_INVALID_NAME: return "AL_INVALID_NAME";
        case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
        default:
          return "Unknown error code";
    }
}

bool SMAudioPlayer::CheckError(const std::string &pFile, int pLine, const std::string &pfunc)
{
    ALenum tErrCode = alGetError();
    if(tErrCode != 0)
    {
        auto tMsg = alGetString(tErrCode);
        CCLOG("[[[[[ %s at %s (%d) : AL call %s failed.", tMsg, pFile.c_str(), pLine, pfunc.c_str());
        return true;
    }
    return false;
}

void SMAudioPlayer::EnqueueBuffer(const uint8_t * pData, int size, short fmt)
{
    static int called = 0;
    ++called;

    ALint tState;
//    alCall(alGetSourcei(_alSourceID, AL_SOURCE_TYPE, &tState));
//    if(tState == AL_STATIC)
//    {
//        stopAudioFn();
////            Stop();
////      alCall(alSourcei(mSourceId, AL_BUFFER, NULL));
//    }

    ALuint tBufId = AL_NONE;
    int tQueuedBuffers = QueuedUpBuffers();
    int tReady = ProcessedBuffers();
    if(tReady > 0)
    {
        // the fifth time through, this code gets hit
        alCall(alSourceUnqueueBuffers(_alSourceID, 1, &_bufferID));

        // debug code: make sure these values go down by one
        tQueuedBuffers = QueuedUpBuffers();
        tReady = ProcessedBuffers();
    }
    else
    {
        return; // no update needed yet.
    }


    // the fifth time through, we get AL_INVALID_OPERATION, and call abort()
//    alBufferData(_buffer, _format, pData, size, _bit_rate);
    CCLOG("[[[[[ alBufferData : (%d==%d), (%d==%d)", _bufferID, tBufId, _format, _fmt);
    alCall(alBufferData(tBufId, _format, pData, size*fmt/8, _bit_rate));
//    alCall(alBufferData(tBufId, _fmt, pData, size, _bit_rate));

    alCall(alSourceQueueBuffers(_alSourceID, 1, &_bufferID));
}

int SMAudioPlayer::QueuedUpBuffers()
{
    int tCount = 0;
    alCall(alGetSourcei(_alSourceID, AL_BUFFERS_QUEUED, &tCount));
    return tCount;
}

int SMAudioPlayer::ProcessedBuffers()
{
    int tCount = 0;
    alCall(alGetSourcei(_alSourceID, AL_BUFFERS_PROCESSED, &tCount));
    return tCount;
}


/*
 void SoundyOutport::EnqueueBuffer(const float* pData, int pFrames)
 {
     static int called = 0;
     ++called;

     ALint tState;
     alCall(alGetSourcei(mSourceId, AL_SOURCE_TYPE, &tState));
     if(tState == AL_STATIC)
     {
         Stop();
 //      alCall(alSourcei(mSourceId, AL_BUFFER, NULL));
     }

     ALuint tBufId = AL_NONE;
     int tQueuedBuffers = QueuedUpBuffers();
     int tReady = ProcessedBuffers();
     if(tQueuedBuffers < MAX_BUFFER_COUNT)
     {
         tBufId = mBufferIds[tQueuedBuffers];
     }
     else if(tReady > 0)
     {
         // the fifth time through, this code gets hit
         alCall(alSourceUnqueueBuffers(mSourceId, 1, &tBufId));

         // debug code: make sure these values go down by one
         tQueuedBuffers = QueuedUpBuffers();
         tReady = ProcessedBuffers();
     }
     else
     {
         return; // no update needed yet.
     }

     void* tConverted = convert(pData, pFrames);

     // the fifth time through, we get AL_INVALID_OPERATION, and call abort()
     alCall(alBufferData(tBufId, mFormat, tConverted, pFrames * mBitdepth/8, mSampleRate));

     alCall(alSourceQueueBuffers(mSourceId, 1, &mBufferId));
     if(mBitdepth == BITDEPTH_8)
     {
         delete (uint8_t*)tConverted;
     }
     else // if(mBitdepth == BITDEPTH_16)
     {
         delete (uint16_t*)tConverted;
     }
 }

 void SoundyOutport::PlayBufferedStream()
 {
     if(!StreamingMode() || !QueuedUpBuffers())
     {
         Log::w(ro::TAG) << "Attempted to play an unbuffered stream" << end;
         return;
     }

     alCall(alSourcei(mSourceId, AL_LOOPING, AL_FALSE)); // never loop streams
     alCall(alSourcePlay(mSourceId));
 }

 int SoundyOutport::QueuedUpBuffers()
 {
     int tCount = 0;
     alCall(alGetSourcei(mSourceId, AL_BUFFERS_QUEUED, &tCount));
     return tCount;
 }

 int SoundyOutport::ProcessedBuffers()
 {
     int tCount = 0;
     alCall(alGetSourcei(mSourceId, AL_BUFFERS_PROCESSED, &tCount));
     return tCount;
 }

 void SoundyOutport::Stop()
 {
     if(Playing())
     {
         alCall(alSourceStop(mSourceId));
     }

     int tBuffers;
     alCall(alGetSourcei(mSourceId, AL_BUFFERS_QUEUED, &tBuffers));
     if(tBuffers)
     {
         ALuint tDummy[tBuffers];
         alCall(alSourceUnqueueBuffers(mSourceId, tBuffers, tDummy));
     }
     alCall(alSourcei(mSourceId, AL_BUFFER, AL_NONE));
 }

 bool SoundyOutport::Playing()
 {
     ALint tPlaying;
     alCall(alGetSourcei(mSourceId, AL_SOURCE_STATE, &tPlaying));
     return tPlaying == AL_PLAYING;
 }

 bool SoundyOutport::StreamingMode()
 {
     ALint tState;
     alCall(alGetSourcei(mSourceId, AL_SOURCE_TYPE, &tState));
     return tState == AL_STREAMING;
 }

 bool SoundyOutport::StaticMode()
 {
     ALint tState;
     alCall(alGetSourcei(mSourceId, AL_SOURCE_TYPE, &tState));
     return tState == AL_STATIC;
 }
 */
