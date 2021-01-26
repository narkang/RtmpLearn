package com.example.rtmplearn;

public interface LivePushInterface{

    void setVideoEncInfo(int width, int height, int fps, int bitrate);
    void pushVideo(byte[] data);
    int audioGetSamples();
    void audioPush(byte[] bytes, int len);
}