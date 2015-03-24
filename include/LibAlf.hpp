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

// LibAlf.hpp
// Class for managing global information for a single session, such as
// available algorithms and their instantiation.
// Author: Malte Isberner

#ifndef LEARNLIB_LIBALF_NATIVE_LIBALF_HPP
#define LEARNLIB_LIBALF_NATIVE_LIBALF_HPP

#include <jni.h>
#include <vector>

class LibalfLearner;

typedef LibalfLearner *LearnerInit(jint alphabetSize, size_t otherOptsLen, jint *otherOptions);

class LibAlf {
	friend class LibalfInstanceMgr;

public:
	LibAlf(JNIEnv *env, jobjectArray algIds);
	LibalfLearner *createLearner(jint algorithmId, jint alphabetSize, size_t otherOptsLen, jint *otherOptions) const;

private:
	std::vector<LearnerInit *> m_inits;
	std::list<LibAlf *>::iterator m_ref;
};

#endif // LEARNLIB_LIBALF_NATIVE_LIBALF_HPP
