#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/class_file/JD_Attribute.hpp>

namespace jdc
{

    void xJavaMethod::DoExtend()
    {
        X_DEBUG_PRINTF("xJavaMethod::DoExtend: %s\n", OriginalName.c_str());
        Extend.AttributeMap = LoadAttributeInfo(MethodInfoPtr->Attributes,  ClassInfoPtr);
    }

}
