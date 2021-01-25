package com.example.rtmplearn;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import com.example.rtmplearn.util.FileUtils;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {

    private AudioRecord audioRecord;
    private int minBufferSize = 0;
    private final ExecutorService executor;
    private boolean isPushing = false;
    private LivePushInterface livePushInterface;

    public AudioChannel(LivePushInterface livePushInterface, int sampleRate){

        this.livePushInterface = livePushInterface;

        executor = Executors.newSingleThreadExecutor();

        minBufferSize = AudioRecord.getMinBufferSize(44100,
                AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        audioRecord = new AudioRecord(
                MediaRecorder.AudioSource.MIC, 44100,
                AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT, minBufferSize);
    }

    public void stopLive() {
        isPushing = false;
        if(audioRecord != null){
            audioRecord.stop();
        }
    }

    public void startLive() {
        //录音
        isPushing = true;
        executor.execute(new RecordingTask());
    }

    class RecordingTask implements Runnable {

        @Override
        public void run() {
            //启动录音机
            audioRecord.startRecording();
            byte[] bytes = new byte[minBufferSize];
            while (isPushing) {
                int len = audioRecord.read(bytes, 0, bytes.length);
                if (len > 0) {
                    if(livePushInterface != null){
//                        FileUtils.byteToString(bytes);
//                        livePushInterface.audioPush(bytes);
                    }
                }
            }
        }
    }
}
