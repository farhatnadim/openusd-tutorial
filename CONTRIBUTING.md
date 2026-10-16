# Contributing

Changes land through GitHub, the same way for everyone, the author included.

1. **An issue first.** Open one from the forms (bug, feature or task); it goes
   onto the project board. The title is one sentence that states the fact.
2. **A branch per issue.** With push access, `gh issue develop N --checkout`
   makes it, named `N-slug` and linked to the issue; from a fork, any name.
3. **`make check` before the pull request.** Every lesson is built and run
   against your OpenUSD install, compiled and in the REPL. CI has no OpenUSD:
   it runs `make check-tree`, the part that needs none, on the pull request.
4. **A pull request that closes the issue.** `Closes #N` in its description,
   or CI fails it; `make pr` does steps 3 and 4 for a branch named `N-slug`.
   Merges are rebases: keep commits that each say what they changed, as
   `<area>: <what is now true>`, like the history around them.

[AGENTS.md](AGENTS.md) has the build, the REPL and the rule in full.
