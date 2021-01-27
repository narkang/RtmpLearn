#include <jni.h>
#include <string>
#include <pthread.h>
#include "safe_queue.h"
#include "log.h"

extern "C" {
#include  "librtmp/rtmp.h"
}

#include "VideoChannel.h"
#include "JavaCallHelper.h"
#include "AudioChannel.h"

VideoChannel *videoChannel = 0;
AudioChannel *audioChannel = 0;
JavaCallHelper *helper = 0;
//记录子线程的对象
pthread_t pid;
int isStart = 0;
//推流标志位
int readyPushing = 0;
//阻塞式队列
SafeQueue<RTMPPacket *> packets;

uint32_t start_time;
//虚拟机的引用
JavaVM *javaVM = 0;
RTMP *rtmp = 0;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    LOGI("保存虚拟机的引用");
    return JNI_VERSION_1_6;
}

void releasePackets(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

//编码层的回调
void callback(RTMPPacket *packet) {
    if (packet) {

        //避免oom
        if(packets.size() > 50){
            packets.clear();
        }

        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        packets.push(packet);
    }
}

void *start(void *arg) {
    char *url = static_cast<char *>(arg);
    do {
        rtmp = RTMP_Alloc();
        if (!rtmp) {
            LOGI("rtmp创建失败");
            break;
        }
        RTMP_Init(rtmp);
        //设置超时时间
        rtmp->Link.timeout = 10;
        int ret = RTMP_SetupURL(rtmp, (char *) url);
        if (!ret) {
            LOGI("rtmp设置地址失败:%s", url);
            break;
        }
        //开启输出模式
        RTMP_EnableWrite(rtmp);
        ret = RTMP_Connect(rtmp, 0);
        if (!ret) {
            LOGI("rtmp连接地址失败:%s", url);
            break;
        }
        ret = RTMP_ConnectStream(rtmp, 0);
        if (!ret) {
            LOGI("连接失败");
            break;
        }
        LOGI("连接成功");
        //连接成功，不让重复了
        isStart = 1;

        //准备好了，可以开始推流了
        readyPushing = 1;
        //记录一个开始时间
        start_time = RTMP_GetTime();
        packets.setWork(1);

        callback(audioChannel->getAudioConfig());

        RTMPPacket *packet = 0;
        //循环从队列取包 然后发送
        while (isStart) {
            packets.pop(packet);
            if (!isStart) {
                break;
            }
            if (!packet) {
                continue;
            }
            // 给rtmp的流id
            packet->m_nInfoField2 = rtmp->m_stream_id;
            //发送包 1:加入队列发送
            ret = RTMP_SendPacket(rtmp, packet, 1);
            releasePackets(packet);
            if (!ret) {
                LOGI("发送数据失败");
                break;
            }
        }
        releasePackets(packet);
    } while (0);
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete url;
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmplearn_LivePusher_native_1init(JNIEnv *env, jobject thiz, jint sampleRate, jint channels) {
    helper = new JavaCallHelper(javaVM, env, thiz);
    //实例化编码层
    videoChannel = new VideoChannel;
    videoChannel->javaCallHelper = helper;

    audioChannel = new AudioChannel;
    audioChannel->init(sampleRate, channels);
    audioChannel->javaCallHelper = helper;
    //设置回调
    videoChannel->setVideoCallback(callback);
    audioChannel->setAudioCallback(callback);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmplearn_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject thiz, jint width,
                                                              jint height, jint fps, jint bitrate) {
    if (videoChannel) {
        videoChannel->setVideoEncInfo(width, height, fps, bitrate);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmplearn_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path_) {

    //避免重复连接
    if (isStart) {
        return;
    }

    //传NULL和0都是一样的，此时是一个引用，如果传非0，内部会malloc，此时需要手动释放
    const char *path = env->GetStringUTFChars(path_, 0);
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);

    //开始直播
//    isStart = 1;
    //开子线程链接服务器
    pthread_create(&pid, 0, start, url);
    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmplearn_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz,
                                                        jbyteArray data_) {
    if (!videoChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    videoChannel->encodeData(data);
    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_rtmplearn_LivePusher_native_1audioGetSamples(JNIEnv *env, jobject thiz) {
    if(audioChannel){
        return audioChannel->getSamples();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmplearn_LivePusher_native_1audioPush(JNIEnv *env, jobject thiz,
                                                        jbyteArray bytes, jint len) {
    if (!audioChannel || !readyPushing) {
        return;
    }
    if(audioChannel){
        jbyte *data = env->GetByteArrayElements(bytes, NULL);
        audioChannel->encodeData(data, len);
        env->ReleaseByteArrayElements(bytes, data, 0);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmplearn_LivePusher_native_1stop(JNIEnv *env, jobject thiz) {

    isStart = 0;
    readyPushing = 0;

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_rtmplearn_LivePusher_native_1release(JNIEnv *env, jobject thiz) {

    isStart = 0;
    readyPushing = 0;

    if(rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp = 0;
    }
    if (videoChannel) {
        delete (videoChannel);
        videoChannel = 0;
    }
    if(audioChannel){
        delete (audioChannel);
        audioChannel = 0;
    }
    if (helper) {
        delete (helper);
        helper = 0;
    }

}