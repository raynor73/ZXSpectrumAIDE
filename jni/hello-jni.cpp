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
#include <cstring>
#include <map>
#include <jni.h>
//#include <SLES/OpenSLES.h>
//#include <SLES/OpenSLES_Android.h>
#include "Z80Exerciser.h"
#include "ZxSpectrum.h"

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */
extern "C"
{

static Z80Exerciser* g_z80Exerciser;
static ZxSpectrum *g_zxSpectrum = nullptr;

//static bool g_isSoundDataQueueReferenceCreated = false;
static jobject g_soundDataQueue;
static jmethodID g_offerMethodId;
static JNIEnv *g_soundEnv;
	
static const uint16_t BASE_ADDRESS = 0x4000;
static const uint16_t SCREEN_WIDTH = 256;
static const uint16_t SCREEN_HEIGHT = 192;
static const uint16_t BITS_IN_BYTE = 8;
static const uint16_t MEMORY_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT / BITS_IN_BYTE;
static const uint16_t NUMBER_OF_ATTRIBUTE_COLUMNS = 32;
static const uint16_t FLASH_BIT_MASK = 0x80;
static const uint16_t INK_BITS_MASK = 0x07;
static const uint16_t PAPER_BITS_MASK = 0x38;
static const uint16_t BRIGHTNESS_BIT_MASK = 0x40;

/*static std::map<int, uint32_t> g_colorMap;
static std::map<int, uint32_t> g_brightColorMap;*/
static uint32_t g_colorMap[8];
static uint32_t g_brightColorMap[8];

static void renderZxSpectrumScreen(const uint8_t* memory, uint32_t* bitmap, const bool isFlash) {
	/*for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			bitmap[x + y * SCREEN_WIDTH] = isFlash ? 0xff00ff00 : 0xff008000;
		}
	}*/
	for (int i = 0; i < MEMORY_SIZE; i++) {
		const uint8_t videoByte = uint8_t(memory[BASE_ADDRESS + i]);
		const uint16_t attributeColumn = uint16_t(i % NUMBER_OF_ATTRIBUTE_COLUMNS);
		const uint16_t y = uint16_t((((i >> 11) & 0x03) << 6) | (((i >> 5) & 0x07) << 3) | ((i >> 8) & 0x07));
		const uint16_t attributeRow = y >> 3;
		for (int j = 0; j < BITS_IN_BYTE; j++) {
			const uint16_t videoBit = uint16_t((videoByte >> (BITS_IN_BYTE - 1 - j)) & 0x01);
			const uint16_t attributeAddress =
					BASE_ADDRESS + MEMORY_SIZE + attributeRow * NUMBER_OF_ATTRIBUTE_COLUMNS + attributeColumn;
			const uint8_t attribute = uint8_t(memory[attributeAddress]);
			uint32_t color;
			/*const std::map<int, uint32_t> &colorMap =
					(attribute & BRIGHTNESS_BIT_MASK) == 0 ? g_colorMap : g_brightColorMap;*/
			const uint32_t* colorMap = (attribute & BRIGHTNESS_BIT_MASK) == 0 ? g_colorMap : g_brightColorMap;
			if ((attribute & FLASH_BIT_MASK) == 0 || !isFlash) {
				if (videoBit != 0) {
					color = colorMap[attribute & INK_BITS_MASK];
				} else {
					color = colorMap[(attribute & PAPER_BITS_MASK) >> 3];
				}
			} else {
				if (videoBit != 0) {
					color = colorMap[(attribute & PAPER_BITS_MASK) >> 3];
				} else {
					color = colorMap[attribute & INK_BITS_MASK];
				}
			}
			const uint16_t x = uint16_t((i & 0x1f) * BITS_IN_BYTE + j);
			bitmap[x + y * SCREEN_WIDTH] = color;
		}
	}
}

static void soundCallback(int16_t* data, int size) {
	jshortArray javaData;
	javaData = g_soundEnv->NewShortArray(size);
	
	/*if (result == NULL) {
		return NULL; // out of memory error thrown 
	}
	int i;
	// fill a temp structure to use to populate the java int array
	jint fill[size];
	for (i = 0; i < size; i++) {
		fill[i] = 0; // put whatever logic you want to populate the values here.
	}*/
	
	// move from the temp structure to the java structure
	g_soundEnv->SetShortArrayRegion(javaData, 0, size, data);
	
	g_soundEnv->CallBooleanMethod(g_soundDataQueue, g_offerMethodId, javaData);
	g_soundEnv->DeleteLocalRef(javaData);
	
	//return result;
}

jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
	/*g_colorMap.insert({0, 0xff000000});
	g_colorMap.insert({1, 0xff0000d7});
	g_colorMap.insert({2, 0xffd70000});
	g_colorMap.insert({3, 0xffd700d7});
	g_colorMap.insert({4, 0xff00d700});
	g_colorMap.insert({5, 0xff00d7d7});
	g_colorMap.insert({6, 0xffd7d700});
	g_colorMap.insert({7, 0xffd7d7d7});

	g_brightColorMap.insert({0, 0xff000000});
	g_brightColorMap.insert({1, 0xff0000ff});
	g_brightColorMap.insert({2, 0xffff0000});
	g_brightColorMap.insert({3, 0xffff00ff});
	g_brightColorMap.insert({4, 0xff00ff00});
	g_brightColorMap.insert({5, 0xff00ffff});
	g_brightColorMap.insert({6, 0xffffff00});
	g_brightColorMap.insert({7, 0xffffffff});*/
	
	g_colorMap[0] = 0xff000000;
	g_colorMap[1] = 0xff0000d7;
	g_colorMap[2] = 0xffd70000;
	g_colorMap[3] = 0xffd700d7;
	g_colorMap[4] = 0xff00d700;
	g_colorMap[5] = 0xff00d7d7;
	g_colorMap[6] = 0xffd7d700;
	g_colorMap[7] = 0xffd7d7d7;

	g_brightColorMap[0] = 0xff000000;
	g_brightColorMap[1] = 0xff0000ff;
	g_brightColorMap[2] = 0xffff0000;
	g_brightColorMap[3] = 0xffff00ff;
	g_brightColorMap[4] = 0xff00ff00;
	g_brightColorMap[5] = 0xff00ffff;
	g_brightColorMap[6] = 0xffffff00;
	g_brightColorMap[7] = 0xffffffff;
	
	return JNI_VERSION_1_6;
}
	
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

