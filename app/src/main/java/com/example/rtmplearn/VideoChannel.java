package com.example.rtmplearn;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.media.MediaCodec;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.TextureView;
import android.view.ViewGroup;

import androidx.camera.core.CameraX;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageAnalysisConfig;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.core.PreviewConfig;
import androidx.lifecycle.LifecycleOwner;

import com.example.rtmplearn.util.ImageUtil;

import java.util.concurrent.locks.ReentrantLock;

public class VideoChannel implements Preview.OnPreviewOutputUpdateListener, ImageAnalysis.Analyzer{
    private int width;
    private int height;

    private HandlerThread handlerThread;
    private CameraX.LensFacing currentFacing = CameraX.LensFacing.BACK;
    private TextureView textureView;
    private boolean isLiving;
    private int bitrate;
    private int fps;
    private LifecycleOwner lifecycleOwner;

    public VideoChannel(LifecycleOwner lifecycleOwner, TextureView textureView, int width, int height, int bitrate,
                        int fps) {
        this.textureView = textureView;
        this.lifecycleOwner = lifecycleOwner;
        //子线程中回调
        handlerThread = new HandlerThread("Analyze-thread");
        handlerThread.start();
        CameraX.bindToLifecycle(lifecycleOwner, getPreView(currentFacing), getAnalysis());

        this.width = width;
        this.height = height;
        this.bitrate = bitrate;
        this.fps = fps;
    }

    public void switchCamera(){
        if(currentFacing == CameraX.LensFacing.BACK){
            currentFacing = CameraX.LensFacing.FRONT;
        }else {
            currentFacing = CameraX.LensFacing.BACK;
        }
        CameraX.unbindAll();
        CameraX.bindToLifecycle(lifecycleOwner, getPreView(currentFacing), getAnalysis());
    }

    private Preview getPreView(CameraX.LensFacing currentFacing) {
        PreviewConfig previewConfig = new PreviewConfig.Builder().setTargetResolution(new Size(width, height)).setLensFacing(currentFacing).build();
        Preview preview = new Preview(previewConfig);
        preview.setOnPreviewOutputUpdateListener(this);
        return preview;
    }

    private ImageAnalysis getAnalysis() {
        ImageAnalysisConfig imageAnalysisConfig = new ImageAnalysisConfig.Builder()
                .setCallbackHandler(new Handler(handlerThread.getLooper()))
                .setLensFacing(currentFacing)
                .setImageReaderMode(ImageAnalysis.ImageReaderMode.ACQUIRE_LATEST_IMAGE)
                .setTargetResolution(new Size(width, height))
                .build();

        ImageAnalysis imageAnalysis = new ImageAnalysis(imageAnalysisConfig);
        imageAnalysis.setAnalyzer(this);
        return imageAnalysis;
    }

    public void startLive() {
        isLiving = true;
    }
    public void stopLive() {
        isLiving = false;
    }

    private ReentrantLock lock = new ReentrantLock();
    private byte[] y;
    private byte[] u;
    private byte[] v;
    private MediaCodec mediaCodec;
    private byte[] nv21;
    byte[] nv21_rotated;
    byte[] nv12;
    @Override
    public void analyze(ImageProxy image, int rotationDegrees) {
        if (!isLiving) {
            return;
        }
//        Log.i("ruby", "analyze: " + image.getWidth() + "  height " + image.getHeight());

        lock.lock();
        ImageProxy.PlaneProxy[] planes =  image.getPlanes();
        //重复使用同一批byte数组，减少gc频率
        if (y == null) {
//            初始化y v  u
            y = new byte[planes[0].getBuffer().limit() - planes[0].getBuffer().position()];
            u = new byte[planes[1].getBuffer().limit() - planes[1].getBuffer().position()];
            v = new byte[planes[2].getBuffer().limit() - planes[2].getBuffer().position()];
            if(livePushInterface != null){
                livePushInterface.setVideoEncInfo(image.getHeight(), image.getWidth(), fps, bitrate);
            }
        }

        if (image.getPlanes()[0].getBuffer().remaining() == y.length) {
            planes[0].getBuffer().get(y);
            planes[1].getBuffer().get(u);
            planes[2].getBuffer().get(v);
            int stride = planes[0].getRowStride();
            Size size = new Size(image.getWidth(), image.getHeight());
            int width = size.getHeight();
            int heigth = image.getWidth();
            if (nv21 == null) {
                nv21 = new byte[heigth * width * 3 / 2];
                nv21_rotated = new byte[heigth * width * 3 / 2];
            }
            ImageUtil.yuvToNv21(y, u, v, nv21, heigth, width);
            ImageUtil.nv21_rotate_to_90(nv21, nv21_rotated, heigth, width);
            if(livePushInterface != null){
                livePushInterface.pushVideo(nv21_rotated);
            }
        }

        lock.unlock();
    }

    @Override
    public void onUpdated(Preview.PreviewOutput output) {
        SurfaceTexture surfaceTexture = output.getSurfaceTexture();
        if (textureView.getSurfaceTexture() != surfaceTexture) {
            if (textureView.isAvailable()) {
                // 当切换摄像头时，会报错
                ViewGroup parent = (ViewGroup) textureView.getParent();
                parent.removeView(textureView);
                parent.addView(textureView, 0);
                parent.requestLayout();
            }
            textureView.setSurfaceTexture(surfaceTexture);
        }
    }

    private LivePushInterface livePushInterface;

    public void setLivePushInterface(LivePushInterface livePushInterface){
        this.livePushInterface = livePushInterface;
    }

    public interface LivePushInterface{

        void setVideoEncInfo(int width, int height, int fps, int bitrate);
        void pushVideo(byte[] data);

    }
}
