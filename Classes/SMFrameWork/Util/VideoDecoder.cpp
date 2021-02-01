//
//  VideoDecoder.cpp
//  LiveOn
//
//  Created by SteveKim on 03/07/2019.
//

#include "VideoDecoder.h"
#include "VideoFrame.h"
#include "VideoSprite.h"
#include <cocos2d.h>

#define __CHECK_TIME__ std::this_thread::sleep_for(std::chrono::milliseconds(1))

#define IS_NEW_DECODE 0

#ifdef USING_AUDIO
const int MAX_AUDIO_DECODE_QUEUE_SIZE = 4;
#endif

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"

#include "libavutil/imgutils.h"
#include "libavfilter/avfilter.h"
}



const int MAX_VIDEO_DECODE_QUEUE_SIZE = 4;
#define AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

class VideoDecodeThread : public std::enable_shared_from_this<VideoDecodeThread>
{
public:
    VideoDecodeThread() {}
    virtual ~VideoDecodeThread() {
        CCLOG("THREAD END COMPLETE");
    }
    
    static std::shared_ptr<VideoDecodeThread> execute(std::shared_ptr<VideoDecoder> decoder) {
        auto owner = std::make_shared<VideoDecodeThread>();
        
        owner->_decoder = decoder;
        owner->_thread = std::thread(&VideoDecodeThread::threadFunc, owner);
        return owner;
    }
    
    void join() {
        if (_thread.joinable()) {
            _thread.join();
        }
    }
    
private:
    void threadFunc() {
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
            _decoder->threadForVideoPlayback();
            if (_decoder->_interrupted) {
                _decoder->release();
            }
        } while(0);
        
        CCLOG("THREAD WILL EXI.");
        _decoder = nullptr;
    }

private:
    std::shared_ptr<VideoDecoder> _decoder;
    
    std::thread _thread;
};


VideoCodecProtocol::VideoCodecProtocol() : _decoder(nullptr)
{
    
}

VideoCodecProtocol::~VideoCodecProtocol()
{
    _decoder->destroy();
    _decoder = nullptr;
}

bool VideoCodecProtocol::initVideoDecoder(const std::string& videoName)
{
    auto decoder = VideoDecoder::create(this, videoName);
    
    bool ret = (decoder != nullptr);
    
    _decoder = decoder;
    
    return ret;
}

void VideoCodecProtocol::playVideo()
{
    _decoder->playVideo();
}

void VideoCodecProtocol::stopVideo()
{
    _decoder->stopVideo();
}

VideoFrame* VideoCodecProtocol::getVideoFrame()
{
    return _decoder->dequeueDecodedVideoFrame();
}

void VideoCodecProtocol::recycleVideoFrame(VideoFrame* frame)
{
    if (frame) {
        _decoder->queueIdleVideoFrame(frame);
    }
}

#ifdef USING_AUDIO
VideoFrame* VideoCodecProtocol::getAudioFrame()
{
    return _decoder->dequeueDecodedAudioFrame();
}

void VideoCodecProtocol::recycleAudioFrame(VideoFrame *frame)
{
    if (frame) {
        _decoder->queueIdleAudioFrame(frame);
    }
}
#endif




//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Video Decoder Host
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
std::shared_ptr<VideoDecoder> VideoDecoder::create(VideoCodecProtocol* protocol, const std::string& videoName)
{
    
    std::shared_ptr<VideoDecoder> decoder = std::make_shared<VideoDecoder>();
    decoder->_sharedThis = decoder;
    
    if (decoder->init(protocol, videoName)) {
        return decoder;
    } else {
        decoder.reset();
    }
    
    return nullptr;
}

VideoDecoder::VideoDecoder()  : _interrupted(false)
, _initialized(false)
, _playing(true)
, _videoDurationTime(0)
, _videoFrameRate(0)
, _videoWidth(0)
, _videoHeight(0)
, _formatContext(nullptr)
, _avPacket(nullptr)
, _videoStreamIndex(-1)
, _videoCodecContext(nullptr)
, _avFrame(nullptr)
, _videoMaxFrameCount(0)
, _swsContext(nullptr)
#ifdef USING_AUDIO
, _swrContext(nullptr)
, _fmt(0)
, _bit_rate(0)
, _samples(0)
, _channels(0)
, _audioCodecContext(nullptr)
, _audioStreamIndex(-1)
, _audioMaxFrameCount(0)
#endif
{
}

VideoDecoder::~VideoDecoder()
{
    CCLOG("VIDEO DEOCDER DESTRUCTOR.");
}

