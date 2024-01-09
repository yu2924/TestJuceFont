//
//  PlatformGlue_win32.cpp
//  TestJuceFont_App
//
//  created by yu2924 on 2024-01-02
//

#include "PlatformGlue.h"

#if JUCE_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace PlatformGlue
{
	juce::String getSystemSansSerifTypefaceName()
	{
			LOGFONTW lf = {};
			SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
			return lf.lfFaceName;
	}
} // namespace PlatformGlue

#endif
