#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "./chunk.h"
#include "./poolset.h"

#define LOWORLD_TEMPLATE_MAX_CHARACTERS_PER_CHUNK 5
#define LOWORLD_TEMPLATE_MAX_GROUNDS_PER_CHUNK    5

typedef struct {
  loworld_chunk_t* target;

  const loworld_poolset_t* pools;
  uint64_t                 seed;
} loworld_template_building_param_t;

bool
loworld_template_building_param_valid(
    const loworld_template_building_param_t* param
);

/* [metaphysical gate]
 *   2 grounds (=) and the one encephalon statue (E)
 *
 *           E
 *        ======
 *
 *   ==================
 */
void
loworld_template_metaphysical_gate_build_chunk(
    const loworld_template_building_param_t* param
);

/* [open space]
 *   1 ground (=) and 0~3 enemies (E)
 *
 *
 *        E    E   E
 *   ==================
 */
void
loworld_template_open_space_build_chunk(
    const loworld_template_building_param_t* param
);

/* [broken open space]
 *   1 ground (=) and 0~2 enemies (E)
 *
 *
 *       E         E
 *   ========   ========
 *
 *         ======
 */
void
loworld_template_broken_open_space_build_chunk(
    const loworld_template_building_param_t* param
);

/* [passage]
 *   1 ground (=), ceiling (-), and 0~2 enemies (E)
 *
 *           E
 *       ----------
 *       E
 *   =================
 */
void
loworld_template_passage_build_chunk(
    const loworld_template_building_param_t* param
);

/* [broken passage]
 *    3 grounds (=), 1 or 2 ceilings (-), and 0~5 enemies (E)
 *
 *        E    E
 *      ----  ----
 *    E     E      E
 *   ====  ====  =====
 */
void
loworld_template_broken_passage_build_chunk(
   const loworld_template_building_param_t* param
);

/* [stairs]
 *  4 grounds (=), and 0~4 enemies (E)
 *
 *         E
 *     =========
 *    E
 *  ======
 *            E
 *        =========
 *         E
 * ==================
 */
void
loworld_template_stairs_build_chunk(
    const loworld_template_building_param_t* param
);

/* [BOSS: thiest's child]
 *    1 ground (=), and The Theist's Child (E)
 *
 *            E
 *    =================
 */
void
loworld_template_boss_theists_child_build_chunk(
    const loworld_template_building_param_t* param
);

/* [BOSS: big warder]
 *    1 ground (=), and The Big Warder (E)
 *
 *            E
 *    =================
 */
void
loworld_template_boss_big_warder_build_chunk(
    const loworld_template_building_param_t* param
);

/* [BOSS: greedy scientist]
 *    1 ground (=), and The Greedy Scientist (E)
 *
 *            E
 *    =================
 */
void
loworld_template_boss_greedy_scientist_build_chunk(
    const loworld_template_building_param_t* param
);
