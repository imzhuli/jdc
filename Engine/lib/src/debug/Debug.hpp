#pragma once

#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <string>
#include <map>
#include <set>

namespace jdc
{

    class iDebugManager
    {
    public:
        virtual bool Init(const std::string & ConfigFilename) = 0;
        virtual void Clean() = 0;

        virtual bool IsDebugMethod(const class xJavaMthod * JavaMethodPtr) = 0;

    private:
        std::set<std::string> DebugMethodNames;
    };

    X_PRIVATE iDebugManager * DebugManagerPtr;


}
