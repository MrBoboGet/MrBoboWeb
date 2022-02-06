#pragma once

namespace MBUnicode
{
	typedef unsigned int Codepoint;
	enum class GraphemeBreakProperty
	{
		Control,
		L,
		V,
		LV,
		LVT,
		CR,
		LF,
		Extend,
		ZWJ,
		SOF,
		EndOfFile,
		SpacingMark,
		Prepend,
		Extended_Pictographic,
		RI,
		Null,
	};
	struct GraphemeBreakPropertySpecifier
	{
		Codepoint Lower = 0;
		Codepoint Higher = 0;
		GraphemeBreakProperty Type = GraphemeBreakProperty::Null;
	};
}