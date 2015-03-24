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

// SAF.cpp
// Implementation of the serialization into SAF facility
// Author: Malte Isberner

// for htonl
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <jni.h>

#include "SAF.hpp"

namespace SAF {

enum AutomatonType {
	DFA = 0,
	NFA = 1,
	Mealy = 2
};

class Sink {
public:
	virtual ~Sink() {}
	virtual void writeInt8(jbyte v) = 0;
	virtual void writeInt32(jint v) = 0;
};

class ArraySink : public Sink {
public:
	ArraySink(void *array, size_t len) : m_array(array), m_curr(array), m_len(len)
	{}

	inline void *getArray()
	{
		return m_array;
	}

	template<class T>
	void write(T val)
	{
		T *curr = static_cast<T *>(m_curr);
		*curr++ = val;
		m_curr = curr;
	}

	virtual void writeInt8(jbyte v) { write(v); }
	virtual void writeInt32(jint v) { write(htonl(static_cast<uint32_t>(v))); }

private:
	void *m_array;
	void *m_curr;
	size_t m_len;
};

void writeHeader(Sink &sink, AutomatonType type)
{
	sink.writeInt8('S');
	sink.writeInt8('A');
	sink.writeInt8('F');
	jbyte id = static_cast<jbyte>(type);
	sink.writeInt8(id);
}


typedef std::map<int, std::set<int> > StateTransitions;
typedef std::map<int, StateTransitions > Transitions;

size_t computeNFASize(const libalf::finite_automaton &fa)
{
	int alphabetSize = fa.input_alphabet_size;
	int numStates = fa.state_count;

	size_t numWords = 3; // header/automaton type + input alphabet size + state count
	numWords += fa.initial_states.size() + 1; // initial state set
	numWords += (numStates - 1)/32 + 1; // acceptance info
	numWords += numStates * alphabetSize; // transition set sizes

	const Transitions &transitions = fa.transitions;
	Transitions::const_iterator it = transitions.begin();
	for (int i = 0; i < numStates; i++) {
		if (it == transitions.end() || it->first != i) {
			Transitions::const_iterator it2 = transitions.find(i);
			if (it2 != transitions.end()) {
				it = it2;
			}
		}

		if (it != transitions.end() && it->first == i) {
			const StateTransitions &strans = it->second;
			StateTransitions::const_iterator sit = strans.begin();
			for (int j = 0; j < alphabetSize; j++) {
				if (sit == strans.end() || sit->first != j) {
					StateTransitions::const_iterator sit2 = strans.find(j);
					if (sit2 != strans.end()) {
						sit = sit2;
					}
				}

				if (sit != strans.end() && sit->first == j) {
					numWords += sit->second.size();
					++sit;
				}
			}
			++it;
		}
	}

	size_t size = numWords * 4;

	return size;
}

void writeAcceptance(Sink &snk, const libalf::finite_automaton &fa)
{
	jint currAcc = 0;
	jint currMask = 1;

	std::map<int, bool>::const_iterator it = fa.output_mapping.begin();

	int numStates = fa.state_count;

	for (int i = 0; i < numStates; i++) {
		if (i % 32 == 0 && i != 0) {
			snk.writeInt32(currAcc);
			currAcc = 0;
			currMask = 1;
		}

		if (it == fa.output_mapping.end() || it->first != i) {
			std::map<int, bool>::const_iterator it2 = fa.output_mapping.find(i);
			if (it2 != fa.output_mapping.end()) {
				it = it2;
			}
		}
		bool acc = false;
		if (it != fa.output_mapping.end() && it->first == i) {
			acc = it->second;
			++it;
		}

		if (acc) {
			currAcc |= currMask;
		}
		currMask <<= 1;
	}
	snk.writeInt32(currAcc);
}

void writeSet(Sink &snk, const std::set<int> &set)
{
	snk.writeInt32(static_cast<jint>(set.size()));
	for (std::set<int>::const_iterator it = set.begin(); it != set.end(); ++it) {
		snk.writeInt32(*it);
	}
}

void writeNFATransitions(Sink &snk, int alphabetSize, int numStates, const Transitions &transitions)
{
	Transitions::const_iterator it = transitions.begin();
	for (int i = 0; i < numStates; i++) {
		if (it == transitions.end() || it->first != i) {
			Transitions::const_iterator it2 = transitions.find(i);
			if (it2 != transitions.end()) {
				it = it2;
			}
		}

		if (it != transitions.end() && it->first == i) {
			const StateTransitions &strans = it->second;
			StateTransitions::const_iterator sit = strans.begin();
			for (int j = 0; j < alphabetSize; j++) {
				if (sit == strans.end() || sit->first != j) {
					StateTransitions::const_iterator sit2 = strans.find(j);
					if (sit2 != strans.end()) {
						sit = sit2;
					}
				}

				if (sit != strans.end() && sit->first == j) {
					writeSet(snk, sit->second);
					++sit;
				}
				else {
					snk.writeInt32(0);
				}
			}
			++it;
		}
		else {
			for (int j = 0; j < alphabetSize; j++) {
				snk.writeInt32(0);
			}
		}
	}
}

size_t computeDFASize(const libalf::finite_automaton &fa)
{
	int alphabetSize = fa.input_alphabet_size;
	int numStates = fa.state_count;

	size_t size =
		(4 // header/automaton type + input alphabet size + state count + initial state id
		+ (numStates - 1)/32 + 1 // acceptance info
		+ numStates * alphabetSize) * 4; // transition info

	return size;
}


void writeDFATransitions(Sink &snk, int alphabetSize, int numStates, const Transitions &transitions)
{
	Transitions::const_iterator it = transitions.begin();

	for (int i = 0; i < numStates; i++) {
		if (it == transitions.end() || it->first != i) {
			Transitions::const_iterator it2 = transitions.find(i);
			if (it2 != transitions.end()) {
				it = it2;
			}
		}

		if (it != transitions.end() && it->first == i) {
			const StateTransitions &strans = it->second;
			StateTransitions::const_iterator sit = strans.begin();
			for (int j = 0; j < alphabetSize; j++) {
				if (sit == strans.end() || sit->first != j) {
					StateTransitions::const_iterator sit2 = strans.find(j);
					if (sit2 != strans.end()) {
						sit = sit2;
					}
				}

				if (sit != strans.end() && sit->first == j) {
					const std::set<int> &targets = sit->second;
					if (targets.empty()) {
						snk.writeInt32(-1);
					}
					else {
						snk.writeInt32(*targets.begin());
					}
					++sit;
				}
				else {
					snk.writeInt32(-1);
				}
			}
			++it;
		}
		else {
			for (int j = 0; j < alphabetSize; j++) {
				snk.writeInt32(-1);
			}
		}
	}
}

void writeNFA(Sink &snk, const libalf::finite_automaton &fa)
{
	writeHeader(snk, NFA);
	int alphabetSize = fa.input_alphabet_size;
	snk.writeInt32(alphabetSize);
	int numStates = fa.state_count;
	snk.writeInt32(numStates);

	writeSet(snk, fa.initial_states);

	writeAcceptance(snk, fa);

	writeNFATransitions(snk, alphabetSize, numStates, fa.transitions);
}


void writeDFA(Sink &snk, const libalf::finite_automaton &fa)
{
	writeHeader(snk, DFA);
	snk.writeInt32(fa.input_alphabet_size);
	int numStates = fa.state_count;
	snk.writeInt32(numStates);

	if (fa.initial_states.size() != 1) {
		throw std::exception();
	}
	int init = *fa.initial_states.begin();
	snk.writeInt32(init);

	writeAcceptance(snk, fa);

	writeDFATransitions(snk, fa.input_alphabet_size, numStates, fa.transitions);
}

jbyte *encodeDFA(const libalf::finite_automaton &fa, size_t &sizeOut)
{
	size_t size = computeDFASize(fa);
	sizeOut = size;

	jbyte *data = new jbyte[size];
	ArraySink snk(data, size);

	writeDFA(snk, fa);

	return data;
}
jbyte *encodeNFA(const libalf::finite_automaton &fa, size_t &sizeOut)
{
	size_t size = computeNFASize(fa);
	sizeOut = size;

	jbyte *data = new jbyte[size];
	ArraySink snk(data, size);

	writeNFA(snk, fa);

	return data;
}

};
