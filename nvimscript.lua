l = require'llama'

l.load_model 'models/7B/ggml-model-q4_0.bin'

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

runn [[manuscript 1: skinners house
[mood: comedic]
Skinner: welcome to dinner!
Chalmers: well it cannot be worse than last time
Skinner: here is an omelette!
Chalmers: but I asked for a sandwich
Skinner: it is an old family recipe :)
Chalmers: SO YOUR FOOD]]

runn [[l.tokenize(prompt)

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

function]]
