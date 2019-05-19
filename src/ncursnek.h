#include "coordinate.h"
#include "direction.h"
#include "mapcontent.h"
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <iterator>
#include <ncurses.h>
#include <string>
#include <thread>
#include <chrono>
#include <map>
#include <random>
#include <deque>
#include <algorithm>

using namespace std;

class ncursnek 
{
public:
  ncursnek();
  void setUpCurses();
  Direction setUpStartWindow();
  void setUpSnek(int, int, mapcontent [][26], deque<Coordinate> &);
  void getWindowSize(int &, int &, int &, int &);
  void initializeGameGrid();
  void refreshScreen(WINDOW *, mapcontent[][26]);
  void gameLoop(WINDOW *, mapcontent[][26], deque<Coordinate> &, Direction);
  bool moveSnek(Direction, mapcontent[][26], deque<Coordinate> &);
  bool collisionCheck(mapcontent[][26], Coordinate, deque<Coordinate> &);
  void eat(mapcontent[][26], Coordinate);
  void addFood(mapcontent[][26]);
  string getUserName(WINDOW *);
  void createHighScoreFile();
  void showHighScores(vector<string>);
  void readHighScores(string);
  void gameOver(WINDOW *);
};

