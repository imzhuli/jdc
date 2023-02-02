#pragma once
#include "./JD_Base.hpp"
#include "./JD_BaseError.hpp"
#include <string>
#include <vector>

namespace xjd
{

    struct xConstantPoolInfo
    {
        uint8_t Tag;
        std::vector<uint8_t> Info;
    };

    struct xClassInterface
    {

    };

    struct xClassFieldInfo
    {

    };

    struct xClassMethodInfo
    {

    };

    struct xClassAttributeInfo
    {

    };

    struct xClass
    {
        std::string                        Name;
        uint32_t                           Magic;
        uint16_t                           MinorVersion;
        uint16_t                           MajorVersion;
        std::vector<xConstantPoolInfo>     ConstantPoolInfo;
        uint16_t                           AccessFlags;
        uint16_t                           ThisClass;
        uint16_t                           SuperClass;
        std::vector<xClassInterface>       Interfaces;
        std::vector<xClassFieldInfo>       Fields;
        std::vector<xClassMethodInfo>      Methods;
        std::vector<xClassAttributeInfo>   Attributes;
    };

    X_GAME_API xJDResult<xClass> LoadClassInfoFromFile(const std::string & Filename);
    X_GAME_API std::string DumpStringFromClass(const xClass & JavaClass);
}
