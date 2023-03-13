l = require'llama'

l.load_model 'models/7B/ggml-model-q4_0.bin'

s = l.sample('kalle anka vill ha',8)
