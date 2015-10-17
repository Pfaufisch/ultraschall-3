////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014-2015 Ultraschall (http://ultraschall.fm)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include <UnicodeStringTraits.h>

namespace ultraschall { namespace framework {

template<> const size_t UnicodeStringTraits<char>::strlen(const char* str)
{
   return strlen(str);
}
   
template<> const size_t UnicodeStringTraits<wchar_t>::strlen(const wchar_t* str)
{
   return wcslen(str);
}
   
template<> const int UnicodeStringTraits<char>::strcmp(const char* left, const char* right)
{
   return strcmp(left, right);
}

template<> const int UnicodeStringTraits<wchar_t>::strcmp(const wchar_t* left, const wchar_t* right)
{
   return wcscmp(left, right);
}

template<> void UnicodeStringTraits<char>::strcpy(char* target, const char* source)
{
   strcpy(target, source);
}

template<> void UnicodeStringTraits<wchar_t>::strcpy(wchar_t* target, const wchar_t* source)
{
   wcscpy(target, source);
}

template<> const char* UnicodeStringTraits<char>::strdup(const char* source)
{
   return strdup(source);
}
   
template<> const wchar_t* UnicodeStringTraits<wchar_t>::strdup(const wchar_t* source)
{
   return wcsdup(source);
}
   
}}