void VideoDecoder::destroy()
{
    CCLOG("SIGNAL VIDEO END.");
    
    _videoClient = nullptr;
    
    {
        _interrupted = true;
        _playCond.notify_all();
    }
    
    std::thread t([&]() {
        _videoDecodeThread->join();
        _sharedThis = nullptr;
    });
    t.detach();
}

bool VideoDecoder::init(VideoCodecProtocol* protocol, const std::string& videoName)
{
    if (protocol == nullptr || videoName.empty())
        return false;
    
    _videoClient = protocol;
    _videoName = videoName;
    _videoDecodeThread = VideoDecodeThread::execute(_sharedThis);
    
    

//    // init open al
//    _alDevice = alcOpenDevice(NULL);
//
//    if (_alDevice) {
//        _alContext = alcCreateContext(_alDevice, NULL);
//        alcMakeContextCurrent(_alContext);
//    }
//
//    alGenSources(1, &_alOutSourceId);
//    alSpeedOfSound(1.0f);
//    alDopplerVelocity(1.0f);
//    alDopplerFactor(1.0f);
//    alSourcef(_alOutSourceId, AL_PITCH, 1.0f);
//    alSourcef(_alOutSourceId, AL_GAIN, 1.0f);
//    alSourcef(_alOutSourceId, AL_LOOPING, AL_FALSE);
//
//    // fix???
//    alSourcef(_alOutSourceId, AL_SOURCE_TYPE, AL_STREAMING);
    
    return true;
}

void VideoDecoder::release()
{
    
#ifdef USING_AUDIO
    if (_swrContext) {
        swr_free(&_swrContext);
        _swrContext = nullptr;
    }
    _audioStreamIndex = -1;
    if (_audioCodecContext) {
        avcodec_flush_buffers(_audioCodecContext);
        avcodec_close(_audioCodecContext);
        _audioCodecContext = nullptr;
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
#endif
    
    if (_swsContext) {
        sws_freeContext(_swsContext);
        _swsContext = nullptr;
    }
    
    _videoStreamIndex = -1;
    if (_videoCodecContext) {
        avcodec_flush_buffers(_videoCodecContext);
        avcodec_close(_videoCodecContext);
        _videoCodecContext = nullptr;
    }
    
    if (_formatContext) {
        avformat_close_input(&_formatContext);
        _formatContext = nullptr;
    }
    
    if (_avFrame) {
        av_frame_free(&_avFrame);
        _avFrame = nullptr;
    }
    
    if (_avPacket) {
//        av_free_packet(_avPacket);
        av_packet_unref(_avPacket);
        _avPacket = nullptr;
    }
    
    while (!_idleVideoQueue.empty()) {
        auto frame = _idleVideoQueue.front();
        _idleVideoQueue.pop();
        delete frame;
    }
    
    while (!_decodedVideoQueue.empty()) {
        auto frame = _decodedVideoQueue.front();
        _decodedVideoQueue.pop();
        delete frame;
    }
    
    CCLOG("RELEASE VIDEO");
}

void VideoDecoder::playVideo()
{
    _playing = true;
    if (_initialized) {
        _playCond.notify_all();
    }
}

void VideoDecoder::stopVideo()
{
    _playing = false;
    if (_initialized) {
        _playCond.notify_all();
    }
    
}

void VideoDecoder::onVideoInitResult(bool success, int width, int height, double frameRate, double durationTime)
{
    _videoWidth = width;
    _videoHeight = height;
    _videoFrameRate = frameRate;
    _videoDurationTime = durationTime;
    
    if (success) {
        if (_videoClient) {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&] {
                if (_videoClient) {
                    _videoClient->onVideoInitResult(true, _videoWidth, _videoHeight, _videoFrameRate, _videoDurationTime);
                }
            });
        }
    } else {
        if (_videoClient) {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&] {
                if (_videoClient) {
                    _videoClient->onVideoInitResult(false, 0, 0, 0, 0);
                }
            });
        }
    }
}


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

