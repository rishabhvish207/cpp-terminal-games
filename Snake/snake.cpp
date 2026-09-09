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
    
    struct dir {
      int i = 0;
      int f = 1;
      bool equal(int d) {
        return (i == d) || (f == d);
      }
    };
    
    struct sprite {
      int fgc = 0;
      int bgc = 0;
      char c = ' ';
      void print(int i = 1) {
        cout << "\033[3" << fgc << "m" 
             << "\033[4" << bgc << "m";
        while(i-- > 0)
          cout << c;
        cout << "\033[0m";
      }
    };
    
    struct spriteSet {
      sprite head;
      sprite body;
      sprite fruit;
      sprite obstacle;
      sprite wall;
      sprite border;
      sprite floor;
      sprite def;
    };
    
    static void printC(char c, int fgc = 0, int bgc = 0) {
      cout << "\033[3" << fgc << "m" <<"\033[4" << bgc << "m" << c << "\033[0m";
    }

    static void printS(string s, int fgc = 0, int bgc = 0) {
      cout << "\033[3" << fgc << "m" << "\033[4" << bgc << "m" << s << "\033[0m";
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
    
    int random(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }
};

class Snake : Tools {
  private:
    coord SIZE;
    coord head;
    vector <coord> body;
    vector <coord> obstacle;
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
      Snake (B, H, B/2, H/2, 3, 1) {}
      
    Snake (int B, int H, 
           int x, int y,
           int l, int di) :
      SIZE{B, H},
      head{x, y},
      len{l},
      d{di}
      {}
      
    void Start(){
      for(int i = 0; i <= SIZE.y; i++) {
        for(int j = 0; j <= SIZE.x; j++) {
          
        }
      }
    }

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
      s.obstacle.c = 'X';
      s.obstacle.fgc = 1;
      s.obstacle.bgc = 7;
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
      if (body.size() > len - 1)
        body.pop_back();
    }
    
    void Logic() {
      if(head.x <= 1
      || head.y <= 1
      || head.x >= SIZE.x - 1
      || head.y >= SIZE.y - 1) {
        GameOver();
      }
      for(coord parts: body){
        if(parts.x == head.x
        && parts.y == head.y)
        GameOver();
      }
      for(coord box: obstacle){
        if(box.x == head.x
        && box.y == head.y)
        GameOver();
      }
      if(head.x == fruit.x
      && head.y == fruit.y) {
        Eat();
      }
    }
    
    void Draw() {
    for(int i = 0; i <= SIZE.y; i++) {
        for(int j = 0; j <= SIZE.x; j++) {

            bool isHead = (i == head.y && j == head.x);
            bool isBody = false;
            bool isFruit = (i == fruit.y && j == fruit.x);
            bool isObstacle = false;
            for(coord part : body) {
                if(i == part.y && j == part.x) {
                    isBody = true;
                    break;
                }
            }
            for(coord box : obstacle) {
                if(i == box.y && j == box.x) {
                    isObstacle = true;
                    break;
                }
            }

            if(isHead)
                s.head.print(2);
            else if(isBody) 
                s.body.print(2);
            else if(isFruit)
                s.fruit.print(2);
            else if(isObstacle)
                s.obstacle.print(2);
            else if((i == 0 || i == SIZE.y)
                 || (j == 0 || j == SIZE.x))
                s.border.print(2);
            else if((i == 1 || i == SIZE.y - 1)
                 || (j == 1 || j == SIZE.x - 1))
                s.wall.print(2);
            else
                s.floor.print(2);
        }
        cout << '\n';
      }
    }

    void Stats() {
      s.def.print(2 * (SIZE.x + 1));
      cout << "\n";
      string scoreS = (game_over ? " FINAL SCORE : ": " SCORE : ") + to_string(score) + " ";
      printS(scoreS, 0, 6);
      s.def.print(2 * (SIZE.x + 1) - scoreS.size());
      cout << "\n";
      s.def.print(2 * (SIZE.x + 1));
      cout << "\n";
    }
    
    void Eat() {
      SpawnFruit();
      len++;
      score+=10;
      speed+= 0.5;
    }
    
    void SpawnFruit() {
      fruit.x = random(2, SIZE.x-2);
      fruit.y = random(2, SIZE.y-2);
      
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
      for(coord box: obstacle){
        if(box.x == fruit.x
        && box.y == fruit.y){
          SpawnFruit();
          return;
        }
      }
    }

    void SpawnObstacleX(int x1, int x2, int y){
      for(int i = x1; i < x2; i++)
        obstacle.push_back({i, y});
    }

    void SpawnObstacleY(int y1, int y2, int x){
      for(int j = y1; j < y2; j++)
        obstacle.push_back({x, j});
    }
    
    void GameOver() {
      game_over = true;
      cout << '\n';
      printS("GAME OVER!", 41);
      cout << "\n\n";
    }
    
    void Loop() {
      Setup();
      SpawnObstacleX(1, 20,18);
      do{
        cout << "\033[2J\033[H";
        Input();
        Move();
        Logic();
        Stats();
        Draw();
        usleep(1000000/speed);
      } while(!game_over);
    }
};

int main(){
  //Snake game = Snake(30,30,10,10,11,3);
  Snake game = Snake();
  game.Loop();
  usleep(3000000);
}
