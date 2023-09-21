#include "cursnek.h"

const int WINDOWSIZEX = 56;
const int WINDOWSIZEY = 24;
const int GAMEGRIDXSIZE = WINDOWSIZEX - 2;
const int GAMEGRIDYSIZE = WINDOWSIZEY - 4;
const int SNEKSPEED = 100; // ms between snek movements
int score = 0;             // Users score
mapcontent gamemap[GAMEGRIDXSIZE][GAMEGRIDYSIZE]; 

// TODO: Separate highscore code,
//       Make snek blink on death

/*
  Initializes the game.
*/
cursnek::cursnek() 
{
  setUpCurses();
  initializeGameGrid();

  // Create snek and place in the middle of the gamemap
  deque<Coordinate> snekCoordinates;
  setUpSnek(WINDOWSIZEX / 2 - 2, WINDOWSIZEY / 2 - 2, gamemap, snekCoordinates);

  // Present start window and get initial direction for snek
  Direction startdir = setUpStartWindow();

  // Create a window for the gamemap
  WINDOW *gamewin = newwin(WINDOWSIZEY - 2, WINDOWSIZEX, 1, 2);
  nodelay(gamewin, true);
  notimeout(gamewin, true);
  box(gamewin, 0, 0);

  // Show highscore sidebar
  showSidebarHighScores();

  // Start the gameloop
  gameLoop(gamewin, gamemap, snekCoordinates, startdir);
}

/*
   Initializes ncurses.
 */
void cursnek::setUpCurses() 
{
  initscr();            // Initiate curses
  cbreak();             // Line buffering disabled
  keypad(stdscr, true); // Allows keyinputs
  noecho();             // Don't echo keyinputs
  curs_set(0);          // Make cursor invisible
}

/*
  Gets the size of the terminal window.
  Can be used to create dynamically sized games.
  Currently unused.
*/ 
void cursnek::getTerminalSize(int &beg_y, int &beg_x, 
                               int &max_y, int &max_x) 
{
  getbegyx(stdscr, beg_y, beg_x);
  getmaxyx(stdscr, max_y, max_x);
}

/*
  Initializes the gamegrid.
*/
void cursnek::initializeGameGrid() 
{
  int row, col;
  for (row = 0; row < GAMEGRIDXSIZE; row++) {
    for (col = 0; col < GAMEGRIDYSIZE; col++) {
      gamemap[row][col] = EMPTY;
    }
  }
}

