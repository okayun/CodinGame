#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
#include <queue>
#include <random>
#include <map>
#include <cassert>
#include <cstring>
#include <cmath>

//#define DEBUG

// 場の縦と横の大きさ
static constexpr int kHeight = 7;
static constexpr int kWidth = 7; 
// ターンのタイプ
static constexpr int kPush = 0;
static constexpr int kMove = 1;
// ベクトル
static constexpr int vx[4] = { 0, 1, 0, -1 };
static constexpr int vy[4] = { -1, 0, 1, 0 };
// 出力のときに使うやつ
std::string str_dir[4] = { "UP", "RIGHT", "DOWN", "LEFT" };

/**
 * 移動する方向
 */
enum Direction {
  UP, RIGHT, DOWN, LEFT
};

class Unit {
 public:
  int x, y;

  Unit() {}
  Unit(int x, int y) : x(x), y(y) {}
};

/**
 * アイテムを表すクラス
 */
class Item {
 public:
  std::string item_name;
  int player_id = -1;

  Item(int player_id = -1, std::string item_name = "") : player_id(player_id), item_name(item_name) {}
};

/**
 * プレイヤを表すクラス
 */
class Player : public Unit {
 public:
  
  Player() {}
  Player(int x, int y) : Unit(x, y) {}
};

/**
 * Quest とやらを表すクラス
 */
class Quest {
 public:
  std::string item_name;

  Quest() {}
  Quest(std::string item_name) : item_name(item_name) {}
};

/**
 * 各プレイヤの着手の組を表す
 */
struct Move {
  int idx[2];
  Direction dir[2];

  Move(int my_idx = 0, Direction my_dir = UP, int opp_idx = 0, Direction opp_dir = UP) : idx{ my_idx, opp_idx }, dir{ my_dir, opp_dir } {}
    //: idx[0](my_idx), dir[0](my_dir), idx[1](opp_idx), dir[1](opp_dir) {}
  
  /**
   * 最初に着手できるプレイヤの番号を返す
   * 着手できないときは -1
   * そうでないときに, どちらも行もしくは列を選択していた場合は自プレイヤの番号を返す
   */
  int firstMovePlayer() {
    /*
    if ((idx[0] == idx[1]) and ((dir[0] % 2) == (dir[1] % 2))) {
      return -1;
    }
    */
    return int((dir[0] % 2) < (dir[1] % 2));
  }
};

/**
 * 状態を表す
 * いい感じにしたい
 */
class State {
 private:
  // 過去の着手の系列とその長さ
  Move past_moves[3]; // 添字は適当
  int moves_num;

 public:
  // タイルの情報
  std::string field[kHeight][kWidth];
  // アイテム
  Item items[kHeight][kWidth];
  Item players_item[2];
  // プレイヤ
  Player player[2];
  // クエスト
  std::vector<Quest> quests[2];
  // プレイヤの持つタイルの情報
  std::string tile[2];

  State() {}

  void Clear() {
    for (int i = 0; i < kHeight; ++i) {
      for (int j = 0; j < kWidth; ++j) {
        items[i][j] = Item(-1, "");
      }
    }
    players_item[0] = players_item[1] = Item();
    quests[0].clear();
    quests[1].clear();
    moves_num = 0;
  }

  void Push(Move move) {
    // 着手を保存
    past_moves[moves_num++] = move;

    int player_id = move.firstMovePlayer();

    // 同じ行もしくは列を選んだ場合何もおこらない
    /*
    if (player_id == -1) {
      return;
    }
    */

    /**
     * やること : 
     * 1. タイルを取り出す
     * 2. 持っているタイルを挿入
     * 3. アイテムの位置も更新
     * 4. プレイヤの位置を更新
     */

    for (int id = player_id; id < 2 + player_id; id++) {
      //std::cerr << "先行は " << id << " です！" << std::endl;

      if (id % 2 == 1) {
        continue;
      }
      // プレイヤが持っているタイル
      std::string tmp_tile = tile[id % 2];
      // プレイヤが持っているアイテム
      Item tmp_item = players_item[id % 2];
      // プレイヤが指定した行または列のインデックス
      int move_idx = move.idx[id % 2];
      
      switch(move.dir[id % 2]) {
        case UP:
          tile[id % 2] = field[0][move_idx];
          players_item[id % 2] = items[0][move_idx];
          for (int h = 0; h < kHeight - 1; h++) {
            field[h][move_idx] = field[h + 1][move_idx];
            items[h][move_idx] = items[h + 1][move_idx];
          }
          field[kHeight - 1][move_idx] = tmp_tile;
          items[kHeight - 1][move_idx] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].x == move_idx) {
              player[i].y = (player[i].y + kHeight - 1) % kHeight;
            }
          }
          break;

