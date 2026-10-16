#!/usr/bin/env python3
"""The checks that need no OpenUSD, which CI runs on every pull request
(make check-tree):

- repl/usd-repl compiles;
- nvidia_tutorials/CMakeLists.txt lists every lesson that has a CMakeLists.txt,
  every lesson it lists exists with a source, and nvidia_tutorials/README.md
  indexes each;
- the cpp_repl pin in cmake/OpenUSDExample.cmake is a commit GitHub has, and
  README.md links the usd profile's header at the same commit;
- cpp-repl at that commit reads every lesson (repl/usd-repl --emit), and its
  check_tutorial.py reads repl/TUTORIAL.md's markers.

cpp_repl is fetched at the pin into a temporary directory. --cpp-repl DIR uses
a checkout instead, and says which commit it is at.
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LESSONS = ROOT / 'nvidia_tutorials'
LAUNCHER = ROOT / 'repl' / 'usd-repl'
CPP_REPL_URL = 'https://github.com/farhatnadim/cpp_repl.git'


def listed_lessons():
    """The lessons nvidia_tutorials/CMakeLists.txt adds, in its order."""
    text = (LESSONS / 'CMakeLists.txt').read_text()
    return re.findall(r'^\s*add_subdirectory\(\s*([^\s)]+)\s*\)', text, re.M)


def check_tree(problems):
    """The launcher compiles; the lesson list, the lesson directories and the
    README index agree. Returns the listed lessons."""
    try:
        compile(LAUNCHER.read_text(), str(LAUNCHER), 'exec')
    except SyntaxError as e:
        problems.append(f'repl/usd-repl:{e.lineno}: {e.msg}')

    listed = listed_lessons()
    index = (LESSONS / 'README.md').read_text()
    for name in listed:
        lesson = LESSONS / name
        if not (lesson / 'CMakeLists.txt').is_file():
            problems.append(f'nvidia_tutorials/CMakeLists.txt adds {name}, which has no CMakeLists.txt')
        elif not list(lesson.glob('*.cpp')):
            problems.append(f'nvidia_tutorials/{name} has no .cpp source')
        if f']({name})' not in index:
            problems.append(f'nvidia_tutorials/README.md has no entry for {name}')
    for path in sorted(LESSONS.rglob('CMakeLists.txt')):
        name = str(path.parent.relative_to(LESSONS))
        if path.parent != LESSONS and name not in listed:
            problems.append(f'nvidia_tutorials/{name} has a CMakeLists.txt and nvidia_tutorials/CMakeLists.txt does not add it')
    return listed


def pin(problems):
    """The commit cmake/OpenUSDExample.cmake pins cpp_repl at, which README.md
    links too, or None."""
    cmake = (ROOT / 'cmake' / 'OpenUSDExample.cmake').read_text()
    m = re.search(r'GIT_TAG\s+([0-9a-f]{40})\b', cmake)
    if not m:
        problems.append('cmake/OpenUSDExample.cmake pins cpp_repl at no 40-character commit')
        return None
    sha = m.group(1)
    if f'cpp_repl/blob/{sha}/' not in (ROOT / 'README.md').read_text():
        problems.append(f'README.md links cpp_repl at another commit than the pin {sha[:7]}')
    return sha


def fetch(sha, into):
    """cpp_repl at sha, from GitHub, checked out in the directory `into`."""
    for args in (['init', '-q'], ['fetch', '-q', '--depth', '1', CPP_REPL_URL, sha], ['checkout', '-q', 'FETCH_HEAD']):
        subprocess.run(['git', '-C', str(into), *args], check=True, capture_output=True, text=True)


def check_lessons_read(cpp_repl, lessons, problems):
    """Every lesson source passes through cpp-repl's reader: repl/usd-repl --emit,
    which needs no build, no OpenUSD and no clang-repl."""
    env = dict(os.environ, CPP_REPL=str(cpp_repl / 'bin' / 'cpp-repl'))
    count = 0
    for name in lessons:
        for source in sorted((LESSONS / name).glob('*.cpp')):
            run = subprocess.run([str(LAUNCHER), '--emit', str(source)], env=env, capture_output=True, text=True)
            count += 1
            if run.returncode != 0:
                problems.append(f'repl/usd-repl --emit {source.relative_to(ROOT)}: exit {run.returncode}\n'
                                + run.stderr.strip())
    return count


def check_tutorial_markers(cpp_repl, problems):
    """repl/TUTORIAL.md's check markers and transcripts parse, read by the pinned
    cpp_repl's checker (what ctest runs them with)."""
    sys.path[:0] = [str(cpp_repl / 'scripts'), str(cpp_repl / 'src')]
    import check_tutorial
    problems.extend(check_tutorial.read_tutorial(ROOT / 'repl' / 'TUTORIAL.md').errors)


def main():
    parser = argparse.ArgumentParser(description='The checks of this tree that need no OpenUSD.')
    parser.add_argument('--cpp-repl', metavar='DIR', help='a cpp_repl checkout to use instead of fetching the pin')
    args = parser.parse_args()

    problems = []
    lessons = check_tree(problems)
    sha = pin(problems)
    with tempfile.TemporaryDirectory(prefix='cpp_repl-') as tmp:
        cpp_repl = None
        if args.cpp_repl:
            cpp_repl = Path(args.cpp_repl).resolve()
            at = subprocess.run(['git', '-C', str(cpp_repl), 'rev-parse', 'HEAD'],
                                capture_output=True, text=True).stdout.strip()
            print(f'cpp_repl: {cpp_repl} at {at[:7]}' + ('' if at == sha else f'; the pin is {sha[:7] if sha else "missing"}'))
        elif sha:
            try:
                fetch(sha, tmp)
                cpp_repl = Path(tmp)
                print(f'cpp_repl: {sha[:7]}, fetched from GitHub')
            except subprocess.CalledProcessError as e:
                problems.append(f'the pin {sha[:7]} is not on GitHub: {e.stderr.strip()}')
        read = 0
        if cpp_repl:
            read = check_lessons_read(cpp_repl, lessons, problems)
            check_tutorial_markers(cpp_repl, problems)

    if problems:
        print(f'check-tree: {len(problems)} problem(s)', file=sys.stderr)
        for problem in problems:
            print(f'  {problem}', file=sys.stderr)
        return 1
    print(f'check-tree: {len(lessons)} lessons listed, built and indexed; {read} sources read by cpp-repl; '
          'repl/TUTORIAL.md parses')
    return 0


if __name__ == '__main__':
    sys.exit(main())
