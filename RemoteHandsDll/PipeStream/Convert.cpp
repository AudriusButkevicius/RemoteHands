#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 
#include <codecvt>

std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> strconverter;

std::string to_string(std::wstring wstr)
{
    return strconverter.to_bytes(wstr);
}

std::wstring to_wstring(std::string str)
{
    return strconverter.from_bytes(str);
}
