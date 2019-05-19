#include "ncursnek.h"

// Window and gamegrid size variables
const int sizex = 60;
const int sizey = 30;
const int gamegridxsize = sizex - 2;
const int gamegridysize = sizey - 4;

// The users score
int score = 0;

// The mapgrid
mapcontent gamemap[gamegridxsize][gamegridysize];

// Structure for storing and comparing highscores
struct hhscore {
  int hscore;
  string hsname;
  bool operator<(const hhscore &rhs) const { return hscore > rhs.hscore; }
};

// Initializes the game
ncursnek::ncursnek() {
  setUpCurses();
  initializeGameGrid();

  // Create snek and place in the middle of the gamemap
  deque<Coordinate> snekCoordinates;
  setUpSnek(sizex / 2 - 2, sizey / 2 - 2, gamemap, snekCoordinates);

  // Present start window and get initial direction for snek
  Direction startdir = setUpStartWindow();

  // Create a window for the gamemap
  WINDOW *gamewin = newwin(sizey - 2, sizex, 1, 2);
  nodelay(gamewin, true);
  notimeout(gamewin, true);
  box(gamewin, 0, 0);

  // Start the gameloop
  gameLoop(gamewin, gamemap, snekCoordinates, startdir);
}

// Initializes ncurses
void ncursnek::setUpCurses() {
  initscr();            // Initiate curses
  cbreak();             // Line buffering disabled
  keypad(stdscr, true); // Allows keyinputs
  noecho();             // Don't echo keyinputs
  curs_set(0);
}

// Gets the size of the terminal window.
// Can be used to create dynamically sized games
// Currently unused
void ncursnek::getWindowSize(int &beg_y, int &beg_x, int &max_y, int &max_x) {
  getbegyx(stdscr, beg_y, beg_x);
  getmaxyx(stdscr, max_y, max_x);
}

// Initializes the gamegrid 
void ncursnek::initializeGameGrid() {
  int row, col;
  for (row = 0; row < gamegridxsize; row++) {
    for (col = 0; col < gamegridysize; col++) {
      gamemap[row][col] = EMPTY;
    }
  }
}

// Displays a startup splashscreen, returns a starting direction.
Direction ncursnek::setUpStartWindow() {
  WINDOW *startwin = newwin(sizey - 2, sizex, 1, 2);
  notimeout(startwin, true);
  mvwprintw(startwin, 10, 18, "NCURSNEK");
  mvwprintw(startwin, 12, 18, "Move with wasd / hjkl.");
  mvwprintw(startwin, 13, 18, "Exit with q.");
  mvwprintw(startwin, 15, 18, "Press any key to start.");
  box(startwin, 0, 0);
  wrefresh(startwin);

  // Wait for user input to start game
  Direction startdir = UP;
  char c = wgetch(startwin);
  switch (c) {
	case 'a': case 'h':
      startdir = LEFT;
	  break;
	case 's': case 'j':
	  startdir = DOWN;
	  break;
	case 'd': case 'l':
  	  startdir = RIGHT;
	  break;
	case 'w': case 'k':
      startdir = UP;
	  break;
  }
  return startdir;
}

void ncursnek::setUpSnek(int startx, int starty,
                         mapcontent gamemap[][gamegridysize],
                         deque<Coordinate> &snekCoordinates) {
  gamemap[startx][starty] = SNEK;
  snekCoordinates.push_front(Coordinate(startx, starty));
}

void ncursnek::refreshScreen(WINDOW *win, mapcontent gamemap[][gamegridysize]) {
  box(win, 0, 0);
  for (int row = 0; row < gamegridysize; row++) {
    for (int col = 0; col < gamegridxsize; col++) {
      mapcontent coordinateContent = gamemap[col][row];
      if (coordinateContent == EMPTY) {
        mvwprintw(win, row + 1, col + 1, " ");
      } else if (coordinateContent == FOOD) {
        mvwprintw(win, row + 1, col + 1, "@");
      } else if (coordinateContent == SNEK)
        mvwprintw(win, row + 1, col + 1, "S");
    }
  }
  string scoreString = "SCORE: ";
  scoreString.append(to_string(score));
  mvwprintw(win, 0, 2, scoreString.c_str());
  wrefresh(win);
}