        case RIGHT:
          tile[id % 2] = field[move_idx][kWidth - 1];
          players_item[id % 2] = items[move_idx][kWidth - 1];
          for (int w = kWidth - 1; w >= 1; w--) {
            field[move_idx][w] = field[move_idx][w - 1];
            items[move_idx][w] = items[move_idx][w - 1];
          }
          field[move_idx][0] = tmp_tile;
          items[move_idx][0] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].y == move_idx) {
              player[i].x = (player[i].x + 1) % kWidth;
            }
          }

          break;

        case DOWN:
          tile[id % 2] = field[kHeight - 1][move_idx];
          players_item[id % 2] = items[kHeight - 1][move_idx];
          for (int h = kHeight - 1; h >= 1; h--) {
            field[h][move_idx] = field[h - 1][move_idx];
            items[h][move_idx] = items[h - 1][move_idx];        
          }
          field[0][move_idx] = tmp_tile;
          items[0][move_idx] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].x == move_idx) {
              player[i].y = (player[i].y + 1) % kHeight;
            }
          }

          break;

        case LEFT:
          tile[id % 2] = field[move_idx][0];
          players_item[id % 2] = items[move_idx][0];
          for (int w = 0; w < kWidth - 1; w++) {
            field[move_idx][w] = field[move_idx][w + 1];
            items[move_idx][w] = items[move_idx][w + 1];
          }
          field[move_idx][kWidth - 1] = tmp_tile;
          items[move_idx][kWidth - 1] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].y == move_idx) {
              player[i].x = (player[i].x + kWidth - 1) % kWidth;
            }
          }

          break;

        default:
          assert(false);
          break;
      }
      /* switch(move.dir[id % 2]) */
    }
    /* for (int id = player_id; id < 2 + player_id; ++id) */
  }
  /* void Push(Move move) */

  void Undo() {
    // 未着手の状態でundoはできない
    assert(moves_num > 0);
    moves_num--;

    // 後手から
    int player_id = past_moves[moves_num].firstMovePlayer();

    // 衝突していた場合は何もしなくてOK
    /*
    if (player_id == -1) {
      return;
    }
    */

    //std::cerr << "undo second player : " << player_id << std::endl;

    player_id = !player_id;

    for (int id = player_id; id < 2 + player_id; ++id) {
      //std::cerr << "now id = " << id << " player's pos : " << player[id].x << ", " << player[id].y << std::endl;
      //std::cerr << "id : " << (id % 2) << std::endl;
      
      if (id % 2 == 1) {
        continue;
      }

      // プレイヤが持っているタイル
      std::string tmp_tile = tile[id % 2];
      // プレイヤが持っているアイテム
      Item tmp_item = players_item[id % 2];
      // プレイヤが指定した行または列のインデックス
      int past_move_idx = past_moves[moves_num].idx[id % 2];

      //std::cerr << "Move : " << past_move_idx << ", " << str_dir[int(past_moves[moves_num].dir[id % 2])] << std::endl;
      
      switch(past_moves[moves_num].dir[id % 2]) {
        case DOWN:
          tile[id % 2] = field[0][past_move_idx];
          players_item[id % 2] = items[0][past_move_idx];
          for (int h = 0; h < kHeight - 1; h++) {
            field[h][past_move_idx] = field[h + 1][past_move_idx];
            items[h][past_move_idx] = items[h + 1][past_move_idx];
          }
          field[kHeight - 1][past_move_idx] = tmp_tile;
          items[kHeight - 1][past_move_idx] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].x == past_move_idx) {
              player[i].y = (player[i].y + kHeight - 1) % kHeight;
            }
          }

          break;

        case LEFT:
          tile[id % 2] = field[past_move_idx][kWidth - 1];
          players_item[id % 2] = items[past_move_idx][kWidth - 1];
          for (int w = kWidth - 1; w >= 1; w--) {
            field[past_move_idx][w] = field[past_move_idx][w - 1];
            items[past_move_idx][w] = items[past_move_idx][w - 1];
          }
          field[past_move_idx][0] = tmp_tile;
          items[past_move_idx][0] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].y == past_move_idx) {
              player[i].x = (player[i].x + 1) % kWidth;
            }
          }

          break;

        case UP:
          tile[id % 2] = field[kHeight - 1][past_move_idx];
          players_item[id % 2] = items[kHeight - 1][past_move_idx];
          for (int h = kHeight - 1; h >= 1; h--) {
            field[h][past_move_idx] = field[h - 1][past_move_idx];
            items[h][past_move_idx] = items[h - 1][past_move_idx];        
          }
          field[0][past_move_idx] = tmp_tile;
          items[0][past_move_idx] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].x == past_move_idx) {
              player[i].y = (player[i].y + 1) % kHeight;
            }
          }

          break;

        case RIGHT:
          tile[id % 2] = field[past_move_idx][0];
          players_item[id % 2] = items[past_move_idx][0];
          for (int w = 0; w < kWidth - 1; w++) {
            field[past_move_idx][w] = field[past_move_idx][w + 1];
            items[past_move_idx][w] = items[past_move_idx][w + 1];
          }
          field[past_move_idx][kWidth - 1] = tmp_tile;
          items[past_move_idx][kWidth - 1] = tmp_item;

          for (int i = 0; i < 2; ++i) {
            if (player[i].y == past_move_idx) {
              player[i].x = (player[i].x + kWidth - 1) % kWidth;
            }
          }

          break;

        default:
          assert(false);
          break;
      }
      /* switch (move.dir[id % 2]) */

      //std::cerr << "after id = " << id << " player's pos : " << player[id].x << ", " << player[id].y << std::endl;
    }
    /* for (int id = player_id; id < 2 + player_id; ++id) */
  }
  /* void Undo() */

  /**
   * 各変数を表示
   */
  void Print() {
    std::cerr << "[ players' information ]\n";
    for (int i = 0; i < 2; ++i) {
      std::cerr << "player[" << i << "]'s position : (" << player[i].x << ", " << player[i].y << ")\n";
    }

    std::cerr << "\n--------------------------------------------\n";

    std::cerr << "\n[ players' item information ]\n";
    for (int i = 0; i < 2; ++i) {
      std::cerr << "player[" << i << "]'s item : " << players_item[i].item_name << "\n";
    }

    std::cerr << "\n--------------------------------------------\n";

    std::cerr << "\n[ players' tile information ]\n";
    for (int i = 0; i < 2; ++i) {
      std::cerr << "player[" << i << "]'s tile : [" << tile[i] << "]\n";
    }

    std::cerr << "\n--------------------------------------------\n";

    std::cerr << "\n[ field information ]\n";
    for (int i = 0; i < kHeight; ++i) {
      for (int j = 0; j < kWidth; ++j) {
        std::cerr << field[i][j] << (j == kWidth - 1 ? "\n" : " ");
      }
    }

    std::cerr << "\n--------------------------------------------\n";
    std::cerr << "[ items information ]\n";
    for (int i = 0; i < kHeight; ++i) {
      for (int j = 0; j < kWidth; ++j) {
        std::cerr << (items[i][j].item_name == "" ? "None" : items[i][j].item_name) << (j == kWidth - 1 ? "\n" : " ");
      }
    }

    std::cerr << "\n--------------------------------------------\n";
    std::cerr << "[ quests information ]\n";
    for (int id = 0; id < 2; ++id) {
      std::cerr << "player[" << id << "]'s quests : \n";
      for (int i = 0; i < quests[id].size(); ++i) {
        std::cerr << quests[id][i].item_name << (i == quests[id].size() - 1 ? "\n" : ", ");
      }
    }
  }

};

