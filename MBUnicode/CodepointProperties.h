#pragma once
#include "MBUnicodeDefinitions.h"

namespace MBUnicode
{
	static const MBUnicode::GraphemeBreakPropertySpecifier GraphemeBreakSpecifiers[] = { {0,0,GraphemeBreakProperty::Null} };
	constexpr size_t GraphemeBreakSpecifiersCount = sizeof(GraphemeBreakSpecifiers) / sizeof(MBUnicode::GraphemeBreakPropertySpecifier);
}