void ncursnek::gameLoop(WINDOW *gamewin, mapcontent gamemap[][gamegridysize],
                        deque<Coordinate> &snekCoordinates, Direction startdir) {
  Direction currentDirection = startdir;
  Direction newDirection = startdir;

  addFood(gamemap);

  while (true) {
    // TODO: Incorporate timer function for consistent performance?
    // auto start_time = chrono::high_resolution_clock::now();
    char c = wgetch(gamewin);
    this_thread::sleep_for(chrono::milliseconds(100));
    switch (c) {
    case 'q':
      return;
    case 'h': case 'a': // Navigate left
      if (currentDirection != RIGHT)
        newDirection = LEFT;
      else
        newDirection = currentDirection;
      break;
    case 'j': case 's': // Navigate down
      if (currentDirection != UP)
        newDirection = DOWN;
      else
        newDirection = currentDirection;
      break;
    case 'k': case 'w': // Navigate up
      if (currentDirection != DOWN)
        newDirection = UP;
      else
        newDirection = currentDirection;
      break;
    case 'l': case 'd': // Navigate right
      if (currentDirection != LEFT)
        newDirection = RIGHT;
      else
        newDirection = currentDirection;
      break;
    }
    if (!moveSnek(newDirection, gamemap, snekCoordinates)) {
      break;
    }
    // Checks the speed of the step and waits if necessary
    // auto end_time = std::chrono::high_resolution_clock::now();
    // auto time = end_time - start_time;
    // auto timems = time/std::chrono::milliseconds(1);
    // if (timems < 150) {
    //	this_thread::sleep_for(chrono::milliseconds(150 - timems));
    //}
    currentDirection = newDirection;
    refreshScreen(gamewin, gamemap);
  }
  gameOver(gamewin);
}

bool ncursnek::moveSnek(Direction dir, mapcontent gamemap[][gamegridysize],
                        deque<Coordinate> &snekCoordinates) {
  bool validMove = false;
  Coordinate currentHead = snekCoordinates.front();
  //  Coordinate currentTail = snekCoordinates.back();
  Coordinate newhead(0, 0);

  if (dir == UP) {
    newhead = Coordinate(currentHead.x, currentHead.y - 1);
  } else if (dir == DOWN) {
    newhead = Coordinate(currentHead.x, currentHead.y + 1);
  } else if (dir == LEFT) {
    newhead = Coordinate(currentHead.x - 1, currentHead.y);
  } else if (dir == RIGHT) {
    newhead = Coordinate(currentHead.x + 1, currentHead.y);
  }

  // Checks for food and fatal collisions
  if (!collisionCheck(gamemap, newhead, snekCoordinates)) {
    validMove = true;
    gamemap[newhead.x][newhead.y] = SNEK;
    snekCoordinates.push_front(newhead);
  }
  return validMove;
}

bool ncursnek::collisionCheck(mapcontent gamemap[][gamegridysize],
                              Coordinate nextStep,
                              deque<Coordinate> &snekCoordinates) {
  bool fatalCollision = false;
  mapcontent nextStepContent = gamemap[nextStep.x][nextStep.y];

  if (nextStep.x < 0 || nextStep.y < 0 || nextStep.x >= gamegridxsize ||
      nextStep.y >= gamegridysize) { // Out of bounds
    fatalCollision = true;
  } else if (nextStepContent == SNEK) { // Self collision
    fatalCollision = true;
  } else if (nextStepContent == FOOD) { // Eat, score++
    eat(gamemap, nextStep);
    fatalCollision = false;
  } else if (nextStepContent == EMPTY) { // No collision, keep moving
    fatalCollision = false;
    Coordinate tail =
        Coordinate(snekCoordinates.back().x, snekCoordinates.back().y);
    gamemap[tail.x][tail.y] = EMPTY;
    snekCoordinates.pop_back();
  }
  return fatalCollision;
}

