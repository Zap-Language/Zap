#ifndef ZAP_SEMANTICANALYZER_H
#define ZAP_SEMANTICANALYZER_H

#include <string>
#include <memory>
#include <vector>
#include "ast/Program.h"
#include "ast/FuncStatement.h"
#include "ast/LetStatement.h"
#include "ast/ReturnStatement.h"
#include "ast/AssignStatement.h"
#include "ast/IfStatement.h"
#include "ast/ElseStatement.h"
#include "ast/WhileStatement.h"
#include "ast/ForStatement.h"
#include "ast/ExpressionStatement.h"
#include "ast/BlockStatement.h"
#include "ast/CallExpression.h"
#include "ast/InfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/Identifier.h"
#include "ast/IntLiteral.h"
#include "ast/FloatLiteral.h"
#include "ast/BoolLiteral.h"
#include "ast/CharLiteral.h"
#include "ast/StringLiteral.h"
#include "ast/symbols/SymbolTable.h"

namespace ast {
    struct SemanticError {
        std::string message;
        std::string location;
        
        SemanticError(std::string message, std::string location = "")
            : message(std::move(message)), location(std::move(location)) {}
    };

    class SemanticAnalyzer {
    public:
        SemanticAnalyzer();
        ~SemanticAnalyzer() = default;

        bool AnalyzeProgram(const std::shared_ptr<Program>& program);
        const std::vector<SemanticError>& GetErrors() const;
        const SymbolTable& GetSymbolTable() const;
        
        void PrintErrors() const;
        void PrintSymbolTable() const;

    private:
        SymbolTable symbolTable;
        mutable std::vector<SemanticError> errors;
        std::shared_ptr<DataType> currentFunctionReturnType = nullptr;

        bool AnalyzeStatement(const std::shared_ptr<StatementNode>& stmt);
        bool AnalyzeFuncStatement(const std::shared_ptr<FuncStatement>& func);
        bool AnalyzeLetStatement(const std::shared_ptr<LetStatement>& let);
        bool AnalyzeReturnStatement(const std::shared_ptr<ReturnStatement>& ret);
        bool AnalyzeAssignStatement(const std::shared_ptr<AssignStatement>& assign);
        bool AnalyzeIfStatement(const std::shared_ptr<IfStatement>& ifStmt);
        bool AnalyzeWhileStatement(const std::shared_ptr<WhileStatement>& whileStmt);
        bool AnalyzeForStatement(const std::shared_ptr<ForStatement>& forStmt);
        bool AnalyzeExpressionStatement(const std::shared_ptr<ExpressionStatement>& exprStmt);

        std::shared_ptr<DataType> AnalyzeExpression(const std::shared_ptr<ExpressionNode>& expr);
        std::shared_ptr<DataType> AnalyzeCallExpression(const std::shared_ptr<CallExpression>& call);
        std::shared_ptr<DataType> AnalyzeInfixExpression(const std::shared_ptr<InfixExpression>& infix);
        std::shared_ptr<DataType> AnalyzePrefixExpression(const std::shared_ptr<PrefixExpression>& prefix);
        std::shared_ptr<DataType> AnalyzeIdentifier(const std::shared_ptr<Identifier>& identifier);

        std::shared_ptr<DataType> AnalyzeIntLiteral(const std::shared_ptr<IntLiteral>& intLit);
        std::shared_ptr<DataType> AnalyzeFloatLiteral(const std::shared_ptr<FloatLiteral>& floatLit);
        std::shared_ptr<DataType> AnalyzeBoolLiteral(const std::shared_ptr<BoolLiteral>& boolLit);
        std::shared_ptr<DataType> AnalyzeCharLiteral(const std::shared_ptr<CharLiteral>& charLit);
        std::shared_ptr<DataType> AnalyzeStringLiteral(const std::shared_ptr<StringLiteral>& stringLit);

        bool TypesEqual(const std::shared_ptr<DataType>& a, const std::shared_ptr<DataType>& b) const;
        bool TypesCompatible(const std::shared_ptr<DataType>& expected, const std::shared_ptr<DataType>& actual) const;
        std::shared_ptr<DataType> GetResultTypeForInfixOperation(
            const std::string& operator_,
            const std::shared_ptr<DataType>& left,
            const std::shared_ptr<DataType>& right) const;
        std::shared_ptr<DataType> GetResultTypeForPrefixOperation(
            const std::string& operator_,
            const std::shared_ptr<DataType>& operand) const;

        std::shared_ptr<DataType> AnalyzeBuiltinCall(const std::string& functionName, 
                                                    const std::vector<std::shared_ptr<ExpressionNode>>& args);
        void AddError(const std::string& message, const std::string& location = "") const;
    };
}

#endif //ZAP_SEMANTICANALYZER_H