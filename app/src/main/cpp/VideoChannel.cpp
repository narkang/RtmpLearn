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

    uint8_t sps[100];
    uint8_t pps[100];
    int sps_len, pps_len;

    if (pi_nal > 0) {
        for (int i = 0; i < pi_nal; ++i) {
            //p_payload 编码好的数据 i_payload 编码出来一帧数据长度
//            javaCallHelper->postH264(reinterpret_cast<char *>(pp_nals[i].p_payload),
//                                     pp_nals[i].i_payload, THREAD_CHILD);

            if (pp_nals[i].i_type == NAL_SPS) {
                sps_len = pp_nals[i].i_payload - 4;
                memcpy(sps, pp_nals[i].p_payload + 4, sps_len);
            } else if (pp_nals[i].i_type == NAL_PPS) {
                pps_len = pp_nals[i].i_payload - 4;
                memcpy(pps, pp_nals[i].p_payload + 4, pps_len);
                sendSpsPps(sps, pps, sps_len, pps_len);
            } else{
                //关键帧和非关键帧
                sendFrame(pp_nals[i].i_type, pp_nals[i].i_payload, pp_nals[i].p_payload);
            }
        }
    }
}

void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {

    RTMPPacket *packet = new RTMPPacket;
    int bodysize = 13 + sps_len + 3 + pps_len;
    RTMPPacket_Alloc(packet, bodysize);
    int i = 0;
    //固定头
    packet->m_body[i++] = 0x17;
    //类型
    packet->m_body[i++] = 0x00;
    //composition time 0x000000
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    //版本
    packet->m_body[i++] = 0x01;
    //编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    packet->m_body[i++] = 0xFF;

    //整个sps
    packet->m_body[i++] = 0xE1;
    //sps长度
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = sps_len & 0xff;
    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;

    //pps
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = (pps_len) & 0xff;
    memcpy(&packet->m_body[i], pps, pps_len);


    //视频
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodysize;
    //随意分配一个管道（尽量避开rtmp.c中使用的）
    packet->m_nChannel = 10;
    //sps pps没有时间戳
    packet->m_nTimeStamp = 0;
    //不使用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

    if (this->callback) {
        this->callback(packet);
    }
}

void VideoChannel::sendFrame(int type, int payload, uint8_t *p_payload) {
    if (p_payload[2] == 0x00){
        payload -= 4;
        p_payload += 4;
    } else if(p_payload[2] == 0x01){
        payload -= 3;
        p_payload += 3;
    }
    RTMPPacket *packet = new RTMPPacket;
    int bodysize = 9 + payload;
    RTMPPacket_Alloc(packet, bodysize);
    RTMPPacket_Reset(packet);
//    int type = payload[0] & 0x1f;
    packet->m_body[0] = 0x27;
    //关键帧
    if (type == NAL_SLICE_IDR) {
        LOGI("音频关键帧");
        packet->m_body[0] = 0x17;
    }
    //类型
    packet->m_body[1] = 0x01;
    //时间戳
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;
    //数据长度 int 4个字节 相当于把int转成4个字节的byte数组
    packet->m_body[5] = (payload >> 24) & 0xff;
    packet->m_body[6] = (payload >> 16) & 0xff;
    packet->m_body[7] = (payload >> 8) & 0xff;
    packet->m_body[8] = (payload) & 0xff;

    //图片数据
    memcpy(&packet->m_body[9],p_payload,  payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodysize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

    if (this->callback) {
        this->callback(packet);
    }
}


void VideoChannel::setVideoCallback(VideoCallback callback) {

    this->callback = callback;

}

VideoChannel::VideoChannel() {

}

VideoChannel::~VideoChannel() {
    if(videoCodec){
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }
}


