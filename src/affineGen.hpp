
#ifndef AFFINEGEN_HPP
#define AFFINEGEN_HPP

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
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

#include <cstdlib>
#include <unordered_set>

using namespace clang::tooling;
using namespace llvm;

class AffineCheckerVisitor
  : public clang::RecursiveASTVisitor<AffineCheckerVisitor> {

  private:
    clang::ASTContext * Context;
    std::unordered_set<std::string> encounteredLoopVars;

    bool isIncrementByOne(clang::Expr *Inc);
    bool isAffineCond(clang::Expr *Cond);
    bool isAffineInit(clang::Stmt *Init);
    bool isAffineArithExpr(clang::Expr *InitExpr);
    bool isAffineArrayAccess(clang::Expr *ArrayAccess);

    // functions for debug/printing
    void dprintFatalError(clang::SourceManager & SM,
                         clang::SourceLocation LocStart,
                         clang::SourceLocation LocEnd) const;
    void dprintExprCode(clang::Expr * E,
                        clang::ASTContext &Context) const;
    void dprintStmtCode(clang::Stmt * S,
                        clang::ASTContext &Context) const;

  public:
    explicit AffineCheckerVisitor(clang::ASTContext * Context);
    bool VisitForStmt(clang::ForStmt *forLoop);
    bool VisitIfStmt(clang::IfStmt * ifStmt);
    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr * ArraySubscriptExpr);
    // bool VisitStmt(clang::Stmt *S); DEPRECATED
};


class AffineCheckerASTConsumer : public clang::ASTConsumer {
  public:
    explicit AffineCheckerASTConsumer(clang::ASTContext * Context);
    void HandleTranslationUnit(clang::ASTContext &Context) override;
  private:
    AffineCheckerVisitor Visitor;
};

class AffineCheckerFrontendAction : public clang::ASTFrontendAction {
  public:
    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef InFile) override;
};
#endif
