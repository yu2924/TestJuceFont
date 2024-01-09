//
//  PlatformGlue_macos.mm
//  TestJuceFont_App
//
//  created by yu2924 on 2024-01-02
//


#include "PlatformGlue.h"

#if JUCE_MAC

#import <AppKit/AppKit.h>

namespace PlatformGlue
{
	juce::String getSystemSansSerifTypefaceName()
	{
		@autoreleasepool
		{
			NSCalendar* cal = NSCalendar.currentCalendar;
			NSArray<NSString*>* wdays = cal.standaloneWeekdaySymbols;
			CFStringRef strtest = (CFStringRef)wdays.firstObject; // 日曜日
			CTFontRef sysfont = CTFontCreateUIFontForLanguage(kCTFontUIFontUser, 13, NULL);
			CTFontRef locfont = CTFontCreateForString(sysfont, strtest, CFRangeMake(0, CFStringGetLength(strtest)));
			// CFStringRef fontname = CTFontCopyFamilyName(locfont); // Hiragino Sans
			// CFStringRef fontname = CTFontCopyDisplayName(locfont); // ヒラギノ角ゴシック W3
			CFStringRef fontname = CTFontCopyFullName(locfont); // Hiragino Sans W3
			CFRelease(fontname);
			CFRelease(locfont);
			CFRelease(sysfont);
			return juce::String::fromCFString(fontname);
		}
	}
} // namespace PlatformGlue

#endif
