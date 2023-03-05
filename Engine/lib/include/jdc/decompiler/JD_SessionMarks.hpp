#pragma once
#include "../base/JD_Base.hpp"

namespace jdc
{

    #define JDC_MAKE_SM(x)  SM_##x = ("//__XEL_JDC_SM_" X_STRINGIFY(x))

    constexpr const char * JDC_MAKE_SM(PACKAGE);

    constexpr const char * JDC_MAKE_SM(CLASS_IDENTIFIER_);

    constexpr const char * JDC_MAKE_SM(CLASS_BODY_BEGIN_);
    constexpr const char * JDC_MAKE_SM(CLASS_BODY_END_);

    #undef JDC_MAKE_SM

}
