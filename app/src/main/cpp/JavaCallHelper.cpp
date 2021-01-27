//
// Created by 24137 on 2021/1/24.
//

#include "JavaCallHelper.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj) : javaVM(_javaVM),env(_env) {
    //將局部变量保存为全局变量
    jobj = env->NewGlobalRef(_jobj);
    jclass jclazz = env->GetObjectClass(jobj);
    //获取java层的方法签名
    jmid_postH264Data = env->GetMethodID(jclazz, "postH264Data", "([B)V");
    jmid_postAACData = env->GetMethodID(jclazz, "postAACData", "([B)V");
}

void JavaCallHelper::postH264(char *data, int length, int thread) {

    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv;
        JavaVMAttachArgs jvmArgs;
        jvmArgs.version = JNI_VERSION_1_6;
        //子线程需要先绑定jniEnv
        int attachedHere = 0;
        jint res = javaVM->GetEnv((void**)&jniEnv, JNI_VERSION_1_6); //checks if current env needs attaching or it is already attached
        LOGI("postAAC res = %d", res);
        if(JNI_EDETACHED == res){
            // Supported but not attached yet, needs to call AttachCurrentThread
            if (javaVM->AttachCurrentThread(&jniEnv, &jvmArgs) != JNI_OK) {
                return;
            }
            LOGI("postAAC 1");
            attachedHere = 1;
        }
        else if(JNI_OK == res){
            // Current thread already attached, do not attach 'again' (just to save the attachedHere flag)
            // We make sure to keep attachedHere = 0
            LOGI("postAAC 2");
        }else{
            // JNI_EVERSION, specified version is not supported cancel this..
            LOGI("postAAC 3");
            return;
        }

        //数据放到byteArray中
        jbyteArray array = jniEnv->NewByteArray(length);
        jniEnv->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));

        jniEnv->CallVoidMethod(jobj, jmid_postH264Data, array);
        jniEnv->DeleteLocalRef(array);
        LOGI("postAAC 4");

        if(attachedHere){
            LOGI("postAAC 5");
            javaVM->DetachCurrentThread(); // Done only when attachment was done here
        }
    } else {

        jbyteArray array = env->NewByteArray(length);
        env->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));

        //主线程不需要绑定
        env->CallVoidMethod(jobj, jmid_postH264Data, array);
    }

}

//https://cloud.tencent.com/developer/ask/170239
void JavaCallHelper::postAAC(u_char *data, int length, int thread) {

    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv;
        JavaVMAttachArgs jvmArgs;
        jvmArgs.version = JNI_VERSION_1_6;
        //子线程需要先绑定jniEnv
        int attachedHere = 0;
        jint res = javaVM->GetEnv((void**)&jniEnv, JNI_VERSION_1_6); //checks if current env needs attaching or it is already attached
        LOGI("postAAC res = %d", res);
        if(JNI_EDETACHED == res){
            // Supported but not attached yet, needs to call AttachCurrentThread
            if (javaVM->AttachCurrentThread(&jniEnv, &jvmArgs) != JNI_OK) {
                return;
            }
            LOGI("postAAC 1");
            attachedHere = 1;
        }
        else if(JNI_OK == res){
            // Current thread already attached, do not attach 'again' (just to save the attachedHere flag)
            // We make sure to keep attachedHere = 0
            LOGI("postAAC 2");
        }else{
            // JNI_EVERSION, specified version is not supported cancel this..
            LOGI("postAAC 3");
            return;
        }

        //数据放到byteArray中
        jbyteArray array = jniEnv->NewByteArray(length);
        jniEnv->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));

        jniEnv->CallVoidMethod(jobj, jmid_postAACData, array);
        jniEnv->DeleteLocalRef(array);
        LOGI("postAAC 4");

        if(attachedHere){
            LOGI("postAAC 5");
            javaVM->DetachCurrentThread(); // Done only when attachment was done here
        }
    } else {
        //主线程不需要绑定
        jbyteArray array = env->NewByteArray(length);
        env->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));
        env->CallVoidMethod(jobj, jmid_postAACData, array);
    }
}

JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(jobj);
    jobj = 0;
}