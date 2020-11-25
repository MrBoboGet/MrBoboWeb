#pragma once
#include <cstdint>
#include <string>
enum class MBErrorType : uint64_t
{
	OK, Error, Null
};
class MBError
{
public:
	MBErrorType Type = MBErrorType::Null;
	std::string ErrorMessage = "";
	operator bool() const { return(Type == MBErrorType::OK); }
	MBError(bool BoolInitialiser);
	~MBError();
};