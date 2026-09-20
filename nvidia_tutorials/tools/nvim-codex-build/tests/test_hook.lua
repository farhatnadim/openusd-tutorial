-- Integration tests exercise actual BufWritePost, jobstart and quickfix behavior.
local fn = vim.fn
local fixture = assert(vim.env.CODEX_BUILD_TEST_DIR)
local source = assert(vim.env.CODEX_BUILD_TEST_SOURCE)
local root = fixture .. '/tutorials'
local trace = assert(vim.env.CODEX_BUILD_TEST_TRACE)
local notifications, checks = {}, 0
vim.notify = function(message, level)
  table.insert(notifications, { message = tostring(message), level = level })
end
vim.opt.more = false
vim.opt.swapfile = false
vim.opt.hidden = true
package.path = source .. '/../lua/?.lua;' .. package.path

local function check(condition, message)
  checks = checks + 1
  assert(condition, message)
end

local function write(path, lines)
  fn.mkdir(fn.fnamemodify(path, ':h'), 'p')
  fn.writefile(type(lines) == 'table' and lines or { lines }, path)
end

local function project(name)
  local dir = root .. '/' .. name
  write(dir .. '/CMakeLists.txt', {
    'cmake_minimum_required(VERSION 3.16)',
    'project(CodexHookTest LANGUAGES CXX)',
    'add_executable(sample source.cpp)',
  })
  write(dir .. '/source.cpp', 'int main() { return 0; }')
  return dir
end

local function mode(dir, value)
  write(dir .. '/.fake-codex.json', fn.json_encode(value))
end

local function events(kind, dir)
  local records = {}
  if fn.filereadable(trace) == 1 then
    for _, line in ipairs(fn.readfile(trace)) do
      local ok, record = pcall(fn.json_decode, line)
      if ok and (not kind or record.kind == kind) and (not dir or record.project == dir) then
        table.insert(records, record)
      end
    end
  end
  return records
end

local function pause(ms)
  vim.wait(ms, function() return false end, 10)
end

local function await(predicate, message, timeout)
  check(vim.wait(timeout or 6000, predicate, 10), message)
end

local function edit(path)
  vim.cmd('edit ' .. fn.fnameescape(path))
end

local function save(path)
  edit(path)
  vim.cmd('silent write')
end

