#include "common.h"
#include "llama.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}



// luuuuuua

struct lua_model {
  llama_context *ctx;

  // TODO: set me
  int32_t top_k = 40;
  float   top_p = 0.95f;
  float   temp  = 0.80f;
  float   repeat_penalty  = 1.10f;

  int last_n_size = 64;
  std::vector<llama_token> last_n_tokens;

  std::mt19937 rng;

  int n_past = 0;
  int n_threads = 8;

  std::vector<float> logits;

  lua_model(int seed) : rng(seed) {};

  void lua_pushtokens(lua_State *L, std::vector<llama_token> &embed);

  int load_prompt(lua_State *L);
  int sample(lua_State *L);
  int tokenize(lua_State *L);
};

// testing: TESTING TESTING
static lua_model *testing = NULL;

static int lua_load_model(lua_State *L) {
  const char *loadpath = luaL_checkstring(L, 1);
  int n_threads = -1;
  if (lua_gettop(L) >= 2) {
    n_threads = luaL_checkinteger(L, 2);
  }

  auto params = llama_context_default_params();

  params.seed = time(NULL);
  params.n_ctx = 512;
  // params.f16_kv     = params.memory_f16;
  // params.use_mlock  = params.use_mlock;
  // TODO: obviously a userdata
  testing = new lua_model(params.seed);
  if (n_threads > 0) {
    testing->n_threads = n_threads;
  }

  testing->ctx = llama_init_from_file(loadpath, params);
  if (testing->ctx == NULL) {
    return luaL_error(L, "%s: error: failed to load model '%s'\n", __func__, loadpath);
  }
  return 0;
}

void lua_model::lua_pushtokens(lua_State *L, std::vector<llama_token> &embed) {
  lua_createtable(L, embed.size(), 0);
  int idx = 1;
  for (auto id : embed) {
    lua_pushstring(L, llama_token_to_str(ctx,id));
    lua_rawseti(L, -2, idx++);
  }

}

int lua_model::tokenize(lua_State *L) {
  const char *prompt = luaL_checkstring(L, 1);
  std::vector<llama_token> embd_inp = ::llama_tokenize(ctx, prompt, false);

  lua_pushtokens(L, embd_inp);
  return 1;
}

int lua_model::load_prompt(lua_State *L) {
  const char *prompt = luaL_checkstring(L, 1);
  bool reset = lua_toboolean(L, 2);

  if (reset) {
    n_past = 0;
    last_n_tokens.clear();
  }

  std::vector<llama_token> embd_inp = ::llama_tokenize(ctx, prompt, n_past == 0);


  std::vector<llama_token> embd;
  size_t mem_per_token = 0;


  for (auto id : embd_inp) {
    // main loop evauluates only one input token at a time, likely we could do more?
    embd.push_back(id);

    if (last_n_tokens.size() >= last_n_size) {
      last_n_tokens.erase(last_n_tokens.begin());
    }
    last_n_tokens.push_back(id);

    if (llama_eval(ctx, embd.data(), embd.size(), n_past, n_threads)) {
        printf("Failed to load prompt\n");
        return 0;
    }
    n_past++;
    embd.clear();
  }

  return 0;
}

int lua_model::sample(lua_State *L) {
  int n_predict = 512;
  n_predict = luaL_checknumber(L, 1);

  std::vector<llama_token> sampled;
  size_t mem_per_token = 0;

  std::vector<llama_token> embd;

  const int n_vocab = llama_n_vocab(ctx);

  const int n_ctx = llama_n_ctx(ctx);
  // cannot sample more
  if (n_past > n_ctx) {
    return 0;
  }

  for (int i = 0; i < n_predict ; i++) {
    // no end-of-text in output. booring!
    auto logits = llama_get_logits(ctx);
    logits[llama_token_eos()] = 0;

    auto id = llama_sample_top_p_top_k(ctx,
                        last_n_tokens.data(),
                        last_n_tokens.size(),
                        top_k, top_p, temp, repeat_penalty);
    sampled.push_back(id);

    // out of context
    if (n_past >= n_ctx) {
      n_past++; // don't sample last token again
      break;
    }

    if (last_n_tokens.size() >= last_n_size) {
      last_n_tokens.erase(last_n_tokens.begin());
    }
    last_n_tokens.push_back(id);

    embd.push_back(id);
    if (llama_eval(ctx, embd.data(), embd.size(), n_past, n_threads)) {
        printf("Failed to predict\n");
        return 0;
    }
    n_past++;
    embd.clear();
  }

  lua_pushtokens(L, sampled);

  return 1;
}

static int lua_tokenize(lua_State *L) {
  return testing ? testing->tokenize(L) : luaL_error(L, "WHATEHF-");
}

static int lua_load_prompt(lua_State *L) {
  return testing ? testing->load_prompt(L) : luaL_error(L, "WHATEHF-");
}

static int lua_sample(lua_State *L) {
  return testing ? testing->sample(L) : luaL_error(L, "WHATEHF-");
}

static const luaL_reg llama_functions[] = {
  {"load_model", lua_load_model},
  {"tokenize", lua_tokenize},
  {"load_prompt", lua_load_prompt},
  {"sample", lua_sample},
  {NULL, NULL}
};

extern "C" int luaopen_llama(lua_State *L) {
  // module
  lua_newtable(L);
  luaL_register(L, NULL, llama_functions);
  return 1;
}
