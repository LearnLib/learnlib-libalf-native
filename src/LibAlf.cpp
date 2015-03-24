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

// LibAlf.cpp
// Implementation for the LibAlf class
// Author: Malte Isberner

#include <map>
#include <list>
#include <cstring>

#include "LibAlf.hpp"
#include "LibalfLearner.hpp"
#include "JNIUtil.hpp"

#include <libalf/algorithm_angluin.h>
#include <libalf/algorithm_kearns_vazirani.h>
#include <libalf/algorithm_rivest_schapire.h>
#include <libalf/algorithm_NLstar.h>

 #include <libalf/algorithm_RPNI.h>
 #include <libalf/algorithm_DeLeTe2.h>
 #include <libalf/algorithm_biermann_minisat.h>
 #include <libalf/algorithm_biermann_original.h>


struct StrLess {
	bool operator()(const char *a, const char *b) const
	{
		return std::strcmp(a, b) < 0;
	}
};

static std::map<const char *, LearnerInit *, StrLess> g_learnerInits;

class LearnerMetadataDecl {
public:
	LearnerMetadataDecl(const char *name, LearnerInit *init)
	{
		g_learnerInits[name] = init;
	}
};

#define DEFINE_LEARNER(name, type, alfClass, ...) \
class name ## Learner : public Libalf ## type ## Learner<name ## Learner> { \
public: \
	static LibalfLearner *init(jint alphabetSize, size_t otherOptsLen, jint *otherOptions) \
	{ \
		return new name ## Learner(alphabetSize, otherOptsLen, otherOptions); \
	} \
public: \
	name ## Learner(int alphabetSize, size_t otherOptsLen, jint *otherOptions) \
		: m_algorithm(&m_kb, NULL, alphabetSize, ##__VA_ARGS__) \
	{} \
public: \
	alfClass m_algorithm; \
}; \
static LearnerMetadataDecl g_metadata_ ## name(#name, &name ## Learner::init)



static bool kvUseBinarySearch(size_t otherOptsLen, jint *otherOptions)
{
	bool useBinarySearch = false;
	if (otherOptsLen >= 1) {
		useBinarySearch = *otherOptions;
	}
	return useBinarySearch;
}

DEFINE_LEARNER(ANGLUIN_SIMPLE_DFA, DFA, libalf::angluin_simple_table<bool>);
DEFINE_LEARNER(ANGLUIN_COL_DFA, DFA, libalf::angluin_col_table<bool>);
DEFINE_LEARNER(KV_DFA, DFA, libalf::kearns_vazirani<bool>, kvUseBinarySearch(otherOptsLen, otherOptions));
DEFINE_LEARNER(RS_DFA, DFA, libalf::rivest_schapire_table<bool>);
DEFINE_LEARNER(NLSTAR, NFA, libalf::NLstar_table<bool>);

DEFINE_LEARNER(RPNI, DFA, libalf::RPNI<bool>);
DEFINE_LEARNER(DELETE2, NFA, libalf::DeLeTe2<bool>);
DEFINE_LEARNER(BIERMANN_MINISAT, DFA, libalf::MiniSat_biermann<bool>);
DEFINE_LEARNER(BIERMANN_ORIGINAL_DFA, DFA, libalf::original_biermann<bool>, 1);

class LibalfInstanceMgr {
public:
	~LibalfInstanceMgr(void)
	{
		for (std::list<LibAlf *>::iterator it = m_instances.begin(); it != m_instances.end(); ++it) {
			delete *it;
		}
	}

	LibAlf *create(JNIEnv *env, jobjectArray algIds)
	{
		LibAlf *instance = new LibAlf(env, algIds);
		m_instances.push_front(instance);
		instance->m_ref = m_instances.begin();

		return instance;
	}

	void dispose(LibAlf *instance)
	{
		m_instances.erase(instance->m_ref);
		delete instance;
	}

private:
	std::list<LibAlf *> m_instances;
};

static LibalfInstanceMgr g_instanceMgr;


LibAlf::LibAlf(JNIEnv *env, jobjectArray algIds)
{
	jclass objClazz = env->FindClass("java/lang/Object");
	jmethodID toStringMethod = env->GetMethodID(objClazz, "toString", "()Ljava/lang/String;");

	jint numAlgs = env->GetArrayLength(algIds);
	m_inits.reserve(static_cast<size_t>(numAlgs));

	for (jint i = 0; i < numAlgs; i++) {
		jobject alg = env->GetObjectArrayElement(algIds, i);
		jstring jname = static_cast<jstring>(env->CallObjectMethod(alg, toStringMethod));
		env->DeleteLocalRef(alg);
		const char *name = env->GetStringUTFChars(jname, NULL);
		std::map<const char *, LearnerInit *>::iterator initIt = g_learnerInits.find(name);
		env->ReleaseStringUTFChars(jname, name);
		LearnerInit *init = NULL;
		if (initIt != g_learnerInits.end()) {
			init = initIt->second;
		}
		m_inits.push_back(init);
	}
}

LibalfLearner *LibAlf::createLearner(jint algorithmId, jint alphabetSize, size_t otherOptsLen, jint *otherOpts) const
{
	if (algorithmId < 0 || static_cast<size_t>(algorithmId) >= m_inits.size()) {
		return NULL;
	}
	LearnerInit *init = m_inits[algorithmId];
	if (!init) {
		return NULL;
	}
	return (*init)(alphabetSize, otherOptsLen, otherOpts);
}

// JNI native methods

extern "C" {
/*
 * Class:     de_learnlib_libalf_LibAlf
 * Method:    init
 * Signature: ([Lde/learnlib/libalf/LibAlf/AlgorithmID;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_de_learnlib_libalf_LibAlf_init
  (JNIEnv *env, jclass clazz, jobjectArray jAlgIds)
{
	LibAlf *instance = g_instanceMgr.create(env, jAlgIds);

	return JNIUtil::createPtr(env, instance);
}

/*
 * Class:     de_learnlib_libalf_LibAlf
 * Method:    dispose
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_de_learnlib_libalf_LibAlf_dispose
  (JNIEnv *env, jclass clazz, jbyteArray jptr)
{
	LibAlf *instance = JNIUtil::extractPtr<LibAlf>(env, jptr);
	g_instanceMgr.dispose(instance);
}

/*
 * Class:     de_learnlib_libalf_LibAlf
 * Method:    initAlgorithm
 * Signature: ([BII[I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_de_learnlib_libalf_LibAlf_initAlgorithm
  (JNIEnv *env, jclass clazz, jbyteArray jptr, jint algorithmId, jint alphabetSize, jintArray jOtherOpts)
{
	LibAlf *instance = JNIUtil::extractPtr<LibAlf>(env, jptr);

	size_t otherOptsLen = static_cast<size_t>(env->GetArrayLength(jOtherOpts));
	jint *otherOpts = NULL;
	if (otherOptsLen) {
		otherOpts = env->GetIntArrayElements(jOtherOpts, NULL);
	}

	LibalfLearner *alg = instance->createLearner(algorithmId, alphabetSize, otherOptsLen, otherOpts);

	if (otherOpts) {
		env->ReleaseIntArrayElements(jOtherOpts, otherOpts, 0);
	}

	return JNIUtil::createPtr(env, alg);
}

};

