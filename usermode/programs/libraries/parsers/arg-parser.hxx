#pragma once
#ifndef ARG_PARSER_HXX
#define ARG_PARSER_HXX

#include "../common/list.hxx"

//	Arguments parser:
//		`arg1 arg2 arg3 "arg4 arg5" arg5`
class ArgParser {
	public:
	typedef struct __attribute__((packed)) {
		char*	Value;
		size_t	Index;
	} Argument, *PArgument;

	ArgParser(const char* source);
	~ArgParser();

	size_t GetArgumentsCount();
	Argument GetArgument(size_t index);
	List<Argument> GetArguments();
	protected:
	List<Argument> Arguments_;
};

#endif