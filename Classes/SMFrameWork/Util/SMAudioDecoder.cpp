//
//  SMAudioDecoder.cpp
//  IPCT
//
//  Created by SteveKim on 2020/02/19.
//

#include "SMAudioDecoder.h"
#include "AudioFrame.h"
#include <cocos2d.h>

//#include <SimpleAudioEngine.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"

#include "libavutil/imgutils.h"
#include "libavfilter/avfilter.h"
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)

#else
#endif


const int MAX_AUDIO_DECODE_QUEUE_SIZE = 4;

class AudioDecodeThread : public std::enable_shared_from_this<AudioDecodeThread>
{
public:
    AudioDecodeThread() {}
    virtual ~AudioDecodeThread() {
        CCLOG("THREAD END COMPLETE");
    }
    
    static std::shared_ptr<AudioDecodeThread> execute(std::shared_ptr<SMAudioDecoder> decoder) {
        auto owner = std::make_shared<AudioDecodeThread>();
        
        owner->_decoder = decoder;
        owner->_audioThread = std::thread(&AudioDecodeThread::audioThreadFunc, owner);
        return owner;
    }
    
    void join() {
        if (_audioThread.joinable()) {
            _audioThread.join();
        }
        
    }
    
private:
    void audioThreadFunc() {
        do {
            _decoder->threadForInit();
            if (_decoder->_interrupted) {
                _decoder->release();
                break;
            }
            _decoder->_initialized = true;
            _decoder->waitForPlay();
            if (_decoder->_interrupted) {
                _decoder->release();
                break;
            }
            
            // main play thread...
            
            _decoder->threadForAudioPlayback();
            if (_decoder->_interrupted) {
                _decoder->release();
            }
        } while(0);
        
        CCLOG("THREAD WILL EXI.");
        _decoder = nullptr;
    }


private:
    std::shared_ptr<SMAudioDecoder> _decoder;
    
    std::thread _audioThread;

};


AudioCodecProtocol::AudioCodecProtocol() : _decoder(nullptr)
{
    
}

AudioCodecProtocol::~AudioCodecProtocol()
{
    _decoder->destroy();
    _decoder = nullptr;
//    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
}

bool AudioCodecProtocol::initAudioDecoder(const std::string& mediaName)
{
    auto decoder = SMAudioDecoder::create(this, mediaName);
    
    bool ret = (decoder != nullptr);
    
    _decoder = decoder;
    
    return ret;
}

void AudioCodecProtocol::playAudio()
{
    if (_decoder)
        _decoder->playAudio();
}

void AudioCodecProtocol::stopAudio()
{
    if (_decoder)
        _decoder->stopAudio();
}

AudioFrame* AudioCodecProtocol::getAudioFrame()
{
    if (_decoder)
        return _decoder->dequeueDecodedAudioFrame();
    else
        return nullptr;
}

void AudioCodecProtocol::recycleAudioFrame(AudioFrame* frame)
{
    if (_decoder) {
        if (frame) {
            _decoder->queueIdleAudioFrame(frame);
        }
    }
}




//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Audio Decoder Host
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
std::shared_ptr<SMAudioDecoder> SMAudioDecoder::create(AudioCodecProtocol* protocol, const std::string& mediaName)
{
    
    std::shared_ptr<SMAudioDecoder> decoder = std::make_shared<SMAudioDecoder>();
    decoder->_sharedThis = decoder;
    
    if (decoder->init(protocol, mediaName)) {
        return decoder;
    } else {
        decoder.reset();
    }
    
    return nullptr;
}

SMAudioDecoder::SMAudioDecoder()  : _interrupted(false)
, _initialized(false)
, _playing(true)
, _formatContext(nullptr)
, _avPacket(nullptr)
, _audioStreamIndex(-1)
, _audioCodecContext(nullptr)
, _avFrame(nullptr)
, _audioMaxFrameCount(0)
, _swrContext(nullptr)
{
}

SMAudioDecoder::~SMAudioDecoder()
{
    CCLOG("AUDIO DEOCDER DESTRUCTOR.");
}

void SMAudioDecoder::destroy()
{
    CCLOG("SIGNAL AUDIO END.");
    
    _client = nullptr;
    
    {
        _interrupted = true;
        _playCond.notify_all();
    }
    
    std::thread t([&]() {
        _audioDecodeThread->join();
        _sharedThis = nullptr;
    });
    t.detach();
}

