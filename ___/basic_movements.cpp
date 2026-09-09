#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;

class Tools {
  public:
    struct coord {
      int x;
      int y;
    };
    
    struct sprite {
      char p;
      char hb;
      char vb;
      char cb;
      char def;
    };
    
    static void input_setup() {
      termios t;
      tcgetattr(STDIN_FILENO, &t);
      t.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &t);
      fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    }

    static char getch() {
      char c;
      if (read(STDIN_FILENO, &c, 1) > 0)
          return c;
      return 0;
    }
    
    static void printC(char c, int color = 0) {
      cout << "\033[" << color << "m" << c << "\033[0m";
    }
    /*
    30 = Black
    31 = Red
    32 = Green
    33 = Yellow
    34 = Blue
    35 = Magenta
    36 = Cyan
    37 = White
    */
    
    static void printS(string s, int color = 0) {
      cout << "\033[" << color << "m" << s << "\033[0m" << "\n";
    }
    /*
    40 = Black
    41 = Red
    42 = Green
    43 = Yellow
    44 = Blue
    45 = Magenta
    46 = Cyan
    47 = White
    */
};

class Game : Tools {
  private:
    coord SIZE;
    coord p;
    sprite s;
    bool game_over;
    
  public:
    Game () :
      Game (32, 32, 16, 16) {}
      
    Game (int B, int H) :
      Game (B, H, B/2, H/2) {}
      
    Game (int B, int H, int x, int y) :
      SIZE{B, H},
      p{x, y},
      s{'O' ,'|', '-', '+', ' '},
      game_over{false}
      {}
      
    void Draw() {
      for(int i = 0; i < SIZE.y; i++) {
        for(int j = 0; j < SIZE.x; j++) {
          
          if(i == p.y
          && j == p.x)
            printC(s.p, 34);
          
          else if((i == 0 && j == 0)
          || (i == 0 && j == SIZE.x - 1)
          || (i == SIZE.y - 1 && j == 0)
          || (i == SIZE.y - 1 && j == SIZE.x - 1))
            printC(s.cb, 32);
          else if(i == 0 || i == SIZE.y - 1)
            printC(s.vb, 32);
          else if(j == 0 || j == SIZE.x - 1)
            printC(s.hb, 32);

          else 
            cout << s.def;
            
          cout << " ";
        }
        cout << "\n";
      }
    }
    
    
    
    void Input() {
      char input = getch();

      if (input == 'w' && p.y > 1)
          p.y--;
      else if (input == 's' && p.y < SIZE.y - 2)
          p.y++;
      else if (input == 'a' && p.x > 1)
          p.x--;
      else if (input == 'd' && p.x < SIZE.x - 2)
          p.x++;
      else if(input == 'q')
          game_over = true;
    }
    
    void Loop() {
      input_setup();
      while(!game_over){
        system("clear");
        Input();
        Draw();
        printS("GAME", 41);
        cout << "\n";
      }
    }
};

int main(){
  Game game = Game(20,20,1,1);
  game.Loop();
}
