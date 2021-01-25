//
// Created by 24137 on 2021/1/25.
//

#ifndef RTMPLEARN_AUDIOCHANNEL_H
#define RTMPLEARN_AUDIOCHANNEL_H

#include <sys/types.h>
#include <faac.h>
#include <faaccfg.h>

extern "C" {
#include  "librtmp/rtmp.h"
}

class AudioChannel {
    typedef void (*AudioCallback)(RTMPPacket* packet);
public:
    void init(int sampleRate, int channels);
    void setAudioCallback(AudioCallback callback);
    u_long getSamples();
    RTMPPacket* getAudioConfig();
    void encodeData(int8_t *data);

private:
    AudioCallback callback;
    u_long inputSamples;
    faacEncHandle audioCodec = 0;
    u_long maxOutputBytes;
    int channels;
    u_char *buffer = 0;
};


#endif //RTMPLEARN_AUDIOCHANNEL_H
