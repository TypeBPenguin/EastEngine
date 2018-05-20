#include "stdafx.h"
#include "CommonLib.h"

#include "StringUtil.h"

my_exception::my_exception(const char* expression, const char *fileName, unsigned int lineNo)
	: std::runtime_error(expression)
{
	m_strMsg = eastengine::String::Format("[%s<%d>] : %s\n", fileName, lineNo, expression);
}

const char* my_exception::what() const throw()
{
	return m_strMsg.c_str();
}