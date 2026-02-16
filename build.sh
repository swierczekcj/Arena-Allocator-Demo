#!/usr/bin/env bash

## This is a simple script for compiling the code
## outside an IDE. If the project gets bigger we
## can investigate using Make, CMake, Meson,
## Ninja, etc...

# Always put your script main function at bottom,
# see https://arongriffis.com/2023-11-18-bash-main

# We'll explicitly echo anything we want on screen.
set x

check_os() {
  OSNAME=`uname`
  if [[ "$OSNAME" != "Linux" ]] && [[ "$OSNAME" != "Darwin" ]]
  then
    echo "This build script only supports Linux (including WSL) and MacOS."
    echo "Found: $OSNAME"
    exit 1
  fi
}

set_compiler() {
  GCC=`which gcc`
  CLANG=`which clang`
  if [[ -n "$GCC" ]]
  then
    echo "Found GCC"
    CC="$GCC"
  elif [[ -n "$CLANG" ]]
  then
    echo "Found LLVM Clang"
    CC="$CLANG"
  else
    echo "Could not find GCC or Clang."
    exit 1
  fi
}

build() {
  ## The SCRIPT_DIR snippet taken from StackOverflow.
  SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
  pushd "${SCRIPT_DIR}/code" > /dev/null
  CMD="$CC arena.h arena.c toy.c"
  eval "$CMD"
  if [[ $? -ne 0 ]]
  then
    echo "There was an error compiling."
    exit 1
  else
    echo "Compilation finished."
  fi
  popd > /dev/null
}

main() {
  check_os
  set_compiler
  build
  exit
}

main "$@"
