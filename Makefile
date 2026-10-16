# openusd-tutorial: the checks a pull request needs. `make help` lists the targets.
# The build itself is CMake (README, "Build"); these targets run it.

.PHONY: help check check-tree pr need-issue-branch issue

help:  ## this list
	@grep -E '^[a-z-]+:.*## ' $(MAKEFILE_LIST) | sed -E 's/:.*## /\t/' | expand -t 14

check: check-tree  ## before a pull request: the tree checks, then every lesson built and run, compiled and in the REPL
	@test -f build/CMakeCache.txt || cmake --preset default
	cmake --build --preset default
	@echo "cpp-repl under test: $$(sed -n 's/^CPP_REPL:INTERNAL=//p' build/CMakeCache.txt)$$(grep -q '^FETCHCONTENT_SOURCE_DIR_CPP_REPL:PATH=.' build/CMakeCache.txt && echo ' (a checkout, not the pinned commit)')"
	ctest --preset default

check-tree:  ## what CI runs, without OpenUSD: the launcher compiles, the lesson index is consistent, the cpp_repl pin is on GitHub and reads every lesson
	python3 scripts/check_tree.py

pr: need-issue-branch check  ## make check, push this branch (N-slug, from gh issue develop N --checkout) and open its pull request, which closes issue N
	@n=$$(git rev-parse --abbrev-ref HEAD | sed -n 's/^\([0-9][0-9]*\)-.*/\1/p'); \
	git push -u origin HEAD && \
	gh pr create --assignee @me --title "$$(gh issue view $$n --json title --jq .title)" \
	  --body "$$(printf 'Closes #%s\n\n## What changed\n\n\n\n## Checks\n\n- [x] `make check` passed locally: every lesson built and run, compiled and in the REPL\n- [ ] the lesson index and repl/TUTORIAL.md updated where a lesson changed\n' "$$n")"

# The pull request closes the issue the branch is named after, so the name comes first.
need-issue-branch:
	@git rev-parse --abbrev-ref HEAD | grep -q '^[0-9][0-9]*-' || \
	  { echo "make pr: the branch is not named N-slug; start from: gh issue develop N --checkout" >&2; exit 2; }

# gh 2.45's `gh issue create --project` fails (it still asks for classic projects), hence item-add.
issue:  ## file an issue on the board: make issue KIND=bug|feature|task TITLE="one sentence" [BODY="..."]
	@label=$$(case "$(KIND)" in bug) echo bug;; feature) echo enhancement;; task) echo task;; esac); \
	test -n "$$label" -a -n "$(TITLE)" || { echo 'make issue KIND=bug|feature|task TITLE="one sentence that states the fact" [BODY="..."]' >&2; exit 2; }; \
	url=$$(gh issue create --label "$$label" --title "$(TITLE)" --body "$(BODY)") && echo "$$url" && \
	gh project item-add 14 --owner farhatnadim --url "$$url" > /dev/null
