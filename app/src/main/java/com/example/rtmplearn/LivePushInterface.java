package com.example.rtmplearn;

public interface LivePushInterface{

    void setVideoEncInfo(int width, int height, int fps, int bitrate);
    void pushVideo(byte[] data);
    int audioGetSamples();  //获取样本数
    void audioPush(byte[] bytes, int len);
}