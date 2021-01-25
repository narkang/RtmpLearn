//
// Created by 24137 on 2021/1/24.
//

#include "JavaCallHelper.h"
#include "log.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj) : javaVM(_javaVM),
                                                                                env(_env) {
    //將局部变量保存为全局变量
    jobj = env->NewGlobalRef(_jobj);
    jclass jclazz = env->GetObjectClass(jobj);
    //获取java层的方法签名
    jmid_postData = env->GetMethodID(jclazz, "postData", "([B)V");
}

void JavaCallHelper::postH264(char *data, int length, int thread) {
    //数据放到byteArray中
    jbyteArray array = env->NewByteArray(length);
    env->SetByteArrayRegion(array, 0, length, reinterpret_cast<const jbyte *>(data));
    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv;
        //子线程需要先绑定jniEnv
        LOGI("postH264 AttachCurrentThread");
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }

        jniEnv->CallVoidMethod(jobj, jmid_postData, array);
        LOGI("postH264 DetachCurrentThread");
        javaVM->DetachCurrentThread();
    } else {
        //主线程不需要绑定
        env->CallVoidMethod(jobj, jmid_postData, array);
    }

}

JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(jobj);
    jobj = 0;
}