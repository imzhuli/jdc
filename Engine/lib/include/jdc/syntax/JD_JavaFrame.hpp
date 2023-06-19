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

        auto GetFrame() const { return FramePtr; }
        void SetFrame(xJavaFrame * New) { FramePtr = New; }

        bool IsDeclared() const { return Declared; }
        void SetDeclared(bool New) { Declared = New; }

        auto GetIndex() const { return Index; }

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
        virtual xJavaType * GetType() const       { return nullptr; }
        virtual size_t GetDemension() const       { return 0; }
        virtual bool IsPrimitiveLocalVariable()   { return false; }
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
