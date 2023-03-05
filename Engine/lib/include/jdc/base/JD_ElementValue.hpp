#pragma once
#include "./JD_Base.hpp"
#include <string>
#include <vector>
#include <memory>

namespace jdc
{

    class xElementValue
    : xel::xNonCopyable
    {
    public:
        X_INLINE const eElementType & GetType() const { return _Type; }

        X_INLINE void SetJavaBoolean(bool Value)    { _Type = eElementType::Boolean;  _Numeric.JavaBoolean = Value; }
        X_INLINE void SetJavaByte(int8_t Value)     { _Type = eElementType::Byte;     _Numeric.JavaByte = Value; }
        X_INLINE void SetJavaShort(int16_t Value)   { _Type = eElementType::Short;    _Numeric.JavaShort = Value; }
        X_INLINE void SetJavaInt(int32_t Value)     { _Type = eElementType::Int;      _Numeric.JavaInt = Value; }
        X_INLINE void SetJavaLong(int64_t Value)    { _Type = eElementType::Long;     _Numeric.JavaLong = Value; }
        X_INLINE void SetJavaChar(uint16_t Value)   { _Type = eElementType::Char;     _Numeric.JavaChar = Value; }
        X_INLINE void SetJavaFloat(float Value)     { _Type = eElementType::Float;    _Numeric.JavaFloat = Value; }
        X_INLINE void SetJavaDouble(double Value)   { _Type = eElementType::Double;   _Numeric.JavaDouble = Value; }

        X_INLINE void SetJavaString(const std::string & Value)   { _Type = eElementType::String;   _StringValue = Value; }
        X_INLINE void SetJavaClass(const std::string & ClassBinaryName)   { _Type = eElementType::Class;   _StringValue = ClassBinaryName; }
        X_INLINE void SetJavaAnnotation(const std::string & AnnotationBinaryName)   { _Type = eElementType::Annotation;   _StringValue = AnnotationBinaryName; }
        X_INLINE void SetJavaEnumElement(const std::string & EnumBainaryName, const std::string & MemberName)   { _Type = eElementType::Enum; _StringValue = EnumBainaryName; _EnumMemberStringValue = MemberName; }
        X_INLINE void SetJavaArray(eElementType SubType, size_t Size, size_t Dimension = 1) {
            _Type = eElementType::Array;
            assert(
                SubType == eElementType::Boolean  ||
                SubType == eElementType::Byte     ||
                SubType == eElementType::Short    ||
                SubType == eElementType::Int      ||
                SubType == eElementType::Long     ||
                SubType == eElementType::Char     ||
                SubType == eElementType::Float    ||
                SubType == eElementType::Double   );
            _Numeric.JavaArray.ElementType = SubType;
            _Numeric.JavaArray.Size = Size;
            _Numeric.JavaArray.Dimension = Dimension;
            _ArrayElements.clear();
            _ArrayElements.resize(Size);
        }
        X_INLINE void SetJavaArray(eElementType SubType, const std::string & SubTypeBinaryName, size_t Size, size_t Dimension = 1) {
            _Type = eElementType::Array;
            assert(
                SubType == eElementType::Class        ||
                SubType == eElementType::Annotation   ||
                SubType == eElementType::Enum         );
            _Numeric.JavaArray.ElementType = SubType;
            _Numeric.JavaArray.Size = Size;
            _Numeric.JavaArray.Dimension = Dimension;
            _StringValue = SubTypeBinaryName;
            _ArrayElements.clear();
            _ArrayElements.resize(Size);
        }

        X_INLINE std::vector<std::unique_ptr<xElementValue>> & GetArrayElements() { return _ArrayElements; }
        X_INLINE const std::vector<std::unique_ptr<xElementValue>> & GetArrayElements() const { return _ArrayElements; }

        X_INLINE bool      GetJavaBoolean() const   { assert(_Type == eElementType::Boolean);    return _Numeric.JavaBoolean; }
        X_INLINE int8_t    GetJavaByte() const      { assert(_Type == eElementType::Byte);       return _Numeric.JavaByte; }
        X_INLINE int16_t   GetJavaShort() const     { assert(_Type == eElementType::Short);      return _Numeric.JavaShort; }
        X_INLINE int32_t   GetJavaInt() const       { assert(_Type == eElementType::Int);        return _Numeric.JavaInt; }
        X_INLINE int64_t   GetJavaLong() const      { assert(_Type == eElementType::Long);       return _Numeric.JavaLong; }
        X_INLINE uint16_t  GetJavaChar() const      { assert(_Type == eElementType::Char);       return _Numeric.JavaChar; }
        X_INLINE float     GetJavaFloat() const     { assert(_Type == eElementType::Float);      return _Numeric.JavaFloat; }
        X_INLINE double    GetJavaDouble() const    { assert(_Type == eElementType::Double);     return _Numeric.JavaDouble; }

        X_INLINE const std::string & GetJavaString() const { assert(_Type == eElementType::String);  return _StringValue; }
        X_INLINE const std::string & GetJavaClassBinaryName() const { assert(_Type == eElementType::Class);  return _StringValue; }
        X_INLINE const std::string & GetJavaAnnotationBinaryName() const { assert(_Type == eElementType::Annotation);  return _StringValue; }
        X_INLINE const std::string & GetJavaEnumBinaryName() const { assert(_Type == eElementType::Enum);  return _StringValue; }
        X_INLINE const std::string & GetJavaEnumMemberName() const { assert(_Type == eElementType::Enum);  return _EnumMemberStringValue; }

        X_INLINE int32_t GetJavaArrayDimension() const { assert(_Type == eElementType::Array); return _Numeric.JavaArray.Dimension; }
        X_INLINE int32_t GetJavaArraySize() const { assert(_Type == eElementType::Array); return _Numeric.JavaArray.Size; }
        X_INLINE eElementType GetJavaArrayElementType() const { assert(_Type == eElementType::Array); return _Numeric.JavaArray.ElementType; }
        X_INLINE const std::string & GetJavaArrayClassBinaryName() const
            { assert(_Type == eElementType::Array && _Numeric.JavaArray.ElementType == eElementType::Class); return _StringValue; }
        X_INLINE const std::string & GetJavaArrayAnnotationBinaryName() const
            { assert(_Type == eElementType::Array && _Numeric.JavaArray.ElementType == eElementType::Annotation); return _StringValue; }
        X_INLINE const std::string & GetJavaArrayEnumBinaryName() const
            { assert(_Type == eElementType::Array && _Numeric.JavaArray.ElementType == eElementType::Enum); return _StringValue; }

        X_INLINE void Clear() { _Type = eElementType::Unspecified; _StringValue.clear(); _EnumMemberStringValue.clear(); _ArrayElements.clear(); }

    private:
        eElementType _Type = {};

        // string:
        union {
            bool      JavaBoolean;
            int8_t    JavaByte;
            int16_t   JavaShort;
            int32_t   JavaInt;
            int64_t   JavaLong;
            uint16_t  JavaChar;
            float     JavaFloat;
            double    JavaDouble;
            struct {
                int32_t        Size;
                int32_t        Dimension;
                eElementType   ElementType;
            } JavaArray;
        } _Numeric;
        std::string _StringValue;
        std::string _EnumMemberStringValue;
        std::vector<std::unique_ptr<xElementValue>> _ArrayElements;
    };

}
