#include "affineGen.hpp"

///////////////////////
// AffineCheckerFrontendAction
///////////////////////
std::unique_ptr<clang::ASTConsumer> AffineCheckerFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) {
  return std::make_unique<AffineCheckerASTConsumer>(&CI.getASTContext());
}
///////////////////////
// AffineCheckerASTConsumer
///////////////////////
AffineCheckerASTConsumer::AffineCheckerASTConsumer(clang::ASTContext * Context)
  : Visitor(Context){}

void AffineCheckerASTConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

///////////////////////
// AffineCheckerVisitor
///////////////////////
AffineCheckerVisitor::AffineCheckerVisitor(clang::ASTContext * Context)
  : Context(Context){}

void AffineCheckerVisitor::dprintExprCode(clang::Expr * E,
                                          clang::ASTContext &Context) const {

  if (!E) {
    llvm::errs() << "Error: Expression is null.\n";
    return;
  }

  const clang::SourceManager &SM = Context.getSourceManager();
  clang::SourceLocation Begin = E->getBeginLoc();
  clang::SourceLocation End = clang::Lexer::getLocForEndOfToken(E->getEndLoc(), 0, SM, Context.getLangOpts());

  // Ensure that the end location is valid and within the same file
  if (Begin.isInvalid() || End.isInvalid() || !SM.isWrittenInSameFile(Begin, End)) {
    llvm::errs() << "Error: Invalid source locations.\n";
    return;
  }

  // Print the source code between Begin and End
  llvm::StringRef SourceText(SM.getCharacterData(Begin), SM.getCharacterData(End) - SM.getCharacterData(Begin));
  llvm::errs() << "Source code: " << SourceText << "\n";
}

void AffineCheckerVisitor::dprintStmtCode(clang::Stmt * S,
                                          clang::ASTContext &Context) const {
  if (!S) {
    llvm::errs() << "Error: Expression is null.\n";
    return;
  }

  const clang::SourceManager &SM = Context.getSourceManager();
  clang::SourceLocation Begin = S->getBeginLoc();
  clang::SourceLocation End = clang::Lexer::getLocForEndOfToken(S->getEndLoc(), 0, SM, Context.getLangOpts());

  // Ensure that the end location is valid and within the same file
  if (Begin.isInvalid() || End.isInvalid() || !SM.isWrittenInSameFile(Begin, End)) {
    llvm::errs() << "Error: Invalid source locations.\n";
    return;
  }

  // Print the source code between Begin and End
  llvm::StringRef SourceText(SM.getCharacterData(Begin), SM.getCharacterData(End) - SM.getCharacterData(Begin));
  llvm::errs() << "Source code: " << SourceText << "\n";
}

void AffineCheckerVisitor::dprintFatalError(clang::SourceManager &SM,
                                            clang::SourceLocation LocStart,
                                            clang::SourceLocation LocEnd) const {
  llvm::errs() << "Location of the raised fatal error: "
               << SM.getFilename(LocStart).str() << ":"
               << SM.getSpellingLineNumber(LocStart) << ":"
               << SM.getSpellingColumnNumber(LocStart) << "\n";

  llvm::errs() << "From: "
               << SM.getFilename(LocStart).str() << ":"
               << SM.getSpellingLineNumber(LocStart) << ":"
               << SM.getSpellingColumnNumber(LocStart) << " to "
               << SM.getSpellingLineNumber(LocEnd) << ":"
               << SM.getSpellingColumnNumber(LocEnd) << "\n";

  exit(1);
}

