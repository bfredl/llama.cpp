l = require'llama'

l.load_model 'models/7B/ggml-model-q4_0.bin'

prompt = 'kalle anka vill ha'
prompt = "l = require'llama'\n"

l.tokenize(prompt)

l.load_prompt(prompt)
coroutine.yield(prompt)
for i = 1,1000 do
  utput = l.sample(1)
  if utput == nil then
    break
  end
  coroutine.yield(table.concat(utput))
end
print()

