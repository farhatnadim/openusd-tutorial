-- Project-local nvim config: on save, build the example you're editing and run it.
--
-- Loaded automatically when nvim starts in this directory, provided your
-- global config sets `vim.o.exrc = true` (see README).
--
-- Commands:
--   :Build             build the current example now
--   :Run               build and run the current example now
--   :BuildAll          build every example
--   :Target foo        pin to target `foo` (no argument = back to automatic)
--   :BuildOnSave       toggle build-on-save
--   :RunOnSave         toggle run-after-build
--   :RunArgs a b c     set arguments passed to the example (no args = clear)

local uv = vim.uv or vim.loop  -- vim.uv arrived in Neovim 0.10
local preset = "default"
local root = vim.fn.fnamemodify(vim.fn.resolve(debug.getinfo(1, "S").source:sub(2)), ":h")
local build_dir = root .. "/build"

local state = {
  build_on_save = true,
  run_on_save = true,
  running = false,
  pending = nil,  -- target queued while a build is in flight
  pinned = nil,   -- set by :Target, overrides detection
  args = {},
  last = nil,     -- last target we built, for :Run with no current file
  usdcat = true,  -- dump stages the example wrote into the output split
  view_job = nil, -- usdview we spawned, so :View replaces rather than stacks
}

local usdcat_max_lines = 200

-- Read USD_ROOT back out of the configured cache rather than hardcoding it.
local function usd_root()
  local cache = build_dir .. "/CMakeCache.txt"
  for line in io.lines(cache) do
    local v = line:match("^USD_ROOT:PATH=(.+)$")
    if v then return v end
  end
  return nil
end

-- usdview is a Python app; it needs the bindings on PYTHONPATH.
local function usd_env()
  local root_dir = usd_root()
  if not root_dir then return nil end
  local sp = vim.fn.glob(root_dir .. "/lib/python*/site-packages", false, true)[1]
  if not sp then return nil end
  return root_dir, { PYTHONPATH = sp }
end

local stage_patterns = { "*.usda", "*.usdc", "*.usd", "*.usdz" }

-- Stage files anywhere under `dir` (examples write into _assets/ there).
local function stage_files(dir)
  local files = {}
  for _, pat in ipairs(stage_patterns) do
    vim.list_extend(files, vim.fn.glob(dir .. "/**/" .. pat, false, true))
  end
  return files
end

-- Stage files under `dir` touched at or after `since` (a unix timestamp).
local function stages_written(dir, since)
  local found = {}
  do
    for _, f in ipairs(stage_files(dir)) do
      if vim.fn.getftime(f) >= since then
        table.insert(found, f)
      end
    end
  end
  table.sort(found)
  return found
end

local errorformat = table.concat({
  "%f:%l:%c: %trror: %m",
  "%f:%l:%c: %tarning: %m",
  "%f:%l:%c: %tote: %m",
  "%f:%l: %trror: %m",
  "%f:%l: %tarning: %m",
  "%-G%*[0-9]%%%.%#",
  "%-Gmake%.%#",
  "%-Gninja:%.%#",
  "%-G%.%#",
}, ",")

local function notify(msg, level)
  vim.notify(msg, level, { title = "usd" })
end

