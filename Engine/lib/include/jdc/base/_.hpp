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

    enum struct eElementValueTag : uint8_t
    {
        Unspecified   = '\0',

        Byte          = 'B',
        Short         = 'S',
        Int           = 'I',
        Long          = 'J',
        Char          = 'C',
        Float         = 'F',
        Double        = 'D',
        Boolean       = 'Z',
        String        = 's',

        Enum          = 'e',
        Class         = 'c',
        Annotation    = '@',
        Array         = '[',
    };

    enum struct eConstantTag : uint8_t
    {
        Unspecified        = 0,
        Utf8               = 1,
        Integer            = 3,
        Float              = 4,
        Long               = 5,
        Double             = 6,
        Class              = 7,      // Class
        String             = 8,
        FieldRef           = 9,      // MemberRef
        MethodRef          = 10,     // MemberRef
        InterfaceMethodRef = 11,
        NameAndType        = 12,
        MethodHandle       = 15,
        MethodType         = 16,
        Dynamic            = 17,     // MemberRef
        InvokeDynamic      = 18,     // MemberRef
        Module             = 19,     // Class
        Package            = 20,     // Class
    };

    using xAccessFlag = uint16_t;

    // Access flags for Class, Field, Method, Nested class, Module, Module Requires, Module Exports, Module Opens
    constexpr const xAccessFlag ACC_PUBLIC       = 0x0001; // C  F  M  N  .  .  .  .
    constexpr const xAccessFlag ACC_PRIVATE      = 0x0002; // .  F  M  N  .  .  .  .
    constexpr const xAccessFlag ACC_PROTECTED    = 0x0004; // .  F  M  N  .  .  .  .
    constexpr const xAccessFlag ACC_STATIC       = 0x0008; // C  F  M  N  .  .  .  .
    constexpr const xAccessFlag ACC_FINAL        = 0x0010; // C  F  M  N  .  .  .  .
    constexpr const xAccessFlag ACC_SYNCHRONIZED = 0x0020; // .  .  M  .  .  .  .  .
    constexpr const xAccessFlag ACC_SUPER        = 0x0020; // C  .  .  .  .  .  .  .
    constexpr const xAccessFlag ACC_OPEN         = 0x0020; // .  .  .  .  Mo .  .  .
    constexpr const xAccessFlag ACC_TRANSITIVE   = 0x0020; // .  .  .  .  .  MR .  .
    constexpr const xAccessFlag ACC_VOLATILE     = 0x0040; // .  F  .  .  .  .  .  .
    constexpr const xAccessFlag ACC_BRIDGE       = 0x0040; // .  .  M  .  .  .  .  .
    constexpr const xAccessFlag ACC_STATIC_PHASE = 0x0040; // .  .  .  .  .  MR .  .
    constexpr const xAccessFlag ACC_TRANSIENT    = 0x0080; // .  F  .  .  .  .  .  .
    constexpr const xAccessFlag ACC_VARARGS      = 0x0080; // .  .  M  .  .  .  .  .
    constexpr const xAccessFlag ACC_NATIVE       = 0x0100; // .  .  M  .  .  .  .  .
    constexpr const xAccessFlag ACC_INTERFACE    = 0x0200; // C  .  .  N  .  .  .  .
    constexpr const xAccessFlag ACC_ABSTRACT     = 0x0400; // C  .  M  N  .  .  .  .
    constexpr const xAccessFlag ACC_STRICT       = 0x0800; // .  .  M  .  .  .  .  .
    constexpr const xAccessFlag ACC_SYNTHETIC    = 0x1000; // C  F  M  N  Mo MR ME MO
    constexpr const xAccessFlag ACC_ANNOTATION   = 0x2000; // C  .  .  N  .  .  .  .
    constexpr const xAccessFlag ACC_ENUM         = 0x4000; // C  F  .  N  .  .  .  .
    constexpr const xAccessFlag ACC_MODULE       = 0x8000; // C  .  .  .  .  .  .  .
    constexpr const xAccessFlag ACC_MANDATED     = 0x8000;

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

        X_INLINE bool IsOk() const { return ResultCode == JDR_OK; }
    };

    X_GAME_API const char * GetClassVersionString(uint16_t MajorVersion);
}
