#pragma once
#include "../base/_.hpp"
#include "./JD_Attribute.hpp"
#include "./JD_Constant.hpp"
#include "./JD_Field.hpp"
#include "./JD_Method.hpp"
#include <xel/Byte.hpp>
#include <string>
#include <vector>

namespace jdc
{

    struct xClassInfo
    {
        uint32_t                           Magic;
        uint16_t                           MinorVersion;
        uint16_t                           MajorVersion;
        std::vector<xConstantInfo>         ConstantPool;
        uint16_t                           AccessFlags;
        uint16_t                           ThisClass;
        uint16_t                           SuperClass;
        std::vector<uint16_t>              InterfaceIndices;
        std::vector<xFieldInfo>            Fields;
        std::vector<xMethodInfo>           Methods;
        std::vector<xAttributeInfo>        Attributes;

        X_GAME_API_MEMBER const std::string & GetConstantUtf8(size_t Index) const;
        X_GAME_API_MEMBER const std::string & GetConstantString(size_t Index) const;
        X_GAME_API_MEMBER const std::string & GetConstantClassBinaryName(size_t Index) const;
        X_GAME_API_MEMBER std::string GetOutermostClassBinaryName() const;
        X_GAME_API_MEMBER std::string GetConstantValueString(size_t Index) const;
        X_GAME_API_MEMBER std::vector<std::string> ExtractTypeBinaryNames(const std::string & Descriptor) const;
    };

    X_GAME_API std::string EscapeString(const std::string & S);
    X_GAME_API std::string EscapeStringQuoted(const std::string & S);

    X_GAME_API bool ExtractAttributeInfo(xel::xStreamReader & Reader, ssize_t & RemainSize, xAttributeInfo & AttributeInfo);
    X_GAME_API bool ExtractFieldInfo(xel::xStreamReader & Reader, ssize_t & RemainSize, xFieldInfo & FieldInfo);
    X_GAME_API bool ExtractMethodInfo(xel::xStreamReader & Reader, ssize_t & RemainSize, xMethodInfo & MethodInfo);

    X_INLINE std::string MakeArgumentName(size_t Index) { return "__arg_" + std::to_string(Index); }
    X_INLINE std::string MakeVariableName(size_t Index) { return "__var_" + std::to_string(Index); }
    X_INLINE bool IsLocalVariableName(const std::string & Name) { return 0 == Name.find("__var_"); }

    X_GAME_API xResult<xClassInfo> LoadClassInfoFromFile(const std::string & Filename);

}
