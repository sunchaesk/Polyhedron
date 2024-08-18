
#ifndef POLYHEDRALBUILDER_HPP
#define POLYHEDRALBUILDER_HPP

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

using namespace clang::tooling;
using namespace llvm;

struct forLoopCond {
    clang::Expr * forLoopCondRHS;
    enum clang::BinaryOperatorKind comparatorKind;

    // default constructor
    forLoopCond() : forLoopCondRHS(NULL), comparatorKind(clang::BO_Comma) {}

    forLoopCond(clang::Expr * forLoopCondRHS, enum clang::BinaryOperatorKind comparatorKind)
    :  forLoopCondRHS(forLoopCondRHS), comparatorKind(comparatorKind) {}
};

struct PolyhedralBranchInfo {
    clang::Expr * CondLHS;
    clang::Expr * CondRHS;
    enum clang::BinaryOperatorKind comparatorKind;

    PolyhedralBranchInfo(clang::Expr * CondLHS, clang::Expr * CondRHS, enum clang::BinaryOperatorKind comparatorKind)
        : CondLHS(CondLHS), CondRHS(CondRHS), comparatorKind(comparatorKind) {}
};

struct PolyhedralLoopInfo {
    std::string loopVar;
    clang::Expr * loopLowerBound;
    forLoopCond loopUpperBound;
    short int loopStep;

    PolyhedralLoopInfo(std::string var, clang::Expr * lb, forLoopCond ub, short int s)
        : loopVar(var), loopLowerBound(lb), loopUpperBound(ub), loopStep(s) {}

};

class PolyhedralBuilderVisitor : public clang::RecursiveASTVisitor<PolyhedralBuilderVisitor> {
    private:
        clang::ASTContext * Context;
        std::vector<PolyhedralLoopInfo> loopInfoVec;
        std::vector<PolyhedralBranchInfo> branchInfoVec;

        std::string getLoopVar(clang::ForStmt *forLoop);
        clang::Expr * getLoopLowerBound(clang::ForStmt *forLoop);
        forLoopCond getLoopUpperBound(clang::ForStmt *forLoop);
        short int getLoopStep(clang::ForStmt *forLoop);

        void findArraySubscriptExpr(clang::Expr * expr, std::vector<clang::ArraySubscriptExpr*>& arraySubscriptExprs);
    public:
        explicit PolyhedralBuilderVisitor(clang::ASTContext * Context);
        bool VisitForStmt(clang::ForStmt *forLoop);
        bool VisitIfStmt(clang::IfStmt *ifStmt);
        bool VisitBinaryOperator(clang::BinaryOperator *BinOp);
        // bool VisitStmt(clang::Stmt * Stmt);
        // bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr * ArraySubscriptExpr);
};

class PolyhedralBuilderASTConsumer : public clang::ASTConsumer {
    public:
        explicit PolyhedralBuilderASTConsumer(clang::ASTContext * Context);
        void HandleTranslationUnit(clang::ASTContext &Context) override;
    private:
        PolyhedralBuilderVisitor Visitor;
};

class PolyhedralBuilderFrontendAction : public clang::ASTFrontendAction {
    public:
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                              llvm::StringRef InFile) override;
};

#endif