/**
 * 各変数
 */
int turnType;
State state;
// Evaluate() で使う変数
bool used[kHeight][kWidth];
// 詰み探索のときに使う
int prev[kHeight][kWidth];
int dist[kHeight][kWidth];
// メルセンヌ・ツイスタ
std::random_device rnd;
std::mt19937 mt;
// 連続で取った行動の記録
int cnt[7][4];
bool isSameAction;

/**
 * 初期化
 */
void Init() {
  std::cin.tie(0);
  std::cout.tie(0);

  mt.seed(rnd());

  return;
}

/**
 * 入力を受け取る
 */
void Input() {
  state.Clear();

  // ターンの種類
  std::cin >> turnType;

  // タイルの情報
  for (int i = 0; i < kHeight; ++i) {
    for (int j = 0; j < kWidth; ++j) {
      std::cin >> state.field[i][j];
    }
  }

  // プレイヤの位置, クエスト数, プレイヤの持つタイル情報
  for (int i = 0; i < 2; ++i) {
    int numPlayerCards, playerX, playerY;
    std::string playerTile;

    std::cin >> numPlayerCards >> playerX >> playerY >> playerTile;

    state.player[i] = Player(playerX, playerY);
    state.tile[i] = playerTile;
  }
  
  int numItems;
  // アイテムの数
  std::cin >> numItems;

  for (int i = 0; i < numItems; ++i) {
    std::string itemName;
    int itemX, itemY, itemPlayerId;

    std::cin >> itemName >> itemX >> itemY >> itemPlayerId;
    
    if (itemX == -1) {
      state.players_item[0] = Item(itemPlayerId, itemName);
    } else if (itemX == -2) {
      state.players_item[1] = Item(itemPlayerId, itemName);
    } else {
      state.items[itemY][itemX] = Item(itemPlayerId, itemName);
    }
  }

  int numQuests;
  std::cin >> numQuests;

#ifdef DEBUG
    std::cerr << "numQuests : " << numQuests << std::endl;
#endif

  for (int i = 0; i < numQuests; ++i) {
    std::string questItemName;
    int questPlayerId;

    std::cin >> questItemName >> questPlayerId;
    state.quests[questPlayerId].push_back(Quest(questItemName));

#ifdef DEBUG
    std::cerr << "questItemName : " << questItemName << std::endl;
    std::cerr << "player id : " << questPlayerId << std::endl;
    std::cerr << state.quests[questPlayerId][0].item_name << std::endl;
#endif

  }

  return;
}

