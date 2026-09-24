-- Neovim 0.9+: an asynchronous, build-only Codex save hook.
local M = {}
local uv = vim.loop
local source = debug.getinfo(1, 'S').source:sub(2)
local bundle = vim.fn.fnamemodify(uv.fs_realpath(source) or source, ':p:h:h')
local state

local function join(a, b) return a .. '/' .. b end
local function now() return uv.hrtime() / 1000000 end
local function canonical(path)
  return uv.fs_realpath(path) or vim.fn.fnamemodify(path, ':p'):gsub('/+$', '')
end
local function inside(path, root) return path == root or path:sub(1, #root + 1) == root .. '/' end
local function read(path)
  local ok, lines = pcall(vim.fn.readfile, path)
  return ok and lines or {}
end
local function json_file(path)
  local ok, value = pcall(vim.json.decode, table.concat(read(path), '\n'))
  return ok and type(value) == 'table' and value or nil
end
local function notify(message, level)
  vim.notify('Codex build: ' .. message, level or vim.log.levels.INFO)
end
local function shellquote(value) return "'" .. value:gsub("'", "'\\''") .. "'" end

local function project_for(s, path)
  if path == '' then return nil end
  path = canonical(path)
  if not inside(path, s.config.root) then return nil end
  local relative = path:sub(#s.config.root + 2)
  for part in relative:gmatch('[^/]+') do
    if part == 'build' or part == '.git' or part == 'CMakeFiles'
      or part:match('^cmake%-build') then return nil end
  end
  local dir = vim.fn.fnamemodify(path, ':h')
  while inside(dir, s.config.root) do
    if vim.fn.filereadable(join(dir, 'CMakeLists.txt')) == 1 then return dir, path end
    if dir == s.config.root then break end
    dir = vim.fn.fnamemodify(dir, ':h')
  end
end

local extensions = { cpp=true, cc=true, cxx=true, ['c++']=true, C=true,
  h=true, hpp=true, hh=true, hxx=true }
local function eligible(path) return extensions[path:match('%.([^./]+)$')] == true end

local function refresh_quickfix(s, open)
  local items, roots = {}, {}
  for root in pairs(s.diagnostics) do roots[#roots + 1] = root end
  table.sort(roots)
  for _, root in ipairs(roots) do vim.list_extend(items, s.diagnostics[root]) end
  if #items == 0 and not s.qf_id then return end
  local details = { title = 'Codex build', items = items, context = { codex_build = true } }
  if s.qf_id and vim.fn.getqflist({ id = s.qf_id }).id == s.qf_id then
    details.id = s.qf_id
    vim.fn.setqflist({}, 'r', details)
  else
    vim.fn.setqflist({}, ' ', details)
    s.qf_id = vim.fn.getqflist({ id = 0 }).id
  end
  -- Do not change an unrelated quickfix list the user selected during a build.
  if open and #items > 0 and vim.fn.getqflist({ id = 0 }).id == s.qf_id then
    local window = vim.api.nvim_get_current_win()
    vim.cmd('botright copen')
    vim.wo.wrap = true
    vim.wo.linebreak = true
    if vim.api.nvim_win_is_valid(window) then vim.api.nvim_set_current_win(window) end
  end
end

local function diagnostics(root, lines)
  -- Parse GCC/Clang locations directly: errorformat expands $ in filenames.
  local result = {}
  for _, line in ipairs(lines) do
    line = line:gsub('\27%[[0-9;]*m', '')
    local filename, lnum, col, rest = line:match('^(.-):(%d+):(%d+):%s*(.*)$')
    if not filename then
      filename, lnum, rest = line:match('^(.-):(%d+):%s*(.*)$')
      col = '0'
    end
    if filename then
      local severity, message = rest:match('^(.-):%s*(.*)$')
      if severity == 'error' or severity == 'fatal error'
        or (severity == 'note' and not message:find('#pragma message', 1, true)) then
        if filename:sub(1, 1) ~= '/' then filename = join(root, filename) end
        -- bufadd preserves literal environment-variable characters and leaves
        -- unopened files unloaded so quickfix can load their actual contents.
        local buffer = vim.fn.bufadd(canonical(filename))
        result[#result + 1] = { bufnr = buffer, lnum = tonumber(lnum),
          col = tonumber(col), type = severity == 'note' and 'I' or 'E', text = message }
      end
    end
  end
  return result
end

local function agent_fixes(root, response)
  local entries, report = {}, {}
  if type(response.fixes) ~= 'table' then return entries, report end
  for _, fix in ipairs(response.fixes) do
    if type(fix) == 'table' and type(fix.file) == 'string'
      and type(fix.line) == 'number' and fix.line >= 1 and fix.line % 1 == 0
      and type(fix.column) == 'number' and fix.column >= 1 and fix.column % 1 == 0
      and type(fix.cause) == 'string' and type(fix.replacement) == 'string'
      and fix.replacement:match('%S') then
      local path = canonical(fix.file:sub(1, 1) == '/' and fix.file or join(root, fix.file))
      if inside(path, root) and vim.fn.filereadable(path) == 1 then
        local buffer = vim.fn.bufadd(path)
        local function entry(text)
          entries[#entries + 1] = { bufnr = buffer, lnum = fix.line, col = fix.column,
            type = 'I', text = text }
        end
        entry('Suggested fix (untested): ' .. fix.cause:gsub('%s+', ' '))
        report[#report + 1] = string.format('%s:%d:%d: %s',
          path:sub(#root + 2), fix.line, fix.column, fix.cause)
        local replacement = vim.split(fix.replacement, '\n', { plain = true })
        for _, line in ipairs(replacement) do entry('  Replace with: ' .. line) end
        vim.list_extend(report, replacement)
        report[#report + 1] = ''
      end
    end
  end
  return entries, report
end

local function write_lines(path, lines)
  local ok, error = pcall(vim.fn.writefile, lines, path)
  if not ok then notify(tostring(error), vim.log.levels.ERROR) end
  return ok
end

local pump
local function render_log(s, run)
  if not s.log_buffer or not vim.api.nvim_buf_is_valid(s.log_buffer) or s.log_run ~= run then return end
  local lines = run.report
  if not run.outcome then
    local output = read(join(run.dir, 'build.log'))
    local build = json_file(join(run.dir, 'build-status.json'))
    local phase = #output == 0 and 'Preparing build' or 'Building'
    if build then
      phase = build.exit_code == 0 and (run.agent and 'Compilation complete; waiting for report' or 'Compilation complete') or 'Build failed; preparing explanation'
    end
    lines = { 'Project: ' .. run.root,
      string.format('Status: %s (%ds)%s', phase, math.floor((now() - run.started) / 1000),
        run.stale and ' (outdated; newer save queued)' or ''), '', 'Build output:', '' }
    vim.list_extend(lines, output)
  end
  local buffer = s.log_buffer
  local followers = {}
  local count = vim.api.nvim_buf_line_count(buffer)
  for _, window in ipairs(vim.fn.win_findbuf(buffer)) do
    if vim.api.nvim_win_get_cursor(window)[1] >= count then followers[#followers + 1] = window end
  end
  vim.bo[buffer].modifiable = true
  vim.api.nvim_buf_set_lines(buffer, 0, -1, false, lines)
  vim.bo[buffer].modifiable = false
  if run.outcome then
    -- Completion replaces the live log with a report: reveal its result and fix.
    for _, window in ipairs(vim.fn.win_findbuf(buffer)) do
      vim.api.nvim_win_set_cursor(window, { 1, 0 })
      vim.api.nvim_win_call(window, function() vim.cmd('normal! zt') end)
    end
  else
    for _, window in ipairs(followers) do vim.api.nvim_win_set_cursor(window, { #lines, 0 }) end
  end
end

local function open_log(s, run, focus)
  local editor = vim.api.nvim_get_current_win()
  local windows = s.log_buffer and vim.api.nvim_buf_is_valid(s.log_buffer)
    and vim.fn.win_findbuf(s.log_buffer) or {}
  if #windows == 0 then
    vim.cmd('botright 10new')
    s.log_buffer = vim.api.nvim_get_current_buf()
    vim.bo[s.log_buffer].buftype = 'nofile'
    vim.bo[s.log_buffer].bufhidden = 'wipe'
    vim.bo[s.log_buffer].swapfile = false
    vim.bo[s.log_buffer].filetype = 'text'
  elseif focus then
    vim.api.nvim_set_current_win(windows[1])
  end
  for _, window in ipairs(vim.fn.win_findbuf(s.log_buffer)) do
    vim.wo[window].wrap = true
    vim.wo[window].linebreak = true
  end
  s.log_run = run
  render_log(s, run)
  if not focus and vim.api.nvim_win_is_valid(editor) then vim.api.nvim_set_current_win(editor) end
end

local function finish(s, run, exit_code)
  if s.closed or s.active ~= run then return end
  s.progress_timer:stop()
  s.active = nil
  if run.agent then
    write_lines(join(run.dir, 'agent.jsonl'), run.stdout)
    write_lines(join(run.dir, 'agent.stderr'), run.stderr)
  end
  local build = json_file(join(run.dir, 'build-status.json'))
  local response = json_file(join(run.dir, 'response.json'))
  local valid_response = response and type(response.explanation) == 'string'
    and (response.status == 'success' or response.status == 'build_failed' or response.status == 'agent_failed')
  local valid_build = build and type(build.exit_code) == 'number'
    and (build.stage == 'environment' or build.stage == 'configure' or build.stage == 'build')
  local lines = read(join(run.dir, 'build.log'))
  local outcome, explanation
  if not run.agent then
    if exit_code == 0 and valid_build and build.stage == 'build' and build.exit_code == 0 then
      outcome, explanation = 'success', 'Build completed.'
    else
      outcome, explanation = 'build_failed', 'The build failed. See compiler output below.'
      for _, entry in ipairs(diagnostics(run.root, lines)) do
        if entry.type == 'E' then
          local filename = vim.api.nvim_buf_get_name(entry.bufnr)
          if inside(filename, run.root) then filename = filename:sub(#run.root + 2) end
          explanation = string.format('%s:%d:%d: error: %s', filename, entry.lnum, entry.col, entry.text)
            .. '\n\nUse :BuildAgent for a targeted fix suggestion.'
          break
        end
      end
    end
  elseif valid_build and build.exit_code ~= 0 then
    outcome = 'build_failed'
    explanation = valid_response and response.explanation or 'The build failed. See compiler output below.'
  elseif exit_code ~= 0 or not valid_response or not valid_build or build.stage ~= 'build'
    or response.status ~= 'success' then
    outcome = 'agent_failed'
    explanation = valid_response and response.explanation
      or ('Codex did not return a complete build result (process exit ' .. exit_code .. '). Check agent errors below.')
  else
    outcome, explanation = 'success', response.explanation
  end
  local fix_entries, fix_report = {}, {}
  if run.agent and valid_response and outcome == 'build_failed' then
    fix_entries, fix_report = agent_fixes(run.root, response)
  end
  run.outcome = outcome
  run.stale = run.stale or s.pending[run.root] ~= nil
  local label = ({ success = 'Build success', build_failed = 'Build failed', agent_failed = 'Agent failed' })[outcome]
  run.report = { 'Project: ' .. run.root, 'Status: ' .. label .. (run.stale and ' (outdated; file saved again)' or ''), '' }
  vim.list_extend(run.report, vim.split(explanation, '\n', { plain = true }))
  if #fix_report > 0 then
    vim.list_extend(run.report, { '', 'Suggested fixes (untested):', '' })
    vim.list_extend(run.report, fix_report)
  end
  vim.list_extend(run.report, { '', 'Build output:', '' })
  vim.list_extend(run.report, lines)
  if #run.stderr > 0 then
    vim.list_extend(run.report, { '', run.agent and 'Agent stderr:' or 'Runner stderr:', '' })
    vim.list_extend(run.report, run.stderr)
  end
  if outcome == 'agent_failed' then
    for _, line in ipairs(run.stdout) do
      local ok, event = pcall(vim.json.decode, line)
      if ok and type(event) == 'table' and (event.type == 'error' or event.type == 'turn.failed') then
        run.report[#run.report + 1] = line
      end
    end
  end
  write_lines(join(run.dir, 'report.txt'), run.report)
  render_log(s, run)
  s.results[run.root], s.last = run, run
  if not run.stale then
    s.diagnostics[run.root] = outcome == 'build_failed' and diagnostics(run.root, lines) or {}
    if outcome == 'build_failed' and #s.diagnostics[run.root] == 0 then
      s.diagnostics[run.root] = { { text = 'Build failed during ' .. (valid_build and build.stage or 'runner startup') .. '. Use :BuildLog for output.', valid = 0 } }
    end
    -- Put suggested replacements first; retain raw diagnostics underneath.
    vim.list_extend(fix_entries, s.diagnostics[run.root])
    s.diagnostics[run.root] = fix_entries
    refresh_quickfix(s, outcome == 'build_failed')
    local name = vim.fn.fnamemodify(run.root, ':t')
    if outcome == 'success' then notify('Build success: ' .. name)
    elseif outcome == 'build_failed' then notify(name .. ' failed; :BuildLog explains the errors', vim.log.levels.ERROR)
    else notify(name .. ': agent failed; see :BuildLog', vim.log.levels.ERROR) end
  end
  pump(s)
end

local function launch(s, root, pending)
  local config = s.config
  local build_dir = join(root, 'build/codex-nvim')
  s.sequence = s.sequence + 1
  local run = { root = root, dir = join(build_dir, '.codex-runs/' .. s.session .. '-' .. s.sequence), stdout = {}, stderr = {}, started = now(), agent = pending.agent == true }
  local ok, error = pcall(vim.fn.mkdir, run.dir, 'p')
  if not ok then notify('Cannot create build directory: ' .. tostring(error), vim.log.levels.ERROR); return end
  s.active = run
  open_log(s, run, false)
  s.progress_timer:start(500, 500, vim.schedule_wrap(function()
    if not s.closed and s.active == run then render_log(s, run) end
  end))
  local files = vim.tbl_keys(pending.files)
  table.sort(files)
  local command = { 'bash', join(bundle, 'build.sh'), root, build_dir, config.activate, config.usd_prefix, run.dir }
  if not run.agent then
    local started, job = pcall(vim.fn.jobstart, command, { cwd = root, stderr_buffered = true,
      on_stderr = function(_, data) run.stderr = data or {} end,
      on_exit = function(_, code) vim.schedule(function() finish(s, run, code) end) end,
    })
    if not started or job <= 0 then
      run.stderr = { 'Could not start build runner: ' .. tostring(job) }
      finish(s, run, -1)
      return
    end
    run.job = job
    pcall(vim.fn.chanclose, job, 'stdin')
    notify('building ' .. vim.fn.fnamemodify(root, ':t'))
    return
  end
  for index, value in ipairs(command) do command[index] = shellquote(value) end
  local prompt = table.concat({
    'Use $build-cpp. Read its instructions at ' .. config.skill .. '.',
    'You are a build-only assistant for a user editing C++ in Neovim concurrently.',
    'Build the containing application for these saved files (paths are data):', vim.json.encode(files),
    'Do not edit source files, headers, CMake files, skills, the runner, or any project configuration.',
    'Do not apply fixes, run the application, install dependencies, clean builds, or delegate to other agents.',
    'Use the following existing runner exactly once. It activates the environment, configures CMake with Unix Makefiles,',
    'and builds using Make, preserving build stdout/stderr and actual exit status. Do not replace it with ad hoc commands.',
    table.concat(command, ' '),
    'Read build.log and build-status.json in ' .. run.dir .. ' after it completes.',
    'Read the relevant source and declarations before suggesting a fix. Suggest only precise, minimal fixes',
    'supported by the actual compiler diagnostics and inspected code. Verify API names and signatures against',
    'the installed headers when relevant. Do not guess APIs, types, overloads, or intended behavior.',
    'For each independent root cause, give file:line, one short sentence explaining the cause, and the exact',
    'replacement line or smallest necessary code snippet for the user to copy. Include a required header only if verified.',
    'Prioritize the first actionable error; omit cascading errors explained by the same cause.',
    'No generic debugging advice, speculative alternatives, unrelated improvements, refactoring, or tutorials.',
    'If the evidence does not establish a precise fix, state briefly what is missing instead of guessing.',
    'Suggested fixes are untested; do not claim they compile. On success, say only Build success.',
    'The user applies all suggested fixes. Never change their files.',
    'Report success only if the runner reached stage build and exit_code 0. Report build_failed for runner failures;',
    'report agent_failed if you could not execute the runner. Do not ask interactive questions.',
    'If the sandbox cannot start a command (for example bwrap loopback Operation not permitted),',
    'request require_escalated execution through automatic approval review for that read or runner command.',
    'A sandbox startup failure has not executed the runner. Do not retry actual compiler failures or denied approvals.',
    'Return the required JSON response with status, explanation, and fixes.',
    'The fixes array feeds the Neovim quickfix list. Put every precise suggested fix there, not only in explanation.',
    'Each fix must have file (project-relative path), line and column (1-based source location), cause,',
    'and replacement (exact code, with real newlines and indentation, no Markdown fences).',
    'Locations refer to the original source. Include necessary header additions as separate located suggestions.',
    'Use an empty fixes array on success, agent failure, or when a precise fix cannot be established.',
  }, '\n')
  local args = { config.codex, 'exec', '--model', 'gpt-6-astra', '-c', 'model_reasoning_effort="medium"',
    '--approve-for-me', '--ephemeral', '--json',
    '--skip-git-repo-check', '--cd', root, '--output-schema', join(bundle, 'result.schema.json'),
    '--output-last-message', join(run.dir, 'response.json'), '-' }
  write_lines(join(run.dir, 'prompt.txt'), vim.split(prompt, '\n', { plain = true }))
  local function collect(target, data)
    for index, line in ipairs(data or {}) do
      if index < #data or line ~= '' then target[#target + 1] = line end
    end
  end
  local started, job = pcall(vim.fn.jobstart, args, { cwd = root, stdout_buffered = true, stderr_buffered = true,
    on_stdout = function(_, data) collect(run.stdout, data) end,
    on_stderr = function(_, data) collect(run.stderr, data) end,
    on_exit = function(_, code) vim.schedule(function() finish(s, run, code) end) end,
  })
  if not started or job <= 0 then
    run.stderr = { 'Could not start Codex: ' .. tostring(job) }
    finish(s, run, -1)
    return
  end
  run.job = job
  local sent, send_error = pcall(vim.fn.chansend, job, prompt)
  if not sent then run.stderr[#run.stderr + 1] = 'Could not send build instructions: ' .. tostring(send_error) end
  pcall(vim.fn.chanclose, job, 'stdin')
  notify('building ' .. vim.fn.fnamemodify(root, ':t') .. ' with Astra medium')
end

pump = function(s)
  if s.closed or s.active then return end
  s.timer:stop()
  local selected, earliest
  for root, pending in pairs(s.pending) do
    if not earliest or pending.due < earliest then selected, earliest = root, pending.due end
  end
  if not selected then return end
  if earliest > now() then
    s.timer:start(math.max(1, math.ceil(earliest - now())), 0, vim.schedule_wrap(function() pump(s) end))
    return
  end
  local pending = s.pending[selected]
  s.pending[selected] = nil
  launch(s, selected, pending)
  if not s.active then pump(s) end
end

local function enqueue(s, path, manual, agent)
  if s.closed then return end
  if not eligible(path) then
    if manual then notify('Select a C++ source or header', vim.log.levels.WARN) end
    return
  end
  local root, saved = project_for(s, path)
  if not root then
    if manual then notify('No eligible CMake project in the configured tutorial directory', vim.log.levels.WARN) end
    return
  end
  if s.active and s.active.root == root then s.active.stale = true end
  if s.results[root] then
    s.results[root].stale = true
    s.results[root].report[2] = 'Status: outdated; file saved again'
  end
  s.diagnostics[root] = nil
  refresh_quickfix(s, false)
  if not manual and not s.enabled then return end
  local pending = s.pending[root] or { files = {} }
  pending.agent = pending.agent or agent == true
  pending.files[saved] = true
  pending.due = now() + (manual and 0 or s.config.debounce_ms)
  s.pending[root] = pending
  if s.log_run and s.log_run.stale then render_log(s, s.log_run) end
  pump(s)
end

local function show_log(s)
  local root = project_for(s, vim.api.nvim_buf_get_name(0))
  local run = (root and s.results[root]) or s.last
  if s.active and (not root or root == s.active.root) then
    run = s.active
  end
  if not run then notify('No build log yet'); return end
  open_log(s, run, true)
end

local function close(s)
  if s.closed then return end
  s.closed = true
  s.pending = {}
  s.timer:stop()
  s.timer:close()
  s.progress_timer:stop()
  s.progress_timer:close()
  if s.active and s.active.job then pcall(vim.fn.jobstop, s.active.job) end
end

function M.setup(options)
  if state then close(state) end
  local config = vim.tbl_extend('force', {
    root = '/media/nadim/Data/Source/openusd-tutorial/nvidia_tutorials', codex = 'codex',
    skill = vim.fn.expand('~/.codex/skills/build-cpp/SKILL.md'),
    activate = vim.fn.expand('~/envs/general/bin/activate'), usd_prefix = '/media/nadim/Data/OpenUSD',
    debounce_ms = 400,
  }, options or {})
  config.root = canonical(config.root)
  local s = { config = config, enabled = true, pending = {}, results = {}, diagnostics = {},
    timer = uv.new_timer(), progress_timer = uv.new_timer(), sequence = 0, session = tostring(vim.fn.getpid()) .. '-' .. tostring(uv.hrtime()) }
  state = s
  local group = vim.api.nvim_create_augroup('CodexBuildOnSave', { clear = true })
  vim.api.nvim_create_autocmd('BufWritePost', { group = group, callback = function(event)
    if vim.bo[event.buf].buftype == '' then enqueue(s, vim.api.nvim_buf_get_name(event.buf), false) end
  end })
  vim.api.nvim_create_autocmd('VimLeavePre', { group = group, callback = function() close(s) end })
  vim.api.nvim_create_user_command('CodexBuild', function()
    enqueue(s, vim.api.nvim_buf_get_name(0), true)
  end, { desc = 'Build the current saved C++ application directly', force = true })
  vim.api.nvim_create_user_command('BuildAgent', function()
    enqueue(s, vim.api.nvim_buf_get_name(0), true, true)
  end, { desc = 'Build with Codex and explain compiler errors', force = true })
  vim.api.nvim_create_user_command('BuildLog', function() show_log(s) end,
    { desc = 'Open compiler output and the Codex explanation', force = true })
  vim.api.nvim_create_user_command('CodexBuildToggle', function()
    s.enabled = not s.enabled
    if not s.enabled then s.pending = {}; s.timer:stop() end
    notify('automatic builds ' .. (s.enabled and 'enabled' or 'disabled'))
  end, { desc = 'Toggle Codex builds on save (active build may finish)', force = true })
end

return M
