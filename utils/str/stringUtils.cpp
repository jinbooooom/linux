
#include "stringUtils.h"
#include <string.h>
#include <algorithm>

bool StringUtils::wildCardMatch(const char *str, const char *pattern)
{
    while (*pattern)
    {
        switch (*pattern)
        {
            case '?':
                if (!*str)
                    return false;
                ++str;
                ++pattern;
                break;
            case '*':
                if (wildCardMatch(str, pattern + 1))
                    return true;
                if (*str && wildCardMatch(str + 1, pattern))
                    return true;
                return false;
            default:
                if (*str++ != *pattern++)
                    return false;
                break;
        }
    }
    return !*str && !*pattern;
}

std::string &StringUtils::ltrim(std::string &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c) { return !std::isspace(c); }));
    return str;
}

std::string &StringUtils::rtrim(std::string &str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char c) { return !std::isspace(c); }).base(), str.end());
    return str;
}

std::string &StringUtils::trim(std::string &str)
{
    return ltrim(rtrim(str));
}

bool StringUtils::startsWith(const std::string &str, const std::string &start)
{
    return (str.length() >= start.length()) && (str.compare(0, start.length(), start) == 0);
}

bool StringUtils::endsWith(const std::string &str, const std::string &end)
{
    return (str.length() >= end.length()) && (str.compare(str.length() - end.length(), end.length(), end) == 0);
}

std::string &StringUtils::replaceAll(std::string &str, char replaceWhat, char replaceWith)
{
    std::replace(str.begin(), str.end(), replaceWhat, replaceWith);
    return str;
}

std::string &StringUtils::replaceAll(std::string &str, const std::string &replaceWhat, const std::string &replaceWith)
{
    if (replaceWhat == replaceWith)
        return str;
    std::size_t foundAt = std::string::npos;
    while ((foundAt = str.find(replaceWhat, foundAt + 1)) != std::string::npos)
    {
        str.replace(foundAt, replaceWhat.length(), replaceWith);
    }
    return str;
}

std::string &StringUtils::toUpper(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return static_cast<char>(::toupper(c)); });
    return str;
}

bool StringUtils::cStringEq(const char *s1, const char *s2)
{
    if (s1 == nullptr && s2 == nullptr)
        return true;
    if (s1 == nullptr || s2 == nullptr)
        return false;
    return strcmp(s1, s2) == 0;
}

bool StringUtils::cStringCaseEq(const char *s1, const char *s2)
{
    if (s1 == nullptr && s2 == nullptr)
        return true;
    if (s1 == nullptr || s2 == nullptr)
        return false;

    // With thanks to cygwin for this code
    int d = 0;

    while (true)
    {
        const int c1 = toupper(*s1++);
        const int c2 = toupper(*s2++);

        if (((d = c1 - c2) != 0) || (c2 == '\0'))
        {
            break;
        }
    }

    return d == 0;
}

bool StringUtils::contains(const char *str, char c)
{
    for (; *str; ++str)
    {
        if (*str == c)
            return true;
    }
    return false;
}

char *StringUtils::convertAndAddToBuff(std::size_t n, int len, char *buf, const char *bufLim, bool zeroPadded)
{
    char localBuff[10] = "";
    char *p            = localBuff + sizeof(localBuff) - 2;
    if (n > 0)
    {
        for (; n > 0 && p > localBuff && len > 0; n /= 10, --len)
            *--p = static_cast<char>(n % 10 + '0');
    }
    else
    {
        *--p = '0';
        --len;
    }
    if (zeroPadded)
        while (p > localBuff && len-- > 0)
            *--p = static_cast<char>('0');
    return addToBuff(p, buf, bufLim);
}

char *StringUtils::addToBuff(const char *str, char *buf, const char *bufLim)
{
    while ((buf < bufLim) && ((*buf = *str++) != '\0'))
        ++buf;
    return buf;
}

char *StringUtils::clearBuff(char buff[], std::size_t lim)
{
    strcpy(buff, "");
    (void)lim;  // For *nix we dont have anything using lim in above STRCPY macro
    return buff;
}

/// @brief Converts wchar* to char*
///        NOTE: Need to free return value after use!
char *StringUtils::wcharPtrToCharPtr(const wchar_t *line)
{
    std::size_t len_ = wcslen(line) + 1;
    char *buff_      = static_cast<char *>(malloc(len_ + 1));
    std::wcstombs(buff_, line, len_);
    return buff_;
}