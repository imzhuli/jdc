#pragma once
#include "../base/_.hpp"

namespace jdc
{

    #define JDC_MAKE_CODEMARK(x)         CM_##x = ("//_JDC_CM]" X_STRINGIFY(x) "\n")
    #define JDC_MAKE_CODEMARK_PREFIX(x)  CM_##x = ("//_JDC_CM]" X_STRINGIFY(x) "_]")
    #define CM_ENDLINE ("\n")

    constexpr const char * JDC_MAKE_CODEMARK(PACKAGE_BEGIN);
    constexpr const char * JDC_MAKE_CODEMARK(PACKAGE_END);
    constexpr const char * JDC_MAKE_CODEMARK_PREFIX(CLASS_IDENTIFIER);
    constexpr const char * JDC_MAKE_CODEMARK_PREFIX(CLASS_BODY_BEGIN);
    constexpr const char * JDC_MAKE_CODEMARK_PREFIX(CLASS_BODY_END);
    constexpr const char * JDC_MAKE_CODEMARK(CLASS_FILE_END);

    #undef JDC_MAKE_SM

}
