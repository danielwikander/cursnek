#include "coordinate.h"
#include "direction.h"
#include "mapcontent.h"
#include "hhscore.h"
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
  void setUpSnek(int, int, mapcontent [][20], deque<Coordinate> &);
  void getTerminalSize(int &, int &, int &, int &);
  void initializeGameGrid();
  void gameLoop(WINDOW *, mapcontent[][20], deque<Coordinate> &, Direction);
  bool moveSnek(Direction, mapcontent[][20], deque<Coordinate> &);
  bool collisionCheck(mapcontent[][20], Coordinate, deque<Coordinate> &);
  void eat(mapcontent[][20], Coordinate);
  void addFood(mapcontent[][20]);
  void refreshScreen(WINDOW *, mapcontent[][20]);
  void pauseScreen();
  string getUserName(WINDOW *);
  void writeScore(string);
  void showSidebarHighScores();
  void showGameOverWindow();
  vector<string> readHighScores();
  void gameOver(WINDOW *);
};

