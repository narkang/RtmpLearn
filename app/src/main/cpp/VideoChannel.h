//
// Created by 24137 on 2021/1/23.
//

#ifndef RTMPLEARN_VIDEOCHANNEL_H
#define RTMPLEARN_VIDEOCHANNEL_H

#include <jni.h>
#include <x264.h>
#include "JavaCallHelper.h"

class VideoChannel {
public:
    VideoChannel();
    ~VideoChannel();

    //创建x264编码器
    void setVideoEncInfo(int width, int height, int fps, int bitrate);
    //真正开始编码一帧数据
    void encodeData(int8_t *data);

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
};


#endif //RTMPLEARN_VIDEOCHANNEL_H
