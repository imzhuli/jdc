#pragma once
#include "../base/_.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include "./JD_JavaDeclaration.hpp"
#include <string>
#include <vector>

namespace jdc
{

    class xJavaClass;

    class xJavaMethod
    {
    public:
        const xJavaClass *                JavaClassPtr;
        const xMethodInfo *               MethodInfoPtr;
        std::string                       OriginalName;
        std::string                       FixedName;

        struct {
        } Extend;

        struct {
            std::string                               FixedReturnTypeCodeName;
            std::vector<std::string>                  FixedParameterTypeCodeNames;
            xAttributeMap                             AttributeMap;
            xAnnotationDeclarations                   AnnotationDeclarations;
            std::vector<xAnnotationDeclarations>      ParameterAnnotationDeclarations;
        } Converted;

        X_PRIVATE_MEMBER bool HasAnImplicitParameter() const;
        X_PRIVATE_MEMBER void DoConvert();
    };

}
