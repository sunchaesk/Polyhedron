
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

#include <cstdlib>

using namespace clang::tooling;
using namespace llvm;

class AffineCheckerVisitor
  : public clang::RecursiveASTVisitor<AffineCheckerVisitor> {

  private:
    bool isIncrementByOne(clang::Expr *Inc);

  public:
    bool VisitForStmt(clang::ForStmt *forLoop);
};


class AffineCheckerASTConsumer : public clang::ASTConsumer {
  public:
    void HandleTranslationUnit(clang::ASTContext &Context) override;
};

class AffineCheckerFrontendAction : public clang::ASTFrontendAction {
  public:
    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef InFile) override;
};
#endif
