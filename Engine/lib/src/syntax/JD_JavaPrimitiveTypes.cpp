#include <jdc/syntax/JD_JavaPrimitiveTypes.hpp>

namespace jdc
{
    static std::map<std::string, xJavaPrimitiveType> PrimativeJavaTypeMap;

    std::string xJavaPrimitiveType::TypeNameString() const
    {
        switch(_TypeFlag) {
            case FLAG_BOOLEAN: {
                return "boolean"s;
            }
            case FLAG_CHAR: {
                return "char"s;
            }
            case FLAG_FLOAT: {
                return "float"s;
            }
            case FLAG_DOUBLE: {
                return "double"s;
            }
            case FLAG_BYTE: {
                return "byte"s;
            }
            case FLAG_SHORT: {
                return "short"s;
            }
            case FLAG_INT: {
                return "int"s;
            }
            case FLAG_LONG: {
                return "long"s;
            }
            case FLAG_VOID: {
                return "void"s;
            }
        }
        return {};
    }

    xJavaPrimitiveType::xFlag xJavaPrimitiveType::GetLeastTypeFlagFromValue(int64_t Value)
    {
        if (Value >= 0) {
            if (Value <= 1) {
                return FLAG_BOOLEAN;
            }
            if (Value <= std::numeric_limits<int8_t>::max()) {
                return FLAG_BYTE;
            }
            if (Value <= std::numeric_limits<int16_t>::max()) {
                return FLAG_SHORT;
            }
            if (Value <= std::numeric_limits<int32_t>::max()) {
                return FLAG_INT;
            }
        } else {
            if (Value >= std::numeric_limits<int8_t>::min()) {
                return FLAG_BYTE;
            }
            if (Value >= std::numeric_limits<int16_t>::min()) {
                return FLAG_SHORT;
            }
            if (Value >= std::numeric_limits<int32_t>::min()) {
                return FLAG_INT;
            }
        }
        return FLAG_LONG;
    };

    void AddJavaPrimitiveType(const char * Name, xJavaPrimitiveType::xFlag TypeFlag)
    {
        auto Type = xJavaPrimitiveType();
        Type._UnfixedBinaryName        = Name;
        Type._FixedBinaryName          = Name;
        Type._FixedCodeName            = Name;
        Type._SimpleBinaryName         = Name;
        Type._SimpleCodeName           = Name;
        Type._InnermostName            = Name;
        Type._TypeFlag                 = TypeFlag;

        PrimativeJavaTypeMap.insert(std::make_pair(Name, std::move(Type)));
    }

    bool InitJavaPrimitiveTypes()
    {
        AddJavaPrimitiveType("boolean", xJavaPrimitiveType::FLAG_BOOLEAN);
        AddJavaPrimitiveType("byte",    xJavaPrimitiveType::FLAG_BYTE);
        AddJavaPrimitiveType("char",    xJavaPrimitiveType::FLAG_CHAR);
        AddJavaPrimitiveType("double",  xJavaPrimitiveType::FLAG_DOUBLE);
        AddJavaPrimitiveType("float",   xJavaPrimitiveType::FLAG_FLOAT);
        AddJavaPrimitiveType("int",     xJavaPrimitiveType::FLAG_INT);
        AddJavaPrimitiveType("long",    xJavaPrimitiveType::FLAG_LONG);
        AddJavaPrimitiveType("short",   xJavaPrimitiveType::FLAG_SHORT);
        AddJavaPrimitiveType("void",    xJavaPrimitiveType::FLAG_VOID);
        return true;
    }

    void CleanJavaPrimitiveTypes()
    {
        // xel::Renew(PrimativeJavaTypeMap);
    }

    std::map<std::string, xJavaPrimitiveType> & GetJavaPrimitiveTypeMap()
    {
        return PrimativeJavaTypeMap;
    }

}
