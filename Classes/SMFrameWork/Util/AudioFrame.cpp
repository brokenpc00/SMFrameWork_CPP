//
//  AudioFrame.cpp
//  IPCT
//
//  Created by SteveKim on 2020/02/19.
//

#include "AudioFrame.h"
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

AudioFrame::AudioFrame(AVFrame* avFrame) : _audio_buf(nullptr)
{
    if (avFrame) {
        _channels = avFrame->channels;
        _samples = avFrame->nb_samples;
        _sample_rate = avFrame->sample_rate;
        _fmt = avFrame->format;
        _bit_per_sample = 0;
        _bit_per_channel = 0;
        int size = av_samples_get_buffer_size(NULL, _channels, _samples, AV_SAMPLE_FMT_S16, 1);
        _audio_buf = (uint8_t *)av_malloc(size);
        // no fill data now
    }
};

AudioFrame::~AudioFrame()
{
    if (_audio_buf) {
        free(_audio_buf);
        _audio_buf = nullptr;
    }
}

void AudioFrame::setData(AVFrame* frame, SwrContext* swrContext)
{
    if (swrContext) {
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