bool SMAudioDecoder::init(AudioCodecProtocol* protocol, const std::string& mediaName)
{
    if (protocol == nullptr || mediaName.empty())
        return false;
    
    _client = protocol;
    _mediaName = mediaName;
    _audioDecodeThread = AudioDecodeThread::execute(_sharedThis);

    return true;
}

void SMAudioDecoder::release()
{
    if (_swrContext) {
        swr_close(_swrContext);
        swr_free(&_swrContext);
        _swrContext = nullptr;
    }
    
    if (_avFrame) {
        av_frame_free(&_avFrame);
        _avFrame = nullptr;
    }

    _audioStreamIndex = -1;
    if (_audioCodecContext) {
        avcodec_flush_buffers(_audioCodecContext);
        avcodec_close(_audioCodecContext);
        _audioCodecContext = nullptr;
    }
    
    if (_avPacket) {
//        av_free_packet(_avPacket);
        av_packet_unref(_avPacket);
        _avPacket = nullptr;
    }

    if (_formatContext) {
        avformat_close_input(&_formatContext);
        _formatContext = nullptr;
    }
        
    while (!_idleAudioQueue.empty()) {
        auto frame = _idleAudioQueue.front();
        _idleAudioQueue.pop();
        delete frame;
    }
    
    while (!_decodedAudioQueue.empty()) {
        auto frame = _decodedAudioQueue.front();
        _decodedAudioQueue.pop();
        delete frame;
    }
    

    CCLOG("RELEASE AUDIO");
}

void SMAudioDecoder::playAudio()
{
    _playing = true;
    if (_initialized) {
        _playCond.notify_all();
    }
}

void SMAudioDecoder::stopAudio()
{
    _playing = false;
    if (_initialized) {
        _playCond.notify_all();
    }
}

void SMAudioDecoder::onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime)
{
    _samples = samples;
    _bit_rate = rate;
    _fmt = fmt;

    if (success) {
        if (_client) {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
                if (_client) {
                    _client->onAudioInitResult(true, samples, rate, fmt, durationTime);
                }
            });
        }
    } else {
        if (_client) {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
                if (_client) {
                    _client->onAudioInitResult(false, 0, 0, 0, 0);
                }
            });
        }
    }
}


#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)

static int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt)
{
    int ret;

    *got_frame = 0;

    if (pkt) {
        ret = avcodec_send_packet(avctx, pkt);
        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.
        if (ret < 0)
            return ret == AVERROR_EOF ? 0 : ret;
    }

    ret = avcodec_receive_frame(avctx, frame);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return ret;
    if (ret >= 0)
        *got_frame = 1;
    
    return 0;
}

int SMAudioDecoder::decodeAudioFrame(AudioFrame** audioFrame)
{
    if (_formatContext == nullptr || _avPacket == nullptr)
        return -1;
    
    int ret = 0;    // 0 : SUCCESS
    _avPacket->buf = nullptr;
    
    // 1. read AVFrame from AVPacket
    // 2. decode AVFrame
    // 3. queue decoded bufferd
    
    // first loop - av_read_frame
    
    while ((ret = av_read_frame(_formatContext, _avPacket)) == 0) {
        if (_avPacket->stream_index == _audioStreamIndex) {
            int data_size = 0;
            int used = avcodec_send_packet(_audioCodecContext, _avPacket);
            if (used < 0 && used != AVERROR(EAGAIN) && used != AVERROR_EOF) {
                CCLOG("AUDIO Decode Error.(1)");
                return -1;
            }
            while (ret  >= 0) {
                ret = avcodec_receive_frame(_audioCodecContext, _avFrame);
                if (ret==AVERROR(EAGAIN) || ret==AVERROR_EOF) {
                    return ret;
                } else if (ret<0) {
                    CCLOG("avcodec_receive_frame for audio Error. : %d", ret);
                    return ret;
                }

                // decode audio
                data_size = av_get_bytes_per_sample(_audioCodecContext->sample_fmt);
                SwrContext* pAudioCvtContext;
                pAudioCvtContext = swr_alloc_set_opts(NULL, _audioCodecContext->channel_layout, AV_SAMPLE_FMT_S16, _audioCodecContext->sample_rate, _audioCodecContext->channel_layout, _audioCodecContext->sample_fmt, _audioCodecContext->sample_rate, 0, 0);
                
                swr_init(pAudioCvtContext);
                
                AudioFrame* decodeFrame = nullptr;
                if (audioFrame) {
                    decodeFrame = *audioFrame;
                }
                if (decodeFrame == nullptr) {
                    if (_audioMaxFrameCount >= MAX_AUDIO_DECODE_QUEUE_SIZE) {
                        continue;
                    }

                    decodeFrame = new AudioFrame(_avFrame);
                    _audioMaxFrameCount++;
                }

                decodeFrame->setData(_avFrame, pAudioCvtContext);
                queueDecodedAudioFrame(decodeFrame);

                swr_free(&pAudioCvtContext);
                av_frame_unref(_avFrame);

                if (audioFrame) {
                    *audioFrame = nullptr;
                }

                av_packet_unref(_avPacket);
                _avPacket->buf = nullptr;

                return 0;
            } // while (ret  >= 0) {
        } // if (_avPacket->stream_index == _audioStreamIndex) {
    } // while ((ret = av_read_frame(_formatContext, _avPacket)) == 0) {
    
    
    if (ret<0) {
        CCLOG("end of Audio");
        return 1;
    }
    
    return 0;
}

