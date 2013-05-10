#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

class InvalidEvacuationReqException : public std::runtime_error
{
public:
	InvalidEvacuationReqException(const std::string &msg)
	: std::runtime_error(msg)
	{}
};

class InvalidTagException : public std::runtime_error
{
public:
	InvalidTagException(const std::string &msg)
	: std::runtime_error(msg)
	{}
};

#endif /* EXCEPTIONS_HPP */
