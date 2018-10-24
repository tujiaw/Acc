#pragma once

#include <string>     

std::string getFullWithSeperator(const std::string& inbuf, const std::string& seperator);
std::string getInitialWithSeperator(const std::string& inbuf, const std::string& seperator);
std::string getFullAndInitialWithSeperator(const std::string& inbuf);
std::string getPinYin(const std::string& inbuf, bool initial=false,bool polyphone_support=false,bool m_blnFirstBig=false,bool m_blnAllBiG=false,bool m_LetterEnd=false, bool m_unknowSkip=true,bool m_filterPunc=true);
