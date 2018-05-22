#!/bin/bash

## Compilation is platform-specific, dispatch to helper

if [ "$(uname)" == "Linux" ]; then

	./scripts/compile_linux.sh

else

	./scripts/compile_win32.sh

fi
