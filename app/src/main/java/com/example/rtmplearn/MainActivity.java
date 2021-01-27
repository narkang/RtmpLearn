package com.example.rtmplearn;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.widget.TextView;

//https://github.com/xhunmon/RtmpPush
//https://glumes.com/post/android/android-jni-thread-operation/
public class MainActivity extends AppCompatActivity {

    private LivePusher livePusher;
//    private String url = "rtmp://192.168.182.128:1935/live/ls";
    private String url = "rtmp://live-push.bilivideo.com/live-bvc/?streamname=live_406458993_69012454&key=6396df2a535c62d394de490738092ecf&schedule=rtmp";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextureView textureView = findViewById(R.id.textureView);
        checkPermission();
        livePusher = new LivePusher(this, 480, 640, 800_000, 44100, 10, textureView);
    }

    public boolean checkPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                || checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                || checkSelfPermission(Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED
                || checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.CAMERA,
                    Manifest.permission.RECORD_AUDIO
            }, 1);

        }
        return false;
    }

    public void switchCamera(View view) {
        livePusher.switchCamera();
    }

    public void startLive(View view) {
        livePusher.startLive(url);
    }

    public void stopLive(View view) {
        livePusher.stopLive();
    }

}