void SMAudioDecoder::threadForInit()
{
    std::unique_lock<std::mutex> lock(_audioMutex);
    
    do {
        if (_interrupted)
            break;
        
        int ret;
        ret = avformat_open_input(&_formatContext, _mediaName.c_str(), nullptr, nullptr);
        if (ret != 0) {
            // 영상 오픈 실패
            char errBuf[1024];
            av_strerror(ret, errBuf, 1024);
            CCLOG("Couldn't open input stream. : %s", errBuf);
            break;
        }
        
        if (_interrupted)
            break;
        
        ret = avformat_find_stream_info(_formatContext, nullptr);
        if (ret < 0 ) {
            // 깨진 혹은 알수없는 영상
            CCLOG("Couldn't find stream information.");
            break;
        }
        if (_interrupted)
            break;

        
        int channels = 0;
        int samples = 0;
        int bit_rate = 0;
        int fmt = 0;
        
        // get audio stream index
        _audioStreamIndex = -1;
        for (int i=0; i<_formatContext->nb_streams && !_interrupted; i++) {
            if (_formatContext->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
                if (_audioStreamIndex==-1) {
                    _audioStreamIndex=i;
                    break;
                }
            }
        }
        
        if (_audioStreamIndex==-1) {
            CCLOG("Didn't find a audio stream... but not break at this time.");
        }
        if (_interrupted) {
            break;
        }

        const AVCodec* audioCodec = avcodec_find_decoder(_formatContext->streams[_audioStreamIndex]->codecpar->codec_id);
        if (!audioCodec) {
            CCLOG("Audio Codec not found.(2)... but not break at this time");
        }
        _audioCodecContext = avcodec_alloc_context3(audioCodec);
        
        if (_interrupted) {
            break;
        }
        
        avcodec_parameters_to_context(_audioCodecContext, _formatContext->streams[_audioStreamIndex]->codecpar);
        
        ret = avcodec_open2(_audioCodecContext, audioCodec, nullptr);
        if (ret < 0){
            CCLOG("Could not open audio codec.");
            break;
        }
        if (_interrupted)
            break;
        
        // now get swrcontext
//        _swrContext = swr_alloc_set_opts(NULL, _audioCodecContext->channel_layout, AV_SAMPLE_FMT_S16, _audioCodecContext->sample_rate,
//                                         _audioCodecContext->channel_layout, _audioCodecContext->sample_fmt,
//                                         _audioCodecContext->sample_rate, 0, NULL);
//        swr_init(_swrContext);
        
        _avFrame = av_frame_alloc();
        if (_avFrame==nullptr) {
            break;
        }
        
        if (_interrupted) {
            break;
        }
        
        int cbSize = 0;
        int wFormatTag = 1;
        int sample_per_sec = _audioCodecContext->sample_rate;
        int bit_per_sample = _audioCodecContext->bits_per_coded_sample;
        int channel = _audioCodecContext->channels;
        int block_align = channel * (bit_per_sample/8);
        int avg_byte_per_sec = block_align * sample_per_sec;
        
        
        int buffer_size = _audioCodecContext->frame_size * block_align;
        channels = _audioCodecContext->channel_layout;
        samples = AV_SAMPLE_FMT_S16;
        bit_rate = _audioCodecContext->sample_rate;
        fmt = _audioCodecContext->sample_fmt;
        
        
        _avPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
        
#ifdef _DEBUG_DUMP_FORMAT
        av_dump_format(_formatContext, 0, _mediaName.c_str(), 0);
#endif
        if (_interrupted)
            break;
        
        _playTime = 0;
        
        double durationTime = (float)_formatContext->duration / AV_TIME_BASE;
        
//        _swsContext = sws_getContext(_videoCodecContext->width, _videoCodecContext->height,
//                                     _videoCodecContext->pix_fmt,
//                                     _videoCodecContext->width, _videoCodecContext->height,
//                                     AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
//
//        if (!_swsContext) {
//            CCLOG("SWS Context create failed.");
//        }
                                    
        

        // 첫번째 frame 디코드 후 버림.
        if (decodeAudioFrame() != 0) {
            break;
        }
        // 두번째 프레임 전달.
        AudioFrame * audioFrame = nullptr;
        audioFrame = dequeueDecodedAudioFrame();
        if (decodeAudioFrame(&audioFrame) != 0) {
            break;
        }
        CCLOG("Audio initialize complete!");
        
        // 성공
        onAudioInitResult(true, samples, bit_rate, fmt, durationTime);
        return;
    } while (0);
    
    // 실패
    release();
    onAudioInitResult(false, 0, 0, 0, 0);
}


