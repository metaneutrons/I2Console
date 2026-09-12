#!/bin/sh
# clang-format check over the files handed in.
#
# Usage: check-format.sh <file> [<file> ...]
#
# Exists as a script rather than a one-liner in lefthook.yml for one reason:
# clang-format is frequently not on PATH. Homebrew keeps LLVM keg-only under
# /opt/homebrew/opt/llvm/bin, so `clang-format` is missing in a default shell
# even on a machine that has it installed, and the hook then fails with a bare
# "command not found" that says nothing about what to do.
#
# Missing tool is a hard failure, not a silent skip: a gate that disappears
# when the tool is absent protects nothing.
set -eu

find_clang_format() {
    if command -v clang-format >/dev/null 2>&1; then
        command -v clang-format
        return 0
    fi
    for candidate in \
        /opt/homebrew/opt/llvm/bin/clang-format \
        /usr/local/opt/llvm/bin/clang-format \
        /usr/lib/llvm-18/bin/clang-format
    do
        [ -x "$candidate" ] && { printf '%s\n' "$candidate"; return 0; }
    done
    return 1
}

if ! cf=$(find_clang_format); then
    printf '\033[31mCommit rejected:\033[0m clang-format was not found.\n' >&2
    printf '  macOS:  brew install llvm\n' >&2
    printf '  Debian: apt-get install clang-format-18\n' >&2
    printf '  It does not have to be on PATH; the Homebrew keg-only location\n' >&2
    printf '  is checked too.\n' >&2
    exit 1
fi

[ "$#" -gt 0 ] || exit 0

if ! "$cf" --dry-run --Werror "$@" 2>&1; then
    printf '\033[31mCommit rejected:\033[0m the files above are not formatted.\n' >&2
    printf '  Fix them with: %s -i %s\n' "$cf" "$*" >&2
    exit 1
fi
