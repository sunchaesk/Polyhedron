#include "affineGen.hpp"

///////////////////////
// AffineCheckerASTConsumer
///////////////////////
std::unique_ptr<clang::ASTConsumer> AffineCheckerFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) {
  return std::make_unique<AffineCheckerASTConsumer>();
}
///////////////////////
// AffineCheckerASTConsumer
///////////////////////
void AffineCheckerASTConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  AffineCheckerVisitor Visitor;
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

///////////////////////
// AffineCheckerVisitor
///////////////////////
bool AffineCheckerVisitor::isIncrementByOne(clang::Expr *Inc) {
  if (auto *UnaryOp = llvm::dyn_cast<clang::UnaryOperator>(Inc)) {
    if (UnaryOp->getOpcode() == clang::UO_PreInc || UnaryOp->getOpcode() == clang::UO_PostInc) {
      return true;
    }
  } else if (auto *CompoundOp = llvm::dyn_cast<clang::CompoundAssignOperator>(Inc)) {
    if (CompoundOp->getOpcode() == clang::BO_AddAssign) {
      if (auto *RHS = llvm::dyn_cast<clang::IntegerLiteral>(CompoundOp->getRHS())) {
        return (RHS->getValue() == 1 || RHS->getValue() == -1);
      }
    }
  } else if (auto *BinaryOp = llvm::dyn_cast<clang::BinaryOperator>(Inc)) {
    if (BinaryOp->getOpcode() == clang::BO_Assign) {
      if (auto *ArithOp = llvm::dyn_cast<clang::BinaryOperator>(BinaryOp->getRHS())) {
        if (ArithOp->getOpcode() == clang::BO_Add || ArithOp->getOpcode() == clang::BO_Sub) {
          if (auto *RHS = llvm::dyn_cast<clang::IntegerLiteral>(ArithOp->getRHS())) {
            return (RHS->getValue() == 1 || RHS->getValue() == -1);
          }
        }
      }
    }
  }
  return false;
}

bool AffineCheckerVisitor::VisitForStmt(clang::ForStmt *forLoop) {
  clang::Stmt *Init = forLoop->getInit();
  clang::Expr *Cond = forLoop->getCond();
  clang::Expr *Inc = forLoop->getInc();
  clang::Stmt *Body = forLoop->getBody();


  if (!Init) {
    llvm::errs() << "Error in AffineCheckerVisitor: error occurred when reading for loop init\n";
  }

  if (!Cond) {
    llvm::errs() << "Error in AffineCheckerVisitor: error occurred when reading for loop condition\n";
  }

  if (!Inc) {
    llvm::errs() << "Error in AffineCheckerVisitor: error occurred when reading for loop increment\n";
  } else {
    bool inc_by_one = isIncrementByOne(Inc);
    if (!inc_by_one) {
      llvm::errs() << "FATAL ERROR: the polyhedral compilation only accepts increments by 1\n";
      clang::SourceLocation LocStart = Inc->getBeginLoc();
      clang::SourceLocation LocEnd = Inc->getEndLoc();
      llvm::errs() << "Location of increment issue: ";
    }
  }

  if (!Body) {
    llvm::errs() << "Warning in AffineCheckerVisitor: for loop body does not have content\n";
  }

  return true;
}
