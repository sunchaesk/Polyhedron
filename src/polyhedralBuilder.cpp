
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

/////////
// **** PRIVATE
/////////
std::string PolyhedralBuilderVisitor::getLoopVar(clang::ForStmt *forLoop) {
    if (clang::DeclStmt * declStmt = llvm::dyn_cast<clang::DeclStmt>(forLoop->getInit())) {
        if (clang::VarDecl * var  = llvm::dyn_cast<clang::VarDecl>(declStmt->getSingleDecl())) {
            return var->getNameAsString();
        }
    }
    return NULL;
}

clang::Expr * PolyhedralBuilderVisitor::getLoopLowerBound(clang::ForStmt *forLoop) {
    if (clang::DeclStmt * declStmt = llvm::dyn_cast<clang::DeclStmt>(forLoop->getInit())) {
        if (clang::VarDecl * var = llvm::dyn_cast<clang::VarDecl>(declStmt->getSingleDecl())) {
            return var->getInit();
        }
    }
    return nullptr;
}

// NOTE: it is assumed that the RHS of the loop is the upper bound of the loop variable
// TODO: make it so that the upper bound is the side of the Cond that doesn't have the loop variable
// TODO: have the Cond to allow the loop variable side to not have just the loop variable
// -> meaning do inverse operations to isolate the loop variable
clang::Expr * PolyhedralBuilderVisitor::getLoopUpperBound(clang::ForStmt *forLoop) {
    if (clang::BinaryOperator *binOp = llvm::dyn_cast<clang::BinaryOperator>(forLoop->getCond())) {
        if (binOp->isComparisonOp()) {
            return binOp->getRHS();
        }
    }
    return nullptr;
}

// loopStep can only be values of -1 or 1
short int PolyhedralBuilderVisitor::getLoopStep(clang::ForStmt *forLoop) {
    if (clang::Expr * Inc = llvm::dyn_cast<clang::Expr>(forLoop->getInc())) {
        if (auto *UnaryOp = llvm::dyn_cast<clang::UnaryOperator>(Inc)) {
            if (UnaryOp->getOpcode() == clang::UO_PreInc || UnaryOp->getOpcode() == clang::UO_PostInc) {
                return 1;
            }
            if (UnaryOp->getOpcode() == clang::UO_PreDec || UnaryOp->getOpcode() == clang::UO_PostDec) {
                return -1;
            }
        } else if (auto *CompoundOp = llvm::dyn_cast<clang::CompoundAssignOperator>(Inc)) {
            if (CompoundOp->getOpcode() == clang::BO_AddAssign) {
                if (auto *RHS = llvm::dyn_cast<clang::IntegerLiteral>(CompoundOp->getRHS())) {
                    if (RHS->getValue() == 1) {
                        return 1;
                    }
                    if (RHS->getValue() == -1) {
                        return -1;
                    }
                }
            }
            if (CompoundOp->getOpcode() == clang::BO_SubAssign) {
                if (auto *RHS = llvm::dyn_cast<clang::IntegerLiteral>(CompoundOp->getRHS())) {
                    if (RHS->getValue() == 1) {
                        return -1;
                    }
                    if (RHS->getValue() == -1){
                        return 1; // -(-1) = 1
                    }
                }
            }
        } else if (auto *BinaryOp = llvm::dyn_cast<clang::BinaryOperator>(Inc)) {
            if (BinaryOp->getOpcode() == clang::BO_Assign) {
                if (auto *ArithOp = llvm::dyn_cast<clang::BinaryOperator>(BinaryOp->getRHS())) {

                    if (ArithOp->getOpcode() == clang::BO_Add){
                        if (auto *RHS = llvm::dyn_cast<clang::IntegerLiteral>(ArithOp->getRHS())) {
                            if (RHS->getValue() == 1){
                                return 1;
                            }
                            if (RHS->getValue() == -1){
                                return -1;
                            }
                        }
                    }

                    if (ArithOp->getOpcode() == clang::BO_Sub) {
                        if (auto *RHS = llvm::dyn_cast<clang::IntegerLiteral>(ArithOp->getRHS())) {
                            if (RHS->getValue() == 1){
                                return -1;
                            }
                            if (RHS->getValue() == -1){
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        return NULL;
    }
}

/////////
// **** PUBLIC
/////////

PolyhedralBuilderVisitor::PolyhedralBuilderVisitor(clang::ASTContext * Context)
    : Context(Context) {}


bool PolyhedralBuilderVisitor::VisitForStmt(clang::ForStmt *forLoop) {
    std::string loopVar = getLoopVar(forLoop);
    clang::Expr * lowerBound = getLoopLowerBound(forLoop);
    clang::Expr * upperBound = getLoopUpperBound(forLoop);
    clang::Expr * step = getLoopStep(forLoop);

    if (loopVar == NULL) {
        llvm::errs() << "FATAL ERROR POLY-BUILDER: loopVar returned as NULL\n";
        exit(1);
    }
    if (lowerBound == nullptr) {
        llvm::errs() << "FATAL ERROR POLY-BUILDER: lowerBound returned as NULL\n";
        exit(1);
    }
    if (upperBound == nullptr) {
        llvm::errs() << "FATAL ERROR POLY-BUILDER: upperBound returned as NULL\n";
        exit(1);
    }
    if (step == nullptr) {
        llvm::errs() << "FATAL ERROR POLY-BUILDER: step returned as NULL\n";
        exit(1);
    }

    PolyhedralLoopInfo loopInfo (loopVar, lowerBound, upperBound, step);

    loopInfoVec.push_back(loopInfo);

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

        // first check that LHS is an array access
        if (!llvm::dyn_cast<clang::ArraySubscriptExpr>(LHS)) {
            return true;
        }

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

        // NOTE: now it can be assumed that LHS has an array access
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
