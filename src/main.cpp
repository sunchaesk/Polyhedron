#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/AST/ASTContext.h"
#include "llvm/Support/CommandLine.h"
#include "affineGen.hpp"

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


int main(int argc, const char ** argv) {

    isl_ctx * ctx = isl_ctx_alloc();
    std::cout << "Pointer " << std::endl;

    return 1;
}

// int main(int argc, const char **argv) {
//     // Parse command-line options
//     // CommonOptionsParser OptionsParser(argc, argv, ToolCategory);

//     auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolCategory);
//     if (!ExpectedParser) {
//         llvm::errs() << ExpectedParser.takeError();
//         return 1;
//     }

//     CommonOptionsParser &OptionsParser = ExpectedParser.get();

//     // Create a Clang Tool
//     ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

//     // Run the tool with our custom FrontendAction
//     return Tool.run(newFrontendActionFactory<AffineCheckerFrontendAction>().get());
// }
