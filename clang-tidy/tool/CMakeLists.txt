set(LLVM_LINK_COMPONENTS
  support
  )

add_clang_executable(clang-tidy
  ClangTidyMain.cpp
  )
target_link_libraries(clang-tidy
  clangAST
  clangASTMatchers
  clangBasic
  clangTidy
  clangTidyGoogleModule
  clangTidyLLVMModule
  clangTidyMozillaModule
  clangTidyMiscModule
  clangTidyReadabilityModule
  clangTooling
  )

install(TARGETS clang-tidy
  RUNTIME DESTINATION bin)

