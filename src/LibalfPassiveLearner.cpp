/* Copyright (C) 2015 TU Dortmund
 * This file is part of LearnLib, http://www.learnlib.de/.
 * 
 * LearnLib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 3.0 as published by the Free Software Foundation.
 * 
 * LearnLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with LearnLib; if not, see
 * <http://www.gnu.de/documents/lgpl.en.html>.
 */

// LibalfPassiveLearner.cpp
// JNI method implementations for the LibalfPassiveLearner class
// Author: Malte Isberner

#include "LibalfLearner.hpp"
#include "JNIUtil.hpp"

#include <jni.h>

extern "C" {

/*
 * Class:     de_learnlib_libalf_LibalfPassiveLearner
 * Method:    addSamples
 * Signature: ([BI[I[I)Z
 */
JNIEXPORT jboolean JNICALL Java_de_learnlib_libalf_LibalfPassiveLearner_addSamples
  (JNIEnv *env, jclass clazz, jbyteArray ptr, jint numSamples, jintArray jSamplesEnc, jintArray jOutputsEnc)
{
	LibalfLearner &learner = JNIUtil::extractRef<LibalfLearner>(env, ptr);

	jint *samplesEnc = env->GetIntArrayElements(jSamplesEnc, NULL);
	jint *p = samplesEnc;

	jint *outputsEnc = env->GetIntArrayElements(jOutputsEnc, NULL);
	jint *q = outputsEnc;

	jboolean ok = JNI_TRUE;

	while (numSamples--) {
		jint sampleLen = *p++;
		Word sample;
		while (sampleLen--) {
			sample.push_back(static_cast<int>(*p++));
		}
		if (!learner.addEncodedAnswer(sample, *q++)) {
			ok = JNI_FALSE;
			break;
		}
	}

	env->ReleaseIntArrayElements(jOutputsEnc, outputsEnc, 0);
	env->ReleaseIntArrayElements(jSamplesEnc, samplesEnc, 0);

	return ok;
}

};
