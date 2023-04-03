#pragma once
#include "./JD_JavaType.hpp"
#include <xel/String.hpp>
#include <limits>
#include <map>

namespace jdc
{

    class xJavaPrimitiveType
    : public xJavaType
    {
    public:
        using xFlag = uint16_t;
        static constexpr const xFlag FLAG_BOOLEAN = 1 << 0;
        static constexpr const xFlag FLAG_CHAR    = 1 << 1;
        static constexpr const xFlag FLAG_FLOAT   = 1 << 2;
        static constexpr const xFlag FLAG_DOUBLE  = 1 << 3;
        static constexpr const xFlag FLAG_BYTE    = 1 << 4;
        static constexpr const xFlag FLAG_SHORT   = 1 << 5;
        static constexpr const xFlag FLAG_INT     = 1 << 6;
        static constexpr const xFlag FLAG_LONG    = 1 << 7;
        static constexpr const xFlag FLAG_VOID    = 1 << 8;

    public:
        X_INLINE bool IsPrimitiveType()  const override { return true;  }
        X_INLINE bool IsDirectType()     const override { return true;  }
        X_INLINE bool IsObjectType()     const override { return false; }

        X_INLINE xFlag GetTypeFlag() const { return _TypeFlag; }
        X_PRIVATE_MEMBER std::string TypeNameString() const override;

        X_PRIVATE_STATIC_MEMBER xFlag GetLeastTypeFlagFromValue(int64_t Value);

    private:
        xFlag _TypeFlag;

    private:
        friend void AddJavaPrimitiveType(const char * Name, xJavaPrimitiveType::xFlag TypeFlag);
    };

    X_PRIVATE bool  InitJavaPrimitiveTypes();
    X_PRIVATE void  CleanJavaPrimitiveTypes();
    X_PRIVATE const std::map<std::string, xJavaPrimitiveType> & GetJavaPrimitiveTypeMap();

}