bool AffineCheckerVisitor::isIncrementByOne(clang::Expr *Inc) {
  if (auto *UnaryOp = llvm::dyn_cast<clang::UnaryOperator>(Inc)) {
    if (UnaryOp->getOpcode() == clang::UO_PreInc || UnaryOp->getOpcode() == clang::UO_PostInc) {
      return true;
    }
    if (UnaryOp->getOpcode() == clang::UO_PreDec || UnaryOp->getOpcode() == clang::UO_PostDec) {
      return true;
    }
  } else if (auto *CompoundOp = llvm::dyn_cast<clang::CompoundAssignOperator>(Inc)) {
    if (CompoundOp->getOpcode() == clang::BO_AddAssign) {
      if (auto *RHS = llvm::dyn_cast<clang::IntegerLiteral>(CompoundOp->getRHS())) {
        return (RHS->getValue() == 1 || RHS->getValue() == -1);
      }
    }
    if (CompoundOp->getOpcode() == clang::BO_SubAssign) {
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


// NOTE:
// it becomes a untractble problem that requires either infinite number of case catchings
// or some ingenious recursive algorithm to make this function complete. The codebase would
// simply have to assume that no out of the ordinary non-Affine expressions can sneak in
// to the code base. Some examples that are non-Affine but won't be caught by the algorithm:
// z = i * N * j
// NOTE:
// the checker if sees operator that is not Add, Sub, or Mul, automatically will decide
// it is not Affine. If some expressios that are affine that uses operation that is not
// handled by the affine checker, evaluate the expression outside of the loop definition
// TODO: this is a compilation of the edge cases that I found. will be fixed
// 100 * i * j -> not caught by checker
bool AffineCheckerVisitor::isAffineArithExpr(clang::Expr * InitExpr){
  if (!InitExpr) return false;

  bool isAffine = true;

  if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(InitExpr)) {
    switch(BO->getOpcode()){
      case clang::BO_Add:
      case clang::BO_Sub:
        isAffine &= isAffineArithExpr(BO->getLHS());
        isAffine &= isAffineArithExpr(BO->getRHS());
        break;
      case clang::BO_Mul:
        if (clang::Expr *LHS = BO->getLHS()->IgnoreImplicit()){
          if (clang::Expr *RHS = BO->getRHS()->IgnoreImplicit()){

            // recursive check
            bool LHSAffine = isAffineArithExpr(LHS);
            bool RHSAffine = isAffineArithExpr(RHS);

            bool isLHSLoopVariable = false;
            bool isRHSLoopVariable = false;

            if (clang::DeclRefExpr *DR = llvm::dyn_cast<clang::DeclRefExpr>(LHS)) {
              if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(DR->getDecl())) {
                std::string LHSVarName = VD->getNameAsString();
                // llvm::errs() << "======================\n";
                // llvm::errs() << "145 " << LHSVarName << "\n";
                // for (const auto& element : encounteredLoopVars){
                //   llvm::errs() << element << "\n";
                // }
                // if LHSVarName found in encounteredLoopVars
                if (encounteredLoopVars.find(LHSVarName) != encounteredLoopVars.end()) {
                  // llvm::errs() << "LHSVarName " << LHSVarName << " found\n";
                  isLHSLoopVariable = true;
                }
              }
            }

            if (clang::DeclRefExpr *DR = llvm::dyn_cast<clang::DeclRefExpr>(RHS)) {
              if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(DR->getDecl())) {
                std::string RHSVarName = VD->getNameAsString();
                // llvm::errs() << "======================\n";
                // llvm::errs() << "155 " << RHSVarName << "\n";
                // for (const auto& element : encounteredLoopVars){
                //   llvm::errs() << element << "\n";
                // }
                // if RHSVarName found in encounteredLoopVars
                if (encounteredLoopVars.find(RHSVarName) != encounteredLoopVars.end()) {
                  isRHSLoopVariable = true;
                }
              }
            }

            // means you have multiplication of loop variables e.g:  i * j
            // if (isRHSLoopVariable && isLHSLoopVariable) {
            //   return false;
            // }
            // llvm::errs() << "171 (LHS RHS) " << isLHSLoopVariable << " " << isRHSLoopVariable << "\n";

            // combine
            isAffine &= (LHSAffine && RHSAffine) && (!(isRHSLoopVariable && isLHSLoopVariable));

            break;
          }
        }
      default:
        isAffine = false;
        break;
    }
  }

  return isAffine;
}


