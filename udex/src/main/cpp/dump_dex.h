//
// Created by thehepta on 2024/2/22.
//

#pragma once

jobjectArray getClassLoaderList(JNIEnv *env, jclass thiz);
jobjectArray getBaseDexClassLoaderList(JNIEnv *env, jclass thiz);
jobject dumpDexBuffListByCookie(JNIEnv *env, jclass clazz, jlongArray cookie);
void dumpDexToLocalByCookie(JNIEnv *env, jclass thiz, jlongArray cookie, jstring jdumpDir);
void dumpMethods(JNIEnv *env, jclass clazz, jobjectArray methods, jobjectArray constructors, jstring jdumpDir);
jbyteArray dumpMethodByMember(JNIEnv *env, jclass cls, jobject method);
jbyteArray dumpMethodByString(JNIEnv *env, jclass thiz, jclass cls, jstring method_name, jstring method_sign);