#include <jdc/base/JD_Base.hpp>

namespace jdc
{

    bool IsLoadableConstantTag(const eConstantTag Tag)
    {
        switch (Tag) {
            case eConstantTag::Integer:
            case eConstantTag::Float:
            case eConstantTag::Long:
            case eConstantTag::Double:
            case eConstantTag::Class:
            case eConstantTag::String:
            case eConstantTag::MethodHandle:
            case eConstantTag::MethodType:
            case eConstantTag::Dynamic:
                return true;
            default:
                break;
        }
        return false;
    }




}
