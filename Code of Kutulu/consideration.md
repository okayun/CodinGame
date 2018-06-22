# Code of Kutulu

這い寄る混沌...

## ルールっぽいこと (Wood 3 での場合)
- プレイヤーは 4 人で, そのうちの一人を操作
- 目的は最後まで生き残ること?
- SAN値的なものが 0 になると負け (初期値は 250)
- 呪いのせいで, 1 ターン毎に 3 SAN値が減る
- 他のプレイヤーが 2 マス (以内?) にいる場合, 1 ターンに減るSAN値は 1 になる
- 座標 : 左上の角が $(0, 0)$
- 縦・横の長さは height, width で与えられる (固定じゃなさそう)
- 距離は基本的に<span style="color:cyan;">マンハッタン距離</span>で与えられるらしい
- Wanderer って敵?がいる
- Wanderer はマップ上の固定された召喚陣から出てくる
- 墓を冒そうとするプレイヤーに忍び寄ってくる
- Wanderer は消えた後, 任意のプレイヤーから最も遠い召喚陣から出てくる ("最も遠い"というのは, 壁などを無視した単純なマンハッタン距離で考えたときに一番遠い, という意味っぽい)
- Wanderer に触れられてしまう (プレイヤーと同じマスに Wanderer がいる) と, SAN値が <span style="color:#FF7777;">20</span> 減る (Wanderer は触れたらすぐ消える)
- プレイヤーの取れる行動は以下の通り
  - "WAIT" : 現在の座標に留まる
  - "MOVE x y" : 座標 $(x, y)$ に移動する

# 与えられる入力
## 初期入力
- width (マップの横幅)
- height (マップの縦幅)
- マップとプレイヤーと Wanderer の召喚陣の位置
- 4 つの整数 (Wood 3 では使わないっぽい)

## 1 ターン毎の入力
- "entityCount" : ユニットの数 ? 
- "entityType" : ユニットのタイプ (プレイヤーか Wanderer か)
- "id" : それぞれのユニットの id (-1 は effect ???)
- "x y" : ユニットの位置
- "param0" : 
  - entityType == "EXPLORER" : SAN値
  - entityType == "SPAWNING" : Wanderer が出現するまでの時間
  - entityType == "WANDERER" : Wanderer が再出現するまでの時間???
- "param1" : 
  - entityType == "EXPLORER" : まだ使わない
  - otherwise : 現在の状態とかなんとか (SPAW... : 0, WAND... : 1)
- "param2" :
  - entityType == "EXPLORER" : まだ使わない
  - otherwise : Wanderer ならターゲットのプレイヤー id (Spawn なら -1)
- 自プレイヤーは一番最初に情報が与えられるユニットなので, <span style="color:#FF7777;">今までみたく自プレイヤーの id は 0 というわけではない</span>


# 考察
- 単純なマンハッタン距離だと壁や他プレイヤーを考慮しないので変な動作をしがち
- 
