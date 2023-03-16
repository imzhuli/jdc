#pragma once
#include "./JD_JavaType.hpp"
#include <string>
#include <map>
#include <memory>

namespace jdc
{

    class xJavaObjectType
    : public iJavaType
    {
    public:
        X_INLINE bool IsPrimitiveType()  const override { return false; }
        X_INLINE bool IsDirectType()     const override { return true; }
        X_INLINE bool IsObjectType()     const override { return true; }

        X_PRIVATE_MEMBER static bool IsDefaultAnnotationBase(iJavaType * JavaTypePtr);
        X_PRIVATE_MEMBER static bool IsDefaultAnnotationBase(const std::string & BinaryName);
        X_PRIVATE_MEMBER static bool IsDefaultClassBase(iJavaType * JavaTypePtr);
        X_PRIVATE_MEMBER static bool IsDefaultClassBase(const std::string & BinaryName);

    private:
        friend void AddJavaObjectType(const char * BinaryName);
    };

    X_PRIVATE bool InitJavaObjectTypes();
    X_PRIVATE void CleanJavaObjectTypes();
    X_PRIVATE const std::map<std::string, std::unique_ptr<xJavaObjectType>> & GetJavaObjectTypeMap();
    X_PRIVATE const std::map<std::string, const xJavaObjectType *> & GetJavaObjectTypeCodeNameMap();

}
