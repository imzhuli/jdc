#include <jdc/syntax/JD_JavaObjectTypes.hpp>

namespace jdc
{
    static std::map<std::string, std::unique_ptr<xJavaObjectType>> JavaObjectTypeMap;
    static std::string DefaultBaseClassBinaryName = "java/lang/Object";

    xJavaType * TYPE_BOOLEAN             = nullptr;
    xJavaType * TYPE_BYTE                = nullptr;
    xJavaType * TYPE_CHARACTER           = nullptr;
    xJavaType * TYPE_CLASS               = nullptr;
    xJavaType * TYPE_CLASS_WILDCARD      = nullptr;
    xJavaType * TYPE_DOUBLE              = nullptr;
    xJavaType * TYPE_EXCEPTION           = nullptr;
    xJavaType * TYPE_FLOAT               = nullptr;
    xJavaType * TYPE_INTEGER             = nullptr;
    xJavaType * TYPE_ITERABLE            = nullptr;
    xJavaType * TYPE_LONG                = nullptr;
    xJavaType * TYPE_MATH                = nullptr;
    xJavaType * TYPE_OBJECT              = nullptr;
    xJavaType * TYPE_RUNTIME_EXCEPTION   = nullptr;
    xJavaType * TYPE_SHORT               = nullptr;
    xJavaType * TYPE_STRING              = nullptr;
    xJavaType * TYPE_STRING_BUFFER       = nullptr;
    xJavaType * TYPE_STRING_BUILDER      = nullptr;
    xJavaType * TYPE_SYSTEM              = nullptr;
    xJavaType * TYPE_THREAD              = nullptr;
    xJavaType * TYPE_THROWABLE           = nullptr;

    xJavaObjectType * AddJavaObjectType(const char * BinaryName)
    {
        auto TypeUPtr = std::make_unique<xJavaObjectType>();
        auto TypePtr = TypeUPtr.get();
        TypePtr->_UnfixedBinaryName        = BinaryName;
        TypePtr->_FixedBinaryName          = TypePtr->_UnfixedBinaryName;
        TypePtr->_FixedCodeName            = ConvertBinaryNameToCodeName(BinaryName);
        TypePtr->_SimpleBinaryName         = GetInnermostClassName(BinaryName);
        TypePtr->_SimpleCodeName           = TypePtr->_SimpleBinaryName;
        TypePtr->_InnermostName            = TypePtr->_SimpleBinaryName;

        JavaObjectTypeMap.insert(std::make_pair(BinaryName, std::move(TypeUPtr)));
        return TypePtr;
    }

    bool InitJavaObjectTypes()
    {
        TYPE_BOOLEAN              = AddJavaObjectType("java/lang/Boolean");
        TYPE_BYTE                 = AddJavaObjectType("java/lang/Byte");
        TYPE_CHARACTER            = AddJavaObjectType("java/lang/Character");
        TYPE_CLASS                = AddJavaObjectType("java/lang/Class");
        TYPE_DOUBLE               = AddJavaObjectType("java/lang/Double");
        TYPE_EXCEPTION            = AddJavaObjectType("java/lang/Exception");
        TYPE_FLOAT                = AddJavaObjectType("java/lang/Float");
        TYPE_INTEGER              = AddJavaObjectType("java/lang/Integer");
        TYPE_ITERABLE             = AddJavaObjectType("java/lang/Iterable");
        TYPE_LONG                 = AddJavaObjectType("java/lang/Long");
        TYPE_MATH                 = AddJavaObjectType("java/lang/Math");
        TYPE_OBJECT               = AddJavaObjectType("java/lang/Object");
        TYPE_RUNTIME_EXCEPTION    = AddJavaObjectType("java/lang/RuntimeException");
        TYPE_SHORT                = AddJavaObjectType("java/lang/Short");
        TYPE_STRING               = AddJavaObjectType("java/lang/String");
        TYPE_STRING_BUFFER        = AddJavaObjectType("java/lang/StringBuffer");
        TYPE_STRING_BUILDER       = AddJavaObjectType("java/lang/StringBuilder");
        TYPE_SYSTEM               = AddJavaObjectType("java/lang/System");
        TYPE_THREAD               = AddJavaObjectType("java/lang/Thread");
        TYPE_THROWABLE            = AddJavaObjectType("java/lang/Throwable");

        // for (auto & Entry : JavaObjectTypeMap) {
        //     X_DEBUG_PRINTF("Init object type: %s --> %s\n", Entry.second->GetInnermostName().c_str(), Entry.first.c_str());
        // }

        return true;
    };

    void CleanJavaObjectTypes()
    {
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

    const std::string & GetShorterBinaryName(const std::string & FullBinaryName)
    {
        auto & Map = GetJavaObjectTypeMap();
        auto Iter = Map.find(FullBinaryName);
        if (Iter == Map.end()) {
            return FullBinaryName;
        }
        return Iter->second->GetSimpleBinaryName();
    }

    std::map<std::string, std::unique_ptr<xJavaObjectType>> & GetJavaObjectTypeMap()
    {
        return JavaObjectTypeMap;
    }

}