#else
// for android
int SMAudioDecoder::decodeAudioFrame(AudioFrame** audioFrame) {
    if (_formatContext == nullptr || _avPacket == nullptr)
        return -1;
    
    int ret, got_picture, got_sound;
    _avPacket->buf = nullptr;
    
    while ((ret = av_read_frame(_formatContext, _avPacket)) == 0) {
        if (_avPacket->stream_index == _audioStreamIndex) {
            got_picture = 0;
            ret = avcodec_decode_video2(_videoCodecContext, _videoFrame, &got_picture, _avPacket);
            if (ret < 0) {
                CCLOG("Decode Error.(1)");
                return -1;
            }
            
            if (got_picture) {
                /** queue buffer **/
                
                int64_t timeStamp = av_frame_get_best_effort_timestamp(_videoFrame);
                AVStream* stream = _formatContext->streams[_videoStreamIndex];
                double decodeTime = (timeStamp - stream->start_time) * av_q2d(stream->time_base);
                
                if (decodeTime < _playTime) {
                    _playTime = decodeTime;
                    continue;
                }
                
                AudioFrame* decodeFrame = nullptr;
                
                if (frame) {
                    decodeFrame = *frame;
                }
                if (decodeFrame == nullptr) {
                    if (_videoMaxFrameCount >= MAX_VIDEO_DECODE_QUEUE_SIZE)
                        continue; // skipping
                    
                    decodeFrame = new AudioFrame(_videoFrame);
                    _videoMaxFrameCount++;
                }
                
                decodeFrame->setData(_videoFrame);
                queueDecodedVideoFrame(decodeFrame);
                
                if (frame) {
                    *frame = nullptr;
                }
                
                return 0;
            }
        } else if (_avPacket->stream_index == _videoStreamIndex) {
            got_sound = 0;
        }
        
        _avPacket->buf = nullptr;
    }
    if (ret < 0) {
        CCLOG("end of video");
        return 1;
    }
    
    return ret;
}

