#!/bin/bash
# WARNING: THINK BEFORE YOU TYPE
# This script is not meant for contributors
# Do not use this script during runs.
# The doings of this script are irreversible
# It will delete all of the tools products
# Only meant to be used sparingly
# cleans build*, *.dAlog, *.log, *.out

find ./ -type f -name '*.bc*' -exec rm -rf {} \;
find ./ -type f -name '*.ll' -exec rm -rf {} \;
find ./ -type f -name '*.tr*' -exec rm -rf {} \;
find ./ -type f -name '*.bin' -exec rm -rf {} \;
find ./ -type f -name '*.json' -exec rm -rf {} \;
find ./ -type f -name '*.exec' -exec rm -rf {} \;
find ./ -type f -name '*.elf*' -exec rm -rf {} \;
find ./ -type f -name '*.native' -exec rm -rf {} \;
find ./ -type f -name '*.dot' -exec rm -rf {} \;
find ./ -type f -name '*.obj' -exec rm -rf {} \;
find ./ -type f -name '*.gcda' -exec rm -rf {} \;
find ./ -type f -name '*.gcno' -exec rm -rf {} \;
find ./ -type f -name '*.gcov' -exec rm -rf {} \;
find ./ -type f -name '*.log' -exec rm -rf {} \;
find ./ -type f -name '*.data' -exec rm -rf {} \;
find ./ -type f -name '*.out' -exec rm -rf {} \;
find ./ -type f -name '*_generated*' -exec rm -rf {} \;
find ./ -type f -name '*_output.*' -exec rm -rf {} \;
find ./ -type f -name '*.raw*' -exec rm -rf {} \;
find ./ -type d -name 'oprofile_data' -exec rm -rf {} \;

#FINISH