bool AffineCheckerVisitor::isAffineInit(clang::Stmt *Init){
  if (clang::DeclStmt * DeclStmt = llvm::dyn_cast<clang::DeclStmt>(Init)) {
    if (!DeclStmt->isSingleDecl()){
      // really should be error TODO
      return false;
    }

    // DeclStmt is single decl
    if (clang::VarDecl * VarDecl = llvm::dyn_cast<clang::VarDecl>(DeclStmt->getSingleDecl())) {
      std::string loopVarName = VarDecl->getNameAsString();
      clang::QualType VarType = VarDecl->getType();

      // insert loopVarName to encounteredLoopVars -> if non-linear operator, check if
      // non-linear operation between loop variable and a constant
      // NOTE: when inserting should I check if loopVarName already in?
      this->encounteredLoopVars.insert(loopVarName);

      // check if integer type
      if (!VarType->isIntegerType()){
        return false;
      }

      if (clang::Expr * InitExpr = VarDecl->getInit()){
        // dprintExprCode(InitExpr, *Context);
        // NOTE: by this point InitExpr is the RHS of the assign
        return isAffineArithExpr(InitExpr);
      }
    }
  }
  // catching the cases where init is not the first time the loop variable is being declared
  else if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(Init)){
    if (BO->getOpcode() == clang::BO_Assign) {
      clang::Expr * LHS = BO->getLHS();
      clang::Expr * RHS = BO->getRHS();
      // NOTE: Code could check if the LHS variable is a previously defined loop var
      // but if started catching weird cases like that, then it would be too much
      if (clang::DeclRefExpr *DR = llvm::dyn_cast<clang::DeclRefExpr>(LHS)){
        clang::QualType VarType = DR->getType();
        std::string loopVarName = DR->getNameInfo().getName().getAsString();
        this->encounteredLoopVars.insert(loopVarName);

        if (!VarType->isIntegerType()){
          return false;
        }

        return isAffineArithExpr(RHS);
      }
    }
  }
  return true;
}

// NOTE: basic idea for checking if a loop cond is affine is to check that
// both LHS and RHS are affine
// NOTE: only supported operations for affine condition are:
// BO_LT, BO_LE, BO_GE, BO_GT
bool AffineCheckerVisitor::isAffineCond(clang::Expr *Cond) {
  Cond = Cond->IgnoreImplicit();

  if (auto *BO = llvm::dyn_cast<clang::BinaryOperator>(Cond)) {
    if (BO->getOpcode() == clang::BO_LT ||
        BO->getOpcode() == clang::BO_LE ||
        BO->getOpcode() == clang::BO_GT ||
        BO->getOpcode() == clang::BO_GE ||
        BO->getOpcode() == clang::BO_EQ ||
        BO->getOpcode() == clang::BO_NE) {
      clang::Expr * LHS = BO->getLHS()->IgnoreImplicit();
      clang::Expr * RHS = BO->getRHS()->IgnoreImplicit();

      return isAffineArithExpr(LHS)  && isAffineArithExpr(RHS);
    }
  }
  return false;
}

bool AffineCheckerVisitor::isAffineArrayAccess(clang::Expr * ArrayAccess) {
  return isAffineArithExpr(ArrayAccess);
}

bool AffineCheckerVisitor::VisitForStmt(clang::ForStmt *forLoop) {
  clang::Stmt *Init = forLoop->getInit();
  clang::Expr *Cond = forLoop->getCond();
  clang::Expr *Inc = forLoop->getInc();
  clang::Stmt *Body = forLoop->getBody();

  if (!Init) {
    llvm::errs() << "Error in AffineCheckerVisitor: error occurred when reading for loop init\n";
  } else {
    bool affine_init = isAffineInit(Init);
    // llvm::errs() << "227 DEBUG: " << affine_init << "\n";
    if (!affine_init){
      llvm::errs() << "FATAL ERROR: the polyhedral compilation only accepts affine loop variable init";

      clang::SourceManager &SM = Context->getSourceManager();
      clang::SourceLocation LocStart = Init->getBeginLoc();
      clang::SourceLocation LocEnd = Init->getEndLoc();

      dprintFatalError(SM, LocStart, LocEnd);
    }
  }

  if (!Cond) {
    llvm::errs() << "Error in AffineCheckerVisitor: error occurred when reading for loop condition\n";
  } else {
    bool affine_cond = isAffineCond(Cond);
    if (!affine_cond) {
      llvm::errs() << "FATAL ERROR: the polyhedral compiler only accepts affine loop conditions\n";

      clang::SourceManager &SM = Context->getSourceManager();
      clang::SourceLocation LocStart = Cond->getBeginLoc();
      clang::SourceLocation LocEnd = Cond->getEndLoc();

      dprintFatalError(SM, LocStart, LocEnd);
    }
  }

  if (!Inc) {
    llvm::errs() << "Error in AffineCheckerVisitor: error occurred when reading for loop increment\n";
  } else {
    bool inc_by_one = isIncrementByOne(Inc);
    if (!inc_by_one) {
      llvm::errs() << "FATAL ERROR: the polyhedral compilation only accepts increments by 1\n";

      clang::SourceManager &SM = Context->getSourceManager();
      clang::SourceLocation LocStart = Inc->getBeginLoc();
      clang::SourceLocation LocEnd = Inc->getEndLoc();

      dprintFatalError(SM, LocStart, LocEnd);
    }
  }

  if (!Body) {
    llvm::errs() << "Warning in AffineCheckerVisitor: for loop body does not have content\n";
  }

  return true;
}

