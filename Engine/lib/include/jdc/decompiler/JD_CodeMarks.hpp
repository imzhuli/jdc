#pragma once
#include "../base/_.hpp"

namespace jdc
{

    #define JDC_MAKE_CODEMARK(x)  CM_##x = ("//[XEL_JDC_SM]" X_STRINGIFY(x) "\n")

    constexpr const char * JDC_MAKE_CODEMARK(PACKAGE_BEGIN);
    constexpr const char * JDC_MAKE_CODEMARK(PACKAGE_END);
    constexpr const char * JDC_MAKE_CODEMARK(CLASS_IDENTIFIER_);
    constexpr const char * JDC_MAKE_CODEMARK(CLASS_BODY_BEGIN_);
    constexpr const char * JDC_MAKE_CODEMARK(CLASS_BODY_END_);

    #undef JDC_MAKE_SM

}
