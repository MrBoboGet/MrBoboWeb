#include "MBMime.h"
namespace MBMIME
{
	MIMETypeTuple MIMETypeConnector::GetTupleFromExtension(std::string const& Extension)
	{
		for (size_t i = 0; i < SuppportedTupples.size(); i++)
		{
			for (size_t j = 0; j < SuppportedTupples[i].FileExtensions.size(); j++)
			{
				if (SuppportedTupples[i].FileExtensions[j] == Extension)
				{
					return(SuppportedTupples[i]);
				}
			}
		}
		return(NullTupple);
	}
	MIMETypeTuple MIMETypeConnector::GetTupleFromDocumentType(MIMEType DocumentType)
	{
		for (size_t i = 0; i < SuppportedTupples.size(); i++)
		{
			if (SuppportedTupples[i].FileMIMEType == DocumentType)
			{
				return(SuppportedTupples[i]);
			}
		}
		return(NullTupple);
	}
}