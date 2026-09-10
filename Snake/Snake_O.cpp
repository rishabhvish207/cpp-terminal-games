#include <iostream>
#include <sstream>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <random>
#include <csignal>
#include <cstdlib>
using namespace std;

class Tools {
  std::mt19937 gen{std::random_device{}()};

  static termios orig_termios;
  static bool termios_saved;
  static int curFg;
  static int curBg;

  static void restore_terminal() {
      if (!termios_saved) return;
      tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
  }

  static void signal_handler(int sig) {
      restore_terminal();
      std::signal(sig, SIG_DFL);
      std::raise(sig);
  }

  public:
    static ostream* out;

    struct coord {
      int x;
      int y;
    };

    struct dir {
      int i = 0;
      int f = 1;
      bool equal(int d) {
        return (i == d) || (f == d);
      }
    };

    static void setColor(int fgc, int bgc) {
      if (fgc != curFg) {
        *out << "\033[3" << fgc << "m";
        curFg = fgc;
      }
      if (bgc != curBg) {
        *out << "\033[4" << bgc << "m";
        curBg = bgc;
      }
    }

    static void resetColor() {
      *out << "\033[0m";
      curFg = -1;
      curBg = -1;
    }

    struct sprite {
      int fgc = 0;
      int bgc = 0;
      char c = ' ';
      void print(int i = 1) {
        setColor(fgc, bgc);
        while(i-- > 0)
          *Tools::out << c;
      }
    };

    struct spriteSet {
      sprite head;
      sprite body;
      sprite fruit;
      sprite wall;
      sprite border;
      sprite floor;
      sprite def;
    };

    static void printC(char c, int fgc = 0, int bgc = 0) {
      setColor(fgc, bgc);
      *out << c;
    }

    static void printS(string s, int fgc = 0, int bgc = 0) {
      setColor(fgc, bgc);
      *out << s;
    }

    /*
    Text : n = 3
    Lighter Text : n = 9
    Background : n = 4
    n0 = Black
    n1 = Red
    n2 = Green
    n3 = Yellow
    n4 = Blue
    n5 = Magenta
    n6 = Cyan
    n7 = White
    */

    static void input_setup() {
      tcgetattr(STDIN_FILENO, &orig_termios);
      termios_saved = true;
      atexit(restore_terminal);

      std::signal(SIGSEGV, signal_handler);
      std::signal(SIGABRT, signal_handler);
      std::signal(SIGFPE,  signal_handler);
      std::signal(SIGINT,  signal_handler);
      std::signal(SIGTERM, signal_handler);

      termios t = orig_termios;
      t.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &t);

      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    static char getch() {
      char c;
      if (read(STDIN_FILENO, &c, 1) > 0)
          return c;
      return 0;
    }

    int random(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }
};

termios Tools::orig_termios;
bool Tools::termios_saved = false;
int Tools::curFg = -1;
int Tools::curBg = -1;
ostream* Tools::out = &cout;

class Snake : Tools {
  private:
    static constexpr int MIN_WIDTH = 20;
    static constexpr int MIN_HEIGHT = 12;
    static constexpr int MAX_FRUIT_SPAWN_ATTEMPTS = 10000;

    coord SIZE;
    coord head;
    vector <coord> body;
    coord fruit;
    dir d;
    spriteSet s;
    int len;
    int score;
    float speed;
    bool game_over;

  public:
    Snake () :
      Snake (32, 32, 16, 16, 3, 1) {}

    Snake (int B, int H) :
      Snake (max(B, MIN_WIDTH), max(H, MIN_HEIGHT),
             max(B, MIN_WIDTH)/2, max(H, MIN_HEIGHT)/2, 3, 1) {}

    Snake (int B, int H, 
           int x, int y,
           int l, int di) :
      SIZE{max(B, MIN_WIDTH), max(H, MIN_HEIGHT)},
      head{x, y},
      len{l},
      d{di}
      {}

    void Setup() {
      input_setup();
      switch(d.i) {
        case 1:
          for (int i = 1; i < min(SIZE.y - head.y, len); i++) 
            body.push_back({head.x, head.y + i});
        break;
        case 2:
          for (int i = 1; i < min(head.y, len); i++) 
            body.push_back({head.x, head.y - i});
        break;
        case 3:
          for (int i = 1; i < min(SIZE.x - head.x, len); i++) 
            body.push_back({head.x + i, head.y});
        break;
        case 4:
          for (int i = 1; i < min(head.x, len); i++) 
            body.push_back({head.x - i, head.y});
        break;
        default:
          d.i = 1;
          Setup();
          return;
      }
      game_over = false;
      d.f = 0;
      score = 0;
      speed = 5;
      s.head.c = 'O';
      s.head.fgc = 3;
      s.head.bgc = 0;
      s.body.c = ':';
      s.body.fgc = 0;
      s.body.bgc = 3;
      s.fruit.c = ' ';
      s.fruit.fgc = 1;
      s.fruit.bgc = 1;
      s.wall.bgc = 7;
      s.border.bgc = 5;
      s.floor.bgc = 2;
      s.def.bgc = 4;

      SpawnFruit();
    }

