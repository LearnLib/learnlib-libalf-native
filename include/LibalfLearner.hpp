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

// LibalfLearner.hpp
// The native class storing data associated with a libalf learner,
// plus automaton-type dependent specializations.
// Author: Malte Isberner

#ifndef LEARNLIB_LIBALF_NATIVE_LIBALFLEARNER_HPP
#define LEARNLIB_LIBALF_NATIVE_LIBALFLEARNER_HPP

#include <list>

#include <jni.h>

#include "SAF.hpp"

#include <libalf/learning_algorithm.h>
#include <libalf/conjecture.h>

typedef std::list<int> Word;
typedef std::list<Word> QueryBatch;

class LibalfLearner {
public:
	LibalfLearner(void) {}
	virtual ~LibalfLearner(void) {}

	virtual const libalf::conjecture *advance(void) = 0;
	virtual QueryBatch *getQueries(void) = 0;
	virtual void addCounterExample(Word &ce) = 0;
	virtual bool addEncodedAnswer(Word &w, jint answer) = 0;
	virtual jbyte *encodeConjecture(const libalf::conjecture &cj, size_t &size) const = 0;
};


template<class A, class D>
class TypedLibalfLearner : public LibalfLearner {
public:
	typedef libalf::learning_algorithm<A> LibalfAlgoBase;

public:
	virtual bool addEncodedAnswer(Word &w, jint answer)
	{
		A answerDec = static_cast<D *>(this)->decodeAnswer(answer);
		return m_kb.add_knowledge(w, answerDec);
	}

	virtual QueryBatch *getQueries(void)
	{
		return new QueryBatch(m_kb.get_queries());
	}

	virtual const libalf::conjecture *advance(void)
	{
		return static_cast<D *>(this)->m_algorithm.advance();
	}

	virtual void addCounterExample(Word &ce)
	{
		static_cast<D *>(this)->m_algorithm.add_counterexample(ce);
	}

public:
	// A decodeAnswer(jint encAnswer) const;

protected:
	libalf::knowledgebase<A> m_kb;
	// LibalfAlgoBase m_algorithm;
};

template<class D>
class LibalfFALearner : public TypedLibalfLearner<bool,D> {
public:
	jbyte *encodeConjecture(const libalf::conjecture &cj, size_t &size) const
	{
		const libalf::finite_automaton &fa = dynamic_cast<const libalf::finite_automaton &>(cj);
		return static_cast<const D *>(this)->encodeFAConjecture(fa, size);
	}

public:
	bool decodeAnswer(jint encAnswer) const { return (encAnswer); }
	// jbyte *encodeFAConjecture(const libalf::finite_automaton &fa, size_t &size) const;
};

template<class D>
class LibalfDFALearner : public LibalfFALearner<D> {
public:
	jbyte *encodeFAConjecture(const libalf::finite_automaton &fa, size_t &size) const
	{
		return SAF::encodeDFA(fa, size);
	}
};

template<class D>
class LibalfNFALearner : public LibalfFALearner<D> {
public:
	jbyte *encodeFAConjecture(const libalf::finite_automaton &fa, size_t &size) const
	{
		return SAF::encodeNFA(fa, size);
	}
};


#endif // LEARNLIB_LIBALF_NATIVE_LIBALFLEARNER_HPP
