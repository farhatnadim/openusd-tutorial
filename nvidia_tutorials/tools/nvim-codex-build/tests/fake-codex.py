#!/usr/bin/env python3
"""Local CLI double for exercising the real Neovim job and result boundary."""

import json
import os
from pathlib import Path
import subprocess
import sys
import time


def option(*names):
    for name in names:
        if name in sys.argv:
            return sys.argv[sys.argv.index(name) + 1]
    raise ValueError("missing CLI option: " + "/".join(names))


project = Path(option("--cd", "-C"))
response = Path(option("--output-last-message", "-o"))
run_dir = response.parent
run_dir.mkdir(parents=True, exist_ok=True)
mode_file = project / ".fake-codex.json"
mode = json.loads(mode_file.read_text()) if mode_file.exists() else {}
prompt = sys.stdin.read()
trace = Path(os.environ["CODEX_BUILD_TEST_TRACE"])


def event(kind, **fields):
    with trace.open("a") as stream:
        stream.write(json.dumps(dict(kind=kind, project=str(project),
                                     run_dir=str(run_dir), time=time.time(),
                                     **fields)) + "\n")


event("start", argv=sys.argv[1:], prompt=prompt)
time.sleep(mode.get("delay", 0))
status = mode.get("status", "success")
exit_code = 0 if status == "success" else 2
stage = "build"
log = mode.get("log", "[100%] Built target sample\n")
if mode.get("real_build"):
    build_dir = project / "build" / "test-compiler"
    configured = subprocess.run(
        ["cmake", "-S", str(project), "-B", str(build_dir), "-G", "Unix Makefiles"],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    log = configured.stdout
    exit_code = configured.returncode
    stage = "configure"
    if exit_code == 0:
        built = subprocess.run(["cmake", "--build", str(build_dir), "--parallel", "2"],
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        log += built.stdout
        exit_code = built.returncode
        stage = "build"
    status = "success" if exit_code == 0 else "build_failed"

(run_dir / "build.log").write_text(log)
build_status = dict(exit_code=exit_code, stage=stage)
(run_dir / "build-status.json").write_text(json.dumps(build_status))
explanation = mode.get("explanation", "Build completed." if exit_code == 0 else "Fix the compiler error at the reported source location.")
if mode.get("malformed"):
    response.write_text("this is not JSON")
else:
    response.write_text(json.dumps(dict(status=mode.get("response_status", status), explanation=explanation, fixes=mode.get("fixes", []))))
print(json.dumps(dict(type="thread.started", thread_id="local-test")), flush=True)
print(json.dumps(dict(type="item.completed", item=dict(type="agent_message", text=explanation))), flush=True)
print(json.dumps(dict(type="turn.completed", usage=dict(input_tokens=1, output_tokens=1))), flush=True)
event("finish", status=status)
sys.exit(mode.get("cli_exit", 0))