/* 
  Displays a startup splashscreen, returns a starting direction.
*/
Direction cursnek::setUpStartWindow() 
{
  WINDOW *startwin = newwin(WINDOWSIZEY - 2, WINDOWSIZEX, 1, 2);
  notimeout(startwin, true);
  mvwprintw(startwin, 7, 17, "cursnek");
  mvwprintw(startwin, 9, 17, "Move with wasd / hjkl.");
  mvwprintw(startwin, 10, 17, "Pause with space.");
  mvwprintw(startwin, 11, 17, "Exit with q.");
  mvwprintw(startwin, 13, 17, "Press any key to start.");
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

void cursnek::pauseScreen() {
  WINDOW *pausewin = newwin(5, 14, 9, 24);
  notimeout(pausewin, true);
  mvwprintw(pausewin,2,4, "PAUSED");
  box(pausewin, 0, 0);
  wrefresh(pausewin);

  wgetch(pausewin);
}

/*
  Places snek in the middle of the gamegrid.
*/
void cursnek::setUpSnek(int startx, int starty,
                         mapcontent gamemap[][GAMEGRIDYSIZE],
                         deque<Coordinate> &snekCoordinates) 
{
  gamemap[startx][starty] = SNEK;
  snekCoordinates.push_front(Coordinate(startx, starty));
}

/*
  Redraws the gamegrid.
*/
void cursnek::refreshScreen(WINDOW *win, 
                             mapcontent gamemap[][GAMEGRIDYSIZE]) 
{
  box(win, 0, 0);
  for (int row = 0; row < GAMEGRIDYSIZE; row++) {
    for (int col = 0; col < GAMEGRIDXSIZE; col++) {

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

/*
  The main gameloop, continuously checks user input, moves snek,
  checks for collision and redraws the gamegrid. 
*/
void cursnek::gameLoop(WINDOW *gamewin, 
                        mapcontent gamemap[][GAMEGRIDYSIZE],
                        deque<Coordinate> &snekCoordinates, 
                        Direction startdir) 
{
  Direction currentDirection = startdir;
  Direction newDirection = startdir;

  addFood(gamemap);

  while (true) {
    // Start looptimer
    auto start_time = chrono::high_resolution_clock::now();
    char c = wgetch(gamewin);
    switch (c) {
    case 'q':
      return;

    case ' ':
      pauseScreen();
      break;

    case 'h': case 'a': // Navigate left
      if (currentDirection != RIGHT)
        newDirection = LEFT;
      else
        newDirection = currentDirection;
      break;

    case 'j':
    case 's': // Navigate down
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
	
    // Tries to move snek. If it collides the loop breaks and the game is over.
    if (!moveSnek(newDirection, gamemap, snekCoordinates)) 
      break;

    currentDirection = newDirection;
    refreshScreen(gamewin, gamemap);

    // Waits the appropriate amount of time until next loop
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;
    auto timems = time / std::chrono::milliseconds(1);
    if (timems < SNEKSPEED)
      this_thread::sleep_for(chrono::milliseconds(SNEKSPEED - timems));
  }
  gameOver(gamewin);
}

/*
  Moves the snek. Returns false if the move was invalid (collision detected).
*/
bool cursnek::moveSnek(Direction dir, 
                        mapcontent gamemap[][GAMEGRIDYSIZE],
                        deque<Coordinate> &snekCoordinates) 
{
  bool validMove = false;
  Coordinate currentHead = snekCoordinates.front();
  Coordinate newhead(0, 0);

  switch (dir) {
    case UP: 
      newhead = Coordinate(currentHead.x, currentHead.y -1);
      break;

    case DOWN:
      newhead = Coordinate(currentHead.x, currentHead.y +1);
      break;

    case LEFT: 
      newhead = Coordinate(currentHead.x -1, currentHead.y);
      break;

    case RIGHT:
      newhead = Coordinate(currentHead.x +1, currentHead.y);
      break;
  }

  // Checks for food and fatal collisions
  if (!collisionCheck(gamemap, newhead, snekCoordinates)) {
    validMove = true;
    gamemap[newhead.x][newhead.y] = SNEK;
    snekCoordinates.push_front(newhead);
  }
  return validMove;
}

/*
  Checks for collisions. 
  If snek collides with itself or the walls, returns true.
  If snek collides with food, score++ and return false.
  If there is no collision (nextStepContent == EMPTY) returns false.
*/
bool cursnek::collisionCheck(mapcontent gamemap[][GAMEGRIDYSIZE],
                              Coordinate nextStep,
                              deque<Coordinate> &snekCoordinates) 
{
  bool fatalCollision = false;
  mapcontent nextStepContent = gamemap[nextStep.x][nextStep.y];

  if (nextStep.x < 0 || nextStep.y < 0 ||  
      nextStep.x >= GAMEGRIDXSIZE || nextStep.y >= GAMEGRIDYSIZE) { // Out of bounds
    fatalCollision = true;
  } else if (nextStepContent == SNEK) {  // Self collision
    fatalCollision = true;
  } else if (nextStepContent == FOOD) {  // Eat, score++
    eat(gamemap, nextStep);
    fatalCollision = false;
  } else if (nextStepContent == EMPTY) { // No collision, keep moving
    fatalCollision = false;
    Coordinate tail = Coordinate(snekCoordinates.back().x, 
                                 snekCoordinates.back().y);
    gamemap[tail.x][tail.y] = EMPTY;
    snekCoordinates.pop_back();
  }
  return fatalCollision;
}

/*
  Snek eats the food, adds score and generates new food.
*/
void cursnek::eat(mapcontent gamemap[][GAMEGRIDYSIZE], 
                   Coordinate foodToEat) 
{
  gamemap[foodToEat.x][foodToEat.y] = EMPTY;
  score += 100;
  addFood(gamemap);
}

/*
  Generate new food on the gamegrid.
*/
void cursnek::addFood(mapcontent gamemap[][GAMEGRIDYSIZE]) 
{
  int randRow, randCol;
  do {
    randRow = rand() % GAMEGRIDXSIZE;
    randCol = rand() % GAMEGRIDYSIZE;
  } while (gamemap[randRow][randCol] != EMPTY);
  gamemap[randRow][randCol] = FOOD;
}

/*
  Game over. Called when snek had a fatal collision.
  Prompts the user to enter their name, and displays 
  the Top 10 highscore list.
*/
void cursnek::gameOver(WINDOW *win) 
{
  string scorestring = "FINAL SCORE: ";
  scorestring.append(to_string(score));
  mvwprintw(win, 8, 16, scorestring.c_str());
  wrefresh(win);

  WINDOW *namewin = newwin(3, 25, 11, 17);
  nodelay(namewin, true);
  notimeout(namewin, true);
  box(namewin, 0, 0);
  mvwprintw(namewin, 0, 1, "Enter your name:");
  wmove(namewin, 1, 1);
  wrefresh(namewin);
  string username = getUserName(namewin);

  writeScore(username);
}

/*
  Prompts the user to enter their name.
*/
string cursnek::getUserName(WINDOW *namewin) 
{
  string input;
  nodelay(namewin, false);
  cbreak();
  noecho();
  curs_set(1);
  int i = 1;
  char ch = wgetch(namewin);
  while (ch != '\n') {
    if (ch == '\b' || ch == 127) {
      if (!input.empty()) {
        input.pop_back();
        mvwprintw(namewin, 1, i - 1, " ");
        wmove(namewin, 1, i - 1);
        i--;
      }
    } else {
      mvwaddch(namewin, 1, i, ch);
      i++;
      input.push_back(ch);
    }
    ch = wgetch(namewin);
    wrefresh(namewin);
  }
  cbreak();
  noecho();
  return input;
}


/*
   Writes current users score to file
 */
void cursnek::writeScore(string username) 
{
  ofstream hf_out("highscores.txt", ios::app);

  // Adds current users score to file
  if (hf_out.is_open()) {
    string hscore = to_string(score);
    hscore.append(" ");
    hscore.append(username);
    hf_out << hscore << endl;
  }
}


/*
  Reads the highscores from the highscore file.
*/
vector<string> cursnek::readHighScores() 
{
  // Creates / Opens highscore file and opens stream
  ifstream hf_in("highscores.txt", ios::in);

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
  return topscores;
}

/*
   Displays the top 10 highscores on a sidebar.
 */
void cursnek::showSidebarHighScores() 
{
  vector<string> topscores = readHighScores();

  WINDOW *sidebarhswin = newwin(12, 20, 1, 58);
  int scoresToShow = 10;
  if (topscores.size() < 10) 
    scoresToShow = topscores.size();
  
  for (int i = 0; i < scoresToShow; i++) 
    mvwprintw(sidebarhswin, i + 1, 1, topscores.at(i).c_str());
  
  box(sidebarhswin, 0, 0);
  mvwprintw(sidebarhswin, 0, 1, "TOP 10:");
  curs_set(0);
  wrefresh(sidebarhswin);
}

/*
   Displays a 'play again?' window that prompts the user
   to quit or start a new game.
 */
bool showPlayAgainWindow()
{
  WINDOW *gameoverwin = newwin(8, 30, 7, 16);
  timeout(-1);
  curs_set(0);
  mvwprintw(gameoverwin,2,4, "GAME OVER");
  mvwprintw(gameoverwin,4,4, "r to restart");
  mvwprintw(gameoverwin,5,4, "any other key to exit");

  box(gameoverwin, 0, 0);
  wrefresh(gameoverwin);

  bool playAgain = false;
  char c = wgetch(gameoverwin);
  if(c == 'r') 
    playAgain = true;

  return playAgain;
}

int main() 
{
  bool playAgain;
  do {
    cursnek spp;
    playAgain = showPlayAgainWindow();
    endwin(); // Ends curses
  } while (playAgain == true); 
  return 0;
}
