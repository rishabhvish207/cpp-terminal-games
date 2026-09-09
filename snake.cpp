#include <iostream>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <random>
using namespace std;

class Tools {
  std::mt19937 gen{std::random_device{}()};
  public:
    struct coord {
      int x;
      int y;
    };
    
    struct sprite {
      char head;
      char body;
      char fruit;
      char obstacle;
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
      cout << "\033[" << color << "m" << s << "\033[0m";
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
    
    int random(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }
};

class Snake : Tools {
  private:
    coord SIZE;
    coord head;
    coord fruit;
    sprite s;
    vector <coord> body;
    int len;
    int dir;
    int initDir;
    int score;
    bool game_over;
    
  public:
    Snake () :
      Snake (32, 32, 16, 16, 1, 1) {}
      
    Snake (int B, int H) :
      Snake (B, H, B/2, H/2, 1, 1) {}
      
    Snake (int B, int H, int x, int y, int l, int d) :
      SIZE{B, H},
      head{x, y},
      len{l},
      initDir(d),
      s{'O', 'o' , '*', '#', '|', '-', '+', ' '}
      {}
      
    void Draw() {
      for(int i = 0; i < SIZE.y; i++) {
        for(int j = 0; j < SIZE.x; j++) {
          
          bool drawn = false;
          bool virtBody = false;
          if(i == head.y && j == head.x) {
              printC(s.head, 34);
              drawn = true;
          }
          else {
            bool leftM = false;
            bool rightM = false;
            bool midM = false;
            for(coord part : body) {
              if(i == part.y) {
                if(j == part.x || j == head.x) {
                midM = true;
              }
              if(j == part.x + 1 || j == head.x + 1) {
                rightM = true;
              }
              }
              if(i == part.y && j == part.x) {
                printC(s.body, 34);
                drawn = true;
                break;
              }
            }
            if(midM && (leftM || rightM)) virtBody = true;
          }
          
          if(!drawn) {
              if((i == 0 && j == 0)
              || (i == 0 && j == SIZE.x - 1)
              || (i == SIZE.y - 1 && j == 0)
              || (i == SIZE.y - 1 && j == SIZE.x - 1))
                  printC(s.cb, 42);
              else if(i == 0 || i == SIZE.y - 1)
                  printC(s.vb, 42);
              else if(j == 0 || j == SIZE.x - 1)
                  printC(s.hb, 42);
              else if(i == fruit.y && j == fruit.x)
                  printC(s.fruit, 35);
              else
                  printC(s.def, 47);
          }
          if(virtBody) {
            printC(s.body, 44);
          }
          else if((i == 0 || i == SIZE.y - 1) && j != SIZE.x - 1)
            printC(s.vb, 42);
          else if(j != SIZE.x - 1)
            printC(s.def, 47);
          else
            cout << ' ';
        }
        cout << "\n";
      }
    }
    
    void Input() {
      char input = getch();

      if (input == 'w' && dir != 2 && initDir != 2){
        dir = 1;
      }
      else if (input == 's' && dir != 1 && initDir != 1){
        dir = 2;
      }
      else if (input == 'a' && dir != 4 && initDir != 4){
        dir = 3;
      }
      else if (input == 'd' && dir != 3 && initDir != 3){
        dir = 4;
      }
      else if(input == 'q')
          GameOver();
          
    }
    
    void Logic() {
      if(head.x <= 0
      || head.y <= 0
      || head.x >= SIZE.x-1
      || head.y >= SIZE.y-1) {
        GameOver();
      }
      for(coord parts: body){
        if(parts.x == head.x
        && parts.y == head.y)
        GameOver();
      }
      if(head.x == fruit.x
      && head.y == fruit.y) {
        Eat();
      }
    }
    
    void Move() {
      if(dir != 0){
        initDir = 0;
        body.insert(body.begin(), head);
      }
  
      switch (dir) {
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
      if (body.size() > len - 1)
        body.pop_back();
    }
    
    void Loop() {
      Setup();
      do{
        system("clear");
        Input();
        Move();
        Logic();
        Stats();
        Draw();
        usleep(200000);
      } while(!game_over);
    }
    void Setup() {
      input_setup();
      game_over = false;
      dir = 0;
      score = 0;
      switch(initDir) {
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
          initDir = 1;
          Setup();
          return;
      }
      SpawnFruit();
    }
    
    void GameOver() {
      game_over = true;
      usleep(300000);
      cout << '\n';
      printS("GAME OVER!", 45);
      cout << "\n\n";
    }
    
    void SpawnFruit() {
      fruit.x = random(1, SIZE.x-2);
      fruit.y = random(1, SIZE.y-2);
      
      if(fruit.x == head.x
      && fruit.y == head.y) {
        SpawnFruit();
        return;
      }
      for(coord parts: body){
        if(parts.x == fruit.x
        && parts.y == fruit.y){
          SpawnFruit();
          return;
        }
      }
    }
    
    void Eat() {
      SpawnFruit();
      len++;
      score+=10;
    }
    
    void Stats() {
      string scoreS = (game_over ? "FINAL SCORE : ": " SCORE : ") + to_string(score) + " ";
      printS(scoreS, 44);
      cout << '\n';
    }
};

int main(){
  Snake game = Snake(30,30,10,10,11, 1);
  //Snake game = Snake();
  game.Loop();
}
