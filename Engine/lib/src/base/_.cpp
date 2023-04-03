#include <jdc/base/_.hpp>
#include <xel/String.hpp>

using namespace xel;

namespace jdc
{

    static std::string Unicode(char16_t c) {
        char Buffer[7] = "\\u";

        uint16_t h = (c >> 12);
        Buffer[2] = (char) ((h <= 9) ? (h + '0') : (h + ('A' - 10)));
        h = (c >> 8 ) & 15;
        Buffer[3] = (char) ((h <= 9) ? (h + '0') : (h + ('A' - 10)));
        h = (c >> 4) & 15;
        Buffer[4] = (char) ((h <= 9) ? (h + '0') : (h + ('A' - 10)));
        h = c & 15;
        Buffer[5] = (char) ((h <= 9) ? (h + '0') : (h + ('A' - 10)));

        return std::string(Buffer);
    }

    std::string EscapeJavaChar(char16_t C)
    {
        switch (C) {
        case '\\':
            return "\\\\"s;
        case '\b':
            return "\\b"s;
        case '\f':
            return "\\f"s;
        case '\n':
            return "\\n"s;
        case '\r':
            return "\\r"s;
        case '\t':
            return "\\t"s;
        case '\'':
            return "\\'"s;
        case 173: // SOFT HYPHEN
            return Unicode(C);
        default:
            if (C < ' ') {
                return "\\0"s + ((char)('0' + (C >> 3))) + ((char)('0' + (C & 7)));
            }
            if (C < 127) {
                return { (const char *)X2Ptr((char)C), 1 };
            }
            if (C < 161) {
                return Unicode(C);
            }
            return { (const char *)&C, 1 };
        }
    }




}
