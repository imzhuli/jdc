#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>

namespace jdc
{

    void xJavaMethod::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaMethod::DoExtend: %s\n", OriginalName.c_str());
        Extend.AttributeMap = LoadAttributeInfo(MethodInfoPtr->Attributes,  &JavaClassPtr->ClassInfo);
    }

    void xJavaMethod::DoConvert()
    {
        Converted.AnnotationDeclarations = JavaClassPtr->ExtractAnnotations(Extend.AttributeMap);
    }

}
