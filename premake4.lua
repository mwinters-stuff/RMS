--  ugly hack to use clang
premake.gcc.cc  = 'clang'
premake.gcc.cxx = 'clang++'

solution "rms"
  configurations {"debug","release"}
  platforms {"native"}
  project "rms"
    targetname "rms"
    language "C++"
    kind "ConsoleApp"
    files{
      "*.cpp"
    }
    buildoptions{"-Wall -std=c++14 -pedantic"}
    links {}
