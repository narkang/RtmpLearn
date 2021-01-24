//
// Created by 24137 on 2021/1/23.
//

#include <cstring>
#include "VideoChannel.h"
#include "log.h"

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {

    LOGI("width = %d , height = %d", width, height);

    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;

    ySize = width * height;
    uvSize = ySize / 4;
    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }

    //定义参数
    x264_param_t param;
    //参数赋值
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    //编码等级
    param.i_level_idc = 32;
    //选取显示格式
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = width;
    //B帧
    param.i_bframe = 0;
    //折中    cpu   突发情况   ABR 平均
    param.rc.i_rc_method = X264_RC_ABR;
    //k为单位
    param.rc.i_bitrate = bitrate / 1024;
    //帧率   1s/25帧     40ms  视频 编码      帧时间 ms存储  us   s
    param.i_fps_num = fps;
    //    帧率 时间  分子  分母
    param.i_fps_den = 1;
    //    分母
    param.i_timebase_den = param.i_fps_num;
    //    分子
    param.i_timebase_num = param.i_fps_den;
    //用fps而不是时间戳来计算帧间距离
    param.b_vfr_input = 0;
    //I帧间隔     2s  15*2
    param.i_keyint_max = fps * 2;

    // 是否复制sps和pps放在每个关键帧的前面 该参数设置是让每个关键帧(I帧)都附带sps/pps。
    param.b_repeat_headers = 1;
    //多线程
    param.i_threads = 1;
    x264_param_apply_profile(&param, "baseline");
    //打开编码器
    videoCodec = x264_encoder_open(&param);
    LOGI("videoCodec = %d", videoCodec);

    //容器
    pic_in = new x264_picture_t;
    //设置初始化大小  容器大小就确定的
    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);
}

void VideoChannel::encodeData(int8_t *data) {
    memcpy(pic_in->img.plane[0], data, ySize);
    for (int i = 0; i < uvSize; ++i) {
        //间隔1个字节取一个数据
        //u数据
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2 + 1);
        //v数据
        *(pic_in->img.plane[2] + i) = *(data + ySize + i * 2);
    }
    //编码成H264码流
    //编码出了帧数
    int pi_nal;
    //编码出的数据
    x264_nal_t *pp_nals;
    x264_picture_t pic_out;

    x264_encoder_encode(videoCodec, &pp_nals, &pi_nal, pic_in, &pic_out);
    LOGI("编码出来的帧数 %d", pi_nal);  //sps和pps是单独编码为一帧
    if (pi_nal > 0) {
        for (int i = 0; i < pi_nal; ++i) {
            //p_payload 编码好的数据 i_payload 编码出来一帧数据长度
            LOGI("输出索引:  %d  输出长度 %d", i, pi_nal);
            javaCallHelper->postH264(reinterpret_cast<char *>(pp_nals[i].p_payload),
                                     pp_nals[i].i_payload, THREAD_MAIN);
        }
    }
}

VideoChannel::VideoChannel() {

}

VideoChannel::~VideoChannel() {

}