/**
 * アイテムが, プレイヤ id のクエストの対象かどうか
 */
inline bool isTargetItem(int id, Item item) {
  for (int i = 0; i < state.quests[id].size(); ++i) {
    if (state.quests[id][i].item_name == item.item_name) return true;
  }

  return false;
}

/**
 * プレイヤ id が目標アイテムを持っているかどうか
 * 関数名がアレ
 */
inline bool isHaveItem(int id) {
  if (state.players_item[id].player_id != id) {
    return false;
  }

  for (int i = 0; i < state.quests[id].size(); ++i) {
    if (state.players_item[id].item_name != "" and state.players_item[id].item_name == state.quests[id][i].item_name) {
      return true;
    }
  }
  return false;
}

/**
 * プレイヤ id が 1 手でアイテムを押し出せるかどうか
 */
inline bool isPushOutItem(int id) {
  for (int h = 0; h < kHeight; h += kHeight - 1) {
    for (int w = 0; w < kWidth; ++w) {
      if (state.items[h][w].player_id == id) {
        return true;
      }
    }
  }

  for (int w = 0; w < kWidth; w += kWidth - 1) {
    for (int h = 0; h < kHeight; ++h) {
      if (state.items[h][w].player_id == id) {
        return true;
      }
    }
  }

  return false;
}


/**
 * プレイヤ id が端にいるかどうか
 * 角なら 2, そうでないが端にいるとき 1, いないとき 0
 */
inline int isEndPlayer(int id) {
  int x = state.player[id].x, y = state.player[id].y;
  return (x == 0 or x == kWidth - 1 or y == 0 or y == kHeight - 1);
}

/**
 * start をスタート地点として幅優先探索を行う
 * 経路復元はしない
 * start.first : h, start.second : w
 */
void Bfs(std::pair<int, int> start) {
  std::queue<std::pair<int, int> > que;
  std::memset(used, false, sizeof used);
  std::memset(dist, -1, sizeof dist);

  used[start.first][start.second] = true;
  dist[start.first][start.second] = 0;
  que.push( { start.first, start.second } );

  while (!que.empty()) {
    auto p = que.front();
    que.pop();

    for (int i = 0; i < 4; ++i) {
      int nx = p.second + vx[i], ny = p.first + vy[i];

      if (0 <= nx and nx < kWidth and 0 <= ny and ny < kHeight and !used[ny][nx]
          and state.field[p.first][p.second][i] == '1' and state.field[ny][nx][(i + 2) % 4] == '1') {
        
        dist[ny][nx] = dist[p.first][p.second] + 1;
        used[ny][nx] = true;
        que.push( { ny, nx } );
      }
    }
  }
}

/**
 * プレイヤ id が到達できる目標アイテムの座標リストを返す
 * Bfs() を呼んだあとに行うこと
 */
std::vector<std::pair<int, int> > getItemList(int id) {
  std::vector<std::pair<int, int> > ret;

  for (int h = 0; h < kHeight; ++h) {
    for (int w = 0; w < kWidth; ++w) {
      if (state.player[id].x == w and state.player[id].y == h) continue;

      if (used[h][w] and isTargetItem(id, state.items[h][w]) and (state.items[h][w].player_id == id)) {
        ret.push_back( { h, w } );
      }
    }
  }

  return ret;
}

/**
 * start をスタート地点, target をゴール地点として幅優先を行う
 * 経路復元あり
 * *.first : h, *.second : w
 */
void Bfs(std::pair<int, int> start, std::pair<int, int> target) {
  std::queue<std::pair<int, int> > que;
  std::memset(used, false, sizeof used);
  std::memset(prev, -1, sizeof prev);
  std::memset(dist, -1, sizeof dist);

  used[start.first][start.second] = true;
  dist[start.first][start.second] = 0;
  que.push( { start.first, start.second } );

  while (!que.empty()) {
    auto p = que.front();
    que.pop();

    if (target.first == p.first and target.second == p.second) {
      break;
    }

    for (int i = 0; i < 4; ++i) {
      int nx = p.second + vx[i], ny = p.first + vy[i];

      if (0 <= nx and nx < kWidth and 0 <= ny and ny < kHeight and !used[ny][nx]
          and state.field[p.first][p.second][i] == '1' and state.field[ny][nx][(i + 2) % 4] == '1') {
        
        dist[ny][nx] = dist[p.first][p.second] + 1;
        prev[ny][nx] = i;
        used[ny][nx] = true;
        que.push( { ny, nx } );
      }
    }
  }
}

/**
 * 盤面の評価関数
 */
