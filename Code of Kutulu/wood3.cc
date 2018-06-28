#include <iostream>
#include <algorithm>
#include <functional>
#include <map>
#include <queue>
#include <vector>
#include <cmath>
#include <cstring>
#include <chrono>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;



/*************************** 時間計測 ***************************/

// milli seconds
inline std::chrono::system_clock::time_point getTime() {
  return std::chrono::system_clock::now();
}

// print time diff
inline void printTimeDiff (auto start, auto end) {
  std::cerr << "time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << endl;
}

/***************************************************************/



/********************** 各ユニットのクラス **********************/

class Unit {
 public:
  Unit() {}
  Unit(int x, int y) : x_(x), y_(y) {}

  int getX() {
    return this->x_;
  }
  int getY() {
    return this->y_;
  }
  void setX(int x) {
    this->x_ = x;
  }
  void setY(int y) {
    this->y_ = y;
  }

 private:
  int x_, y_;
};

class Explorer : public Unit {
 public:
  Explorer() {}
  Explorer(int x, int y, int sanity, int id) : Unit(x, y), sanity_(sanity), id_(id) {}

  int getSanity() {
    return this->sanity_;
  }
  int getId() {
    return this->id_;
  }
  void setSanity(int sanity) {
    this->sanity_ = sanity;
  }
  void setId(int id) {
    this->id_ = id;
  }

 private:
  int sanity_, id_;
};

// param1 の値は保持していない
class Wanderer : public Unit {
 public:
  Wanderer() {}
  Wanderer(int x, int y, int remaining_time, int id, int target) : Unit(x, y), remaining_time_(remaining_time), id_(id), target_(target) {}

  int getRemainingTime() {
    return this->remaining_time_;
  }
  int getId() {
    return this->id_;
  }
  int getTarget() {
    return this->target_;
  }
  void setRemainingTime(int remaining_time) {
    this->remaining_time_ = remaining_time;
  }
  void setId(int id) {
    this->id_ = id;
  }
  void setTarget(int target) {
    this->target_ = target;
  }

 private:
  int remaining_time_, id_, target_;
};

// param1 の値は保持していない
class Spawning : public Unit {
 public:
  Spawning() {}
  Spawning(int x, int y, int remaining_time, int id) : Unit(x, y), remaining_time_(remaining_time), id_(id) {}

  int getRemainingTime() {
    return this->remaining_time_;
  }
  int getId() {
    return this->id_;
  }
  void setRemainingTime(int remaining_time) {
    this->remaining_time_ = remaining_time;
  }
  void setId(int id) {
    this->id_ = id;
  }

 private:
  int remaining_time_, id_;

};

/*******************************************************/



/********************* 各種変数 ************************/

int height, width, dummy[4]; // dummy は捨てる用
int exp_num, wand_num, spawn_num, unit_num; // unit_num は全ユニット数
std::string maps[20]; // マップ
Explorer explorer[4]; // 0 番目が自分
Wanderer wanderer[16]; // 這い寄ってくるやつ
Spawning spawning[8]; // wanderer のスポーン位置

// 上下左右
const int vx[4] = { 0, 1, 0, -1 }, vy[4] = { 1, 0, -1, 0 };
// マップ上の要素
constexpr char Wall = '#';
constexpr char Spawn = 'w';
constexpr char Empty = '.';

/******************************************************/



void Init() {
  // width と height を読み込む
  cin >> width >> height;
  // マップを読み込む
  for (int i = 0; i < height; ++i) {
    cin >> maps[i];
  }
  // dummy
  for (int i = 0; i < 4; ++i) {
    cin >> dummy[i];
  }
}

void Input() {
  exp_num = wand_num = spawn_num = 0;

  cin >> unit_num;
  for (int i = 0; i < unit_num; ++i) {
    std::string type;
    int id, x, y, param[3];
    cin >> type >> id >> x >> y;
    for (int i = 0; i < 3; ++i) {
      cin >> param[i];
    }

    if (type == "EXPLORER") {
      explorer[exp_num++] = Explorer(x, y, param[0], id);
    }
    else if (type == "WANDERER") {
      wanderer[wand_num++] = Wanderer(x, y, param[0], id, param[2]);
    }
  }
}

inline int getManhattanDistance(Unit a, Unit b) {
  return abs(a.getX() - b.getX()) + abs(a.getY() - b.getY());
}

// 自キャラクタに最も近い EXPLORER のインデックスを返す (id ではないことに注意)
int nearestExplorer() {
  int ret = 0, min_dist = 1000;

  for (int i = 1; i < exp_num; ++i) {
    int tmp = getManhattanDistance(explorer[0], explorer[i]);
    
    if (tmp < min_dist) {
      min_dist = tmp;
      ret = i;
    }
  }

  return ret;
}

// 自キャラクタに最も近い WANDERER のインデックスを返す (id ではないことに注意)
int nearestWanderer() {
  int ret = 0, min_dist = 1000;

  for (int i = 0; i < wand_num; ++i) {
    int tmp = getManhattanDistance(explorer[0], wanderer[i]);
    
    if (tmp < min_dist) {
      min_dist = tmp;
      ret = i;
    }
  }

  return ret;
}

// unit から最も遠い座標を返す (ただし, 敵のいる方向へは向かわないようにする)
std::pair<int, int> getFurtherestPoint(Unit& unit) {
  std::pair<int, int> ret(explorer[0].getX(), explorer[0].getY());
  int max_distance_from_unit = 0; // unit から最も遠い距離
  int max_distance_between_unit_and_me = getManhattanDistance(unit, explorer[0]);

  // bfs で最も遠い座標を探す
  std::queue<std::pair<int, int> > que({ ret });
  bool used[24][24] = { { false } };
  used[ret.first][ret.second] = true;

  while (!que.empty()) {
    std::pair<int, int> now_point = que.front();
    que.pop();

    for (int i = 0; i < 4; ++i) {
      int nx = now_point.first + vx[i], ny = now_point.second + vy[i];
      int tmp_distance_from_unit = getManhattanDistance(unit, Unit(nx, ny));

      if (0 <= nx && nx < height && 0 <= ny && ny < width && 
          maps[nx][ny] == Empty && !used[nx][ny] && tmp_distance_from_unit > max_distance_from_unit &&
          tmp_distance_from_unit > max_distance_between_unit_and_me) {
        ret = { nx, ny };
        max_distance_from_unit = max_distance_between_unit_and_me = tmp_distance_from_unit;
        que.push({ nx, ny });
        used[nx][ny] = true;
      }
    }
  }

  return ret;
}

void Think() {
  Unit target = explorer[0];

  if (wand_num == 0) {
    target = explorer[nearestExplorer()];
  }
  else {
    Unit nearest_wanderer = wanderer[nearestWanderer()];

    std::pair<int, int> furtherest_point = getFurtherestPoint(nearest_wanderer);
    target.setX(furtherest_point.first);
    target.setY(furtherest_point.second);
  }

  cout << "MOVE " << target.getX() << " " << target.getY() << "\n";
}

int main() {
  Init();

  while(true) {
    Input();
    Think();
    cout.flush();
  }

  return 0;
}