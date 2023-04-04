if not l then
l = require'llama'
-- l.load_model 'models/7B/ggml-model-q4_0.bin'
l.load_model ('models/13B/ggml-model-q4_0.bin', 12)
-- l.load_model ('models/30B/ggml-model-q4_0.bin', 12)
end

prompt = 'kalle anka vill ha'
prompt = "l = require'llama'\n"

l.tokenize(prompt)

function runn(prompt)
  coroutine.yield('\n')
  l.load_prompt(prompt, true)
  coroutine.yield(prompt)
  for i = 1,1000 do
    utput = l.sample(1)
    if utput == nil then
      break
    end
    coroutine.yield(table.concat(utput))
  end
end

function runn_buf()
  coroutine.yield('\n============\n')
  local text = table.concat(vim.api.nvim_buf_get_lines(0, 0, -1, false),'\n')
  if #text > 0 then
    runn(text)
  end
end
vim.keymap.set('n', '<cr>', function() require'luadev'.coro_run(runn_buf) end)

if false then
runn [[consider the following:

A large text model would never]]
end
