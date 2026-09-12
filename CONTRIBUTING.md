# Contributing

## Commit messages

[Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) are
binding, without exception. The version and the changelog are derived from the
commit types, so a subject outside the scheme produces a wrong version or a
missing changelog entry.

Permitted types: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`,
`build`, `ci`, `chore`, `revert`. Scope optional and lower case. A breaking
change is marked with `!` after the type and `BREAKING CHANGE:` in the body.
Subject line at most 100 characters.

```
feat(i2c): add clock stretching control register
fix(lcd): use a single SPI window per character
```

**The pull request title matters most.** Merges are squash merges and the pull
request title becomes the subject line on `main`, so it is the title that
decides the version. It is checked in CI.

Everything written into this repository is English. Commit subjects and bodies,
pull request titles and bodies, branch names, changelog entries. A pushed
subject line cannot be corrected without rewriting history.

No AI attribution trailers. No `Co-authored-by:` naming an assistant, no
`Generated with …` line, no assistant address as author or committer.

## Branches

Cut from `main`, named `<type>/<short-description>` with the same types as
above, for example `fix/lcd-spi-window`. Check what `HEAD` is on before
branching; a branch accidentally cut from another open branch carries that
branch's work into the merge under a title that does not mention it.

One pull request carries changes of one kind. The squash subject decides the
changelog rubric, so a fix that travels along with a `feat` title is filed under
Features where nobody looks for it. Split the work and merge the parts one after
the other.

## Building

Requires an ARM bare-metal toolchain (`arm-none-eabi-gcc`), CMake 3.13 or newer
and `git` for the version string. The Pico SDK comes from the `pico-sdk`
submodule, so clone with submodules or run
`git submodule update --init --recursive`.

```bash
mkdir build
cd build
cmake ..
make -j8
```

This produces `build/I2Console.uf2`, `build/I2Console.elf` and
`build/I2Console.bin`. `make build` in the repository root does the same thing,
and `make flash` additionally puts a connected device into its bootloader and
flashes it with `picotool`.

Warnings are expected to stay at zero.

## Tests

Host tests for the modules with no hardware dependency live in `tests/`:

```bash
make -C tests          # build and run
make -C tests coverage # and enforce the line coverage floor
```

`make coverage` additionally needs `gcovr`. The floor is hard, and CI runs the
same target.

## Formatting and static analysis

`clang-format` is enforced. The tree is formatted, the pre-commit hook checks
staged files, and CI checks all of them. The vendored STM font tables under
`src/font16.c` and `src/fonts.h` are excluded through `.clang-format-ignore`.

CI pins `clang-format-18`. Version 23 produces identical output on this tree,
so a newer local toolchain is fine. Install it with `brew install llvm` on
macOS or `apt-get install clang-format-18` on Debian; it does not have to be on
`PATH`, because the hook also looks in Homebrew's keg-only location.

`clang-tidy` runs against a declared list of files in
`.github/workflows/ci.yml`. The remaining sources still carry findings, tracked
in issue #19; add a file to the list once it is clean.

## Before you push

The hooks run `gitleaks` and a firmware build on push, and `clang-format` plus
the staged-file guards on commit. Install them once with `lefthook install`.
