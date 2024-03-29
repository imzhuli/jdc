#pragma once
#include "./_.hpp"
#include <string>
#include <vector>
#include <memory>
#include <set>

namespace jdc
{
    class iJavaStatement;
    class xJavaFrame;
    class xJavaLocalVariable;
    class xJavaType;

    struct xJavaLocalVariableReference
    {
        xJavaLocalVariable * VariablePtr;
    };

    class xJavaLocalVariable
    : xel::xNonCopyable
    {

    protected:
        std::string                               Name;
        xJavaType *                               TypePtr          = nullptr;

        xJavaFrame *                              FramePtr         = nullptr;
        size_t                                    Index            = 0;
        size_t                                    FromOffset       = 0;
        size_t                                    ToOffset         = 0;
        bool                                      Declared         = false;

        xJavaLocalVariable *                      NextPtr          = nullptr;
        std::vector<xJavaLocalVariableReference>  References       = {};

        std::set<xJavaLocalVariable*>             VariablesOnRight = {};
        std::set<xJavaLocalVariable*>             VariablesOnLeft  = {};

    public:
        xJavaLocalVariable(size_t Index, size_t Offset, const std::string & Name)
        : xJavaLocalVariable(Index, Offset, Name, Offset == 0)
        {}

        xJavaLocalVariable(size_t Index, size_t Offset, const std::string & Name, bool Declared)
        : Name(Name), Index(Index), FromOffset(Offset), ToOffset(Offset), Declared(Declared)
        {}

        auto & GetName() const { return Name; }
        void SetName(const std::string & New) { Name = New; }

        auto GetType() const { return TypePtr; }
        void SetType(xJavaType * New) { TypePtr = New; }

        auto GetFrame() const { return FramePtr; }
        void SetFrame(xJavaFrame * New) { FramePtr = New; }

        bool IsDeclared() const { return Declared; }
        void SetDeclared(bool New) { Declared = New; }

        auto GetIndex() const { return Index; }

        auto GetFromOffset() const { return FromOffset; }
        void SetFromOffset(size_t NewFromOffset) {
            assert(NewFromOffset <= ToOffset);
            FromOffset = NewFromOffset;
        }
        void setToOffset(size_t Offset) {
            FromOffset = FromOffset > Offset ? Offset : FromOffset;
            ToOffset = ToOffset < Offset ? Offset : ToOffset;
        }
        void SetToOffsetForce(size_t Offset) {
            ToOffset = Offset;
        }

        auto GetNext() const { return NextPtr; }
        void SetNext(xJavaLocalVariable * New) { NextPtr = New; }

        auto & GetReferences() const { return References; }
        void AddReference(const xJavaLocalVariableReference & Reference) { References.push_back(Reference); }

        /**
         * Determines if the local variable represented by this object is either the same as, or is a super type variable
         * of, the local variable represented by the specified parameter.
         */
        // virtual boolean IsAssignableFrom (std::map<std::string, xJavaType*> TypeBounds, Type type);
        // virtual void    TypeOnRight      (std::map<std::string, xJavaType*> TypeBounds, Type type);
        // virtual void    TypeOnLeft       (std::map<std::string, xJavaType*> TypeBounds, Type type);

        // virtual boolean IsAssignableFrom (std::map<std::string, xJavaType*> TypeBounds, xJavaLocalVariable * VariablePtr);
        // virtual void    VariableOnRight  (std::map<std::string, xJavaType*> TypeBounds, xJavaLocalVariable * VariablePtr);
        // virtual void    VariableOnLeft   (std::map<std::string, xJavaType*> TypeBounds, xJavaLocalVariable * VariablePtr);

    public:
        virtual size_t GetDemension() const       { return 0; }
        virtual bool IsPrimitiveLocalVariable()   { return false; }
    };

    /**
     * @brief Set of local variables     *
     * @note Array allows null slots
     *
     */
    class xJavaLocalVariableSet
    : xel::xNonCopyable
    {
    private:
        std::vector<xJavaLocalVariable*>                 Array;
        size_t                                           Size = 0;

    public:
        auto& GetList() const { return Array; }
        auto  GetSize() const   { return Size; }
        bool  IsEmpty() const   { return Size == 0; }

        xJavaLocalVariable *    GetRoot(size_t Index);
        void                    Add(size_t Index, xJavaLocalVariable * NewVariablePtr);
        xJavaLocalVariable *    Remove(size_t Index, size_t Offset);
        xJavaLocalVariable *    Get(size_t Index, size_t Offset);
        void                    Update(size_t Index, size_t Offset, xJavaType * TypePtr);

    };

    class xJavaFrame
    {
    public:
        xJavaFrame *                                     ParentFramePtr;
        std::vector<xJavaLocalVariable>                  LocalVariableList;
        std::vector<std::unique_ptr<iJavaStatement>>     Statements;
        std::vector<std::unique_ptr<xJavaFrame>>         ChildFrames;
    };

}
