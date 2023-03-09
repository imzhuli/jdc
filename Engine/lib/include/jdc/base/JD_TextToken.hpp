#pragma once
#include "./_.hpp"

namespace jdc
{

    struct TextTokens final
    {
        static constexpr const char * AT = "@";
        static constexpr const char * COMMA = ",";
        static constexpr const char * COLON = ":";
        static constexpr const char * COLON_COLON = "::";
        static constexpr const char * COMMA_SPACE = ", ";
        static constexpr const char * DIAMOND = "<>";
        static constexpr const char * DOT = ".";
        static constexpr const char * DIMENSION = "[]";
        static constexpr const char * INFINITE_FOR = " (;;)";
        static constexpr const char * LEFTRIGHTCURLYBRACKETS = "{}";
        static constexpr const char * LEFTROUNDBRACKET = "(";
        static constexpr const char * RIGHTROUNDBRACKET = ")";
        static constexpr const char * LEFTRIGHTROUNDBRACKETS = "()";
        static constexpr const char * LEFTANGLEBRACKET = "<";
        static constexpr const char * RIGHTANGLEBRACKET = ">";
        static constexpr const char * QUESTIONMARK = "?";
        static constexpr const char * QUESTIONMARK_SPACE = "? ";
        static constexpr const char * SPACE = " ";
        static constexpr const char * SPACE_AND_SPACE = " & ";
        static constexpr const char * SPACE_ARROW_SPACE = " -> ";
        static constexpr const char * SPACE_COLON_SPACE = " : ";
        static constexpr const char * SPACE_EQUAL_SPACE = " = ";
        static constexpr const char * SPACE_QUESTION_SPACE = " ? ";
        static constexpr const char * SPACE_LEFTROUNDBRACKET = " (";
        static constexpr const char * SEMICOLON = ";";
        static constexpr const char * SEMICOLON_SPACE = "; ";
        static constexpr const char * VARARGS = "... ";
        static constexpr const char * VERTICALLINE = "|";
        static constexpr const char * EXCLAMATION = "!";
    };

}
