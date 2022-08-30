#! /bin/bash
set -e
find fma-common test -name "*.cpp" -o -name "*.h" | xargs python3 ./cpplint/cpplint.py --quiet
