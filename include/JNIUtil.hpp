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

// JNIUtil.hpp
// Utility functions for working with JNI, especially for storing native
// pointers in Java data types.
// Author: Malte Isberner

#ifndef LEARNLIB_LIBALF_NATIVE_JNIUTIL_HPP
#define LEARNLIB_LIBALF_NATIVE_JNIUTIL_HPP

#include <cstring>

#include <jni.h>

namespace JNIUtil {

inline jbyteArray createPtr(JNIEnv *env, void *ptr)
{
	if (!ptr) {
		return NULL;
	}
	jbyteArray jptr = env->NewByteArray(sizeof(ptr));
	jbyte *contents = env->GetByteArrayElements(jptr, NULL);
	std::memcpy(contents, &ptr, sizeof(ptr));
	env->ReleaseByteArrayElements(jptr, contents, 0);
	return jptr;
}

inline void *extractUntypedPtr(JNIEnv *env, jbyteArray jptr)
{
	if (!jptr) {
		return NULL;
	}
	jbyte *contents = env->GetByteArrayElements(jptr, NULL);
	void *ptr;
	std::memcpy(&ptr, contents, sizeof(ptr));
	env->ReleaseByteArrayElements(jptr, contents, 0);
	return ptr;
}

template<class T>
inline T *extractPtr(JNIEnv *env, jbyteArray jptr)
{
	return static_cast<T *>(extractUntypedPtr(env, jptr));
}

template<class T>
inline T &extractRef(JNIEnv *env, jbyteArray jptr)
{
	return *extractPtr<T>(env, jptr);
}

};

#endif // LEARNLIB_LIBALF_NATIVE_JNIUTIL_HPP
