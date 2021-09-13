#pragma once
#include <cstdint>
#include <string>
enum class MBErrorType
{
	OK, Error, Null
};
class MBError
{
public:
	MBErrorType Type = MBErrorType::Null;
	std::string ErrorMessage = "";
	operator bool() const { return(Type == MBErrorType::OK); }
	MBError& operator=(bool ValueToSet);
	MBError(bool BoolInitialiser);
	MBError() {};
	~MBError();
};