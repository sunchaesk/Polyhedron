
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
    llvm::errs() << "HIT VisitForStmt\n";
    return true;
}

bool PolyhedralBuilderVisitor::VisitIfStmt(clang::IfStmt * ifStmt) {
    llvm::errs() << "HIT VisitIfStmt\n";
    return true;
}

// NOTE: this function skips if the current binary operator is DeclRefExpr, or is within
// For or If Stmt.
// NOTE: VisitBinaryOperator doesn't seem to be triggered when running into initializations
bool PolyhedralBuilderVisitor::VisitBinaryOperator(clang::BinaryOperator * BinOp) {
    if (BinOp->isAssignmentOp()) {
        clang::Expr * LHS = BinOp->getLHS();
        clang::Expr * RHS = BinOp->getRHS();

        // llvm::errs() << "HIT VisitBinaryOperator\n";

        // check if the assignment is part of the initialization
        if (auto *LHSDeclRef = llvm::dyn_cast<clang::DeclRefExpr>(LHS)) {
            if (auto * Var = llvm::dyn_cast<clang::VarDecl>(LHSDeclRef->getDecl())) {
                if (Var->hasInit()) {
                    return true;
                }
            }
        }

        // check if assignment is part of for loop, then skip
        const clang::BinaryOperator& currentBinOp = *BinOp;
        const auto& parents = Context->getParents(currentBinOp);
        auto it = Context->getParents(currentBinOp).begin();
        if (it == Context->getParents(currentBinOp).end()) {
            // traverse next node if parent not found
            return true;
        }
        if (!parents.empty()) {
            for (size_t i = 0; i < parents.size(); i++) {
                const clang::Stmt * parentStmt = parents[i].get<clang::Stmt>();
                if (llvm::isa<clang::ForStmt>(parentStmt)) {
                    return true;
                }
            }
        }

    }

    return true;
}

// bool PolyhedralBuilderVisitor::VisitBinaryOperator(clang::BinaryOperator *BinOp){
//     if (BinOp->isAssignmentOp()) {
//         clang::Expr * LHS = BinOp->getLHS();
//         clang::Expr * RHS = BinOp->getRHS();

//         llvm::errs() << "HIT!\n";
//         // check if this is initialization for the first time. Then it is irrelevant
//         // Check if the LHS is a declaration (i.e., initialization) rather than an assignment
//         if (auto *LHSDeclRef = llvm::dyn_cast<clang::DeclRefExpr>(LHS)) {
//             if (auto *Var = llvm::dyn_cast<clang::VarDecl>(LHSDeclRef->getDecl())) {
//                 if (Var->hasInit()) {
//                     return true; // return true means to skip the statement
//                 }
//             }
//         }



//         // Skip if the parent statement is an IfStmt or ForStmt
//         if (auto *ParentStmt = BinOp->getLHS()->IgnoreParenImpCasts()) {
//             if (llvm::isa<clang::IfStmt>(ParentStmt) || llvm::isa<clang::ForStmt>(ParentStmt)) {
//                 llvm::errs() << "HIT 1 \n";
//                 return true;
//             }
//         }
//     }

//     return true;
// }
