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

// SAF.hpp
// Utilities for encoding libalf conjectures into the Simple Automaton
// Format.
// Author: Malte Isberner

#ifndef SAF_HPP
#define SAF_HPP

#include <libalf/conjecture.h>

namespace SAF {

size_t computeDFASize(const libalf::finite_automaton &fa);
void encodeDFA(jbyte *buf, size_t len, const libalf::finite_automaton &fa);
/*
 * Encodes a DFA into SAF, stored in a byte array. The size of the allocated
 * array is returned via the second parameter.
 */
inline jbyte *encodeDFA(const libalf::finite_automaton &fa, size_t &sizeOut)
{
	size_t size = computeDFASize(fa);
	sizeOut = size;
	jbyte *buf = new jbyte[size];
	encodeDFA(buf, size, fa);
	return buf;
}


size_t computeNFASize(const libalf::finite_automaton &fa);
void encodeNFA(jbyte *buf, size_t len, const libalf::finite_automaton &fa);
/*
 * Encodes an NFA into SAF, stored in a byte array. The size of the allocated
 * array is returned via the second parameter.
 */
inline jbyte *encodeNFA(const libalf::finite_automaton &fa, size_t &sizeOut)
{
	size_t size = computeNFASize(fa);
	sizeOut = size;
	jbyte *buf = new jbyte[size];
	encodeNFA(buf, size, fa);
	return buf;
}

};

#endif