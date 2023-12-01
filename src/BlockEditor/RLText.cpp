#include "stdafx.h"
#include "RL.h"

// Get next codepoint in a UTF-8 encoded text, scanning until '\0' is found
// When an invalid UTF-8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: The standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
int GetCodepoint(const char* text, int* codepointSize)
{
	/*
	UTF-8 specs from https://www.ietf.org/rfc/rfc3629.txt

	Char. number range  |        UTF-8 octet sequence
	(hexadecimal)    |              (binary)
	--------------------+---------------------------------------------
	0000 0000-0000 007F | 0xxxxxxx
	0000 0080-0000 07FF | 110xxxxx 10xxxxxx
	0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
	0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	*/
	// NOTE: on decode errors we return as soon as possible

	int codepoint = 0x3f;   // Codepoint (defaults to '?')
	int octet = (unsigned char)(text[0]); // The first UTF8 octet
	*codepointSize = 1;

	if (octet <= 0x7f)
	{
		// Only one octet (ASCII range x00-7F)
		codepoint = text[0];
	}
	else if ((octet & 0xe0) == 0xc0)
	{
		// Two octets

		// [0]xC2-DF    [1]UTF8-tail(x80-BF)
		unsigned char octet1 = text[1];

		if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

		if ((octet >= 0xc2) && (octet <= 0xdf))
		{
			codepoint = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
			*codepointSize = 2;
		}
	}
	else if ((octet & 0xf0) == 0xe0)
	{
		// Three octets
		unsigned char octet1 = text[1];
		unsigned char octet2 = '\0';

		if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

		octet2 = text[2];

		if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; } // Unexpected sequence

		// [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
		// [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
		// [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
		// [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

		if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
			((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) {
			*codepointSize = 2; return codepoint;
		}

		if ((octet >= 0xe0) && (octet <= 0xef))
		{
			codepoint = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
			*codepointSize = 3;
		}
	}
	else if ((octet & 0xf8) == 0xf0)
	{
		// Four octets
		if (octet > 0xf4) return codepoint;

		unsigned char octet1 = text[1];
		unsigned char octet2 = '\0';
		unsigned char octet3 = '\0';

		if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; }  // Unexpected sequence

		octet2 = text[2];

		if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; }  // Unexpected sequence

		octet3 = text[3];

		if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *codepointSize = 4; return codepoint; }  // Unexpected sequence

		// [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
		// [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
		// [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

		if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
			((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) {
			*codepointSize = 2; return codepoint;
		} // Unexpected sequence

		if (octet >= 0xf0)
		{
			codepoint = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
			*codepointSize = 4;
		}
	}

	if (codepoint > 0x10ffff) codepoint = 0x3f;     // Codepoints after U+10ffff are invalid

	return codepoint;
}


// Get index position for a unicode character on font
// NOTE: If codepoint is not found in the font it fallbacks to '?'
int GetGlyphIndex(Font font, int codepoint)
{
	int index = 0;

#define SUPPORT_UNORDERED_CHARSET
#if defined(SUPPORT_UNORDERED_CHARSET)
	int fallbackIndex = 0;      // Get index of fallback glyph '?'

	// Look for character index in the unordered charset
	for (int i = 0; i < font.glyphCount; i++)
	{
		if (font.glyphs[i].value == 63) fallbackIndex = i;

		if (font.glyphs[i].value == codepoint)
		{
			index = i;
			break;
		}
	}

	if ((index == 0) && (font.glyphs[0].value != codepoint)) index = fallbackIndex;
#else
	index = codepoint - 32;
#endif

	return index;
}

// Unload file text data allocated by LoadFileText()
void UnloadFileText(char* text)
{
	RL_FREE(text);
}

// Get text length in bytes, check for \0 character
unsigned int TextLength(const char* text)
{
	unsigned int length = 0;

	if (text != NULL)
	{
		// NOTE: Alternative: use strlen(text)

		while (*text++) length++;
	}

	return length;
}

// Replace text string
// REQUIRES: strlen(), strstr(), strncpy(), strcpy()
// WARNING: Allocated memory must be manually freed
char* TextReplace(char* text, const char* replace, const char* by)
{
	// Sanity checks and initialization
	if (!text || !replace || !by) return NULL;

	char* result = NULL;

	char* insertPoint = NULL;   // Next insert point
	char* temp = NULL;          // Temp pointer
	int replaceLen = 0;         // Replace string length of (the string to remove)
	int byLen = 0;              // Replacement length (the string to replace by)
	int lastReplacePos = 0;     // Distance between replace and end of last replace
	int count = 0;              // Number of replacements

	replaceLen = TextLength(replace);
	if (replaceLen == 0) return NULL;  // Empty replace causes infinite loop during count

	byLen = TextLength(by);

	// Count the number of replacements needed
	insertPoint = text;
	for (count = 0; (temp = strstr(insertPoint, replace)); count++) insertPoint = temp + replaceLen;

	// Allocate returning string and point temp to it
	temp = result = (char*)RL_MALLOC(TextLength(text) + (byLen - replaceLen) * count + 1);

	if (!result) return NULL;   // Memory could not be allocated

	// First time through the loop, all the variable are set correctly from here on,
	//  - 'temp' points to the end of the result string
	//  - 'insertPoint' points to the next occurrence of replace in text
	//  - 'text' points to the remainder of text after "end of replace"
	while (count--)
	{
		insertPoint = strstr(text, replace);
		lastReplacePos = (int)(insertPoint - text);
		temp = strncpy(temp, text, lastReplacePos) + lastReplacePos;
		temp = strcpy(temp, by) + byLen;
		text += lastReplacePos + replaceLen; // Move to next "end of replace"
	}

	// Copy remaind text part after replacement to result (pointed by moving temp)
	strcpy(temp, text);

	return result;
}