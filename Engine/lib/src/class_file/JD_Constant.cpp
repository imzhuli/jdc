#include <jdc/class_file/JD_Constant.hpp>
#include <cstring>

namespace jdc
{

    xConstantInfo::xConstantInfo(const xConstantInfo &Other)
    {
        switch(Other.Tag) {
            case eConstantTag::Utf8: {
                auto & Source = Other.Info.Utf8;
                SetUtf8(Source.DataPtr->data(), Source.DataPtr->size());
                Tag = Other.Tag;
                return;
            }
            default: {
                break;
            }
        }
        Tag = Other.Tag;
        memcpy(&Info, &Other.Info, sizeof(Info));
    }

    xConstantInfo::xConstantInfo(xConstantInfo && Other)
    {
        Tag = Other.Tag;
        memcpy(&Info, &Other.Info, sizeof(Info));
        switch(Other.Tag) {
            case eConstantTag::Utf8: {
                Other.Tag = eConstantTag::Unspecified;
                Other.Info.Utf8.DataPtr = nullptr;
                return;
            }
            default: {
                break;
            }
        }
    }

    xConstantInfo::~xConstantInfo()
    {
        Clear();
    }

    void xConstantInfo::SetUtf8(const char * DataPtr, size_t Length)
    {
        Clear();
        Tag = eConstantTag::Utf8;
        Info.Utf8.DataPtr = new std::string(DataPtr, Length);
    }

    void xConstantInfo::Clear()
    {
        switch(Tag) {
            case eConstantTag::Utf8: {
                delete Info.Utf8.DataPtr;
                break;
            }
            default: {
                break;
            }
        }
        Tag = eConstantTag::Unspecified;
    }

}
