//
//  VideoFrame.cpp
//  LiveOn
//
//  Created by SteveKim on 03/07/2019.
//

#include "VideoFrame.h"
#include <string>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"

#include "libavutil/imgutils.h"
#include "libavfilter/avfilter.h"
}

VideoFrame::VideoFrame(AVFrame* avFrame, bool isVideo)
: _data(nullptr)
, _dataU(nullptr)
, _dataV(nullptr)
, _audio_buf(nullptr)
, _videoFrame(isVideo)
{
    if (isVideo) {
        if (avFrame) {
            _width = avFrame->linesize[0];
            _height = avFrame->height;

            int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, _width, _height, 1);
            _data = (uint8_t*)av_malloc(size);
            av_image_fill_arrays(avFrame->data, avFrame->linesize, _data, AV_PIX_FMT_YUV420P, _width, _height, 1);
            
            _dataU = _dataV = _data;
        }
    } else {
        if (avFrame) {
            _channels = avFrame->channels;
            _samples = avFrame->nb_samples;
            _sample_rate = avFrame->sample_rate;    // freq
            _fmt = avFrame->format;
            _bit_per_sample = 0;
            _bit_per_channel = 0;
            int size = av_samples_get_buffer_size(NULL, _channels, _samples, AV_SAMPLE_FMT_S16, 1);
            _audio_buf = (uint8_t *)av_malloc(size);
            // no fill data now
        }
    }

};

VideoFrame::~VideoFrame()
{
    if (_data) {
        av_free(_data);
        _data = nullptr;
    }
    
    if (_audio_buf) {
        free(_audio_buf);
        _audio_buf = nullptr;
    }

}

void VideoFrame::setData(AVFrame* frame, SwrContext* swrContext)
{
    if (_videoFrame) {
        if (frame) {
            int size0 = frame->linesize[0] * frame->height;
            int size1 = frame->linesize[1] * frame->height / 2;
            int size2 = frame->linesize[2] * frame->height / 2;
            
            memcpy(_data, frame->data[0], size0);
            memcpy(_data+size0, frame->data[1], size1);
            memcpy(_data+size0+size1, frame->data[2], size2);
            
            _dataU = _data+size0;
            _dataV = _data+size0+size1;
        }
    } else {
        if (swrContext && frame) {
            uint8_t *out[] = {_audio_buf};
            int needed_buf_size = av_samples_get_buffer_size(NULL, _channels, _samples, AV_SAMPLE_FMT_S16, 1);
            int bitsPerChannel = 8*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            _bit_per_channel = bitsPerChannel;
            int bitsPerSample=0;
            switch (frame->format) {
                case AVSampleFormat::AV_SAMPLE_FMT_U8:
                case AVSampleFormat::AV_SAMPLE_FMT_U8P:
                    bitsPerSample = 8;
                    break;
                case AVSampleFormat::AV_SAMPLE_FMT_S16:
                case AVSampleFormat::AV_SAMPLE_FMT_S16P:
                    bitsPerSample = 16;
                    break;
                case AVSampleFormat::AV_SAMPLE_FMT_S32:
                case AVSampleFormat::AV_SAMPLE_FMT_S32P:
                    bitsPerSample = 32;
                    break;
                case AVSampleFormat::AV_SAMPLE_FMT_FLT:
                case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
                    bitsPerSample = 32;
                    break;
                case AVSampleFormat::AV_SAMPLE_FMT_DBL:
                case AVSampleFormat::AV_SAMPLE_FMT_DBLP:
                    //dbl is converted by the AvFrame.DecodePacket method
                    bitsPerSample = 32;
                    break;
                default:
                    bitsPerSample = 0;
                    break;
            }
            _bit_per_sample = bitsPerSample;
    //        frame->format==AVSampleFormat::AV_SAMPLE_FMT_S16;
    //        int len = swr_convert(swrContext, out, needed_buf_size, (const uint8_t **)frame->data, frame->nb_samples);
            int len = swr_convert(swrContext, out, needed_buf_size, (const uint8_t **)frame->extended_data, frame->nb_samples);
            // converted real byte (_audio_buf, len)
            _size = len * _channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    //        av_samples_alloc_array_and_samples(out)
    //        CCLOG("Audio SetData Size -> needed_buf : %d, convert_len : %d, total_size : %d", needed_buf_size, len, _size);
        }
    }

}
