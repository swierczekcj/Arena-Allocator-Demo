#!/usr/bin/env bash

## Simple script to run the compiled code

# We don't need anything here printed to screen.
set x

main() {
  SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
  pushd "$SCRIPT_DIR/code" > /dev/null
  ./a.out
  popd > /dev/null
  exit
}

main "$@"