JNIEXPORT void JNICALL Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_getZxSpectrumScreen(JNIEnv *env, jobject instance, jintArray outData_, jboolean isFlash) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	jint *outData = env->GetIntArrayElements(outData_, NULL);

	renderZxSpectrumScreen(g_zxSpectrum->memoryArray(), (uint32_t *) outData, isFlash);

	env->ReleaseIntArrayElements(outData_, outData, 0);
}

JNIEXPORT jint JNICALL 
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_getZxSpectrumScreenBorder(JNIEnv *env, jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return 0xff000000;
	}

	return g_colorMap[g_zxSpectrum->portsArray()[0xfe] & 0x07];
}

JNIEXPORT void JNICALL Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_initZxSpectrum(JNIEnv *env, jobject instance, jbyteArray program_, jint sampleRate, jint bufferSize, jobject queue, jstring logFilePath_) {
	g_soundDataQueue = env->NewGlobalRef(queue);
	jclass queueClass = env->GetObjectClass(g_soundDataQueue);
	g_offerMethodId = env->GetMethodID(queueClass, "offer", "(Ljava/lang/Object;)Z");
	env->DeleteLocalRef(queueClass);
	
	jbyte *program = env->GetByteArrayElements(program_, NULL);
	const char *logFilePath = env->GetStringUTFChars(logFilePath_, 0);

	g_zxSpectrum = new ZxSpectrum(sampleRate, bufferSize, soundCallback, std::string(logFilePath));

	g_zxSpectrum->reset();
	std::memcpy(g_zxSpectrum->memoryArray(), program, size_t(env->GetArrayLength(program_)));

	env->ReleaseStringUTFChars(logFilePath_, logFilePath);
	env->ReleaseByteArrayElements(program_, program, 0);
}

JNIEXPORT void JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_runZxSpectrum(JNIEnv *env, jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	try {
		g_zxSpectrum->loop();
	} catch (std::exception &e) {
		__android_log_print(ANDROID_LOG_DEBUG, "ZX Spectrum", "Exception: %s", e.what());
	}
}

JNIEXPORT void JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_runSound(JNIEnv *env, jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	g_soundEnv = env;
	
	try {
		g_zxSpectrum->soundLoop();
	} catch (std::exception &e) {
		__android_log_print(ANDROID_LOG_DEBUG, "ZX Spectrum", "Exception: %s", e.what());
	}
}

JNIEXPORT void JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_resetZxSpectrum(JNIEnv *env, jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	g_zxSpectrum->reset();
}

JNIEXPORT jfloat JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_getExceededInstructionsPercent(JNIEnv *env,
																							 jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return 0;
	}

	return g_zxSpectrum->exceededInstructionsPercent();
}

JNIEXPORT jint JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_getInterruptCount(JNIEnv *env, jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return 0;
	}

	return g_zxSpectrum->interruptsCount();
}

JNIEXPORT jint JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_getInstructionsCount(JNIEnv *env,
																				   jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return 0;
	}

	return g_zxSpectrum->instructionsCount();
}

JNIEXPORT void JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_onVerticalRefresh(JNIEnv *env, jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	g_zxSpectrum->verticalRefresh();
}

JNIEXPORT void JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_onKeyPressed(JNIEnv *env, jobject instance, jint keyCode) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	g_zxSpectrum->onKeyPressed(keyCode);
}

JNIEXPORT void JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_onKeyReleased(JNIEnv *env, jobject instance, jint keyCode) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	g_zxSpectrum->onKeyReleased(keyCode);
}

JNIEXPORT void JNICALL
Java_ru_ilapin_zxspectrum_ZxSpectrumActivity2_stopZxSpectrum(JNIEnv *env, jobject instance) {
	if (g_zxSpectrum == nullptr) {
		return;
	}

	g_zxSpectrum->quit();
}

}
