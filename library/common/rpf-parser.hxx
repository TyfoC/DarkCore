#pragma once
#ifndef RPF_PARSER_HXX
#define RPF_PARSER_HXX

#include "list.hxx"

//	RPF (Raw Parse File) Parser
class RPFParser {
	public:
	typedef struct Element {
		char*	Key;
		char*	Value;
	} Element, *PElement;

	static List<Element> Parse(const char* data, size_t bytesCount);
	static void FreeElements(List<Element>& elements);
};

#endif