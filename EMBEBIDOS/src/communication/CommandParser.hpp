#pragma once

#include <cstdint>
#include <cstddef>

namespace Communication
{
    struct ParsedCommand
    {
        static constexpr size_t MAX_ARGS    = 6;
        static constexpr size_t MAX_TOK_LEN = 32;

        char  verb[MAX_TOK_LEN]   { 0 };
        char  subverb[MAX_TOK_LEN]{ 0 };
        char  args[MAX_ARGS][MAX_TOK_LEN] {};
        size_t argCount           { 0 };
        bool   valid              { false };
    };

    class CommandParser
    {
    public:
        // Parsea una línea ASCII en `out`. Devuelve true si valid.
        // Termina en '\0', '\n' o '\r'. Trim espacios.
        // Ignora líneas vacías y comentarios (#).
        static bool parse(
            const char* line,
            ParsedCommand& out
        );

        // Helpers para handlers
        static bool argToInt(const char* arg, int& out);
        static bool argToFloat(const char* arg, float& out);

        // Comparación case-insensitive
        static bool equals(const char* a, const char* b);
    };
}