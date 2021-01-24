//
// Created by 24137 on 2021/1/24.
//

#ifndef RTMPLEARN_JAVACALLHELPER_H
#define RTMPLEARN_JAVACALLHELPER_H

#include <jni.h>

//标记线程 因为子线程需要attach
#define THREAD_MAIN 1
#define THREAD_CHILD 2

class JavaCallHelper {

public:
    JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_obj);

    void postH264(char *data,int length, int thread = THREAD_MAIN);

public:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject jobj;
    jmethodID jmid_postData;

};


#endif //RTMPLEARN_JAVACALLHELPER_H
