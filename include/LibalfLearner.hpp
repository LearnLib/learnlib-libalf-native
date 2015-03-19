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

#ifndef LIBALF_LEARNER_HPP
#define LIBALF_LEARNER_HPP

#include <list>

#include <jni.h>

#include "SAF.hpp"

#include <libalf/learning_algorithm.h>
#include <libalf/conjecture.h>

typedef std::list<int> Word;
typedef std::list<Word> QueryBatch;

class LibalfLearner {
public:
	LibalfLearner() {}
	virtual ~LibalfLearner() {}

	virtual libalf::conjecture *advance(void) = 0;
	virtual QueryBatch *getQueries(void) = 0;
	virtual void addCounterExample(Word &ce) = 0;
	virtual void processEncodedAnswer(Word &w, jint answer) = 0;
	virtual jbyte *encodeConjecture(libalf::conjecture *cj, size_t *size) = 0;
};


template<class A>
class TypedLibalfLearner : public LibalfLearner {
protected:
	typedef libalf::learning_algorithm<A> LibalfAlgoBase;
public:
	virtual ~TypedLibalfLearner() {}

	virtual void processEncodedAnswer(Word &w, jint answer)
	{
		A answerDec = decodeAnswer(answer);
		m_kb.add_knowledge(w, answerDec);
	}

	virtual QueryBatch *getQueries(void)
	{
		return new QueryBatch(m_kb.get_queries());
	}

	virtual libalf::conjecture *advance(void)
	{
		return algorithm().advance();
	}

	virtual void addCounterExample(Word &ce)
	{
		algorithm().add_counterexample(ce);
	}

protected:
	virtual A decodeAnswer(jint encAnswer) = 0;
	virtual jint encodeAnswer(A answer) = 0;

	virtual LibalfAlgoBase &algorithm(void) = 0;

protected:
	libalf::knowledgebase<A> m_kb;
};

class LibalfDFALearner : public TypedLibalfLearner<bool> {
public:
	virtual jbyte *encodeConjecture(libalf::conjecture *cj, size_t *size)
	{
		libalf::finite_automaton *fa = dynamic_cast<libalf::finite_automaton *>(cj);
		return SAF::encodeDFA(*fa, size);
	}

protected:
	virtual bool decodeAnswer(jint encAnswer) { return (encAnswer); }
	virtual jint encodeAnswer(bool answer) { return (answer) ? 1 : 0; }
};

class LibalfNFALearner : public TypedLibalfLearner<bool> {
public:
	virtual jbyte *encodeConjecture(libalf::conjecture *cj, size_t *size)
	{
		libalf::finite_automaton *fa = dynamic_cast<libalf::finite_automaton *>(cj);
		return SAF::encodeNFA(*fa, size);
	}

protected:
	virtual bool decodeAnswer(jint encAnswer) { return (encAnswer); }
	virtual jint encodeAnswer(bool answer) { return (answer) ? 1 : 0; }
};


#endif // LIBALF_LEARNER_HPP
