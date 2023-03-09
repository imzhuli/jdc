#include <jdc/syntax/JD_JavaObjectTypes.hpp>

namespace jdc
{
    static std::map<std::string, xJavaObjectType> JavaObjectTypeMap;
    static std::string DefaultBaseClassBinaryName = "java/lang/Object";

    void AddJavaObjectType(const char * BinaryName)
    {
        auto Type = xJavaObjectType();
        Type._UnfixedBinaryName        = BinaryName;
        Type._FixedBinaryName          = Type._UnfixedBinaryName;
        Type._FixedCodeName            = ConvertBinaryNameToCodeName(BinaryName);
        Type._SimpleBinaryName         = GetInnermostClassCodeName(BinaryName);
        Type._SimpleCodeName           = Type._SimpleBinaryName;
        Type._InnermostCodeName        = Type._SimpleBinaryName;

        JavaObjectTypeMap.insert(std::make_pair(BinaryName, std::move(Type)));
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
            X_DEBUG_PRINTF("Init object type: %s --> %s\n", Entry.second.GetInnermostCodeName().c_str(), Entry.first.c_str());
        }

        return true;
    };

    void CleanJavaObjectTypes()
    {
        xel::Renew(JavaObjectTypeMap);
    }

    const std::map<std::string, xJavaObjectType> & GetJavaObjectTypeMap()
    {
        return JavaObjectTypeMap;
    }

    bool xJavaObjectType::IsDefaultAnnotationBase(iJavaType * JavaTypePtr)
    {
        auto & BinaryName = JavaTypePtr->GetUnfixedBinaryName();
        return BinaryName == DefaultBaseClassBinaryName;
    }

    bool xJavaObjectType::IsDefaultAnnotationBase(const std::string & BinaryName)
    {
        return BinaryName == DefaultBaseClassBinaryName;
    }

    bool xJavaObjectType::IsDefaultClassBase(iJavaType * JavaTypePtr)
    {
        auto & BinaryName = JavaTypePtr->GetUnfixedBinaryName();
        return BinaryName == DefaultBaseClassBinaryName;
    }

    bool xJavaObjectType::IsDefaultClassBase(const std::string & BinaryName)
    {
        return BinaryName == DefaultBaseClassBinaryName;
    }



}
