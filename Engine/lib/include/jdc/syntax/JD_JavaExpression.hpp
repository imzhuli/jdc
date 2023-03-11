#include "./_.hpp"
#include "./JD_JavaType.hpp"
#include <string>
#include <map>

namespace jdc
{

    /* Priority  Operator   Operation                              Order of Evaluation
    * 0         new        Object creation
    * 1         [ ]        Array index                            Left to Right
    *           ()         Method call
    *           .          Member access
    *           ++         Postfix increment
    *           --         Postfix decrement
    * 2         ++         Prefix increment                       Right to Left
    *           --         Prefix decrement
    *           + -        Unary plus, minus
    *           ~          Bitwise NOT
    *           !          Boolean (logical) NOT
    * 3         (type)     Type cast                              Right to Left
    * 4         +          String concatenation                   Left to Right
    * 5         * / %      Multiplication, division, remainder    Left to Right
    * 6         + -        Addition, subtraction                  Left to Right
    * 7         <<         Signed bit shift left to right         Left to Right
    *           >>         Signed bit shift right to left
    *           >>>        Unsigned bit shift right to left
    * 8         < <=       Less than, less than or equal to       Left to Right
    *           > >=       Greater than, greater than or equal to
    *           instanceof Reference test
    * 9         ==         Equal to                               Left to Right
    *           !=         Not equal to
    * 10        &          Bitwise AND                            Left to Right
    *           &          Boolean (logical) AND
    * 11        ^          Bitwise XOR                            Left to Right
    *           ^          Boolean (logical) XOR
    * 12        |          Bitwise OR                             Left to Right
    *           |          Boolean (logical) OR
    * 13        &&         Boolean (logical) AND                  Left to Right
    * 14        ||         Boolean (logical) OR                   Left to Right
    * 15        ? :        Conditional                            Right to Left
    * 16        =          Assignment                             Right to Left
    *           *= /= +=   Combinated assignment
    *           -= %=      (operation and assignment)
    *           <<= >>=
    *           >>>=
    *           &= ^= |=
    * 17        ->         Lambda                                 Right to Left
    *
    * References:
    * - http://introcs.cs.princeton.edu/java/11precedence
    * - The Java® Language Specification Java SE 8 Edition, 15.2 Forms of Expressions
    */

    class iJavaExpression
    {
    public:
        virtual bool IsArrayExpression() { return false; }
        virtual bool IsBinaryOperatorExpression() { return false; }
        virtual bool IsBooleanExpression() { return false; }
        virtual bool IsCastExpression() { return false; }
        virtual bool IsConstructorInvocationExpression() { return false; }
        virtual bool IsDoubleConstantExpression() { return false; }
        virtual bool IsFieldReferenceExpression() { return false; }
        virtual bool IsFloatConstantExpression() { return false; }
        virtual bool IsIntegerConstantExpression() { return false; }
        virtual bool IsLengthExpression() { return false; }
        virtual bool IsLocalVariableReferenceExpression() { return false; }
        virtual bool IsLongConstantExpression() { return false; }
        virtual bool IsMethodInvocationExpression() { return false; }
        virtual bool IsNewArray() { return false; }
        virtual bool IsNewExpression() { return false; }
        virtual bool IsNewInitializedArray() { return false; }
        virtual bool IsNullExpression() { return false; }
        virtual bool IsObjectTypeReferenceExpression() { return false; }
        virtual bool IsPostOperatorExpression() { return false; }
        virtual bool IsPreOperatorExpression() { return false; }
        virtual bool IsStringConstantExpression() { return false; }
        virtual bool IsSuperConstructorInvocationExpression() { return false; }
        virtual bool IsSuperExpression() { return false; }
        virtual bool IsTernaryOperatorExpression() { return false; }
        virtual bool IsThisExpression() { return false; }

        virtual const iJavaType * GetResultType() const { return nullptr; };
        virtual size_t GetLineNumber();

    protected:
        size_t _LineNumber = 0;
    };

}
