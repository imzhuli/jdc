#include <jdc/class_file/JD_Constant.hpp>
#include <cstring>

namespace jdc
{

    xConstantInfo::xConstantInfo(const xConstantInfo &Other)
    {
        switch(Other.Tag) {
            case eConstantTag::Utf8: {
                auto & Source = Other.Details.Utf8;
                SetUtf8(Source.DataPtr->data(), Source.DataPtr->size());
                Tag = Other.Tag;
                return;
            }
            default: {
                break;
            }
        }
        Tag = Other.Tag;
        memcpy(&Details, &Other.Details, sizeof(Details));
    }

    xConstantInfo::xConstantInfo(xConstantInfo && Other)
    {
        Tag = Other.Tag;
        memcpy(&Details, &Other.Details, sizeof(Details));
        switch(Other.Tag) {
            case eConstantTag::Utf8: {
                Other.Tag = eConstantTag::Unspecified;
                Other.Details.Utf8.DataPtr = nullptr;
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
        Details.Utf8.DataPtr = new std::string(DataPtr, Length);
    }

    void xConstantInfo::Clear()
    {
        switch(Tag) {
            case eConstantTag::Utf8: {
                delete Details.Utf8.DataPtr;
                break;
            }
            default: {
                break;
            }
        }
        Tag = eConstantTag::Unspecified;
    }

}
