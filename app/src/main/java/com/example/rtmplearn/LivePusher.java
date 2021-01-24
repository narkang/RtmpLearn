package com.example.rtmplearn;

import android.app.Activity;
import android.util.Log;
import android.view.SurfaceHolder;

import com.example.rtmplearn.util.FileUtils;

import java.io.File;

public class LivePusher {

    static {
        System.loadLibrary("native-lib");
    }

    private AudioChannel audioChannel;
    private VideoChannel videoChannel;

    public LivePusher(Activity activity, int width, int height, int bitrate,
                      int fps, int cameraId) {
        native_init();
        videoChannel = new VideoChannel(this,activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel();
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }

    public void switchCamera() {
        videoChannel.switchCamera();
    }
    private void onPrepare(boolean isConnect) {
        //通知UI
    }

    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();
    }

    public void stopLive(){
        videoChannel.stopLive();
        audioChannel.stopLive();
        native_stop();
    }

    //jni回调java层的方法
    private void postData(byte[] data) {

        Log.e("ruby", "java端收到数据：" + FileUtils.byteToString(data));
//        Log.e("ruby", FileUtils.byteToString(data));

    }

    public native void native_init();

    public native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);

    public native void native_start(String path);

    public native void native_pushVideo(byte[] data);

    public native void native_stop();

    public native void native_release();
}

