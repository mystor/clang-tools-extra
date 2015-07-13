set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyMozillaModule
  PrivateRefcntDestructorCheck.cpp
  MozillaTidyModule.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTidy
  clangTidyReadabilityModule
  clangTidyUtils
  clangTooling
  )
