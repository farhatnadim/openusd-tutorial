#!/usr/bin/env python3
"""Install the repository's build hook into the current user's Neovim config."""
from datetime import datetime
import os
from pathlib import Path
import shutil

bundle = Path(__file__).resolve().parent
config = Path(os.environ.get("XDG_CONFIG_HOME", str(Path.home() / ".config"))) / "nvim"
module = config / "lua" / "codex_build.lua"
source = bundle / "lua" / "codex_build.lua"
init = config / "init.lua"
stamp = datetime.now().strftime("%Y%m%d-%H%M%S-%f")
marker = '-- Codex C++ build on save (nvim-codex-build)'

if not source.is_file() or not (bundle / "build.sh").is_file():
    raise SystemExit("Incomplete bundle: Lua module or build.sh missing")
if (config / "init.vim").exists() and not init.exists():
    raise SystemExit("This installer expects init.lua; init.vim configuration was left unchanged")
text = init.read_text() if init.exists() else ""
module.parent.mkdir(parents=True, exist_ok=True)
if module.is_symlink() and module.resolve() == source:
    print("Module already linked:", module)
else:
    if module.exists() or module.is_symlink():
        backup = module.with_name(module.name + ".before-codex-build-" + stamp)
        module.rename(backup)
        print("Previous module backed up:", backup)
    module.symlink_to(source)
    print("Module linked:", module)
if marker not in text:
    if init.exists():
        backup = init.with_name(init.name + ".before-codex-build-" + stamp)
        shutil.copy2(init, backup)
        print("Configuration backed up:", backup)
    init.write_text(text.rstrip() + "\n\n" + marker + "\nrequire('codex_build').setup()\n")
    print("Configuration updated:", init)
else:
    print("Configuration already loads the hook:", init)
print("Restart Neovim or run :luafile " + str(init))