void SMAudioDecoder::threadForInit() {
    std::unique_lock<std::mutex> lock(_audioMutex);
    
    do {
        if (_interrupted)
            break;
        
        int ret = 0;
        ret = avformat_open_input(&_formatContext, _mediaName.c_str(), nullptr, nullptr);
        if (ret != 0) {
            // 영상 오픈 실패
            char errBuf[1024];
            av_strerror(ret, errBuf, 1024);
            CCLOG("Couldn't open input stream. : %s", errBuf);
            break;
        }
        
        if (_interrupted)
            break;
        
        ret = avformat_find_stream_info(_formatContext, nullptr);
        if (ret < 0 ) {
            // 깨진 혹은 알수없는 영상
            CCLOG("Couldn't find stream information.");
            break;
        }
        if (_interrupted)
            break;
        
        _videoStreamIndex = -1;
        for (int i = 0; i < _formatContext->nb_streams && !_interrupted; i++) {
            if (_formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                _videoStreamIndex = i;
                break;
            }
        }
        if (_videoStreamIndex == -1){
            CCLOG("Didn't find a video stream.");
            break;
        }
        if (_interrupted)
            break;
        
        _videoCodecContext = _formatContext->streams[_videoStreamIndex]->codec;
        if (!_videoCodecContext) {
            CCLOG("Codec not found.(1)");
            break;
        }
        
        const AVCodec* codec = avcodec_find_decoder(_videoCodecContext->codec_id);
        if (!codec) {
            CCLOG("Codec not found.(2)");
            break;
        }
        if (_interrupted)
            break;
        
        ret = avcodec_open2(_videoCodecContext, codec, nullptr);
        if (ret < 0){
            CCLOG("Could not open codec.");
            break;
        }
        if (_interrupted)
            break;
        
        int width = _videoCodecContext->width;
        int height = _videoCodecContext->height;
        double frameRate = av_q2d(_formatContext->streams[_videoStreamIndex]->avg_frame_rate);
        double durationTime = (float)_formatContext->duration / AV_TIME_BASE;
        
        if (width <= 0 || height <= 0 || frameRate < 1.0 || frameRate > 100.0) {
            // 잘못된 비디오 정보
            CCLOG("Wrong video information.");
            break;
        }
        if (_interrupted)
            break;
        
        _videoFrame = av_frame_alloc();
        if (_videoFrame == nullptr)
            break;
        
        if (_interrupted)
            break;
        
        uint8_t* buf  = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, _videoCodecContext->width, _videoCodecContext->height));
        ret = avpicture_fill((AVPicture *)_videoFrame, buf, AV_PIX_FMT_YUV420P, _videoCodecContext->width, _videoCodecContext->height);
        if (ret < 0) {
            CCLOG("avpicture_fill failed.");
            break;
        }
        if (_interrupted)
            break;
        
        _avPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
        
#ifdef _DEBUG_DUMP_FORMAT
        av_dump_format(_formatContext, 0, _mediaName.c_str(), 0);
#endif
        if (_interrupted)
            break;
        
        _playTime = 0;
                
        
        // 첫번째 frame 디코드 후 버림.
        if (decodeAudioFrame() != 0) {
            break;
        }
        // 두번째 프레임 전달.
        auto videoFrame = dequeueDecodedVideoFrame();
        if (decodeAudioFrame(&videoFrame) != 0) {
            break;
        }
        CCLOG("Video initialize complete!");
        
        // 성공
        onVideoInitResult(true, width, height, frameRate, durationTime);
        return;
    } while (0);
    
    // 실패
    release();
    onVideoInitResult(false, 0, 0, 0, 0);
}
#endif

void SMAudioDecoder::waitForPlay()
{
    std::unique_lock<std::mutex> lock(_audioMutex);
    _playCond.wait(lock);
}

void SMAudioDecoder::threadForAudioPlayback()
{
    std::unique_lock<std::mutex> lock(_audioMutex);
    
    AudioFrame* audioFrame = nullptr;

    while (!_interrupted) {
        audioFrame = dequeueIdleAudioFrame();
        if (audioFrame) {
            int ret = decodeAudioFrame(&audioFrame);
            if (audioFrame) {
                queueIdleAudioFrame(audioFrame);
            }
            if (ret < 0) {
                // error
                break;
            } else if (ret == 1) {
                // end of Audio
                
                avcodec_flush_buffers(_audioCodecContext);
                
                _playTime = 0;
                
                av_seek_frame(_formatContext, _audioStreamIndex, 0, 0);
                continue;
            }
            
            /** queue buffer **/
            continue;
        } else {
            // 빈 프레임 없음
            if (_interrupted)
                break;
            
            if (!_playing) {
                _playCond.wait(lock);
            }
            
            if (_interrupted)
                break;
        }
    }
    CCLOG("AUDIO PLAYBACK END");
}

AudioFrame* SMAudioDecoder::dequeueDecodedAudioFrame()
{
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    AudioFrame* frame = nullptr;
    
    if (!_decodedAudioQueue.empty()) {
        frame = _decodedAudioQueue.front();
        _decodedAudioQueue.pop();
    }
    
    if (_initialized) {
        _playCond.notify_all();
    }
    
    return frame;
}

void SMAudioDecoder::queueDecodedAudioFrame(AudioFrame* frame)
{
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    _decodedAudioQueue.push(frame);
}

AudioFrame* SMAudioDecoder::dequeueIdleAudioFrame()
{
    AudioFrame* frame = nullptr;
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    
    if (!_idleAudioQueue.empty()) {
        frame = _idleAudioQueue.front();
        _idleAudioQueue.pop();
    }
    
    return frame;
}

void SMAudioDecoder::queueIdleAudioFrame(AudioFrame* frame)
{
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    _idleAudioQueue.push(frame);
}