int VideoDecoder::readVideoPacket(VideoFrame **queueVideoFrame)
{
    int ret = 0;
    int used = avcodec_send_packet(_videoCodecContext, _avPacket);
    if (used < 0 && used != AVERROR(EAGAIN) && used != AVERROR_EOF) {
        CCLOG("VIDEO Decode Error.(1)");
        return -1;
    }
    
    while (ret  >= 0) {
        ret = avcodec_receive_frame(_videoCodecContext, _avFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
//                    CCLOG("receive done.. no more data ");
            break;
        } else if (ret<0) {
            CCLOG("avcodec_receive_frame for video Error. : %d", ret);
            return -1;
        }
        
        int64_t timeStamp = _avFrame->best_effort_timestamp;
        AVStream* stream = _formatContext->streams[_videoStreamIndex];
        double decodeTime = (timeStamp - stream->start_time) * av_q2d(stream->time_base);

        if (decodeTime < _playTime) {
            _playTime = decodeTime;
            continue;
        }

        VideoFrame* decodeFrame = nullptr;

        if (queueVideoFrame) {
            decodeFrame = *queueVideoFrame;
        }
        if (decodeFrame == nullptr) {
            if (_videoMaxFrameCount >= MAX_VIDEO_DECODE_QUEUE_SIZE)
                continue; // skipping

            decodeFrame = new VideoFrame(_avFrame);
            _videoMaxFrameCount++;
        }

        decodeFrame->setData(_avFrame);
        queueDecodedVideoFrame(decodeFrame);

//        av_frame_unref(_avFrame);
        
        if (queueVideoFrame) {
            *queueVideoFrame = nullptr;
        }

//        av_packet_unref(_avPacket);
//        _avPacket->buf = nullptr;
        return 0;
    }
    return -999;
}

#ifdef USING_AUDIO

int VideoDecoder::readAudioPacket(VideoFrame **queueAudioFrame)
{
    int ret = 0;
    int data_size = 0;
    int used = avcodec_send_packet(_audioCodecContext, _avPacket);
    if (used < 0 && used != AVERROR(EAGAIN) && used != AVERROR_EOF) {
        CCLOG("AUDIO Decode Error.(1)");
        return -1;
    }
    while (ret  >= 0) {
        ret = avcodec_receive_frame(_audioCodecContext, _avFrame);
        if (ret==AVERROR(EAGAIN) || ret==AVERROR_EOF) {
            break;
        } else if (ret<0) {
            CCLOG("avcodec_receive_frame for audio Error. : %d", ret);
            return ret;
        }

        // decode audio
        data_size = av_get_bytes_per_sample(_audioCodecContext->sample_fmt);
        SwrContext* pAudioCvtContext;
        pAudioCvtContext = swr_alloc_set_opts(NULL, _audioCodecContext->channel_layout, AV_SAMPLE_FMT_S16, _audioCodecContext->sample_rate, _audioCodecContext->channel_layout, _audioCodecContext->sample_fmt, _audioCodecContext->sample_rate, 0, 0);
        
        swr_init(pAudioCvtContext);
        
        VideoFrame* decodeFrame = nullptr;
        if (queueAudioFrame) {
            decodeFrame = *queueAudioFrame;
        }
        if (decodeFrame == nullptr) {
            if (_audioMaxFrameCount >= MAX_AUDIO_DECODE_QUEUE_SIZE) {
                continue;
            }

            decodeFrame = new VideoFrame(_avFrame);
            _audioMaxFrameCount++;
        }

        decodeFrame->setData(_avFrame, pAudioCvtContext);
        queueDecodedAudioFrame(decodeFrame);

        swr_free(&pAudioCvtContext);
//        av_frame_unref(_avFrame);

        if (queueAudioFrame) {
            *queueAudioFrame = nullptr;
        }

//        av_packet_unref(_avPacket);
//        _avPacket->buf = nullptr;

        return 0;
    } // while (ret  >= 0) {
    
    return -999;
}

