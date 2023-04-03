#include <jdc/syntax/JD_JavaObjectTypes.hpp>

namespace jdc
{
    static std::map<std::string, std::unique_ptr<xJavaObjectType>> JavaObjectTypeMap;
    static std::map<std::string, const xJavaObjectType *>          JavaObjectTypeCodeNameMap;
    static std::string DefaultBaseClassBinaryName = "java/lang/Object";

    void AddJavaObjectType(const char * BinaryName)
    {
        auto TypeUPtr = std::make_unique<xJavaObjectType>();
        auto & Type = *TypeUPtr;
        Type._UnfixedBinaryName        = BinaryName;
        Type._FixedBinaryName          = Type._UnfixedBinaryName;
        Type._FixedCodeName            = ConvertBinaryNameToCodeName(BinaryName);
        Type._SimpleBinaryName         = GetInnermostClassName(BinaryName);
        Type._SimpleCodeName           = Type._SimpleBinaryName;
        Type._InnermostName            = Type._SimpleBinaryName;

        JavaObjectTypeMap.insert(std::make_pair(BinaryName, std::move(TypeUPtr)));
        JavaObjectTypeCodeNameMap.insert(std::make_pair(Type._FixedCodeName, &Type));
    }

    bool InitJavaObjectTypes()
    {
        AddJavaObjectType("java/lang/Boolean");
        AddJavaObjectType("java/lang/Byte");
        AddJavaObjectType("java/lang/Character");
        AddJavaObjectType("java/lang/Class");
        AddJavaObjectType("java/lang/Double");
        AddJavaObjectType("java/lang/Exception");
        AddJavaObjectType("java/lang/Float");
        AddJavaObjectType("java/lang/Integer");
        AddJavaObjectType("java/lang/Iterable");
        AddJavaObjectType("java/lang/Long");
        AddJavaObjectType("java/lang/Math");
        AddJavaObjectType("java/lang/Object");
        AddJavaObjectType("java/lang/RuntimeException");
        AddJavaObjectType("java/lang/Short");
        AddJavaObjectType("java/lang/String");
        AddJavaObjectType("java/lang/StringBuffer");
        AddJavaObjectType("java/lang/StringBuilder");
        AddJavaObjectType("java/lang/System");
        AddJavaObjectType("java/lang/Thread");
        AddJavaObjectType("java/lang/Throwable");

        for (auto & Entry : JavaObjectTypeMap) {
            X_DEBUG_PRINTF("Init object type: %s --> %s\n", Entry.second->GetInnermostName().c_str(), Entry.first.c_str());
        }
        for (auto & Entry : JavaObjectTypeCodeNameMap) {
            X_DEBUG_PRINTF("Init object type (code name): %s --> %s\n", Entry.second->GetSimpleCodeName().c_str(), Entry.first.c_str());
        }

        return true;
    };

    void CleanJavaObjectTypes()
    {
        xel::Renew(JavaObjectTypeCodeNameMap);
        xel::Renew(JavaObjectTypeMap);
    }

    bool xJavaObjectType::IsDefaultAnnotationBase(xJavaType * JavaTypePtr)
    {
        auto & BinaryName = JavaTypePtr->GetUnfixedBinaryName();
        return BinaryName == DefaultBaseClassBinaryName;
    }

    bool xJavaObjectType::IsDefaultAnnotationBase(const std::string & BinaryName)
    {
        return BinaryName == DefaultBaseClassBinaryName;
    }

    bool xJavaObjectType::IsDefaultClassBase(xJavaType * JavaTypePtr)
    {
        auto & BinaryName = JavaTypePtr->GetUnfixedBinaryName();
        return BinaryName == DefaultBaseClassBinaryName;
    }

    bool xJavaObjectType::IsDefaultClassBase(const std::string & BinaryName)
    {
        return BinaryName == DefaultBaseClassBinaryName;
    }

    const std::map<std::string, std::unique_ptr<xJavaObjectType>> & GetJavaObjectTypeMap()
    {
        return JavaObjectTypeMap;
    }

    const std::map<std::string, const xJavaObjectType *> & GetJavaObjectTypeCodeNameMap()
    {
        return JavaObjectTypeCodeNameMap;
    }

}
