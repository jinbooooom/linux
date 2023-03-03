#pragma once

#include <cctype>
#include <string>

class StringUtils
{
  public:
    /// @brief Checks if character is digit. Dont use libc implementation of it to prevent locale issues.
    static inline bool isDigit(char c) { return c >= '0' && c <= '9'; }

    /// @brief Matches wildcards, '*' and '?' only supported.
    static bool wildCardMatch(const char *str, const char *pattern);

    static std::string &ltrim(std::string &str);
    static std::string &rtrim(std::string &str);
    static std::string &trim(std::string &str);

    /// @brief Determines whether or not str starts with specified string
    /// @param str String to check
    /// @param start String to check against
    /// @return Returns true if starts with specified string, false otherwise
    static bool startsWith(const std::string &str, const std::string &start);

    /// @brief Determines whether or not str ends with specified string
    /// @param str String to check
    /// @param end String to check against
    /// @return Returns true if ends with specified string, false otherwise
    static bool endsWith(const std::string &str, const std::string &end);

    /// @brief Replaces all instances of replaceWhat with 'replaceWith'. Original variable is changed for performance.
    /// @param [in,out] str String to replace from
    /// @param replaceWhat Character to replace
    /// @param replaceWith Character to replace with
    /// @return Modified version of str
    static std::string &replaceAll(std::string &str, char replaceWhat, char replaceWith);

    /// @brief Replaces all instances of 'replaceWhat' with 'replaceWith'. (String version) Replaces in place
    /// @param str String to replace from
    /// @param replaceWhat Character to replace
    /// @param replaceWith Character to replace with
    /// @return Modified (original) str
    static std::string &replaceAll(std::string &str, const std::string &replaceWhat, const std::string &replaceWith);

    /// @brief Converts string to uppercase
    /// @param str String to convert
    /// @return Uppercase string
    static std::string &toUpper(std::string &str);

    /// @brief Compares cstring equality - uses strcmp
    static bool cStringEq(const char *s1, const char *s2);

    /// @brief Compares cstring equality (case-insensitive) - uses toupper(char)
    /// Dont use strcasecmp because of CRT (VC++)
    static bool cStringCaseEq(const char *s1, const char *s2);

    /// @brief Returns true if c exist in str
    static bool contains(const char *str, char c);

    static char *convertAndAddToBuff(std::size_t n, int len, char *buf, const char *bufLim, bool zeroPadded = true);
    static char *addToBuff(const char *str, char *buf, const char *bufLim);
    static char *clearBuff(char buff[], std::size_t lim);

    /// @brief Converts wchar* to char*
    ///        NOTE: Need to free return value after use!
    static char *wcharPtrToCharPtr(const wchar_t *line);

  private:
    StringUtils(void);
    StringUtils(const StringUtils &);
    StringUtils &operator=(const StringUtils &);
};