//
// Created by 24137 on 2021/1/23.
//

#ifndef RTMPLEARN_VIDEOCHANNEL_H
#define RTMPLEARN_VIDEOCHANNEL_H

#include <jni.h>
#include <x264.h>
#include "JavaCallHelper.h"
extern "C" {
#include  "librtmp/rtmp.h"
}

class VideoChannel {
    typedef void (*VideoCallback)(RTMPPacket *packet);
public:
    VideoChannel();
    ~VideoChannel();

    //创建x264编码器
    void setVideoEncInfo(int width, int height, int fps, int bitrate);
    //真正开始编码一帧数据
    void encodeData(int8_t *data);
    //sps和pps
    void sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len);
    //发送关键帧和非关键帧
    void sendFrame(int type, int payload, uint8_t *p_payload);
    //设置回调接口
    void setVideoCallback(VideoCallback callback);

public:
    JavaCallHelper *javaCallHelper;

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;

    x264_picture_t  *pic_in = 0;
    int ySize;
    int uvSize;

    x264_t *videoCodec = 0;

    VideoCallback callback;
};


#endif //RTMPLEARN_VIDEOCHANNEL_H
