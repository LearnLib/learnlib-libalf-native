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

// LibalfLearner.cpp
// Implementation of native methods for the LibalfLearner class
// Author: Malte Isberner

#include "LibalfLearner.hpp"
#include "LibAlf.hpp"
#include "JNIUtil.hpp"

#include <libalf/alf.h>
#include <libalf/learning_algorithm.h>



// JNI native methods

extern "C" {

/*
 * Class:     de_learnlib_libalf_LibalfLearner
 * Method:    advance
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_de_learnlib_libalf_LibalfLearner_advance
  (JNIEnv *env, jclass clazz, jbyteArray jptr)
{
	LibalfLearner &learner = JNIUtil::extractRef<LibalfLearner>(env, jptr);
	const libalf::conjecture *cj = learner.advance();
	if (!cj) {
		return NULL;
	}
	size_t cjLen;
	jbyte *cjEnc = learner.encodeConjecture(*cj, cjLen);
	delete cj;
	if (!cjEnc) {
		return NULL;
	}

	jbyteArray result = env->NewByteArray(cjLen);
	if (!result) {
		delete[] cjEnc;
		return NULL;
	}
	env->SetByteArrayRegion(result, 0, cjLen, cjEnc);
	delete[] cjEnc;

	return result;
}

/*
 * Class:     de_learnlib_libalf_LibalfLearner
 * Method:    dispose
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_de_learnlib_libalf_LibalfLearner_dispose
  (JNIEnv *env, jclass clazz, jbyteArray ptr)
{
	LibalfLearner *learner = JNIUtil::extractPtr<LibalfLearner>(env, ptr);
	delete learner;
}

};
