#include <MBErrorHandling.h>
//class MBError
//operator bool() const { return(Type == MBErrorType::OK); }
MBError::MBError(bool BoolInitialiser)
{
	if (true)
	{
		Type = MBErrorType::OK;
	}
	else
	{
		Type = MBErrorType::Error;
	}
}
MBError::~MBError()
{

}