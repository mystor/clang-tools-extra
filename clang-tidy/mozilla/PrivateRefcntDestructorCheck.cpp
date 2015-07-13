//===--- PrivateRefcntDestructorCheck.cpp - clang-tidy ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PrivateRefcntDestructorCheck.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"

namespace clang {
namespace tidy {
namespace mozilla {

namespace {

/// A cached data of whether classes are refcounted or not.
typedef llvm::DenseMap<const CXXRecordDecl *,
  std::pair<const Decl *, bool> > RefCountedMap;
RefCountedMap refCountedClasses;

bool classHasAddRefRelease(const CXXRecordDecl *D) {
  const RefCountedMap::iterator& it = refCountedClasses.find(D);
  if (it != refCountedClasses.end()) {
    return it->second.second;
  }

  bool seenAddRef = false;
  bool seenRelease = false;
  for (CXXRecordDecl::method_iterator method = D->method_begin();
       method != D->method_end(); ++method) {
    if (!method->getIdentifier()) continue;

    const auto &name = method->getName();
    if (name == "AddRef") {
      seenAddRef = true;
    } else if (name == "Release") {
      seenRelease = true;
    }
  }
  refCountedClasses[D] = std::make_pair(D, seenAddRef && seenRelease);
  return seenAddRef && seenRelease;
}

bool isClassRefCounted(QualType T);

bool isClassRefCounted(const CXXRecordDecl *D) {
  // Normalize so that D points to the definition if it exists.
  if (!D->hasDefinition())
    return false;
  D = D->getDefinition();
  // Base class: anyone with AddRef/Release is obviously a refcounted class.
  if (classHasAddRefRelease(D))
    return true;

  // Look through all base cases to figure out if the parent is a refcounted class.
  for (CXXRecordDecl::base_class_const_iterator base = D->bases_begin();
       base != D->bases_end(); ++base) {
    bool super = isClassRefCounted(base->getType());
    if (super) {
      return true;
    }
  }

  return false;
}

bool isClassRefCounted(QualType T) {
  while (const ArrayType *arrTy = T->getAsArrayTypeUnsafe())
    T = arrTy->getElementType();
  CXXRecordDecl *clazz = T->getAsCXXRecordDecl();
  return clazz ? isClassRefCounted(clazz) : false;
}

} // namespace

void PrivateRefcntDestructorCheck::registerMatchers(ast_matchers::MatchFinder *Finder) {
  Finder->addMatcher(ast_matchers::recordDecl(ast_matchers::decl().bind("decl")), this);
}

void PrivateRefcntDestructorCheck::check(const ast_matchers::MatchFinder::MatchResult &Result) {
  const CXXRecordDecl *Decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");
  if (!isClassRefCounted(Decl)) return;
  if (Decl->isAbstract()) return;
  if (!Decl->hasDefinition()) return;
  Decl = Decl->getDefinition();

  bool InvalidIndent = false;
  int Indent = Result.SourceManager->getSpellingColumnNumber(Decl->getLocStart(), &InvalidIndent);
  if (InvalidIndent) Indent = 0;
  std::string IndentStr;
  for (int I = Indent; I > 1; --I) {
    IndentStr.push_back(' ');
  }

  CXXDestructorDecl* Destructor;
  SourceLocation InsertLocation;
  bool FoundInsertLocation = false;

  // Discover the location for the insertion
  for (DeclContext::decl_iterator InnerDecl = Decl->decls_begin(), e = Decl->decls_end();
       InnerDecl != e; ++InnerDecl) {
    // We use this instead of getDestructor as if there is an explicit declaration,
    // we don't want to get it, we only want the declaration lexically within the struct
    CXXDestructorDecl* DD = dyn_cast<CXXDestructorDecl>(*InnerDecl);
    if (DD) {
      if (DD->getAccess() != AS_public) return;
      if (DD->isDeleted()) return;
      Destructor = DD;
    }

    AccessSpecDecl* AS = dyn_cast<AccessSpecDecl>(*InnerDecl);
    if (AS && AS->getAccess() == AS_protected) {
      FoundInsertLocation = true;
      InsertLocation = AS->getColonLoc().getLocWithOffset(1);
      break;
    }
  }
  // WOO Useless diagnostics! (It's OK)
  auto D = diag(Decl->getLocStart(), "Refcounted class with public explicit destructor");
  if (FoundInsertLocation) {
    D << FixItHint::CreateInsertion(InsertLocation, "\n");
  } else {
    InsertLocation = Decl->getRBraceLoc();
    D << FixItHint::CreateInsertion(InsertLocation, "protected:\n");
  }

  // Either move the original destructor here, or create a new one.
  if (Destructor && !Destructor->isImplicit()) {
    SourceLocation StartLoc = Destructor->getLocStart();
    SourceLocation EndLoc = Destructor->getLocEnd();
    if (!Destructor->doesThisDeclarationHaveABody()) {
      EndLoc = EndLoc.getLocWithOffset(1); // Also bring the semicolon
    }
    SourceRange Range(StartLoc, EndLoc);
    CharSourceRange DestructorSourceRange(Range, true);
    D << FixItHint::CreateInsertion(InsertLocation, IndentStr);
    D << FixItHint::CreateInsertion(InsertLocation, "  ");
    D << FixItHint::CreateInsertionFromRange(InsertLocation, DestructorSourceRange);
    D << FixItHint::CreateInsertion(InsertLocation, "\n");
    D << FixItHint::CreateInsertion(InsertLocation, IndentStr);
    D << FixItHint::CreateRemoval(DestructorSourceRange);
  } else {
    D << FixItHint::CreateInsertion(InsertLocation, IndentStr);
    D << FixItHint::CreateInsertion(InsertLocation, "  ~");
    D << FixItHint::CreateInsertion(InsertLocation, Decl->getName());
    D << FixItHint::CreateInsertion(InsertLocation, "() {}\n");
    D << FixItHint::CreateInsertion(InsertLocation, IndentStr);
  }
}

} // namespace llvm
} // namespace tidy
} // namespace clang
