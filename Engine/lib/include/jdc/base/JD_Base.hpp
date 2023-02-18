#pragma once
#include <xel/Common.hpp>
#include <cinttypes>

#if defined(X_GAME_OPTION_STATIC)
	#if defined(X_GAME_OPTION_EXPORT_API)
		#error X_GAME_OPTION_STATIC is used with X_GAME_OPTION_EXPORT_API
	#endif
	#define X_GAME_API                      X_EXTERN
	#define X_GAME_API_MEMBER               X_MEMBER
	#define X_GAME_API_STATIC_MEMBER        X_STATIC_MEMBER
#else
	#if defined(X_GAME_OPTION_EXPORT_API)
		#define X_GAME_API                  X_EXPORT
		#define X_GAME_API_MEMBER           X_EXPORT_MEMBER
		#define X_GAME_API_STATIC_MEMBER    X_EXPORT_STATIC_MEMBER
	#else
		#define X_GAME_API                  X_IMPORT
		#define X_GAME_API_MEMBER           X_IMPORT_MEMBER
		#define X_GAME_API_STATIC_MEMBER    X_IMPORT_STATIC_MEMBER
	#endif
#endif

#include <string>

namespace jdc
{

    enum struct eFieldType : uint8_t
    {
        Void,
        Byte,
        Short,
        Integer,
        Long,
        Char,
        Float,
        Double,
        Boolean,
        Class,
        Array,

        // the following are used only when extracting types form descriptor:
        ParamStart,
        ParamEnd,
        Invalid,
    };

    enum struct eConstantTag : uint8_t
    {
        Unspecified        = 0,
        Utf8               = 1,
        Integer            = 3,
        Float              = 4,
        Long               = 5,
        Double             = 6,
        Class              = 7,
        String             = 8,
        FieldRef           = 9,
        MethodRef          = 10,
        InterfaceMethodRef = 11,
        NameAndType        = 12,
        MethodHandle       = 15,
        MethodType         = 16,
        Dynamic            = 17,
        InvokeDynamic      = 18,
        Module             = 19,
        Package            = 20,
    };

    using xAccessFlag = uint16_t;
    constexpr const xAccessFlag ACC_PUBLIC       = 0x0001; // field | method
    constexpr const xAccessFlag ACC_PRIVATE      = 0x0002; // field | method
    constexpr const xAccessFlag ACC_PROTECTED    = 0x0004; // field | method
    constexpr const xAccessFlag ACC_STATIC       = 0x0008; // field | method
    constexpr const xAccessFlag ACC_FINAL        = 0x0010; // field | method
    constexpr const xAccessFlag ACC_SYNCHRONIZED = 0x0020; // method
    constexpr const xAccessFlag ACC_SUPER        = 0x0020; // class | interface
    constexpr const xAccessFlag ACC_OPEN         = 0x0020; // module
    constexpr const xAccessFlag ACC_BRIDGE       = 0x0040; // method
    constexpr const xAccessFlag ACC_VOLATILE     = 0x0040; // field
    constexpr const xAccessFlag ACC_VARARGS      = 0x0080; // method
    constexpr const xAccessFlag ACC_TRANSIENT    = 0x0080; // field
    constexpr const xAccessFlag ACC_NATIVE       = 0x0100; // method
    constexpr const xAccessFlag ACC_INTERFACE    = 0x0200; // class
    constexpr const xAccessFlag ACC_ABSTRACT     = 0x0400; // method
    constexpr const xAccessFlag ACC_STRICT       = 0x0800; // method
    constexpr const xAccessFlag ACC_SYNTHETIC    = 0x1000; // class
    constexpr const xAccessFlag ACC_ANNOTATION   = 0x2000; // class
    constexpr const xAccessFlag ACC_ENUM         = 0x4000; // class
    constexpr const xAccessFlag ACC_MODULE       = 0x8000; // class
    constexpr const xAccessFlag ACC_MANDATED     = 0x8000; // module

    enum eResultCode : uint16_t
    {
        JDR_OK = 0,
        JDR_FILE_ERROR,
        JDR_DATA_SIZE_ERROR,

        JDR_UNEXPECTED_ERROR = 65535,
    };

    template<typename tData = xel::xNone>
    struct xResult
    {
        eResultCode     ResultCode = JDR_OK;
        tData           Data = {};
        std::string     ErrorMessage = {};

        xResult() = default;
        xResult(const tData & Data) : Data(Data) {}
        xResult(tData && Data) : Data(std::move(Data)) {}
        xResult(eResultCode ResultCode, const std::string & ErrorMessage)
        : ResultCode(ResultCode), ErrorMessage(ErrorMessage)
        {}
    };

    X_GAME_API bool IsLoadableConstantTag(const eConstantTag Tag);

    X_GAME_API const char * GetClassVersionString(uint16_t MajorVersion);
    X_GAME_API const char * GetConstantTagString(const eConstantTag Tag);
    X_GAME_API const char * GetFieldTypeString(const eFieldType Type);
}
