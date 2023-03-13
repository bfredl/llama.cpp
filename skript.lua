l = require'llama'

l.load_model 'models/7B/ggml-model-q4_0.bin'

prompt = 'kalle anka vill ha'
l.load_prompt(prompt)

io.stdout:write(prompt)
for i = 1,1000 do
  utput = l.sample(1)
  if utput == nil then
    break
  end
  io.stdout:write(table.concat(utput))
  io.stdout:flush()
end
print()

