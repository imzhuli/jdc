#pragma once
#include "../base/JD_.hpp"
#include <string>
#include <vector>

namespace jdc
{

    class xJavaPackage;
    class xJavaClass;
    class xJavaSpace;

    class xJavaPackage
    {
    public:
        xJavaSpace * JavaSpacePtr = nullptr;

        std::string UnfixedBinaryName;
        std::string UnfixedPathName;
        std::string UnfixedCodeName;

        std::string FixedBinaryName;
        std::string FixedPathName;
        std::string FixedCodeName;

        std::vector<xJavaClass*> Classes;
    };


}
