#include <jdc/decompiler/JD_JavaField.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>

namespace jdc
{

    void xJavaField::DoConvert()
    {
        Converted.FixedTypeCodeName = JavaClassPtr->JavaSpacePtr->GetShortClassCodeName(UnfixedTypeBinaryName);
        Converted.AttributeMap = LoadAttributeInfo(FieldInfoPtr->Attributes,  &JavaClassPtr->ClassInfo);
        Converted.AnnotationDeclarations = JavaClassPtr->ExtractAnnotations(Converted.AttributeMap);
    }

}
