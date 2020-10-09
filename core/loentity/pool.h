#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>

#include "util/memory/memory.h"

/* You must define prefix##_pool_new function yourself. */

#define LOENTITY_POOL_SOURCE_TEMPLATE(prefix)  \
    struct prefix##_pool_t {  \
      locommon_counter_t* idgen;  \
      size_t              length;  \
      prefix##_base_t     items[1];  \
    };  \
  \
    static size_t prefix##_pool_find_unused_item_index_(  \
        const prefix##_pool_t* pool) {  \
      assert(pool != NULL);  \
  \
      for (size_t i = 0; i < pool->length; ++i) {  \
        if (!pool->items[i].used) return i;  \
      }  \
      fprintf(stderr, #prefix " pool overflow\n");  \
      abort();  \
    }  \
  \
    void prefix##_pool_delete(prefix##_pool_t* pool) {  \
      assert(pool != NULL);  \
  \
      for (size_t i = 0; i < pool->length; ++i) {  \
        prefix##_base_deinitialize(&pool->items[i]);  \
      }  \
      memory_delete(pool);  \
    }  \
  \
    prefix##_base_t* prefix##_pool_create(prefix##_pool_t* pool) {  \
      assert(pool != NULL);  \
  \
      const size_t i = prefix##_pool_find_unused_item_index_(pool);  \
  \
      prefix##_base_reinitialize(  \
          &pool->items[i], locommon_counter_count(pool->idgen));  \
  \
      pool->items[i].used = true;  \
      return &pool->items[i];  \
    }  \
  \
    prefix##_base_t* prefix##_pool_unpack_item(  \
        prefix##_pool_t* pool, const msgpack_object* obj) {  \
      assert(pool != NULL);  \
  \
      const size_t i = prefix##_pool_find_unused_item_index_(pool);  \
  \
      if (!prefix##_base_unpack(&pool->items[i], obj)) return NULL;  \
  \
      pool->items[i].used = true;  \
      return &pool->items[i];  \
    }
