#include <jdc/decompiler/JD_JavaField.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>

namespace jdc
{

    void xJavaField::DoConvert()
    {
        X_DEBUG_PRINTF("xJavaField::DoConvert: %s : UnfixedTypeBinaryName:%s\n", Name.c_str(), UnfixedTypeBinaryName.c_str());
        Converted.AttributeMap = LoadAttributeInfo(FieldInfoPtr->Attributes,  &JavaClassPtr->ClassInfo);
        Converted.AnnotationDeclarations = JavaClassPtr->ExtractAnnotations(Converted.AttributeMap);
    }

}