int VideoDecoder::decodeVideoFrame(VideoFrame** queueVideoFrame, VideoFrame** queueAudioFrame)
{
    if (_formatContext == nullptr || _avPacket == nullptr)
        return -1;
    
    int ret = 0;    // 0 : SUCCESS
    _avPacket->buf = nullptr;
    
    // 1. read AVFrame from AVPacket
    // 2. decode AVFrame
    // 3. queue decoded bufferd
//    bool exist_videoFrame = false;
//    bool exist_Audioframe = false;
    
    
    
    bool existVideo = false;
    bool existAudio = false;
    
    if (_audioStreamIndex==-1) {
        existAudio = true;
    }

    bool firstCall = false;
    
    bool passVideo = false;
    bool passAudio = false;
    int frameFinished;
    
    
    while ((ret = av_read_frame(_formatContext, _avPacket)) >= 0) {
        if (_avPacket->stream_index == _videoStreamIndex) {
            int used = avcodec_send_packet(_videoCodecContext, _avPacket);
            if (used<0) {
                CCLOG("Error sending packet for decoding");
                return -1;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(_videoCodecContext, _avFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    // EOF exit loop
                    break;
                } else if (ret < 0) {
                    // could not decode packet
                    CCLOG("Error while decoding");
                    // exit with error
                    return -1;
                } else {
                    frameFinished = 1;
                }
                
                if (frameFinished) {
                    int64_t timeStamp = _avFrame->best_effort_timestamp;
                    AVStream* stream = _formatContext->streams[_videoStreamIndex];
                    double decodeTime = (timeStamp - stream->start_time) * av_q2d(stream->time_base);

                    if (decodeTime < _playTime) {
                        _playTime = decodeTime;
                        continue;
                    }

                    VideoFrame* decodeFrame = nullptr;

                    if (queueVideoFrame) {
                        decodeFrame = *queueVideoFrame;
                    }
                    if (decodeFrame == nullptr) {
                        if (_videoMaxFrameCount >= MAX_VIDEO_DECODE_QUEUE_SIZE)
                            continue; // skipping

                        decodeFrame = new VideoFrame(_avFrame);
                        _videoMaxFrameCount++;
                    }

                    decodeFrame->setData(_avFrame);
                    queueDecodedVideoFrame(decodeFrame);

                    av_frame_unref(_avFrame);
                    if (queueVideoFrame) {
                        *queueVideoFrame = nullptr;
                    }
                    av_packet_unref(_avPacket);
                    _avPacket->buf = nullptr;

                    return 0;
                }
            } // while (ret >= 0) {
            av_packet_unref(_avPacket);
        } else if (_avPacket->stream_index == _audioStreamIndex) {
        }
    } // while ((ret = av_read_frame(_formatContext, _avPacket)) == 0) {

    
    av_frame_unref(_avFrame);
    av_packet_unref(_avPacket);
    _avPacket->buf = nullptr;


    if (ret<0) {
        CCLOG("end of video");
        return 1;
    }

    if (queueVideoFrame) {
        *queueVideoFrame = nullptr;
    }

    if (queueAudioFrame) {
        *queueAudioFrame = nullptr;
    }

    return 0;
    
    
    
    if (queueVideoFrame==nullptr) {
        firstCall = true;
        while ((ret = av_read_frame(_formatContext, _avPacket)) == 0) {
            if (_avPacket->stream_index == _videoStreamIndex) {
            } else if (_avPacket->stream_index == _audioStreamIndex) {
            }
        }
        return 0;
    } else {
        while ((ret = av_read_frame(_formatContext, _avPacket)) == 0) {
            if (_avPacket->stream_index == _videoStreamIndex) {
                auto queuFrame = *queueVideoFrame;
                ret = readVideoPacket(queueVideoFrame);
                if (queuFrame==nullptr) {
                    passVideo = true;
                }
            } else if (_avPacket->stream_index == _audioStreamIndex) {
                auto queuFrame = *queueAudioFrame;
                ret = readAudioPacket(queueAudioFrame);
                if (queuFrame==nullptr) {
                    passAudio = true;
                }
            }
            if (ret<0 && ret!=-999) {
                CCLOG("[[[[[ decode error : %d", ret);
                return -1;
            }

            if (passVideo && passAudio) {
                av_frame_unref(_avFrame);
                av_packet_unref(_avPacket);
                _avPacket->buf = nullptr;
    //            CCLOG("[[[[[ decode complete");
                return 0;
    //            if (isfirst) {
    //                return 0;
    //            }
            }
        }
    }


}

