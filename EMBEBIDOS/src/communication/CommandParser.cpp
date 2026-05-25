#include "communication/CommandParser.hpp"

#include <cstring>
#include <cstdlib>
#include <cctype>

namespace Communication
{
    static char tolower_ascii(char c)
    {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
    }

    bool CommandParser::equals(const char* a, const char* b)
    {
        if (a == nullptr || b == nullptr) return false;

        size_t i = 0;
        while (a[i] != '\0' && b[i] != '\0')
        {
            if (tolower_ascii(a[i]) != tolower_ascii(b[i]))
            {
                return false;
            }
            ++i;
        }
        return a[i] == '\0' && b[i] == '\0';
    }

    bool CommandParser::argToInt(const char* arg, int& out)
    {
        if (arg == nullptr || *arg == '\0') return false;

        char* end = nullptr;
        const long v = strtol(arg, &end, 10);
        if (end == arg) return false;

        out = static_cast<int>(v);
        return true;
    }

    bool CommandParser::argToFloat(const char* arg, float& out)
    {
        if (arg == nullptr || *arg == '\0') return false;

        char* end = nullptr;
        const float v = strtof(arg, &end);
        if (end == arg) return false;

        out = v;
        return true;
    }

    static void copyToken(
        const char* src,
        size_t len,
        char* dst,
        size_t dstSize
    )
    {
        if (len >= dstSize) len = dstSize - 1;
        for (size_t i = 0; i < len; ++i) dst[i] = src[i];
        dst[len] = '\0';
    }

    bool CommandParser::parse(const char* line, ParsedCommand& out)
    {
        out = ParsedCommand {};

        if (line == nullptr) return false;

        // Saltar whitespace inicial
        while (*line == ' ' || *line == '\t') ++line;

        // Comentario o vacío
        if (*line == '\0' || *line == '\n' || *line == '\r' || *line == '#')
        {
            return false;
        }

        // Tokenizar
        size_t tokenIndex = 0;
        const char* p = line;

        while (*p != '\0' && *p != '\n' && *p != '\r')
        {
            // Saltar separadores
            while (*p == ' ' || *p == '\t') ++p;
            if (*p == '\0' || *p == '\n' || *p == '\r') break;

            // Marca inicio de token
            const char* tokStart = p;
            while (*p != '\0' && *p != ' ' && *p != '\t' &&
                   *p != '\n' && *p != '\r')
            {
                ++p;
            }
            const size_t tokLen = static_cast<size_t>(p - tokStart);

            if (tokenIndex == 0)
            {
                copyToken(tokStart, tokLen, out.verb, ParsedCommand::MAX_TOK_LEN);
            }
            else if (tokenIndex == 1)
            {
                copyToken(tokStart, tokLen, out.subverb, ParsedCommand::MAX_TOK_LEN);
            }
            else
            {
                const size_t ai = tokenIndex - 2;
                if (ai < ParsedCommand::MAX_ARGS)
                {
                    copyToken(tokStart, tokLen,
                              out.args[ai], ParsedCommand::MAX_TOK_LEN);
                    out.argCount = ai + 1;
                }
            }

            ++tokenIndex;
        }

        out.valid = (out.verb[0] != '\0');
        return out.valid;
    }
}