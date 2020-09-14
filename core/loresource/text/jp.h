#pragma once

#include <inttypes.h>

#include "util/dictres/dictres.h"

static dictres_item_t loresource_text_jp_[] = {
  {"app_name", u8"LEFTONE"},
  {"author",   u8"catfoot"},

  {"biome_metaphysical_gate",     u8"不可視の大門"},
  {"biome_cavias_camp",           u8"収容棟"},
  {"biome_laboratory",            u8"研究棟"},
  {"biome_boss_theists_child",    u8"信者の落胤"},
  {"biome_boss_big_warder",       u8"巨頭の看守"},
  {"biome_boss_greedy_scientist", u8"貪欲な科学者"},

  {"boss_big_warder_line0", u8"知能すら持たぬ家畜に踏み殺された仲間たちへの哀れみは"},
  {"boss_big_warder_line1", u8"いまや羨望へとその形を変えた"},

  {"boss_greedy_scientist_line0", u8"その選択はお前自身のものか？"},
  {"boss_greedy_scientist_line1", u8"胸を張ってそうだと，私達の前で言えるのか？"},

  {"boss_theists_child_line0", u8"全ては計画通りだった"},
  {"boss_theists_child_line1", u8"しかし空論は未だ宙に浮いたまま"},
  {"boss_theists_child_line2", u8"それが\"創造主\"の意志だというのか"},

  {"effect_curse",   u8"呪縛"},
  {"effect_amnesia", u8"忘却"},
  {"effect_lost",    u8"喪失"},

  {"stance_missionary_name", u8"宣教師"},
  {"stance_missionary_desc",
      u8"  あなたは自らの使命を果たすために歩き続ける．\n\n"
      u8"    - 攻撃を受けたとき，信仰をすべて失ったとき，あなたは狂気を徐々に失う．\n"
      u8"    - 狂気をすべて失ったとき，最後に触れた不可視の大門に転送される．"},
  {"stance_missionary_note",
      u8"「ああ，そうだ，君が見つけてきてくれよ．"
      u8"終幕を望む科学者がきっとどこかにいるはずだ．」"},

  {"stance_philosopher_name", u8"哲学者"},
  {"stance_philosopher_desc",
      u8"  鋭く冷たい慧眼は狂気を涵養し，創造主の思惑さえも明るみに引きずり出す．\n\n"
      u8"    - 攻撃を受けた際の狂気の減少量が低下する．"},
  {"stance_philosopher_note",
      u8"「あらゆる学問が発展したが世界の成り立ちだけが解明されず，誰も興味を持たない．"
      u8"まるで洗脳されたように永遠を希求し，脳髄は脳髄自身を疑わず．」"},

  {"stance_revolutioner_name", u8"革命者"},
  {"stance_revolutioner_desc",
      u8"  あくまでも優秀な血脈より生まれた意志はあなたに唯一の能動的攻撃手段を授ける．\n\n"
      u8"    - 攻撃を行うことで，僅かなダメージとノックバックを生む弾丸を発砲する．"},
  {"stance_revolutioner_note",
      u8"「地階の同胞は今までこの境遇に一切の疑問を持たなかった程に愚かだ．"
      u8"しかしそれ故に扇動も容易い．着火するのは少しの血だ．」"},

  {"stance_unfinisher_name", u8"未完走者"},
  {"stance_unfinisher_desc",
      u8"  幾度締め上げようと再生する細胞は永遠への敬遠と共に能力をもたらす．\n\n"
      u8"    - 信仰が50%以上残っている場合にあなたの狂気は徐々に回復する．\n"
      u8"    - 信仰をすべて失ったとき，狂気の減少速度が増加する．"},
  {"stance_unfinisher_note",
      u8"「忌々しい奴らに殺された仲間たちを，昔は気の毒だと思っていたよ．"
      u8"今は妬みしか感じない．」"},

  {"menu_exit", "EXIT"},

  {"title_authors", u8"catfoot"},
  {"title_buttons", u8"ESC / CLK"},

  {"tutorial_title_dead_by_combat", u8"近接戦闘"},
  {"tutorial_text_dead_by_combat",
      u8"  薄弱な意志には能動的攻撃の手段は限られる．"
      u8"凶悪な敵意を跳ね返すことのみがあなたにできるせめてもの抵抗だ．\n\n"
      u8"    - リング上のマーカーが赤色の部分に触れている間，防御をし続けることで相手を攻撃する．\n"
      u8"    - 防御に失敗した場合，ダメージを受ける．\n"
      u8"    - 背後から攻撃を受けた場合，防御が困難になる．"},
  {"tutorial_title_dead_by_curse", u8"呪縛"},
  {"tutorial_text_dead_by_curse",
      u8"  世界とは裏腹に意志はより強い意志に惹かれ残留する．\n\n"
      u8"    - あなたが呪縛に苦しめられている間，相手は狂気が尽きようと立ち続ける．\n"
      u8"    - 呪縛の終了時に相手の狂気が残っている場合，あなたの狂気は消滅する．"},
  {"tutorial_title_dead_by_lost", u8"喪失"},
  {"tutorial_text_dead_by_lost",
      u8"  悠久の時を信仰なくして立ち続けることはできない．\n\n"
      u8"    - 信仰が全て失われたとき狂気が徐々に減少する．"},
};