int Evaluate() {
  // 適当に、アイテムとのマンハッタン距離で決めてみる
  //int eval[2] = { 0, 0 };

  /*
  // 到達できるかどうかも考慮
  // 現在は、アイテムとのマンハッタン距離だけで決定している
  for (int id = 0; id < 2; ++id) {
    //std::cerr << "player[" << id << "]'s position : " << state.player[id].x << ", " << state.player[id].y << std::endl;

    //std::cerr << "id : " << id << " update!" << std::endl;
    //std::cerr << "item's position : " << i << ", " << j << std::endl;

    // アイテムに到達できるかを考慮
    std::queue<std::pair<int, int> > que;
    std::memset(used, false, sizeof used);

    used[state.player[id].y][state.player[id].x] = true;
    que.push( { state.player[id].x, state.player[id].y } );

    while (!que.empty()) {
      auto p = que.front();
      que.pop();

      for (int k = 0; k < 4; ++k) {
        int nx = p.first + vx[k], ny = p.second + vy[k];

        if (0 <= nx and nx < kWidth and 0 <= ny and ny < kHeight and !used[ny][nx]
            and state.field[p.second][p.first][k] == '1' and state.field[ny][nx][(k + 2) % 4] == '1') {

          used[ny][nx] = true;
          que.push( { nx, ny } );
        }
      }
    }

    for (int i = 0; i < kHeight; ++i) {
      for (int j = 0; j < kWidth; ++j) {
        if (state.items[i][j].player_id == id && isTargetItem(id, state.items[i][j])) {
          eval[id] += (used[i][j] ? (id == 1 ? -10000 : -1000) : 0);//(id == 1 ? - abs(state.player[id].x - j) - abs(state.player[id].y - i) : abs(state.player[id].x - j) + abs(state.player[id].y - i)));
          //eval[id] += (id == 1 ? -10000 : -1000);
        }
      }
    }
  }
  */

  Bfs( { state.player[0].y, state.player[0].x } );
  auto m = getItemList(0);
  Bfs( { state.player[1].y, state.player[1].x } );
  auto o = getItemList(1);

  return int(m.size() - o.size());
}

/**
 * タイルを挿入する行または列を決定する
 */
