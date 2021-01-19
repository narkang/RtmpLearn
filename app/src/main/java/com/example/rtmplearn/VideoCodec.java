package com.example.rtmplearn;

import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.projection.MediaProjection;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;

import com.example.rtmplearn.util.FileUtils;

import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.locks.ReentrantLock;

public class VideoCodec extends Thread {
    //    录屏工具类
    private MediaProjection mediaProjection;
    //    虚拟的画布
    private VirtualDisplay virtualDisplay;

    private MediaCodec mediaCodec;
    //传输层的引用
    private ScreenLive screenLive;

    public VideoCodec(ScreenLive screenLive) {
        this.screenLive = screenLive;
    }

    //    每一帧编码时间
    private long timeStamp;
    private long startTime;
    //    编码
    private boolean isLiving;

    @Override
    public void run() {
        isLiving = true;
        mediaCodec.start();
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
//        手动触发I帧
        while (isLiving) {
            if (System.currentTimeMillis() - timeStamp >= 2000) {
                Bundle params = new Bundle();
                params.putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0);
//                dsp 芯片触发I帧
                mediaCodec.setParameters(params);
                timeStamp = System.currentTimeMillis();
            }
            int index = mediaCodec.dequeueOutputBuffer(bufferInfo, 100000);
            if (index >= 0) {
                if (startTime == 0) {
//                    毫秒
                    startTime = bufferInfo.presentationTimeUs / 1000;
                }
                ByteBuffer buffer = mediaCodec.getOutputBuffer(index);
                byte[] outData = new byte[bufferInfo.size];
                buffer.get(outData);
                RTMPPackage rtmpPackage = new RTMPPackage(outData, (bufferInfo.presentationTimeUs / 1000) - startTime);
                screenLive.addPackage(rtmpPackage);
                mediaCodec.releaseOutputBuffer(index, false);
            }
        }
        isLiving = false;
        mediaCodec.stop();
        mediaCodec.release();
        mediaCodec = null;
        virtualDisplay.release();
        virtualDisplay = null;
        mediaProjection.stop();
        mediaProjection = null;
        startTime = 0;
    }

    public void startLive(MediaProjection mediaProjection) {
        this.mediaProjection = mediaProjection;
        MediaFormat format = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC,
                720,
                1280);

        format.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);

        format.setInteger(MediaFormat.KEY_BIT_RATE, 400_000);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 15);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);

        try {
            mediaCodec = MediaCodec.createEncoderByType("video/avc");
            mediaCodec.configure(format, null, null,
                    MediaCodec.CONFIGURE_FLAG_ENCODE);
            Surface surface = mediaCodec.createInputSurface();
            virtualDisplay = mediaProjection.createVirtualDisplay(
                    "screen-codec",
                    720, 1280, 1,
                    DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC,
                    surface, null, null);

        } catch (IOException e) {
            e.printStackTrace();
        }
        start();
    }

}

