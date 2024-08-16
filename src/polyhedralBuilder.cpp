
#include "polyhedralBuilder.hpp"

////////////////////
// PolyhedralBuilderFrontEndAction
////////////////////
std::unique_ptr<clang::ASTConsumer> PolyhedralBuilderFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI,
                                                                                       llvm::StringRef InFile) {
    return std::make_unique<PolyhedralBuilderASTConsumer>(&CI.getASTContext());
}

////////////////////
// PolyhedralBuilderASTConsumer
////////////////////
PolyhedralBuilderASTConsumer::PolyhedralBuilderASTConsumer(clang::ASTContext * Context)
    : Visitor(Context) {}

void PolyhedralBuilderASTConsumer::HandleTranslationUnit(clang::ASTContext & Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

////////////////////
// PolyhedralBuilderVisitor
////////////////////
PolyhedralBuilderVisitor::PolyhedralBuilderVisitor(clang::ASTContext * Context)
    : Context(Context) {}

bool PolyhedralBuilderVisitor::VisitForStmt(clang::ForStmt *forLoop) {

    return true;
}

bool PolyhedralBuilderVisitor::VisitIfStmt(clang::IfStmt * ifStmt) {

    return true;
}

bool PolyhedralBuilderVisitor::VisitBinaryOperator(clang::BinaryOperator *BinOp){
    if (BinOp->isAssignmentOp()) {
        clang::Expr * LHS = BinOp->getLHS();
        clang::Expr * RHS = BinOp->getRHS();

        // check if this is initialization for the first time. Then it is irrelevant
        if (llvm::isa<clang::DeclRefExpr>(LHS)) {
            clang::DeclRefExpr *LHSDeclRef = llvm::cast<clang::DeclRefExpr>(LHS);
            clang::ValueDecl *ValueDecl = LHSDeclRef->getDecl();

            if (ValueDecl & ValueDecl->isLocalVarDecl()) {
                clang::VarDecl * Var = llvm::dyn_cast<clang::VarDecl>(ValueDecl);
                if (Var && Var->hasInit()) {
                    return true;
                }
            }
        }

        // check if currently in ForStmt or IfStmt. If yes then skip
        if (llvm::isa<clang::IfStmt>(BinOp->getLHS()->IgnoreParenImpCasts()->getStmtClass()) ||
            llvm::isa<clang::ForStmt>(BinOp->getLHS()->IgnoreParenImpCasts()->getStmtClass())){
            llvm::errs() << "HIT 1 \n";
            return true;
        }

        llvm::errs() << "HIT!\n";
    }

    return true;
}
