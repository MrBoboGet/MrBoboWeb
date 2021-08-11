#include <MBErrorHandling.h>
//class MBError
//operator bool() const { return(Type == MBErrorType::OK); }

//#define MBERROR_ASSERT

#ifdef MBERROR_ASSERT
#include <assert.h>
#endif // MBERROR_ASSERT

MBError::MBError(bool BoolInitialiser)
{
	if (BoolInitialiser)
	{
		Type = MBErrorType::OK;
	}
	else
	{
		Type = MBErrorType::Error;
	}
}
MBError& MBError::operator=(bool BoolToSet)
{
	if (BoolToSet)
	{
		Type = MBErrorType::OK;
	}
	else
	{
#ifdef MBERROR_ASSERT
		assert(false);
#endif // MBERROR_ASSERT
		Type = MBErrorType::Error;
	}
	return(*this);
}
MBError::~MBError()
{

}