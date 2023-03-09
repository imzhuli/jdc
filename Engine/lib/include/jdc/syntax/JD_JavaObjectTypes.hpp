#include "./JD_JavaType.hpp"
#include <string>
#include <map>

namespace jdc
{

    class xJavaObjectType
    : public iJavaType
    {
    public:
        X_INLINE bool IsPrimitiveType()  const override { return false; }
        X_INLINE bool IsDirectType()     const override { return true; }
        X_INLINE bool IsObjectType()     const override { return true; }

    private:
        friend void AddJavaObjectType(const char * BinaryName);
    };

    X_PRIVATE bool InitJavaObjectTypes();
    X_PRIVATE void CleanJavaObjectTypes();
    X_PRIVATE const std::map<std::string, xJavaObjectType> & GetJavaObjectTypeMap();

}
