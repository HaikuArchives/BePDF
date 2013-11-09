//*** LICENSE ***
//ColumnListView, its associated classes and source code, and the other components of Santa's Gift Bag are
//being made publicly available and free to use in freeware and shareware products with a price under $25
//(I believe that shareware should be cheap). For overpriced shareware (hehehe) or commercial products,
//please contact me to negotiate a fee for use. After all, I did work hard on this class and invested a lot
//of time into it. That being said, DON'T WORRY I don't want much. It totally depends on the sort of project
//you're working on and how much you expect to make off it. If someone makes money off my work, I'd like to
//get at least a little something.  If any of the components of Santa's Gift Bag are is used in a shareware
//or commercial product, I get a free copy.  The source is made available so that you can improve and extend
//it as you need. In general it is best to customize your ColumnListView through inheritance, so that you
//can take advantage of enhancements and bug fixes as they become available. Feel free to distribute the 
//ColumnListView source, including modified versions, but keep this documentation and license with it.

#include <string.h>
#include <Font.h>
#include <Looper.h>

#include "NewStrings.h"

float GetStringsMaxWidth(const char** strings, int32 num_strings, const BFont* font,
	float* string_widths)
//Fills the stringsWidths array with the width of each individual string in the array using 
//BFont::GetStringWidths(), then finds the longest string width in the array and returns that width.
//If a string_widths array is provided, it fills it in with the length of each string.
{
	int32 strlens_fixed[20];
	int32* strlens;
	bool release_string_lengths = false;
	if(num_strings <= 20)
		strlens = strlens_fixed;
	else
	{
		strlens = new int32[num_strings];
		release_string_lengths = true;
	}

	float strwidths_fixed[20];
	bool release_string_widths = false;
	if(string_widths == NULL)
	{
		if(num_strings <= 20)
			string_widths = strwidths_fixed;
		else
		{
			string_widths = new float[num_strings];
			release_string_widths = true;
		}
	}

	int32 i;
	for(i = 0; i < num_strings; i++)
		strlens[i] = strlen(strings[i]);
	font->GetStringWidths(strings,strlens,num_strings,string_widths);
	float max = string_widths[0];
	for(i = 1; i < num_strings; i++)
		if(string_widths[i] > max)
			max = string_widths[i];

	if(release_string_widths)
		delete[] string_widths;
	if(release_string_lengths)
		delete[] strlens;
	return max;
}


char* Strdup_new(const char* source)
{
	char* dest = new char[strlen(source)+1];
	strcpy(dest,source);
	return dest;
}


char* Strcat_new(const char* string_1, const char* string_2)
{
	int len_1 = strlen(string_1);
	char* dest = new char[len_1+strlen(string_2)+1];
	strcpy(dest,string_1);
	strcpy(&dest[len_1],string_2);
	return dest;
}


char* Strcat_new(const char* string_1, const char* string_2, const char* string_3)
{
	int len_1 = strlen(string_1);
	int len_2 = strlen(string_2);
	char* dest = new char[len_1+len_2+strlen(string_3)+1];
	strcpy(dest,string_1);
	strcpy(&dest[len_1],string_2);
	strcpy(&dest[len_1+len_2],string_3);
	return dest;
}


char* Strcat_new(const char* string_1, const char* string_2, const char* string_3, const char* string_4)
{
	int len_1 = strlen(string_1);
	int len_2 = strlen(string_2);
	int len_3 = strlen(string_3);
	char* dest = new char[len_1+len_2+len_3+strlen(string_4)+1];
	strcpy(dest,string_1);
	strcpy(&dest[len_1],string_2);
	strcpy(&dest[len_1+len_2],string_3);
	strcpy(&dest[len_1+len_2+len_3],string_4);
	return dest;
}


char *Strtcpy(char *dst, const char *src, int len)
{
	strncpy(dst,src,len-1);
	dst[len-1] = 0;
	return dst;
}


void StrToUpper(char* string)
{
	while(*string != 0)
	{
		if(*string >= 'a' && *string <= 'z')
			*string = *string - 'a' + 'A';
		string++;
	}
}


void StrToLower(char* string)
{
	while(*string != 0)
	{
		if(*string >= 'A' && *string <= 'Z')
			*string = *string - 'A' + 'a';
		string++;
	}
}
