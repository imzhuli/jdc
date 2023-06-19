#pragma once
#include "./JD_JavaType.hpp"
#include <string>
#include <map>
#include <memory>

namespace jdc
{

    class xJavaObjectType
    : public xJavaType
    {
    public:
        X_INLINE bool IsPrimitiveType()  const override { return false; }
        X_INLINE bool IsDirectType()     const override { return true; }
        X_INLINE bool IsObjectType()     const override { return true; }

        X_PRIVATE_STATIC_MEMBER bool IsDefaultAnnotationBase(xJavaType * JavaTypePtr);
        X_PRIVATE_STATIC_MEMBER bool IsDefaultAnnotationBase(const std::string & BinaryName);
        X_PRIVATE_STATIC_MEMBER bool IsDefaultClassBase(xJavaType * JavaTypePtr);
        X_PRIVATE_STATIC_MEMBER bool IsDefaultClassBase(const std::string & BinaryName);

    private:
        friend xJavaObjectType * AddJavaObjectType(const char * BinaryName);
    };

    X_PRIVATE bool InitJavaObjectTypes();
    X_PRIVATE void CleanJavaObjectTypes();
    X_PRIVATE const std::map<std::string, std::unique_ptr<xJavaObjectType>> & GetJavaObjectTypeMap();
    X_PRIVATE const std::map<std::string, const xJavaObjectType *> & GetJavaObjectTypeCodeNameMap();

}