void VideoDecoder::threadForInit()
{
    std::unique_lock<std::mutex> lock(_videoMutex);
    
    do {
        if (_interrupted)
            break;
        
        int ret;
        ret = avformat_open_input(&_formatContext, _videoName.c_str(), nullptr, nullptr);
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
        
#ifdef USING_AUDIO
        _audioStreamIndex = -1;
        for (int i=0; i<_formatContext->nb_streams && !_interrupted; i++) {
            if (_formatContext->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
                if (_audioStreamIndex==-1) {
                    _audioStreamIndex = i;
                    break;
                }
            }
        }
        if (_audioStreamIndex==-1) {
            CCLOG("[[[[[ Didn't find a audio stream.");
        }
        if (_interrupted) {
            break;
        }
        
        const AVCodec* audioCodec = avcodec_find_decoder(_formatContext->streams[_audioStreamIndex]->codecpar->codec_id);
        if (!audioCodec) {
            CCLOG("[[[[[Audio Codec not found.");
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
#endif
        
        
        // get video stream index
        _videoStreamIndex = -1;
        for (int i = 0; i < _formatContext->nb_streams && !_interrupted; i++) {
            if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                //            if (_formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (_videoStreamIndex==-1) {
                    _videoStreamIndex = i;
                    break;
                }
            }
        }
        if (_videoStreamIndex == -1){
            CCLOG("Didn't find a video stream.");
            break;
        }
        if (_interrupted)
            break;

        // video codec & context
        const AVCodec* videoCodec = avcodec_find_decoder(_formatContext->streams[_videoStreamIndex]->codecpar->codec_id);
        if (!videoCodec) {
            CCLOG("Video Codec not found.(2)");
            break;
        }
        
        _videoCodecContext = avcodec_alloc_context3(videoCodec);
        
        if (_interrupted)
            break;
        
        avcodec_parameters_to_context(_videoCodecContext, _formatContext->streams[_videoStreamIndex]->codecpar);
        
        ret = avcodec_open2(_videoCodecContext, videoCodec, nullptr);
        if (ret < 0){
            CCLOG("Could not open video codec.");
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
        
        _avFrame = av_frame_alloc();
        if (_avFrame == nullptr)
            break;
        
        if (_interrupted)
            break;
        
        int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, _videoCodecContext->width, _videoCodecContext->height, 1);
            uint8_t* buf = (uint8_t *)av_malloc(size);
        av_image_fill_arrays(_avFrame->data, _avFrame->linesize, buf, AV_PIX_FMT_YUV420P, _videoCodecContext->width, _videoCodecContext->height, 1);
        
        if (ret < 0) {
            CCLOG("avpicture_fill failed.");
            break;
        }
        if (_interrupted)
            break;
        
        _avPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
        
#ifdef _DEBUG_DUMP_FORMAT
        av_dump_format(_formatContext, 0, _videoName.c_str(), 0);
#endif
        if (_interrupted)
            break;
        
        _playTime = 0;
#ifdef USING_AUDIO
        // decodeVideoFrame을 한번 호출해서 queue에 하나 frame을 생성함.
        if (IS_NEW_DECODE) {
//            if (initDecodeFrame()) {
//                break;
//            }
        } else {
            if (decodeVideoFrame() != 0) {
                break;
            }
            // 두번째 프레임 전달.
            auto videoFrame = dequeueDecodedVideoFrame();
            auto audioFrme = dequeueDecodedAudioFrame();
            if (decodeVideoFrame(&videoFrame, &audioFrme) != 0) {
                break;
            }
        }
//
        
        
        CCLOG("Video initialize complete!");
        int samples = AV_SAMPLE_FMT_S16;
        int bit_rate = _audioCodecContext->sample_rate;
        int fmt = _audioCodecContext->sample_fmt;
        
        _freq = _audioCodecContext->sample_rate;
        _format = 0x8010; //Signed 16-bit samples
        _channels = _audioCodecContext->channels;
        _samples = AUDIO_BUFFER_SIZE;
        
        

        // open audio
        onAudioInitResult(true, samples, bit_rate, fmt, durationTime);
#else
        // decodeVideoFrame을 한번 호출해서 queue에 하나 frame을 생성함.
        if (decodeVideoFrame() != 0) {
            break;
        }
        // 두번째 프레임 전달.
        auto videoFrame = dequeueDecodedVideoFrame();
        if (decodeVideoFrame(&videoFrame) != 0) {
            break;
        }
        CCLOG("Video initialize complete!");
#endif
        
        // 성공
        onVideoInitResult(true, width, height, frameRate, durationTime);
        return;
    } while (0);
    
    // 실패
    release();
    onAudioInitResult(false, 0, 0, 0, 0);
    onVideoInitResult(false, 0, 0, 0, 0);
}


