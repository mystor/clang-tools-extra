//===--- PrivateRefcntDestructorCheck.h - clang-tidy -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MOZILLA_PRIVATE_REFCNT_DESTRUCTOR_CHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MOZILLA_PRIVATE_REFCNT_DESTRUCTOR_CHECK_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace mozilla {

/// \brief Checks for public refcnt destructors
class PrivateRefcntDestructorCheck : public ClangTidyCheck {
public:
  PrivateRefcntDestructorCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace mozilla
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MOZILLA_PRIVATE_REFCNT_DESTRUCTOR_CHECK_H
