#pragma once

#include "core/locommon/input.h"

struct loscene_t;
typedef struct loscene_t loscene_t;

typedef struct {
  void
  (*delete)(
      loscene_t* scene
  );

  loscene_t*
  (*update)(
      loscene_t*              scene,
      const locommon_input_t* input
  );
  void
  (*draw)(
      loscene_t* scene
  );
} loscene_vtable_t;

struct loscene_t {
  loscene_vtable_t vtable;
};

void
loscene_delete(
    loscene_t* scene
);

loscene_t*
loscene_update(
    loscene_t*              scene,
    const locommon_input_t* input
);

void
loscene_draw(
    loscene_t* scene
);