void InsertTile() {
  // 自プレイヤが端にいるかどうか
  int is_end_player = isEndPlayer(0);

  // 自分がアイテムを持っているか
  bool is_have_item = isHaveItem(0);

  // アイテムを 1 手で押し出せるかどうか
  bool is_push_out_item = isPushOutItem(0);

  int max_eval = -int(1e9);
  Move max_move;

  for (int my_idx = 0; my_idx < kHeight; ++my_idx) {
    for (int m = 0; m < 4; ++m) {
      /*
      for (int opp_idx = 0; opp_idx < kHeight; ++opp_idx) {
        for (int o = 0; o < 4; ++o) {
      */
          Move tmp_move = Move(my_idx, Direction(m), 0, Direction(0));

          state.Push(tmp_move);
          int tmp_eval = Evaluate();
          state.Undo();
        /*
          if (tmp_eval < min_eval) {
            min_eval = tmp_eval;
            min_move = tmp_move;
          }
        }
      }
        */

      if (tmp_eval > max_eval) {
        max_eval = tmp_eval;
        max_move = tmp_move;
      }
    }
  }

  /*
  if (isSameAction) {
    std::memset(cnt, 0, sizeof cnt);
    isSameAction = false;

    std::cout << "PUSH " << max_move.idx[0] << " " << str_dir[int(max_move.dir[0])] << std::endl;
  }
  else */
  if (max_eval > 0) {
    std::cout << "PUSH " << max_move.idx[0] << " " << str_dir[int(max_move.dir[0])] << std::endl;
  }
  else if (is_end_player == 2 and is_have_item) {
    std::cerr << "is_end_player == 2 and is_have_item" << std::endl;
    // 左端なら右からタイルを突っ込む
    if (state.player[0].x == 0) {
      std::cout << "PUSH " << state.player[0].y << " LEFT" << std::endl;
      int tmp = cnt[state.player[0].y][3];
      std::memset(cnt, 0, sizeof cnt);
      cnt[state.player[0].y][3] = tmp + 1;
      if (cnt[state.player[0].y][3] == 3) {
        isSameAction = true;
      }
    }
    // 右端なら左から
    else {
      std::cout << "PUSH " << state.player[0].y << " RIGHT" << std::endl;
      int tmp = cnt[state.player[0].y][1];
      std::memset(cnt, 0, sizeof cnt);
      cnt[state.player[0].y][1] = tmp + 1;
      if (cnt[state.player[0].y][1] == 3) {
        isSameAction = true;
      }
    }
    /**
     * 上もしくは下から突っ込む必要あるのかな・・・？
     */
  }
  // 自プレイヤが角ではないが端にいるかつアイテムを持っている場合
  // アイテムが取れそうなら取りに行く
  // そうでないならプレイヤを角に持っていく
  else if (is_end_player == 1 and is_have_item) {
    std::cerr << "is_end_player == 1 and is_have_item" << std::endl;
    int x = state.player[0].x, y = state.player[0].y;

    if (x == 0) {
      std::cout << "PUSH " << y << " LEFT" << std::endl;
      int tmp = cnt[y][3];
      std::memset(cnt, 0, sizeof cnt);
      cnt[y][3] = tmp + 1;
      if (cnt[y][3] == 3) {
        isSameAction = true;
      }
    }
    else if (x == kWidth - 1) {
      std::cout << "PUSH " << y << " RIGHT" << std::endl;
      int tmp = cnt[y][1];
      std::memset(cnt, 0, sizeof cnt);
      cnt[y][1] = tmp + 1;
      if (cnt[y][1] == 3) {
        isSameAction = true;
      }
    }
    else if (y == 0) {
      std::cout << "PUSH " << x << " UP" << std::endl;
      int tmp = cnt[x][0];
      std::memset(cnt, 0, sizeof cnt);
      cnt[x][0] = tmp + 1;
      if (cnt[x][0] == 3) {
        isSameAction = true;
      }
    }
    else if (y == kHeight - 1) {
      std::cout << "PUSH " << x << " DOWN" << std::endl;
      int tmp = cnt[x][2];
      std::memset(cnt, 0, sizeof cnt);
      cnt[x][2] = tmp + 1;
      if (cnt[x][2] == 3) {
        isSameAction = true;
      }
    }
    else {
      assert(false);
    }
  }
  // 自プレイヤが端にいるかつアイテムを持っていない場合
  // (プレイヤをできるだけ動かさないように)アイテムを端に持っていく
  else if (is_end_player > 0) {
    std::cerr << "is_end_player > 0" << std::endl;
    std::vector<std::pair<int, int> > item_position, end_item_position;

    for (int h = 0; h < kHeight; ++h) {
      for (int w = 0; w < kWidth; ++w) {
        if (state.items[h][w].player_id == 0 and isTargetItem(0, state.items[h][w])) {
          bool is_end = ((h == 0 or h == kHeight - 1 or w == kWidth - 1 or w == 0) ? true : false);
          if (is_end) {
            end_item_position.push_back( { h, w } );
          }
          else {
            item_position.push_back( { h, w } );
          }
        }
      }
    }

    // とりあえず適当に
    std::cerr << "item and end_item pos : " << item_position.size() << ", " << end_item_position.size() << std::endl;

    // アイテムが残り一つで, それをどちらかのプレイヤが持っている場合
    if (item_position.size() == 0 and end_item_position.size() == 0) {
      if (isHaveItem(0) and isTargetItem(0, state.players_item[0])) {
        int x = state.player[0].x, y = state.player[0].y;

        if (x == 0) {
          std::cout << "PUSH " << y << " LEFT" << std::endl;
          int tmp = cnt[y][3];
          std::memset(cnt, 0, sizeof cnt);
          cnt[y][3] = tmp + 1;
          if (cnt[y][3] == 3) {
            isSameAction = true;
          }
        }
        else if (x == kWidth - 1) {
          std::cout << "PUSH " << y << " RIGHT" << std::endl;
          int tmp = cnt[y][1];
          std::memset(cnt, 0, sizeof cnt);
          cnt[y][1] = tmp + 1;
          if (cnt[y][1] == 3) {
            isSameAction = true;
          }
        }
        else if (y == 0) {
          std::cout << "PUSH " << x << " UP" << std::endl;
          int tmp = cnt[x][0];
          std::memset(cnt, 0, sizeof cnt);
          cnt[x][0] = tmp + 1;
          if (cnt[x][0] == 3) {
            isSameAction = true;
          }
        }
        else if (y == kHeight - 1) {
          std::cout << "PUSH " << x << " DOWN" << std::endl;
          int tmp = cnt[x][2];
          std::memset(cnt, 0, sizeof cnt);
          cnt[x][2] = tmp + 1;
          if (cnt[x][2] == 3) {
            isSameAction = true;
          }
        }
        else {
          assert(false);
        }
      }
      else {
        max_eval = -int(1e9);

        for (int my_idx = 0; my_idx < kHeight; ++my_idx) {
          for (int m = 0; m < 4; ++m) {
            /*
            int min_eval = int(1e9);
            Move min_move;

            for (int opp_idx = 0; opp_idx < kHeight; ++opp_idx) {
              for (int o = 0; o < 4; ++o) {
            */
            //Move tmp_move = Move(my_idx, Direction(m), opp_idx, Direction(o));
            Move tmp_move = Move(my_idx, Direction(m), 0, Direction(0));

            state.Push(tmp_move);
            int tmp_eval = Evaluate();
            state.Undo();
            /*
                if (tmp_eval < min_eval) {
                  min_eval = tmp_eval;
                  min_move = tmp_move;
                }
              }
            }
            */
            /*
            if (min_eval > max_eval) {
              max_eval = min_eval;
              max_move = min_move;
            }
            */
            if (tmp_eval > max_eval) {
              max_eval = tmp_eval;
              max_move = tmp_move;
            }
          }
        }

        std::cout << "PUSH " << max_move.idx[0] << " " << str_dir[int(max_move.dir[0])] << std::endl;
      }

      return;
    }

    std::pair<int, int> target_item = (item_position.size() > 0 ? item_position[mt() % item_position.size()] : end_item_position[0]);

    /* */
    if (end_item_position.size() > 0) {
      target_item = end_item_position[mt() % end_item_position.size()];
      // 押し出せるならそのアイテムをターゲットとする
      for (int i = 0; i < end_item_position.size(); ++i) {
        if (end_item_position[i].second == 0 or end_item_position[i].second == kWidth - 1 or end_item_position[i].first == 0 or end_item_position[i].first == kHeight - 1) {
          target_item = end_item_position[i];
          break;
        }
      }
    }
    // */

    if (target_item.second == 0) {
      std::cout << "PUSH " << target_item.first << " LEFT" << std::endl;
      int tmp = cnt[target_item.first][3];
      std::memset(cnt, 0, sizeof cnt);
      cnt[target_item.first][3] = tmp + 1;
      if (cnt[target_item.first][3] == 3) {
        isSameAction = true;
      }
    }
    else if (target_item.second == kWidth - 1) {
      std::cout << "PUSH " << target_item.first << " RIGHT" << std::endl;
      int tmp = cnt[target_item.first][1];
      std::memset(cnt, 0, sizeof cnt);
      cnt[target_item.first][1] = tmp + 1;
      if (cnt[target_item.first][1] == 3) {
        isSameAction = true;
      }
    }
    else if (target_item.first == 0) {
      std::cout << "PUSH " << target_item.second << " UP" << std::endl;
      int tmp = cnt[target_item.second][0];
      std::memset(cnt, 0, sizeof cnt);
      cnt[target_item.second][0] = tmp + 1;
      if (cnt[target_item.second][0] == 3) {
        isSameAction = true;
      }
    }
    else if (target_item.first == kHeight - 1) {
      std::cout << "PUSH " << target_item.second << " DOWN" << std::endl;
      int tmp = cnt[target_item.second][2];
      std::memset(cnt, 0, sizeof cnt);
      cnt[target_item.second][2] = tmp + 1;
      if (cnt[target_item.second][2] == 3) {
        isSameAction = true;
      }
    }
    // 端にアイテムが無いので端に近づける
    else {
      int uh = target_item.first, dh = kHeight - target_item.first,
          lw = target_item.second, rw = kWidth - target_item.second;
      int min_dist = std::min( { uh, dh, lw, rw } );

      if (lw == min_dist) {
        std::cout << "PUSH " << target_item.first << " LEFT" << std::endl;
        int tmp = cnt[target_item.first][3];
        std::memset(cnt, 0, sizeof cnt);
        cnt[target_item.first][3] = tmp + 1;
        if (cnt[target_item.first][3] == 3) {
          isSameAction = true;
        }
      }
      else if (rw == min_dist) {
        std::cout << "PUSH " << target_item.first << " RIGHT" << std::endl;
        int tmp = cnt[target_item.first][1];
        std::memset(cnt, 0, sizeof cnt);
        cnt[target_item.first][1] = tmp + 1;
        if (cnt[target_item.first][1] == 3) {
          isSameAction = true;
        }
      }
      else if (uh == min_dist) {
        std::cout << "PUSH " << target_item.second << " UP" << std::endl;
        int tmp = cnt[target_item.second][0];
        std::memset(cnt, 0, sizeof cnt);
        cnt[target_item.second][0] = tmp + 1;
        if (cnt[target_item.second][0] == 3) {
          isSameAction = true;
        }
      }
      else if (dh == min_dist) {
        std::cout << "PUSH " << target_item.second << " DOWN" << std::endl;
        int tmp = cnt[target_item.second][2];
        std::memset(cnt, 0, sizeof cnt);
        cnt[target_item.second][2] = tmp + 1;
        if (cnt[target_item.second][2] == 3) {
          isSameAction = true;
        }
      }
    }
  }
  // 自プレイヤが端にいない場合
  else {
    std::cerr << "else" << std::endl;
    /* */
    int uh = state.player[0].y, dh = kHeight - state.player[0].y,
        lw = state.player[0].x, rw = kWidth - state.player[0].x;
    int min_dist = std::min( { uh, dh, lw, rw } );

    if (uh == min_dist) {
      std::cout << "PUSH " << state.player[0].x << " UP" << std::endl;
      int tmp = cnt[state.player[0].x][0];
      std::memset(cnt, 0, sizeof cnt);
      cnt[state.player[0].x][0] = tmp + 1;
      if (cnt[state.player[0].x][0] == 3) {
        isSameAction = true;
      }
    }
    else if (dh == min_dist) {
      std::cout << "PUSH " << state.player[0].x << " DOWN" << std::endl;
      int tmp = cnt[state.player[0].x][2];
      std::memset(cnt, 0, sizeof cnt);
      cnt[state.player[0].x][2] = tmp + 1;
      if (cnt[state.player[0].x][2] == 3) {
        isSameAction = true;
      }
    }
    else if (lw == min_dist) {
      std::cout << "PUSH " << state.player[0].y << " LEFT" << std::endl;
      int tmp = cnt[state.player[0].y][3];
      std::memset(cnt, 0, sizeof cnt);
      cnt[state.player[0].y][3] = tmp + 1;
      if (cnt[state.player[0].y][3] == 3) {
        isSameAction = true;
      }
    }
    else {
      std::cout << "PUSH " << state.player[0].y << " RIGHT" << std::endl;
      int tmp = cnt[state.player[0].y][1];
      std::memset(cnt, 0, sizeof cnt);
      cnt[state.player[0].y][1] = tmp + 1;
      if (cnt[state.player[0].y][1] == 3) {
        isSameAction = true;
      }
    }
    //*/

    /*
    max_eval = -int(1e9);

    for (int my_idx = 0; my_idx < kHeight; ++my_idx) {
          for (int m = 0; m < 4; ++m) {
            /*
            int min_eval = int(1e9);
            Move min_move;

            for (int opp_idx = 0; opp_idx < kHeight; ++opp_idx) {
              for (int o = 0; o < 4; ++o) {
            */
            //Move tmp_move = Move(my_idx, Direction(m), opp_idx, Direction(o));
            /*
            Move tmp_move = Move(my_idx, Direction(m), 0, Direction(0));

            state.Push(tmp_move);
            int tmp_eval = Evaluate();
            state.Undo();
            */
            /*
                if (tmp_eval < min_eval) {
                  min_eval = tmp_eval;
                  min_move = tmp_move;
                }
              }
            }
            */
            /*
            if (min_eval > max_eval) {
              max_eval = min_eval;
              max_move = min_move;
            }
            */
            /*
            if (tmp_eval > max_eval) {
              max_eval = tmp_eval;
              max_move = tmp_move;
            }
          }
        }

    std::cout << "PUSH " << max_move.idx[0] << " " << str_dir[int(max_move.dir[0])] << std::endl;
    */
  }

  return;
}

