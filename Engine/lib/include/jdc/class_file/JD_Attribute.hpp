#pragma once
#include "../base/JD_Base.hpp"
#include <xel/Byte.hpp>
#include <vector>


namespace jdc
{

    using xAttributeBinary = std::vector<xel::ubyte>;

    struct xAttributeInfo
    {
        uint16_t                      NameIndex;
        std::vector<xel::ubyte>       Binary;
    };

    struct xCodeAttribute
    {
        struct xExceptionTable
        {
            uint16_t StartPC;
            uint16_t EndPC;
            uint16_t HandlePC;
            uint16_t CatchType;
        };

        uint16_t MaxStack;
        uint16_t MaxLocals;
        std::vector<xel::ubyte>      CodeBinary;
        std::vector<xExceptionTable> ExceptionTables;
        std::vector<xAttributeInfo>  SubAttributes;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

    struct xInnerClassAttribute
    {
        uint16_t InnerClassInfoIndex;
        uint16_t OuterClassInfoIndex;
        uint16_t InnerNameIndex;
        uint16_t InnerAccessFlags;

        X_GAME_API_MEMBER bool Extract(const xAttributeBinary & AttributeBinary);
    };

}
