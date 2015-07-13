//===--- LLVMTidyModule.cpp - clang-tidy ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "PrivateRefcntDestructorCheck.h"

namespace clang {
namespace tidy {
namespace mozilla {

class MozillaModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<PrivateRefcntDestructorCheck>("mozilla-private-refcnt-destructor");
  }
};

// Register the LLVMTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<MozillaModule> X("mozilla-module",
                                                     "Adds Mozilla lint checks.");

} // namespace mozilla

// This anchor is used to force the linker to link in the generated object file
// and thus register the MozillaModule.
volatile int MozillaModuleAnchorSource = 0;

} // namespace tidy
} // namespace clang