/**
 * 移動先を決定する
 * 出力 : PASS または MOVE
 */
void DecideMove() {
  int x = state.player[0].x, y = state.player[0].y;
  int target_x = 0, target_y = 0;
  int min_dist = int(1e9);

  // たどり着けるアイテムを列挙する
  Bfs( { y, x } );
  auto list = getItemList(0);

  std::cerr << "list size : " << list.size() << std::endl;

  // 詰めそうなら詰む
  if (list.size() > 0) {
    int sx = x, sy = y;

    std::cout << "MOVE";

    for (int i = 0; i < list.size(); ++i) {
      Bfs( { sy, sx }, list[i] );
      std::string output = "";

      int tx = list[i].second, ty = list[i].first;
      std::cerr << "item_name : " << state.items[list[i].first][list[i].second].item_name << std::endl;
      while (!(tx == sx and ty == sy)) {
        std::cerr << "tx, ty = " << tx << ", " << ty << std::endl;
        int idx = prev[ty][tx];
        output = " " + str_dir[idx] + output;
        tx += vx[(idx + 2) % 4], ty += vy[(idx + 2) % 4];
      }

      sx = list[i].second, sy = list[i].first;

      std::cout << output;
    }
    std::cout << std::endl;

    return ;
  }

  // 端っこに行くようにする
  std::pair<int, int> corner[4] = { { 0, 0 }, { 0, kWidth - 1 }, { kHeight - 1, 0 }, { kHeight - 1, kWidth - 1 } };

  for (int i = 0; i < 4; ++i) {
    if (corner[i].first == y and corner[i].second == x) {
      std::cout << "PASS" << std::endl;
      return;
    }
  }

  Bfs( { y, x } );

  for (int i = 0; i < 4; ++i) {
    if (used[corner[i].first][corner[i].second]) {
      Bfs( { y, x }, corner[i]);

      std::string output = "";
      int tx = corner[i].second, ty = corner[i].first;
      while (!(tx == x and ty == y)) {
        std::cerr << "loop2" << std::endl;
        int idx = prev[ty][tx];
        output = " " + str_dir[idx] + output;
        tx += vx[(idx + 2) % 4], ty += vy[(idx + 2) % 4];
      }

      std::cout << "MOVE" << output << std::endl;

      return;
    }
  }


  std::cout << "PASS" << std::endl;

  return;
}

int main() {
  Init();
  while(true) {
    Input();
    /* */
    if (turnType == kPush) {
      std::chrono::system_clock::time_point start, end;
      start = std::chrono::system_clock::now();
      InsertTile();
      end = std::chrono::system_clock::now();
      double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); //処理に要した時間をミリ秒に変換
      std::cerr << "time : " << elapsed << std::endl;
    } else {
      DecideMove();
    }
    // */
  }

  return 0;
}