-- Map a source file to the target it belongs to.
--
-- Lessons under nvidia_tutorials/ (and anything else with its own
-- CMakeLists.txt) register their target explicitly, so walk up from the file
-- to the nearest CMakeLists.txt and read the name out of add_usd_example().
-- The top-level CMakeLists.txt globs examples/ instead, mirroring:
--   examples/foo.cpp      -> foo
--   examples/foo/bar.cpp  -> foo
local function target_for(file)
  if state.pinned then
    return state.pinned
  end
  if not file or file == "" then
    return nil
  end
  local abs = vim.fn.fnamemodify(vim.fn.resolve(file), ":p")
  if not vim.startswith(abs, root .. "/") then
    return nil
  end

  local dir = vim.fn.fnamemodify(abs, ":h")
  while dir ~= root and vim.startswith(dir, root .. "/") do
    local lists = dir .. "/CMakeLists.txt"
    if vim.fn.filereadable(lists) == 1 then
      for _, line in ipairs(vim.fn.readfile(lists)) do
        local name = line:match("^%s*add_usd_example%s*%(%s*([%w_%-]+)")
        if name then
          return name
        end
      end
    end
    dir = vim.fn.fnamemodify(dir, ":h")
  end

  local rel = abs:sub(#root + 2)
  return rel:match("^examples/([^/]+)/") or rel:match("^examples/([^/]+)%.%w+$")
end

-- A reusable scratch split for program output.
local out_buf, out_win

local function output_window()
  if not (out_buf and vim.api.nvim_buf_is_valid(out_buf)) then
    out_buf = vim.api.nvim_create_buf(false, true)
    vim.bo[out_buf].bufhidden = "hide"
    vim.bo[out_buf].swapfile = false
    vim.api.nvim_buf_set_name(out_buf, "usd://output")
  end
  if not (out_win and vim.api.nvim_win_is_valid(out_win)) then
    local cur = vim.api.nvim_get_current_win()
    vim.cmd("botright 15split")
    out_win = vim.api.nvim_get_current_win()
    vim.api.nvim_win_set_buf(out_win, out_buf)
    vim.wo[out_win].number = false
    vim.wo[out_win].relativenumber = false
    vim.wo[out_win].wrap = false
    vim.api.nvim_set_current_win(cur)  -- never steal focus from the source
  end
  return out_buf, out_win
end

local function write_output(lines)
  local buf, win = output_window()
  vim.bo[buf].modifiable = true
  vim.api.nvim_buf_set_lines(buf, 0, -1, false, lines)
  vim.bo[buf].modifiable = false
  if vim.api.nvim_win_is_valid(win) then
    vim.api.nvim_win_set_cursor(win, { math.max(#lines, 1), 0 })
  end
end

local function run(target)
  local exe = build_dir .. "/examples/" .. target
  if vim.fn.executable(exe) ~= 1 then
    notify("no executable for target " .. target, vim.log.levels.WARN)
    return
  end

  -- Each example runs in its own scratch dir, matching the ctest setup, so
  -- files one example writes can't clobber another's.
  local cwd = build_dir .. "/output/" .. target
  vim.fn.mkdir(cwd, "p")

  local lines = { "$ ./examples/" .. target .. " " .. table.concat(state.args, " "), "" }
  local function collect(_, data)
    for _, line in ipairs(data or {}) do
      table.insert(lines, line)
    end
  end

  local root_dir, env = usd_env()
  local cat_env = env
  if root_dir then
    cat_env = vim.tbl_extend("force", env or {},
      { PATH = root_dir .. "/bin:" .. (vim.env.PATH or "") })
  end

  local started = uv.hrtime()
  local wall_start = os.time()
  vim.fn.jobstart(vim.list_extend({ exe }, state.args), {
    cwd = cwd,
    stdout_buffered = true,
    stderr_buffered = true,
    on_stdout = collect,
    on_stderr = collect,
    on_exit = function(_, code)
      local ms = (uv.hrtime() - started) / 1e6
      table.insert(lines, "")
      table.insert(lines, ("[exit %d in %.0f ms]"):format(code, ms))

      if code ~= 0 then
        write_output(lines)
        notify(("%s exited %d"):format(target, code), vim.log.levels.ERROR)
        return
      end

      notify(("%s ran ok (%.0f ms)"):format(target, ms))

      -- Show whatever stages the example just wrote, resolved through usdcat
      -- so binary .usdc shows up as readable text too.
      local written = state.usdcat and stages_written(cwd, wall_start) or {}
      if #written == 0 then
        write_output(lines)
        return
      end

      local remaining = #written
      for _, file in ipairs(written) do
        local name = file:sub(#cwd + 2)
        local out = {}
        vim.fn.jobstart({ "usdcat", file }, {
          cwd = cwd,
          env = cat_env,
          stdout_buffered = true,
          stderr_buffered = true,
          on_stdout = function(_, d)
            for _, l in ipairs(d or {}) do table.insert(out, l) end
          end,
          on_stderr = function(_, d)
            for _, l in ipairs(d or {}) do
              if l ~= "" then table.insert(out, l) end
            end
          end,
          on_exit = function()
            table.insert(lines, "")
            table.insert(lines, ("--- usdcat %s ---"):format(name))
            for i, l in ipairs(out) do
              if i > usdcat_max_lines then
                table.insert(lines,
                  ("... %d more lines (usdcat %s)"):format(#out - usdcat_max_lines, name))
                break
              end
              table.insert(lines, l)
            end
            remaining = remaining - 1
            if remaining == 0 then
              write_output(lines)
            end
          end,
        })
      end
    end,
  })
end

local function build(target, then_run)
  if state.running then
    state.pending = { target = target, run = then_run }
    return
  end
  state.running = true

  local cmd = { "cmake", "--build", "--preset", preset }
  if target then
    vim.list_extend(cmd, { "--target", target })
    state.last = target
  end

  local output = {}
  local function collect(_, data)
    for _, line in ipairs(data or {}) do
      if line ~= "" then
        table.insert(output, line)
      end
    end
  end

  vim.fn.jobstart(cmd, {
    cwd = root,
    stdout_buffered = true,
    stderr_buffered = true,
    on_stdout = collect,
    on_stderr = collect,
    on_exit = function(_, code)
      state.running = false

      vim.fn.setqflist({}, " ", { title = "usd build", lines = output, efm = errorformat })
      local items = vim.fn.getqflist()

      if code == 0 then
        vim.cmd("cclose")
        if #items > 0 then
          notify(("built with %d warning(s)"):format(#items), vim.log.levels.WARN)
        end
        if then_run and target then
          run(target)
        end
      else
        vim.cmd("copen")
        vim.cmd("wincmd p")
        notify(("build failed (%d issue(s))"):format(#items), vim.log.levels.ERROR)
      end

      local queued = state.pending
      state.pending = nil
      if queued then
        vim.defer_fn(function() build(queued.target, queued.run) end, 0)
      end
    end,
  })
end

local function current_target()
  return target_for(vim.api.nvim_buf_get_name(0)) or state.last
end

local group = vim.api.nvim_create_augroup("UsdBuildOnSave", { clear = true })

vim.api.nvim_create_autocmd("BufWritePost", {
  group = group,
  pattern = { "*.cpp", "*.cc", "*.cxx", "*.h", "*.hpp", "CMakeLists.txt", "CMakePresets.json" },
  callback = function(args)
    if not state.build_on_save then
      return
    end
    local file = vim.fn.fnamemodify(args.file, ":p")
    if not vim.startswith(file, root .. "/") then
      return
    end

    local name = vim.fn.fnamemodify(file, ":t")
    if name == "CMakeLists.txt" or name == "CMakePresets.json" then
      build(nil, false)  -- build everything, run nothing
      return
    end

    local target = target_for(file)
    if not target then
      return
    end
    build(target, state.run_on_save)
  end,
})

vim.api.nvim_create_user_command("Build", function()
  build(current_target(), false)
end, { desc = "Build the current example" })

vim.api.nvim_create_user_command("Run", function()
  build(current_target(), true)
end, { desc = "Build and run the current example" })

vim.api.nvim_create_user_command("BuildAll", function()
  build(nil, false)
end, { desc = "Build every example" })

vim.api.nvim_create_user_command("Target", function(opts)
  state.pinned = (opts.args ~= "") and opts.args or nil
  notify("target: " .. (state.pinned or "automatic"))
end, { nargs = "?", desc = "Pin builds to one target" })

vim.api.nvim_create_user_command("RunArgs", function(opts)
  state.args = opts.fargs
  notify("args: " .. (#opts.fargs > 0 and table.concat(opts.fargs, " ") or "(none)"))
end, { nargs = "*", desc = "Arguments passed to the example" })

vim.api.nvim_create_user_command("BuildOnSave", function()
  state.build_on_save = not state.build_on_save
  notify("build on save " .. (state.build_on_save and "on" or "off"))
end, { desc = "Toggle build on save" })

vim.api.nvim_create_user_command("RunOnSave", function()
  state.run_on_save = not state.run_on_save
  notify("run on save " .. (state.run_on_save and "on" or "off"))
end, { desc = "Toggle run after build" })

-- Pick the stage to look at: newest file the current target wrote.
local function newest_stage(target)
  local dir = build_dir .. "/output/" .. target
  local best, best_time
  do
    for _, f in ipairs(stage_files(dir)) do
      local t = vim.fn.getftime(f)
      if not best_time or t > best_time then
        best, best_time = f, t
      end
    end
  end
  return best
end

local function view(file)
  local root_dir, env = usd_env()
  if not root_dir then
    notify("could not locate the USD install or its python bindings",
      vim.log.levels.ERROR)
    return
  end

  local target = current_target()
  file = file or (target and newest_stage(target))
  if not file then
    notify("no stage file to view; run the example first", vim.log.levels.WARN)
    return
  end

  -- Replace the previous viewer rather than piling up windows.
  if state.view_job then
    pcall(vim.fn.jobstop, state.view_job)
    state.view_job = nil
  end

  state.view_job = vim.fn.jobstart({ root_dir .. "/bin/usdview", file }, {
    env = env,
    on_exit = function(id)
      if state.view_job == id then
        state.view_job = nil
      end
    end,
  })
  notify("usdview: " .. vim.fn.fnamemodify(file, ":t"))
end

vim.api.nvim_create_user_command("View", function(opts)
  view(opts.args ~= "" and opts.args or nil)
end, { nargs = "?", complete = "file", desc = "Open the current example's stage in usdview" })

vim.api.nvim_create_user_command("ViewClose", function()
  if state.view_job then
    pcall(vim.fn.jobstop, state.view_job)
    state.view_job = nil
    notify("usdview closed")
  end
end, { desc = "Close the usdview window we opened" })

vim.api.nvim_create_user_command("UsdCat", function()
  state.usdcat = not state.usdcat
  notify("usdcat after run " .. (state.usdcat and "on" or "off"))
end, { desc = "Toggle dumping written stages through usdcat" })

vim.api.nvim_create_user_command("Check", function()
  local root_dir, env = usd_env()
  local target = current_target()
  local file = target and newest_stage(target)
  if not (root_dir and file) then
    notify("no stage file to check; run the example first", vim.log.levels.WARN)
    return
  end
  local out = {}
  vim.fn.jobstart({ root_dir .. "/bin/usdchecker", file }, {
    env = env,
    stdout_buffered = true,
    stderr_buffered = true,
    on_stdout = function(_, d) for _, l in ipairs(d or {}) do table.insert(out, l) end end,
    on_stderr = function(_, d) for _, l in ipairs(d or {}) do table.insert(out, l) end end,
    on_exit = function(_, code)
      write_output(vim.list_extend(
        { "$ usdchecker " .. vim.fn.fnamemodify(file, ":t"), "" }, out))
      notify(code == 0 and "usdchecker: valid"
        or "usdchecker: problems found",
        code == 0 and vim.log.levels.INFO or vim.log.levels.ERROR)
    end,
  })
end, { desc = "Run usdchecker on the current example's stage" })
