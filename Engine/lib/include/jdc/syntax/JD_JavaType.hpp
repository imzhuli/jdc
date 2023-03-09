#pragma once
#include "../base/_.hpp"
#include "./_.hpp"
#include <string>

namespace jdc
{

    class iJavaType
    {
    public:
        virtual const std::string & GetUnfixedPackageBinaryName()   const { return _UnfixedPackageBinaryName; }
        virtual const std::string & GetUnfixedBinaryName()          const { return _UnfixedBinaryName; }
        virtual const std::string & GetFixedBinaryName()            const { return _FixedBinaryName; }
        virtual const std::string & GetFixedCodeName()              const { return _FixedCodeName; }
        virtual const std::string & GetSimpleBinaryName()           const { return _SimpleBinaryName; }
        virtual const std::string & GetSimpleCodeName()             const { return _SimpleCodeName; }
        virtual const std::string & GetInnermostCodeName()          const { return _InnermostCodeName; }
        virtual const std::string & GetSourceFilename()             const { return _SourceFilename.empty() ? _InnermostCodeName : _SourceFilename; }

        virtual bool               IsPrimitiveType()       const { return false; }
        virtual bool               IsObjectType()          const { return false; }
        virtual bool               IsInnerObjectType()     const { return false; }
        virtual bool               IsDirectType()          const { return false; }
        virtual size_t             GetDimension()          const { return 0; } // non-array type return 0;
        virtual const iJavaType *  GetOutterType()         const { return _OuterTypePtr; }

        virtual std::string ToString() const { return {}; }

    protected:
        std::string        _UnfixedPackageBinaryName;
        std::string        _UnfixedBinaryName;
        std::string        _FixedBinaryName;
        std::string        _FixedCodeName;
        std::string        _SimpleBinaryName;
        std::string        _SimpleCodeName;
        std::string        _InnermostCodeName;
        std::string        _SourceFilename;
        const iJavaType *  _OuterTypePtr = nullptr;
    };

}
