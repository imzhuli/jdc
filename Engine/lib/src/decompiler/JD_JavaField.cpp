#include <jdc/decompiler/JD_JavaField.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>

namespace jdc
{

    void xJavaField::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaField::DoExtend: %s : FixedTypeCodeName:%s\n", Name.c_str(), FixedTypeCodeName.c_str());
        Extend.AttributeMap = LoadAttributeInfo(FieldInfoPtr->Attributes,  &JavaClassPtr->ClassInfo);
    }

    void xJavaField::DoConvert()
    {
        Converted.AnnotationDeclarations = JavaClassPtr->ExtractAnnotations(Extend.AttributeMap);
    }

}