#else
// for android
int VideoDecoder::decodeVideoFrame(VideoFrame** videoFrame) {
    if (_formatContext == nullptr || _avPacket == nullptr)
        return -1;
    
    int ret, got_picture, got_sound;
    _avPacket->buf = nullptr;
    
    while ((ret = av_read_frame(_formatContext, _avPacket)) == 0) {
        if (_avPacket->stream_index == _videoStreamIndex) {
            got_picture = 0;
            ret = avcodec_decode_video2(_videoCodecContext, _avFrame, &got_picture, _avPacket);
            if (ret < 0) {
                CCLOG("Decode Error.(1)");
                return -1;
            }
            
            if (got_picture) {
                /** queue buffer **/
                
                int64_t timeStamp = av_frame_get_best_effort_timestamp(_avFrame);
                AVStream* stream = _formatContext->streams[_videoStreamIndex];
                double decodeTime = (timeStamp - stream->start_time) * av_q2d(stream->time_base);
                
                if (decodeTime < _playTime) {
                    _playTime = decodeTime;
                    continue;
                }
                
                VideoFrame* decodeFrame = nullptr;
                
                if (videoFrame) {
                    decodeFrame = *videoFrame;
                }
                if (decodeFrame == nullptr) {
                    if (_videoMaxFrameCount >= MAX_VIDEO_DECODE_QUEUE_SIZE)
                        continue; // skipping
                    
                    decodeFrame = new VideoFrame(_avFrame);
                    _videoMaxFrameCount++;
                }
                
                decodeFrame->setData(_avFrame);
                queueDecodedVideoFrame(decodeFrame);
                
                if (videoFrame) {
                    *videoFrame = nullptr;
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

void VideoDecoder::threadForInit() {
    std::unique_lock<std::mutex> lock(_videoMutex);
    
    do {
        if (_interrupted)
            break;
        
        int ret = 0;
        ret = avformat_open_input(&_formatContext, _videoName.c_str(), nullptr, nullptr);
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
        
        _avFrame = av_frame_alloc();
        if (_avFrame == nullptr)
            break;
        
        if (_interrupted)
            break;
        
        uint8_t* buf  = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, _videoCodecContext->width, _videoCodecContext->height));
        ret = avpicture_fill((AVPicture *)_avFrame, buf, AV_PIX_FMT_YUV420P, _videoCodecContext->width, _videoCodecContext->height);
        if (ret < 0) {
            CCLOG("avpicture_fill failed.");
            break;
        }
        if (_interrupted)
            break;
        
        _avPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
        
#ifdef _DEBUG_DUMP_FORMAT
        av_dump_format(_formatContext, 0, _videoName.c_str(), 0);
#endif
        if (_interrupted)
            break;
        
        _playTime = 0;
                
        
        // 첫번째 frame 디코드 후 버림.
        if (decodeVideoFrame() != 0) {
            break;
        }
        // 두번째 프레임 전달.
        auto videoFrame = dequeueDecodedVideoFrame();
        if (decodeVideoFrame(&videoFrame) != 0) {
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

void VideoDecoder::waitForPlay()
{
    std::unique_lock<std::mutex> lock(_videoMutex);
    _playCond.wait(lock);
}

#ifdef USING_AUDIO

int VideoDecoder::initDecodeFrame()
{
    if (_formatContext == nullptr || _avPacket == nullptr)
        return -1;
    
    int ret = 0;    // 0 : SUCCESS
    _avPacket->buf = nullptr;

    while ((ret = av_read_frame(_formatContext, _avPacket)) >= 0) {
        if (_avPacket->stream_index == _videoStreamIndex) {
            int used = avcodec_send_packet(_videoCodecContext, _avPacket);
            if (used<0) {
                CCLOG("Error sending packet for decoding");
                return -1;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(_videoCodecContext, _avFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    // EOF exit loop
                    break;
                } else if (ret < 0) {
                    // could not decode packet
                    CCLOG("Error while decoding");
                    // exit with error
                    return -1;
                }
                
                int64_t timeStamp = _avFrame->best_effort_timestamp;
                AVStream* stream = _formatContext->streams[_videoStreamIndex];
                double decodeTime = (timeStamp - stream->start_time) * av_q2d(stream->time_base);

                if (decodeTime < _playTime) {
                    _playTime = decodeTime;
                    continue;
                }

                VideoFrame* decodeFrame = nullptr;

//                for (int i=0; i<4; i++) {
                    decodeFrame = new VideoFrame(_avFrame);
                    _videoMaxFrameCount++;

                    decodeFrame->setData(_avFrame);
                    queueDecodedVideoFrame(decodeFrame);
//                }

                av_frame_unref(_avFrame);
                av_packet_unref(_avPacket);
                _avPacket->buf = nullptr;

                return 0;
                
            } // while (ret >= 0) {
            av_packet_unref(_avPacket);
        }
    }
    CCLOG("[[[[[ Something is strange...");
    
    return 0;
}

int VideoDecoder::newVideoDecodeFrame(VideoFrame **queueFrame)
{
    if (_formatContext == nullptr || _avPacket == nullptr)
        return -1;
    
    int ret = 0;    // 0 : SUCCESS
    _avPacket->buf = nullptr;

    int used = avcodec_send_packet(_videoCodecContext, _avPacket);
     if (used<0) {
         CCLOG("Error sending packet for decoding");
         return -1;
     }

     while (ret >= 0) {
         ret = avcodec_receive_frame(_videoCodecContext, _avFrame);
         if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
             // EOF exit loop
             break;
         } else if (ret < 0) {
             // could not decode packet
             CCLOG("Error while decoding");
             // exit with error
             return -1;
         }
         
         int64_t timeStamp = _avFrame->best_effort_timestamp;
         AVStream* stream = _formatContext->streams[_videoStreamIndex];
         double decodeTime = (timeStamp - stream->start_time) * av_q2d(stream->time_base);

         if (decodeTime < _playTime) {
             _playTime = decodeTime;
             continue;
         }

         VideoFrame* decodeFrame = nullptr;

         if (queueFrame) {
             decodeFrame = *queueFrame;
         }
         if (decodeFrame == nullptr) {
             if (_videoMaxFrameCount >= MAX_VIDEO_DECODE_QUEUE_SIZE)
                 continue; // skipping

             decodeFrame = new VideoFrame(_avFrame);
             _videoMaxFrameCount++;
         }

         decodeFrame->setData(_avFrame);
         queueDecodedVideoFrame(decodeFrame);

         av_frame_unref(_avFrame);
         if (queueFrame) {
             *queueFrame = nullptr;
         }
         av_packet_unref(_avPacket);
         _avPacket->buf = nullptr;

         return 0;
     }
    av_frame_unref(_avFrame);
    av_packet_unref(_avPacket);
    _avPacket->buf = nullptr;


    if (ret<0) {
        CCLOG("end of video");
        return 1;
    }

    if (queueFrame) {
        *queueFrame = nullptr;
    }

    return 0;
}

int VideoDecoder::newAudioDecodeFrame(VideoFrame **queueFrame)
{
    
}

void VideoDecoder::threadForVideoPlayback()
{
    // 실질적인 video decode thread loop
    
    
    
    // 1. 빈 frame을 하나 꺼내온다. 빈 frame이 없으면 skip 또는 play 중이지 않을때는 대기
    // 2. AVPacket으로 부터 decode 된 videoFramed을 얻어옴
    // 3. ret가 1이면 비디오 종료로 대기, 0보다 작으면 오류로 LOOP 종료
    
    std::unique_lock<std::mutex> lock(_videoMutex);
    
    VideoFrame* videoframe = nullptr;
    VideoFrame* audioFrame = nullptr;

    // FFMpeg player decode_thread이 for(;;)와 같다.
    /*
    loop
     1. av_read_frame
     2.
     */
if (IS_NEW_DECODE) {
//    bool isVideoFirst = true;
    while (!_interrupted) {
        _avPacket->buf = nullptr;
        if (av_read_frame(_formatContext, _avPacket)<0) {
            if (_formatContext->pb->error==0) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                continue;
            } else {
                break;
            }
        }
        
        if (_avPacket->stream_index==_videoStreamIndex) {
            videoframe = dequeueIdleVideoFrame();
            if (videoframe) {
                int ret = newVideoDecodeFrame(&videoframe);
                if (videoframe) {
                    // 정상인 경우 여기에 오지 않는다. 빈슬롯 추가는 recycle에서만 함
                    queueIdleVideoFrame(videoframe);
                }
                
                if (ret < 0) {
                    // error
                    break;
                } else if (ret == 1) {
                    // end of video

                    avcodec_flush_buffers(_videoCodecContext);
                    avcodec_flush_buffers(_audioCodecContext);

                    _playTime = 0;

                    av_seek_frame(_formatContext, _videoStreamIndex, 0, 0);
                    av_seek_frame(_formatContext, _audioStreamIndex, 0, 0);
                    continue;
                }
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
            
        } else if (_avPacket->stream_index==_audioStreamIndex) {
            
        } else {
            av_packet_unref(_avPacket);
        }
    }

    CCLOG("PLAYBACK END");
} else {

    while (!_interrupted) {
        videoframe = dequeueIdleVideoFrame();
        audioFrame = dequeueIdleAudioFrame();
        if (videoframe) {
            // AVFrame을 decode하고 _decodedVideoQueue에 넣는다.
            int ret = decodeVideoFrame(&videoframe, &audioFrame);
            if (videoframe) {
                // 정상인 경우 여기에 오지 않는다. 빈슬롯 추가는 recycle에서만 함
                queueIdleVideoFrame(videoframe);
            }
            if (audioFrame) {
                queueIdleAudioFrame(audioFrame);
            }
            if (ret < 0) {
                // error
                break;
            } else if (ret == 1) {
                // end of video

                avcodec_flush_buffers(_videoCodecContext);
                avcodec_flush_buffers(_audioCodecContext);

                _playTime = 0;

                av_seek_frame(_formatContext, _videoStreamIndex, 0, 0);
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
    CCLOG("PLAYBACK END");
}
}
#else
void VideoDecoder::threadForVideoPlayback()
{
    // 실질적인 video decode thread loop
    
    // 1. 빈 frame을 하나 꺼내온다. 빈 frame이 없으면 skip 또는 play 중이지 않을때는 대기
    // 2. AVPacket으로 부터 decode 된 videoFramed을 얻어옴
    // 3. ret가 1이면 비디오 종료로 대기, 0보다 작으면 오류로 LOOP 종료
    
    std::unique_lock<std::mutex> lock(_videoMutex);
    
    VideoFrame* videoframe = nullptr;
    
    while (!_interrupted) {
        videoframe = dequeueIdleVideoFrame();
        if (videoframe) {
            // AVFrame을 decode하고 _decodedVideoQueue에 넣는다.
            int ret = decodeVideoFrame(&videoframe);
            if (videoframe) {
                // 정상인 경우 여기에 오지 않는다. 빈슬롯 추가는 recycle에서만 함
                queueIdleVideoFrame(videoframe);
            }
            if (ret < 0) {
                // error
                break;
            } else if (ret == 1) {
                // end of video
                
                avcodec_flush_buffers(_videoCodecContext);
                
                _playTime = 0;
                
                av_seek_frame(_formatContext, _videoStreamIndex, 0, 0);
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
    CCLOG("PLAYBACK END");
}
#endif

VideoFrame* VideoDecoder::dequeueDecodedVideoFrame()
{
    std::lock_guard<std::mutex> guard(_videoQueueMutex);
    VideoFrame* frame = nullptr;
    
    if (!_decodedVideoQueue.empty()) {
        frame = _decodedVideoQueue.front();
        _decodedVideoQueue.pop();
    }
    
    if (_initialized) {
        _playCond.notify_all();
    }
    
    return frame;
}

void VideoDecoder::queueDecodedVideoFrame(VideoFrame* frame)
{
    std::lock_guard<std::mutex> guard(_videoQueueMutex);
    _decodedVideoQueue.push(frame);
}

VideoFrame* VideoDecoder::dequeueIdleVideoFrame()
{
    VideoFrame* frame = nullptr;
    std::lock_guard<std::mutex> guard(_videoQueueMutex);
    
    if (!_idleVideoQueue.empty()) {
        frame = _idleVideoQueue.front();
        _idleVideoQueue.pop();
    }
    
    return frame;
}

void VideoDecoder::queueIdleVideoFrame(VideoFrame* frame)
{
    std::lock_guard<std::mutex> guard(_videoQueueMutex);
    _idleVideoQueue.push(frame);
}



#ifdef USING_AUDIO
void VideoDecoder::onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime)
{
 
    _samples = samples;
    _bit_rate = rate;
    _fmt = fmt;
    
    if (success) {
        if (_videoClient) {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&] {
                if (_videoClient) {
                    _videoClient->onAudioInitResult(success, samples, rate, fmt, durationTime);
                }
            });
        }
    } else {
        if (_videoClient) {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&] {
                if (_videoClient) {
                    _videoClient->onAudioInitResult(false, 0, 0, 0, 0);
                }
            });
        }
    }
}

VideoFrame* VideoDecoder::dequeueDecodedAudioFrame()
{
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    VideoFrame* frame = nullptr;
    
    if (!_decodedAudioQueue.empty()) {
        frame = _decodedAudioQueue.front();
        _decodedAudioQueue.pop();
    }
    
    if (_initialized) {
        _playCond.notify_all();
    }
    
    return frame;
}

void VideoDecoder::queueDecodedAudioFrame(VideoFrame *frame)
{
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    _decodedAudioQueue.push(frame);
}

VideoFrame* VideoDecoder::dequeueIdleAudioFrame()
{
    VideoFrame* frame = nullptr;
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    
    if (!_idleAudioQueue.empty()) {
        frame = _idleAudioQueue.front();
        _idleAudioQueue.pop();
    }
    
    return frame;
}

void VideoDecoder::queueIdleAudioFrame(VideoFrame *frame)
{
    std::lock_guard<std::mutex> guard(_audioQueueMutex);
    _idleAudioQueue.push(frame);
}
#endif
