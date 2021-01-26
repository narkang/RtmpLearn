package com.example.rtmplearn;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

public class AudioChannel {

    private AudioRecord audioRecord;
    private int minBufferSize = 0;
    private boolean isPushing = false;
    private LivePushInterface livePushInterface;
    private int channelConfig;
    private Handler handler;
    private HandlerThread handlerThread;

    public AudioChannel(LivePushInterface livePushInterface, int sampleRate, int channels){

        this.livePushInterface = livePushInterface;

        channelConfig = channels == 2 ? AudioFormat.CHANNEL_IN_STEREO :
                AudioFormat.CHANNEL_IN_MONO;

        minBufferSize = AudioRecord.getMinBufferSize(sampleRate,
                channelConfig,
                AudioFormat.ENCODING_PCM_16BIT);

        handlerThread = new HandlerThread("Audio-Record");
        handlerThread.start();
        handler = new Handler(handlerThread.getLooper());

        int audioGetSamples = livePushInterface.audioGetSamples() * 2;
        minBufferSize = audioGetSamples > minBufferSize? audioGetSamples:minBufferSize;
        Log.i("ruby", "audioGetSamples: " + audioGetSamples + "  minBufferSize: " + minBufferSize);

        audioRecord = new AudioRecord(
                MediaRecorder.AudioSource.MIC, sampleRate,
                channelConfig,
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
        handler.post(new RecordingTask());
    }

    class RecordingTask implements Runnable {

        @Override
        public void run() {
            //启动录音机
            audioRecord.startRecording();
            byte[] bytes = new byte[livePushInterface.audioGetSamples() * 2];
            while (isPushing) {
                int len = audioRecord.read(bytes, 0, bytes.length);
                if (len > 0) {
                    if(livePushInterface != null){
                        //一个样本数 两个字节
                        livePushInterface.audioPush(bytes, len>>1);
                    }
                }
            }
        }
    }
}
