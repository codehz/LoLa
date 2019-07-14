#ifndef AST_HPP
#define AST_HPP

#include <vector>
#include <string>
#include <memory>

namespace LoLa::AST
{
    template<typename T>
    using List = std::vector<T>;

    using String = std::string;

    enum class Operator
    {
        LessOrEqual,
        GreaterOrEqual,
        Equals,
        Differs,
        Less,
        More,

        Plus,
        Minus,
        Multiply,
        Divide,
        Modulus,

        And,
        Or,
        Not
    };

    struct StatementBase { virtual ~StatementBase(); };
    struct ExpressionBase { virtual ~ExpressionBase(); };

    using Statement = std::unique_ptr<StatementBase>;
    using Expression = std::unique_ptr<ExpressionBase>;

    Expression ArrayIndexer(Expression var, Expression index);
    Expression VariableRef(String var);
    Expression ArrayLiteral(List<Expression> initializer);
    Expression FunctionCall(String name, List<Expression> args);
    Expression MethodCall(Expression object, String name, List<Expression> args);
    Expression NumberLiteral(String literal);
    Expression StringLiteral(String literal);

    Expression UnaryOperator(Operator op, Expression value);
    Expression BinaryOperator(Operator op, Expression lhs, Expression rhs);

    Statement Assignment(Expression target, Expression value);

    Statement Return();
    Statement Return(Expression value);
    Statement WhileLoop(Expression condition, Statement body);
    Statement ForLoop(String var, Expression source, Statement body);
    Statement IfElse(Expression condition, Statement true_body);
    Statement IfElse(Expression condition, Statement true_body, Statement false_body);

    Statement DiscardResult(Expression value);

    Statement Declaration(String name);
    Statement Declaration(String name, Expression value);

    Statement SubScope(List<Statement> body);

    struct Function
    {
        std::string name;
        std::vector<std::string> params;
        Statement body;
    };

    struct Program
    {
        std::vector<Function> functions;
        std::vector<Statement> statements;
    };
}

#endif // AST_HPP
