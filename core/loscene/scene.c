#include "./scene.h"

#include <assert.h>
#include <stddef.h>

#include "core/locommon/input.h"

void loscene_delete(loscene_t* scene) {
  if (scene == NULL) return;

  assert(scene->vtable.delete != NULL);
  scene->vtable.delete(scene);
}

loscene_t* loscene_update(loscene_t* scene, const locommon_input_t* input) {
  assert(scene != NULL);
  assert(input != NULL);

  assert(scene->vtable.update != NULL);
  return scene->vtable.update(scene, input);
}

void loscene_draw(loscene_t* scene) {
  assert(scene != NULL);

  assert(scene->vtable.draw != NULL);
  scene->vtable.draw(scene);
}
