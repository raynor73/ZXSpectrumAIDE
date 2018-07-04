/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <android/log.h>
#include <string>
#include <jni.h>
#include "Z80Exerciser.h"

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */
extern "C"
{
	static Z80Exerciser* g_z80Exerciser;
	
	JNIEXPORT jstring JNICALL Java_ru_ilapin_zxspectrum_HelloJni_stringFromJNI(JNIEnv* env, jobject thiz)
	{
		return env->NewStringUTF("Hello from JNI !");
	}
	
	JNIEXPORT void JNICALL Java_ru_ilapin_zxspectrum_Z80ExerciserActivity_initExerciser(JNIEnv *env, jobject instance, jbyteArray program_, jstring logFilePath_)
	{
		jbyte *program = env->GetByteArrayElements(program_, NULL);
		const char *logFilePath = env->GetStringUTFChars(logFilePath_, 0);

		std::string logFilePathString = logFilePath;
		g_z80Exerciser = new Z80Exerciser(logFilePathString);

		const int length = env->GetArrayLength(program_);
		uint8_t *memoryArray = g_z80Exerciser->memoryArray();
		for (int i = 0; i < length; i++) {
			memoryArray[i + 0x100] = uint8_t(program[i]);
		}

		env->ReleaseStringUTFChars(logFilePath_, logFilePath);
		env->ReleaseByteArrayElements(program_, program, 0);
	}

	
	
	JNIEXPORT void JNICALL Java_ru_ilapin_zxspectrum_Z80ExerciserActivity_runTest(JNIEnv *env, jobject instance)
	{
		try {
			/*uint8_t foo = 0;
			uint8_t bar = 0xff;
			uint16_t baz = foo - bar;
			__android_log_print(ANDROID_LOG_DEBUG, "!@#", "foo: %d", baz);*/
			g_z80Exerciser->runTest();
		} catch (std::exception &e) {
			__android_log_print(ANDROID_LOG_DEBUG, "!@#", "Exception: %s", e.what());
		}
	}
}