void ncursnek::eat(mapcontent gamemap[][gamegridysize], Coordinate foodToEat) {
  gamemap[foodToEat.x][foodToEat.y] = EMPTY;
  score += 100;
  addFood(gamemap);
}

void ncursnek::addFood(mapcontent gamemap[][gamegridysize]) {
  int randRow, randCol;
  do {
    randRow = rand() % gamegridxsize;
    randCol = rand() % gamegridysize;
  } while (gamemap[randRow][randCol] != EMPTY);
  gamemap[randRow][randCol] = FOOD;
}

void ncursnek::gameOver(WINDOW *win) {
  string scorestring = "FINAL SCORE: ";
  scorestring.append(to_string(score));
  mvwprintw(win, 10, 15, scorestring.c_str());
  wrefresh(win);

  WINDOW *namewin = newwin(3, 25, 13, 16);
  nodelay(namewin, true);
  notimeout(namewin, true);
  box(namewin, 0, 0);
  mvwprintw(namewin, 0, 1, "Enter your name:");
  wmove(namewin, 1, 1);
  wrefresh(namewin);
  string username = getUserName(namewin);

  readHighScores(username);
}

string ncursnek::getUserName(WINDOW *namewin) {
  string input;
  nodelay(namewin, false);
  cbreak();
  echo();
  curs_set(1);
  char ch = wgetch(namewin);
  while (ch != '\n') {
    if (ch == '\b') {
      input.pop_back();
    }
    input.push_back(ch);
    ch = wgetch(namewin);
    wrefresh(namewin);
  }
  cbreak();
  noecho();
  return input;
}

void ncursnek::readHighScores(string username) {
  // Creates highscore file and opens streams
  createHighScoreFile();
  ifstream hf_in("highscores.txt", ios::in);
  ofstream hf_out("highscores.txt", ios::app);

  // Adds current users score to file
  if (hf_out.is_open()) {
    string hscore = to_string(score);
    hscore.append(" ");
    hscore.append(username);
    hf_out << hscore << endl;
  }

  // Reads file and sorts scores
  vector<string> topscores;
  vector<hhscore> scoresInFile;
  if (hf_in.is_open()) {
    string line;
    string hsname;
    int hsscore;
    while (getline(hf_in, line)) {
      istringstream iss(line);
      iss >> hsscore >> hsname;
      hhscore thisscore = {hsscore, hsname};
      scoresInFile.push_back(thisscore);
    }

    // Sort scores.
    sort(scoresInFile.begin(), scoresInFile.end());

    // Place sorted scores in vector
    for (hhscore score : scoresInFile) {
      string scorestring = to_string(score.hscore);
      scorestring.append(" ");
      scorestring.append(score.hsname);
      scorestring.append("\n");

      topscores.push_back(scorestring);
    }
  }
  hf_in.close();
  hf_out.close();
  showHighScores(topscores);
}

void ncursnek::createHighScoreFile() {
  ofstream createFile;
  string fileName = "../highscores.txt";
  createFile.open(fileName, ios::app);
  createFile << " ";
  createFile.close();
}

void ncursnek::showHighScores(vector<string> topscores) {
  WINDOW *scorewin = newwin(12, 36, 8, 15);
  nodelay(scorewin, true);
  notimeout(scorewin, true);
  int scoresToShow = 10;
  if (topscores.size() < 10) {
    scoresToShow = topscores.size();
  }
  for (int i = 0; i < scoresToShow; i++) {
    mvwprintw(scorewin, i + 1, 1, topscores.at(i).c_str());
  }
  box(scorewin, 0, 0);
  mvwprintw(scorewin, 0, 1, "TOP 10:");
  mvwprintw(scorewin, 11, 20, "Press q to exit");
  curs_set(0);
  wrefresh(scorewin);
  // wait for user to exit
  timeout(-1);
  while (char c = wgetch(scorewin)) {
    if (c == 'q') {
      return;
    } else {
      this_thread::sleep_for(chrono::milliseconds(200));
    }
  }
}

int main() {
  ncursnek spp;
  endwin(); // Deallocates memory and ends curses
  return 0;
}
