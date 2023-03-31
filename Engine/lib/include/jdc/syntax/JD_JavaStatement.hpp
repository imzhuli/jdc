#pragma once
#include "./_.hpp"
#include "./JD_JavaExpression.hpp"
#include <string>
#include <vector>

namespace jdc
{

    class xJavaExpression;
    class xJavaCatchClause;

    struct iJavaStatement
    {
        virtual bool IsBreak() const                               { return false; }
        virtual bool IsContinue() const                            { return false; }
        virtual bool IsExpression() const                          { return false; }
        virtual bool IsFor() const                                 { return false; }
        virtual bool IsIf() const                                  { return false; }
        virtual bool IsElse() const                                { return false; }
        virtual bool IsLabel() const                               { return false; }
        virtual bool IsLambdaExpression() const                    { return false; }
        virtual bool IsLocalVariableDeclaration() const            { return false; }
        virtual bool IsMonitorEnter() const                        { return false; }
        virtual bool IsMonitorExit() const                         { return false; }
        virtual bool IsReturn() const                              { return false; }
        virtual bool IsSwitch() const                              { return false; }
        virtual bool IsSwitchLabelBlock() const                    { return false; }
        virtual bool IsSwitchMultiLabelBlock() const               { return false; }
        virtual bool IsThrow() const                               { return false; }
        virtual bool IsTry() const                                 { return false; }
        virtual bool IsWhile() const                               { return false; }

        virtual xJavaExpression * GetExpression() const            { return nullptr; }
        virtual xJavaExpression * GetConditionExpresssion() const  { return nullptr; }
        virtual xJavaExpression * GetMonitorExpression() const     { return nullptr; }

        virtual iJavaStatement * GetStatements() const             { return nullptr; }
        virtual iJavaStatement * GetInitStatements() const         { return nullptr; }
        virtual iJavaStatement * GetUpdateStatements() const       { return nullptr; }
        virtual iJavaStatement * GetElseStatements() const         { return nullptr; }
        virtual iJavaStatement * GetTryStatements() const          { return nullptr; }
        virtual iJavaStatement * GetFinallyStatements() const      { return nullptr; }

        virtual std::vector<xJavaCatchClause *> GetCatchClauses() const { return {}; }

        virtual size_t GetLineNumber() const { return -1; }
    };


}
