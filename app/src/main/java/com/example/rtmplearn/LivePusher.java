package com.example.rtmplearn;

import android.util.Log;
import android.view.TextureView;

import androidx.lifecycle.LifecycleOwner;

import com.example.rtmplearn.util.FileUtils;

public class LivePusher {

    static {
        System.loadLibrary("native-lib");
    }

    private AudioChannel audioChannel;
    private VideoChannel videoChannel;

    public LivePusher(LifecycleOwner lifecycleOwner, int width, int height, int bitrate, int sampleRate,
                      int fps, TextureView textureView) {
        native_init(sampleRate, 2);
        videoChannel = new VideoChannel(lifecycleOwner, textureView, width, height, bitrate, fps);

        videoChannel.setLivePushInterface(livePushInterface);

        audioChannel = new AudioChannel(livePushInterface, sampleRate, 2);
    }

    private LivePushInterface livePushInterface = new LivePushInterface() {
        @Override
        public void setVideoEncInfo(int width, int height, int fps, int bitrate) {
            native_setVideoEncInfo(width, height, fps, bitrate);
        }

        @Override
        public void pushVideo(byte[] data) {
            native_pushVideo(data);
        }

        @Override
        public int audioGetSamples() {
            return native_audioGetSamples();
        }

        @Override
        public void audioPush(byte[] bytes, int len) {
            native_audioPush(bytes, len);
        }
    };

    public void switchCamera() {
        videoChannel.switchCamera();
    }

    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();
    }

    public void stopLive(){
        native_stop();
        videoChannel.stopLive();
        audioChannel.stopLive();
    }

    //jni回调java层的方法
    private void postH264Data(byte[] data) {

        Log.e("ruby", "postH264Data 编码好的视频数据");
        FileUtils.byteToString(data);

    }

    private void postAACData(byte[] data) {

        Log.e("ruby", "postAACData 编码好的音频数据");
        FileUtils.byteToString(data);

    }

    public native void native_init(int sampleRate, int channels);

    public native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);

    public native void native_start(String path);

    public native void native_pushVideo(byte[] data);

    public native int native_audioGetSamples();

    public native void native_audioPush(byte[] bytes, int len);

    public native void native_stop();

    public native void native_release();
}

