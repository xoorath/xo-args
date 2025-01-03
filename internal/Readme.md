# [Readme.md](../Readme.md) > Internal / Readme.md

This sub-directory of xo-args contains a build system and unit tests for xo-args
that aren't as relevant to users of xo-args.

# Supported Compilers and Platforms

* ✅ clang on Linux: Supported and regularly tested.
* ✅ gcc on Linux: Supported and regularly tested.
* ✅ msvc on Windows: Supported and regularly tested.
* ❔ clang everywhere else: Unknown. Probably supported.
* ❔ gcc everywhere else: Unknown. Probably supported.
* ❔ other compilers and platforms: Unknown. Probably supported.

If you know how to use [Premake5](https://premake.github.io/) you should be able
to get other build systems to work. The above listed support is what's being
tested during development of xo-args and is expected to work with scripts in
this repository.

# Contributor Setup

Follow the contributor guide for [Linux](./docs/dev-setup-linux.md) or
[Windows](./docs/dev-setup-windows.md).

# Contributor Guide

## 1. Test your changes.

Make sure all projects in the solution build and all tests pass for msvc, clang,
and gcc. There's a [comprehensive-test.bat](./scripts/comprehensive-test.bat)
script that is intended to make this easier: it builds all projects in every
configuration across Windows and Linux (using WSL).

## 2. Format your code.

Changes to the code should be followed by running
[ClangFormat](https://clang.llvm.org/docs/ClangFormat.html). I recommend using
[Clang  Power Tools](https://clangpowertools.com/) for Visual Studio which can
be used to invoke ClangFormat.

## 3. Participate in discussion.

If you're submitting a pull request make sure to leave a useful description
that can help me understand the intention of your changes and any major
decisions that went into it.

If you're thinking of making a large change please reach out to discuss it first.

# Contact

I can be reached on [Mastodon](https://merveilles.town/@xo) or by
[email](jared+thomson@xoorath.com).