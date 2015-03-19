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

#include <stdio.h>


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
	libalf::conjecture *cj = learner.advance();
	if (!cj) {
		return NULL;
	}
	size_t cjLen;
	jbyte *cjEnc = learner.encodeConjecture(cj, &cjLen);
	delete cj;

	jbyteArray result = env->NewByteArray(cjLen);
	if (!result) {
		return NULL;
	}
	env->SetByteArrayRegion(result, 0, cjLen, cjEnc);
	delete[] cjEnc;

	return result;
}

/*
 * Class:     de_learnlib_libalf_LibalfLearner
 * Method:    fetchQueryBatch
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_de_learnlib_libalf_LibalfLearner_fetchQueryBatch
  (JNIEnv *env, jclass clazz, jbyteArray ptr)
{
	LibalfLearner &learner = JNIUtil::extractRef<LibalfLearner>(env, ptr);
	QueryBatch *queryBatch = learner.getQueries();
	return JNIUtil::createPtr(env, queryBatch);
}

/*
 * Class:     de_learnlib_libalf_LibalfLearner
 * Method:    getQueries
 * Signature: ([B)[I
 */
JNIEXPORT jintArray JNICALL Java_de_learnlib_libalf_LibalfLearner_getQueries
  (JNIEnv *env, jclass clazz, jbyteArray batchPtr)
{
	QueryBatch &queryBatch = JNIUtil::extractRef<QueryBatch>(env, batchPtr);
	size_t numQueries = queryBatch.size();
	size_t totalSpace = 1;
	for (QueryBatch::iterator it = queryBatch.begin(); it != queryBatch.end(); ++it) {
		totalSpace += it->size() + 1;
	}
	jint *queriesEnc = new jint[totalSpace];
	jint *p = queriesEnc;
	*p++ = static_cast<jint>(numQueries);
	for (QueryBatch::iterator it = queryBatch.begin(); it != queryBatch.end(); ++it) {
		Word &word = *it;
		*p++ = static_cast<jint>(word.size());
		for (Word::iterator it2 = word.begin(); it2 != word.end(); ++it2) {
			*p++ = static_cast<jint>(*it2);
		}
	}

	jintArray result = env->NewIntArray(totalSpace);
	if (!result) {
		return NULL;
	}
	env->SetIntArrayRegion(result, 0, totalSpace, queriesEnc);
	delete[] queriesEnc;

	return result;
}

/*
 * Class:     de_learnlib_libalf_LibalfLearner
 * Method:    processAnswers
 * Signature: ([B[B[I)V
 */
JNIEXPORT void JNICALL Java_de_learnlib_libalf_LibalfLearner_processAnswers
  (JNIEnv *env, jclass clazz, jbyteArray ptr, jbyteArray batchPtr, jintArray jAnswers)
{
	LibalfLearner &learner = JNIUtil::extractRef<LibalfLearner>(env, ptr);

	QueryBatch *queryBatch = JNIUtil::extractPtr<QueryBatch>(env, batchPtr);

	jint *answers = env->GetIntArrayElements(jAnswers, NULL);
	jint *answp = answers;

	for (QueryBatch::iterator it = queryBatch->begin(); it != queryBatch->end(); ++it) {
		Word &word = *it;
		learner.processEncodedAnswer(word, *answp++);
	}

	env->ReleaseIntArrayElements(jAnswers, answers, 0);

	delete queryBatch;
}


/*
 * Class:     de_learnlib_libalf_LibalfLearner
 * Method:    addCounterExample
 * Signature: ([B[I)V
 */
JNIEXPORT void JNICALL Java_de_learnlib_libalf_LibalfLearner_addCounterExample
  (JNIEnv *env, jclass clazz, jbyteArray ptr, jintArray jWord)
{
	LibalfLearner &learner = JNIUtil::extractRef<LibalfLearner>(env, ptr);

	jint wordLen = env->GetArrayLength(jWord);
	jint *word = env->GetIntArrayElements(jWord, NULL);
	jint *wordp = word;

	Word w;
	while (wordLen--) {
		w.push_back(*wordp++);
	}

	env->ReleaseIntArrayElements(jWord, word, 0);

	learner.addCounterExample(w);
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