    char Input() {
      char input = getch();

      if (input == 'w' && !d.equal(2)){
        d.f = 1;
      }
      else if (input == 's' && !d.equal(1)){
        d.f = 2;
      }
      else if (input == 'a' && !d.equal(4)){
        d.f = 3;
      }
      else if (input == 'd' && !d.equal(3)){
        d.f = 4;
      }
      else if(input == 'q')
          GameOver();

      return input;
    }

    void Move() {
      if(d.f != 0){
        d.i = 0;
        body.insert(body.begin(), head);
      }

      switch (d.f) {
        case 1 :
          head.y--;
          break;
        case 2 :
          head.y++;
          break;
        case 3 :
          head.x--;
          break;
        case 4 :
          head.x++;
          break;
      }
      if ((int)body.size() > len - 1)
        body.pop_back();
    }

    void Logic() {
      if(head.x <= 1
      || head.y <= 1
      || head.x >= SIZE.x - 2
      || head.y >= SIZE.y - 2) {
        GameOver();
        return;
      }
      for(coord parts: body){
        if(parts.x == head.x
        && parts.y == head.y) {
          GameOver();
          return;
        }
      }
      if(head.x == fruit.x
      && head.y == fruit.y) {
        Eat();
      }
    }

    void Draw() {
      for(int i = 0; i < SIZE.y; i++) {
        for(int j = 0; j < SIZE.x; j++) {

            bool isHead = (i == head.y && j == head.x);
            bool isBody = false;
            bool isFruit = (i == fruit.y && j == fruit.x);
            for(coord part : body) {
                if(i == part.y && j == part.x) {
                    isBody = true;
                    break;
                }
            }

            if(isHead)
                s.head.print(2);
            else if(isBody) 
                s.body.print(2);
            else if(isFruit)
                s.fruit.print(2);
            else if((i == 0 || i == SIZE.y - 1)
                 || (j == 0 || j == SIZE.x - 1))
                s.border.print(2);
            else if((i == 1 || i == SIZE.y - 2)
                 || (j == 1 || j == SIZE.x - 2))
                s.wall.print(2);
            else
                s.floor.print(2);
        }
        resetColor();
        *out << '\n';
      }
    }

    void Stats() {
      s.def.print(2 * SIZE.x);
      resetColor();
      *out << "\n";

      if (game_over) {
        string msg = " GAME OVER!   FINAL SCORE : " + to_string(score) + " ";
        int pad = 2 * SIZE.x - (int)msg.size();
        int left = pad / 2;
        int right = pad - left;
        s.def.print(left);
        printS(msg, 1, 7);
        s.def.print(right);
      } else {
        string scoreS = " SCORE : " + to_string(score) + " ";
        printS(scoreS, 0, 6);
        s.def.print(2 * SIZE.x - (int)scoreS.size());
      }
      resetColor();
      *out << "\n";

      s.def.print(2 * SIZE.x);
      resetColor();
      *out << "\n";
    }

    void Eat() {
      SpawnFruit();
      len++;
      score+=10;
      speed+= 0.5;
    }

    void SpawnFruit() {
      for(int attempt = 0; attempt < MAX_FRUIT_SPAWN_ATTEMPTS; attempt++) {
        fruit.x = random(2, SIZE.x - 3);
        fruit.y = random(2, SIZE.y - 3);

        bool occupied = (fruit.x == head.x && fruit.y == head.y);

        if(!occupied) {
            for(coord part : body) {
                if(fruit.x == part.x && fruit.y == part.y) {
                    occupied = true;
                    break;
                }
            }
        }

        if(!occupied)
            return;
      }
      GameOver();
    }

    void GameOver() {
      game_over = true;
    }

    void Loop() {
      do{
        ostringstream frame;
        out = &frame;
        frame << "\033[2J\033[H";
        
        Input();
        Move();
        Logic();
        Stats();
        Draw();
        
        out = &cout;
        cout << frame.str();
        cout.flush();
        usleep((useconds_t)(1000000/speed));
      } while(!game_over);
      resetColor();
    }
    
    void Play() {
      Setup();
      Loop();
    }

};

int main(){
  Snake game = Snake();
  game.Play();
}
