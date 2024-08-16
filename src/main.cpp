
#include "affineGen.hpp"
#include "polyhedralBuilder.hpp"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/AST/ASTContext.h"
#include "llvm/Support/CommandLine.h"

#include <isl/ctx.h>
#include <isl/set.h>
#include <isl/map.h>
#include <isl/space.h>
#include <iostream>

using namespace clang::tooling;
using namespace llvm;

// Command-line options for the tool
static cl::OptionCategory ToolCategory("affine-checker options");

// Custom FrontendAction to run the AffineCheckerVisitor
// class AffineCheckerFrontendAction : public clang::ASTFrontendAction {
// public:
//     std::unique_ptr<clang::ASTConsumer>
//     CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) override {
//         return std::make_unique<AffineCheckerFrontendAction>();
//     }
// };



int main(int argc, const char **argv) {
    // Parse command-line options
    // CommonOptionsParser OptionsParser(argc, argv, ToolCategory);

    auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    CommonOptionsParser &OptionsParser = ExpectedParser.get();

    // Create a Clang Tool
    ClangTool AffineCheckerTool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    int AffineCheckerResult = AffineCheckerTool.run(newFrontendActionFactory<AffineCheckerFrontendAction>().get());
    if (AffineCheckerResult != 0) {
        llvm::errs() << "Affine Checker Failed\n"; // shouldn't reach this code because affine checker will terminate running code if checker fails
        return AffineCheckerResult;
    }

}