bool AffineCheckerVisitor::VisitIfStmt(clang::IfStmt * ifStmt) {
  clang::Expr *Cond = ifStmt->getCond();

  bool is_affine_cond = isAffineCond(Cond);
  if (!is_affine_cond) {
    llvm::errs() << "FATAL ERROR: the polyhedral compilation only accepts affine conditional expression in if stmt\n";

    clang::SourceManager &SM = Context->getSourceManager();
    clang::SourceLocation LocStart = ifStmt->getBeginLoc();
    clang::SourceLocation LocEnd = ifStmt->getEndLoc();

    dprintFatalError(SM, LocStart, LocEnd);
  }
  // the function will return true or terminate
  return true;
}

bool AffineCheckerVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr * ArraySubscriptExpr) {
  // get the index expr
  clang::Expr * Index = ArraySubscriptExpr->getIdx();

  bool is_affine_array_subscript = isAffineArithExpr(Index);

  if (!is_affine_array_subscript) {
    llvm::errs() << "FATAL ERROR: the polyhedral compiler does not allow non-affine array accesses\n";

    clang::SourceManager &SM = Context->getSourceManager();
    clang::SourceLocation LocStart = Index->getBeginLoc();
    clang::SourceLocation LocEnd = Index->getEndLoc();

    dprintFatalError(SM, LocStart, LocEnd);
  }

  return true;
}

// DEPRECATED: using VisitArraySubscriptExpr
// NOTE: for checking statements are affine except for and if which are already handled
// have to recursively check that all array accesses are affine accesses
// regardless of LHS or RHS
// NOTE: essentially (at least for now) affine array access checker
// bool AffineCheckerVisitor::VisitStmt(clang::Stmt * S) {
//   if (llvm::isa<clang::ForStmt>(S) || llvm::isa<clang::IfStmt>(S)){
//     // skip considering ForStmt & IfStmt because handled by other functions
//     return true;
//   }


//   if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(S)) {
//     if (BO->isAssignmentOp()) {
//       clang::Expr * LHS = BO->getLHS();
//       clang::Expr * RHS = BO->getRHS();
//       // first check that LHS has a array access bcuz if not no need to process
//       if (!llvm::dyn_cast<clang::ArraySubscriptExpr>(LHS)) {
//         llvm::errs() << "LHS array access detected\n";
//         exit(1);
//         return true;
//       }

//       // separated checking LHS and RHS for better error message
//       if (!isAffineArrayAccess(LHS)) {
//         llvm::errs() << "FATAL ERROR: the polyhedral compiler does not allow write non-affine array accesses\n";

//         clang::SourceManager &SM = Context->getSourceManager();
//         clang::SourceLocation LocStart = LHS->getBeginLoc();
//         clang::SourceLocation LocEnd = LHS->getEndLoc();

//         dprintFatalError(SM, LocStart, LocEnd);
//       }
//       if (!isAffineArrayAccess(RHS)){
//         llvm::errs() << "FATAL ERROR: the polyhedral compiler does not allow read non-affine array accesses\n";

//         clang::SourceManager &SM = Context->getSourceManager();
//         clang::SourceLocation LocStart = RHS->getBeginLoc();
//         clang::SourceLocation LocEnd = RHS->getEndLoc();

//         dprintFatalError(SM, LocStart, LocEnd);
//       }
//     }
//   }


//   return true;
// }
