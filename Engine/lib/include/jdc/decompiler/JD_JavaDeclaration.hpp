#include "./_.hpp"
#include <string>
#include <vector>

namespace jdc
{

    struct xAnnotationElementValuePair
    {
        std::string ElementName;
        std::string ElementValueString;
    };

    struct xAnnotationDeclaration
    {
        std::string TypeName;
        std::vector<xAnnotationElementValuePair> ElementValuePairs;
    };

    using xAnnotationDeclarations = std::vector<xAnnotationDeclaration>;

    struct xClassDeclaration
    {
        std::string FixedPackageName;

        xAccessFlag AccessFlags;
        std::string CodeName;
    };

}