local function finished(count)
  await(function() return #events('finish') >= count end, 'Expected ' .. count .. ' completed CLI invocations')
  pause(180)
end

local function notice_since(index, pattern)
  for i = index + 1, #notifications do
    if notifications[i].message:lower():find(pattern) then return true end
  end
  return false
end

local function run()
  local a = project('project with spaces')
  local b = project('second-project')
  write(fixture .. '/activate', 'sleep 0.6')
  write(fixture .. '/USD prefix/pxrConfig.cmake', '# Fixture prefix')
  write(fixture .. '/SKILL.md', '# Test build skill')
  local fake = fixture .. '/fake codex $literal'
  write(fake, fn.readfile(source .. '/fake-codex.py'))
  fn.setfperm(fake, 'rwxr-xr-x')
  require('codex_build').setup({
    root = root, codex = fake, skill = fixture .. '/SKILL.md',
    activate = fixture .. '/activate', usd_prefix = fixture .. '/USD prefix',
    debounce_ms = 80,
  })
  for _, command in ipairs({ 'CodexBuild', 'BuildAgent', 'BuildLog', 'CodexBuildToggle' }) do
    check(fn.exists(':' .. command) == 2, 'Missing :' .. command)
  end
  local function reports(dir)
    return fn.glob(dir .. '/build/codex-nvim/.codex-runs/*/report.txt', false, true)
  end
  local function complete(dir, count)
    await(function() return #reports(dir) >= count end, 'Build did not finish', 10000)
    pause(50)
  end
  local function panel()
    for _, window in ipairs(vim.api.nvim_list_wins()) do
      local buffer = vim.api.nvim_win_get_buf(window)
      local lines = vim.api.nvim_buf_get_lines(buffer, 0, -1, false)
      if lines[1] and lines[1]:find('Project: ', 1, true) == 1 then return buffer, lines end
    end
  end
  local function panel_text()
    local _, lines = panel()
    return table.concat(lines or {}, '\n')
  end

  write(a .. '/notes.txt', 'ignore')
  write(a .. '/build/generated.cpp', 'ignore')
  save(a .. '/notes.txt')
  save(a .. '/build/generated.cpp')
  pause(200)
  check(#reports(a) == 0 and #events('start') == 0, 'Unrelated saves must not build')

  edit(a .. '/source.cpp')
  local editor = vim.api.nvim_get_current_win()
  save(a .. '/source.cpp')
  await(function() return panel_text():find('Preparing build', 1, true) ~= nil end,
    'Save must open progress panel')
  check(vim.api.nvim_get_current_win() == editor, 'Progress panel stole focus')
  -- Seed an in-progress log while the fixture environment is still starting.
  local dirs = fn.glob(a .. '/build/codex-nvim/.codex-runs/*', false, true)
  write(dirs[1] .. '/build.log', '[ 50%] test progress')
  await(function() return panel_text():find('[ 50%]', 1, true) ~= nil end,
    'Progress did not refresh', 1000)
  complete(a, 1)
  check(#events('start') == 0, 'Default save must never invoke Codex')
  check(panel_text():find('Status: Build success', 1, true) ~= nil, 'Missing persistent success')
  check(notice_since(0, 'build success'), 'Missing success notification')
  check(fn.executable(a .. '/build/codex-nvim/sample') == 1, 'Direct build did not compile fixture')

  -- Rapid writes coalesce; a save after launch queues one more direct build.
  save(a .. '/source.cpp')
  pause(15)
  save(a .. '/source.cpp')
  complete(a, 2)
  pause(150)
  check(#reports(a) == 2, 'Rapid saves must coalesce')
  save(a .. '/source.cpp')
  await(function()
    return #fn.glob(a .. '/build/codex-nvim/.codex-runs/*', false, true) == 3
  end, 'Queued-save test did not start')
  save(a .. '/source.cpp')
  complete(a, 4)
  check(#events('start') == 0, 'Queued save must remain direct')

  write(a .. '/source.cpp', 'int main() { this is invalid C++; }')
  save(a .. '/source.cpp')
  complete(a, 5)
  check(#fn.getqflist() > 0, 'Direct compiler failure must populate quickfix')
  check(panel_text():find('Status: Build failed', 1, true) ~= nil, 'Missing failure result')
  local _, failure_lines = panel()
  check(failure_lines[4]:find('source.cpp:', 1, true) ~= nil
    and failure_lines[4]:find('error:', 1, true) ~= nil,
    'Direct build must show first compiler error above raw output')
  check(vim.api.nvim_get_current_win() == editor, 'Failure must preserve focus')
  vim.cmd('silent! cclose')
  write(a .. '/source.cpp', 'int main() { return 0; }')
  save(a .. '/source.cpp')
  complete(a, 6)
  check(#fn.getqflist() == 0, 'Successful direct rebuild must clear errors')

  vim.cmd('CodexBuildToggle')
  save(a .. '/source.cpp')
  pause(200)
  check(#reports(a) == 6, 'Toggle must disable automatic builds')
  vim.cmd('CodexBuild')
  complete(a, 7)
  check(#events('start') == 0, 'Manual normal build must not invoke Codex')

  -- Explicit agent command works even while automatic builds are disabled.
  mode(a, { status = 'success', delay = 0.7 })
  vim.cmd('BuildAgent')
  await(function() return #events('start') == 1 end, 'BuildAgent must invoke Codex')
  complete(a, 8)
  local first = events('start')[1]
  local args = table.concat(first.argv, '\n')
  check(first.project == a and args:find('gpt-6-astra', 1, true) ~= nil, 'Wrong agent project/model')
  check(first.prompt:find('$build-cpp', 1, true) ~= nil, 'Agent must use build-cpp')
  check(args:find('--approve-for-me', 1, true) ~= nil, 'Agent must retain approval review')
  check(panel_text():find('Status: Build success', 1, true) ~= nil, 'Agent success missing')

  mode(a, { status = 'build_failed', response_status = 'success',
    log = 'source.cpp:1:5: error: expected expression\n', explanation = 'Investigate the expression.',
    fixes = {
      { file = 'source.cpp', line = 1, column = 5, cause = 'The expression is missing.',
        replacement = 'int main() {\n  return 0;\n}' },
      { file = '../outside.cpp', line = 1, column = 1, cause = 'Ignore outside file.', replacement = 'bad' },
      { file = 'source.cpp', line = 0, column = 1, cause = 'Ignore invalid location.', replacement = 'bad' },
    } })
  vim.cmd('BuildAgent')
  complete(a, 9)
  check(#fn.getqflist() > 0, 'Actual build failure must override agent success')
  local fixes = fn.getqflist()
  check(fixes[1].text:find('Suggested fix (untested):', 1, true) ~= nil,
    'Agent fixes must appear first in quickfix')
  check(fixes[2].text:find('int main()', 1, true) ~= nil
    and fixes[3].text:find('  return 0;', 1, true) ~= nil,
    'Quickfix must contain multiline replacement code')
  check(#fixes == 5, 'Keep compiler error but ignore invalid fix entries')
  vim.cmd('cfirst')
  check(fn.expand('%:p') == a .. '/source.cpp' and fn.line('.') == 1 and fn.col('.') == 5,
    'Fix entry must navigate to the original source location')
  check(fn.readfile(a .. '/source.cpp')[1] == 'int main() { return 0; }',
    'Suggested fixes must not edit source')
  check(panel_text():find('Investigate the expression.', 1, true) ~= nil, 'Agent explanation missing')
  local result_buffer = panel()
  for _, window in ipairs(fn.win_findbuf(result_buffer)) do
    check(vim.api.nvim_win_get_cursor(window)[1] == 1, 'Completed report must reveal the suggestion at the top')
    check(vim.wo[window].wrap and vim.wo[window].linebreak, 'Agent explanations must wrap')
  end
  check(vim.api.nvim_get_current_win() == editor, 'Showing agent suggestion must preserve editor focus')
  vim.cmd('silent! cclose')
  mode(a, { malformed = true })
  vim.cmd('BuildAgent')
  complete(a, 10)
  check(panel_text():find('Status: Agent failed', 1, true) ~= nil, 'Invalid agent result must fail')

  -- A save during an agent run must queue a direct build.
  vim.cmd('CodexBuildToggle')
  mode(a, { status = 'success', delay = 0.7 })
  vim.cmd('BuildAgent')
  await(function() return #events('start') == 4 end, 'Agent did not start')
  save(a .. '/source.cpp')
  complete(a, 12)
  check(#events('start') == 4, 'Save during agent run must queue a direct build')
  check(panel_text():find('Status: Build success', 1, true) ~= nil, 'Queued direct build failed')

  -- Runner startup/environment failures are normal build failures.
  write(fixture .. '/activate', 'return 7')
  save(b .. '/source.cpp')
  complete(b, 1)
  check(panel_text():find('Status: Build failed', 1, true) ~= nil, 'Environment failure not shown')
  check(#events('start') == 4, 'Direct failure must never fall back to agent')
  print(string.format('PASS: %d Neovim integration assertions; direct CMake success/failure, live progress, queued saves, and explicit agent builds.', checks))
end

local ok, err = xpcall(run, debug.traceback)
if not ok then
  io.stderr:write('FAIL: ' .. tostring(err) .. '\n')
  for _, notice in ipairs(notifications) do
    io.stderr:write('notification: ' .. notice.message .. '\n')
  end
  vim.cmd('cquit 1')
else
  vim.cmd('qa!')
end
