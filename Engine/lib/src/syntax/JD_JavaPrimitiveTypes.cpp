#include <jdc/syntax/JD_JavaPrimitiveTypes.hpp>

namespace jdc
{
    static std::map<std::string, xJavaPrimitiveType> PrimativeJavaTypeMap;

    void AddJavaPrimitiveType(const char * Name, xJavaPrimitiveType::xFlag TypeFlag)
    {
        auto Type = xJavaPrimitiveType();
        Type._UnfixedBinaryName        = Name;
        Type._FixedBinaryName          = Name;
        Type._FixedCodeName            = Name;
        Type._SimpleBinaryName         = Name;
        Type._SimpleCodeName           = Name;
        Type._InnermostCodeName        = Name;
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

        for (auto & Entry : PrimativeJavaTypeMap) {
            X_DEBUG_PRINTF("Init primitive type: %s --> %s\n", Entry.second.GetInnermostCodeName().c_str(), Entry.first.c_str());
        }

        return true;
    }

    void CleanJavaPrimitiveTypes()
    {
        xel::Renew(PrimativeJavaTypeMap);
    }

    const std::map<std::string, xJavaPrimitiveType> & GetJavaPrimitiveTypeMap()
    {
        return PrimativeJavaTypeMap;
    }

}
