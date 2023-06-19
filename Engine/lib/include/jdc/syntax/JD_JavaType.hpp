#pragma once
#include "../base/_.hpp"
#include "./_.hpp"
#include <string>

namespace jdc
{

    class xJavaType
    : xel::xVBase
    {
    public:
        virtual const std::string & GetUnfixedDescriptor()          const { return _UnfixedDescriptor; }
        virtual const std::string & GetUnfixedPackageBinaryName()   const { return _UnfixedPackageBinaryName; }
        virtual const std::string & GetUnfixedBinaryName()          const { return _UnfixedBinaryName; }
        virtual const std::string & GetFixedBinaryName()            const { return _FixedBinaryName; }
        virtual const std::string & GetFixedCodeName()              const { return _FixedCodeName; }
        virtual const std::string & GetSimpleBinaryName()           const { return _SimpleBinaryName; }
        virtual const std::string & GetSimpleCodeName()             const { return _SimpleCodeName; }
        virtual const std::string & GetInnermostName()              const { return _InnermostName; }
        virtual const std::string & GetSourceFilename()             const { return _SourceFilename.empty() ? _InnermostName : _SourceFilename; }

        virtual bool               IsPrimitiveType()       const { return false; }
        virtual bool               IsObjectType()          const { return false; }
        virtual bool               IsInnerObjectType()     const { return false; }
        virtual bool               IsDirectType()          const { return false; }
        virtual size_t             GetDimension()          const { return 0; } // non-array type return 0;
        virtual const xJavaType *  GetOutterType()         const { return _OuterTypePtr; }

        virtual std::string TypeNameString() const { return {}; }

    protected:
        std::string        _UnfixedDescriptor;
        std::string        _UnfixedPackageBinaryName;
        std::string        _UnfixedBinaryName;
        std::string        _FixedBinaryName;
        std::string        _FixedCodeName;
        std::string        _SimpleBinaryName;
        std::string        _SimpleCodeName;
        std::string        _InnermostName;
        std::string        _SourceFilename;
        const xJavaType *  _OuterTypePtr = nullptr;
    };

    X_PRIVATE xJavaType * TYPE_BOOLEAN;
    X_PRIVATE xJavaType * TYPE_BYTE;
    X_PRIVATE xJavaType * TYPE_CHARACTER;
    X_PRIVATE xJavaType * TYPE_CLASS;
    X_PRIVATE xJavaType * TYPE_CLASS_WILDCARD;
    X_PRIVATE xJavaType * TYPE_DOUBLE;
    X_PRIVATE xJavaType * TYPE_EXCEPTION;
    X_PRIVATE xJavaType * TYPE_FLOAT;
    X_PRIVATE xJavaType * TYPE_INTEGER;
    X_PRIVATE xJavaType * TYPE_ITERABLE;
    X_PRIVATE xJavaType * TYPE_LONG;
    X_PRIVATE xJavaType * TYPE_MATH;
    X_PRIVATE xJavaType * TYPE_OBJECT;
    X_PRIVATE xJavaType * TYPE_RUNTIME_EXCEPTION;
    X_PRIVATE xJavaType * TYPE_SHORT;
    X_PRIVATE xJavaType * TYPE_STRING;
    X_PRIVATE xJavaType * TYPE_STRING_BUFFER;
    X_PRIVATE xJavaType * TYPE_STRING_BUILDER;
    X_PRIVATE xJavaType * TYPE_SYSTEM;
    X_PRIVATE xJavaType * TYPE_THREAD;
    X_PRIVATE xJavaType * TYPE_THROWABLE;

}
