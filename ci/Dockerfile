# syntax=docker/dockerfile:1
#
# The image of the check job (.github/workflows/check.yml): OpenUSD in
# /opt/USD, clang-repl 23, and the programs that make check calls. A person
# builds it with .github/workflows/ci-image.yml; a check run never builds
# OpenUSD. The plan is plans/objective-1-ci-openusd-image.md (issue #5).
#
# Build it by hand from the repository root:
#
#   docker build -t openusd-tutorial-ci ci/
#
# OPENUSD_REF is the OpenUSD commit (or tag) that goes in /opt/USD. JOBS is
# the number of parallel compile jobs; the default is the number of CPUs.

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

# GCC 13 and make (build-essential), CMake, git for FetchContent and the tree
# checks, Python for build_usd.py, scripts/check_tree.py and cpp-repl, and
# libgl-dev because cmake/OpenUSDExample.cmake calls find_package(OpenGL).
# python3-dev is for cpp-repl only: its usd profile asks for Python.h when
# OpenUSD has no Python support (farhatnadim/cpp_repl#35). No OpenUSD header
# includes Python.h here, because the define is off.
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      ca-certificates \
      cmake \
      curl \
      git \
      libgl-dev \
      make \
      python3 \
      python3-dev \
 && rm -rf /var/lib/apt/lists/*

# gh comes from cli.github.com: the gh 2.45 of Ubuntu 24.04 has no
# closingIssuesReferences, which the step "The pull request closes an issue"
# reads. clang-23 and clang-tools-23 (clang-repl-23) come from apt.llvm.org,
# the same LLVM major version as on the machine of the author.
ADD https://cli.github.com/packages/githubcli-archive-keyring.gpg /etc/apt/keyrings/githubcli-archive-keyring.gpg
ADD https://apt.llvm.org/llvm-snapshot.gpg.key /etc/apt/keyrings/apt.llvm.org.asc
RUN chmod 0644 /etc/apt/keyrings/githubcli-archive-keyring.gpg /etc/apt/keyrings/apt.llvm.org.asc \
 && echo "deb [signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" \
      > /etc/apt/sources.list.d/github-cli.list \
 && echo "deb [signed-by=/etc/apt/keyrings/apt.llvm.org.asc] https://apt.llvm.org/noble/ llvm-toolchain-noble-23 main" \
      > /etc/apt/sources.list.d/apt.llvm.org.list \
 && apt-get update \
 && apt-get install -y --no-install-recommends clang-23 clang-tools-23 gh \
 && rm -rf /var/lib/apt/lists/* \
 && clang-repl-23 --version

# clang++ and clang-repl with no version suffix, as repl/TUTORIAL.md calls
# them. CMake still finds c++ (GCC 13) first for the lessons.
ENV PATH=/usr/lib/llvm-23/bin:${PATH}

# OpenUSD with no Python, no imaging and no tools: the lessons are C++ and
# link usd, usdGeom, usdLux, usdShade and usdPhysics, which this build has.
# The sources and the build tree go in the same step, so no layer keeps them.
ARG OPENUSD_REF=6792941c5d5b42778e6875c5ed5a3279e6936981
ARG JOBS=
RUN git init -q /tmp/OpenUSD \
 && git -C /tmp/OpenUSD fetch -q --depth 1 https://github.com/PixarAnimationStudios/OpenUSD.git "${OPENUSD_REF}" \
 && git -C /tmp/OpenUSD checkout -q FETCH_HEAD \
 && python3 /tmp/OpenUSD/build_scripts/build_usd.py -j "${JOBS:-$(nproc)}" \
      --no-python --no-imaging --no-usdview --no-materialx --no-examples \
      --no-tutorials --no-tools --no-docs --no-tests /opt/USD \
 && echo "${OPENUSD_REF} $(git -C /tmp/OpenUSD rev-parse HEAD)" > /opt/USD/openusd-ref \
 && rm -rf /opt/USD/src /opt/USD/build /tmp/OpenUSD

LABEL org.opencontainers.image.source="https://github.com/farhatnadim/openusd-tutorial" \
      org.opencontainers.image.description="CI image of openusd-tutorial: OpenUSD in /opt/USD and clang-repl 23"
