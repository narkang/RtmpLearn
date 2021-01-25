//
// Created by 24137 on 2021/1/25.
//

#include <cstring>
#include <malloc.h>
#include "AudioChannel.h"
#include "log.h"

extern "C" {
#include  "librtmp/rtmp.h"
}

void AudioChannel::init(int sampleRate, int channels) {
    this->channels = channels;
    //1.打开编码器，获取inputSamples和maxOutputBytes的值，用于后面编码
    //1：采样率；2：声道数；3：单次输入的样本数；4：输出数据最大字节数
    audioCodec = faacEncOpen(sampleRate, channels, &inputSamples, &maxOutputBytes);

    //2.设置编码器参数
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioCodec);
//    config->mpegVersion = MPEG2;
    config->mpegVersion = MPEG4;
    //lc 标准
    config->aacObjectType = LOW;
    //16位
    config->inputFormat = FAAC_INPUT_16BIT;
    // 编码出原始数据；0 = Raw; 1 = ADTS
    config->outputFormat = 0; //aac裸流
    //让配置生效
    faacEncSetConfiguration(audioCodec, config);

    //输出缓冲区 编码后的数据 用这个缓冲区来保存
//    buffer = new u_char[maxOutputBytes];
    buffer = static_cast<u_char *>(malloc(maxOutputBytes));
}

u_long AudioChannel::getSamples() {
    return inputSamples;
}

RTMPPacket *AudioChannel::getAudioConfig() {
    u_char *buf;
    u_long len;

    faacEncGetDecoderSpecificInfo(audioCodec, &buf, &len);

    RTMPPacket  *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, len + 2);

    packet->m_body[0] = 0xAF;
    packet->m_body[1] = 0x00;
    memcpy(&packet->m_body[2], buf, len);

    packet -> m_hasAbsTimestamp = 0;
    packet -> m_nBodySize = len + 2;
    packet -> m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet -> m_nChannel = 0x11;
    packet -> m_headerType = RTMP_PACKET_SIZE_LARGE;

    return packet;
}

void AudioChannel::encodeData(int8_t *data) {
    //3.进行编码
    //1：FAAC的handle；2：采集的pcm的原始数据；3：从faacEncOpen获取的inputSamples；4：至少有从faacEncOpen获取maxOutputBytes大小的缓冲区；5：从faacEncOpen获取maxOutputBytes
    //返回值为编码后数据字节的长度
    LOGI("encodeData 开始编码音频数据");
    int bytelen = faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputSamples, buffer,
                                maxOutputBytes);

    if (bytelen > 0) {
        int bodySize = 2 + bytelen;
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet, bodySize);
        if (channels == 1) {
            packet->m_body[0] = 0xAE;   //单声道
        } else {
            packet->m_body[0] = 0xAF;    //双声道
        }
        //编码出的声音 都是 0x01
        packet->m_body[1] = 0x01;
        //音频数据
        memcpy(&packet->m_body[2], buffer, bytelen);

        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        if(callback){
            LOGI("发送数据");
            callback(packet);
        }
    }
}

void AudioChannel::setAudioCallback(AudioCallback callback) {
    this->callback = callback;
}