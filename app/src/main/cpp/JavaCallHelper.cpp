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
        //子线程需要先绑定jniEnv
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }

        //数据放到byteArray中
        jbyteArray array = jniEnv->NewByteArray(length);
        jniEnv->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));

        jniEnv->CallVoidMethod(jobj, jmid_postH264Data, array);
        jniEnv->DeleteLocalRef(array);
//        javaVM->DetachCurrentThread();
    } else {

        jbyteArray array = env->NewByteArray(length);
        env->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));

        //主线程不需要绑定
        env->CallVoidMethod(jobj, jmid_postH264Data, array);
    }

}

void JavaCallHelper::postAAC(u_char *data, int length, int thread) {

    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv;
        //子线程需要先绑定jniEnv
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }

        //数据放到byteArray中
        jbyteArray array = jniEnv->NewByteArray(length);
        jniEnv->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));

        jniEnv->CallVoidMethod(jobj, jmid_postAACData, array);
        jniEnv->DeleteLocalRef(array);

//        javaVM->DetachCurrentThread();
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