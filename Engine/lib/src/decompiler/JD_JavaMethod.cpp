#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <jdc/syntax/_.hpp>
#include <xel/String.hpp>

namespace jdc
{

    bool xJavaMethod::HasAnImplicitParameter() const
    {
        return JavaClassPtr->IsInnerClass() && !JavaClassPtr->IsStatic() && OriginalName == "<init>"s;
    }

    void xJavaMethod::DoConvert()
    {
        auto & JavaSpace = *JavaClassPtr->JavaSpacePtr;
        auto & ClassInfo = JavaClassPtr->ClassInfo;

        auto Descriptor = ClassInfo.GetConstantUtf8(MethodInfoPtr->DescriptorIndex);
        auto UnfixedTypeBinaryNames = ConvertMethodDescriptorToBinaryNames(Descriptor);

        assert(Converted.FixedReturnTypeCodeName.empty());
        assert(Converted.FixedParameterTypeCodeNames.empty());
        Converted.FixedReturnTypeCodeName = JavaSpace.GetFixedClassCodeName(UnfixedTypeBinaryNames.ReturnTypeBinaryName);

        size_t ParamCounter = 0;
        for (auto & ParameterBinaryName : UnfixedTypeBinaryNames.ParameterTypeBinaryNames) {
            const auto FixedParameterCodeName = JavaSpace.GetFixedClassCodeName(ParameterBinaryName);
            Converted.FixedParameterTypeCodeNames.push_back(FixedParameterCodeName);
            Converted.FixedParameterNames.push_back("arg_"s + std::to_string(ParamCounter++));
        }

        Converted.AttributeMap = LoadAttributeInfo(MethodInfoPtr->Attributes,  &ClassInfo);
        Converted.AnnotationDeclarations = JavaClassPtr->ExtractAnnotations(Converted.AttributeMap);
        Converted.ParameterAnnotationDeclarations = JavaClassPtr->ExtractParameterAnnotations(Converted.AttributeMap);

        if(Converted.ParameterAnnotationDeclarations.size() == 0) {
            return;
        }

        if (Converted.ParameterAnnotationDeclarations.size() == Converted.FixedParameterTypeCodeNames.size()) {
            assert(!JavaClassPtr->IsInnerClass() || JavaClassPtr->IsStatic() || OriginalName != "<init>"s);
        } else {
            assert(HasAnImplicitParameter());
            assert(Converted.ParameterAnnotationDeclarations.size() + 1 == Converted.FixedParameterTypeCodeNames.size());
        }
    }

}
