#include "LedControl.h"
#define clk 13
#define cs 10
#define mosi 11
#define downBtn 8
#define upBtn 7
#define leftBtn 6
#define rightBtn 5 
#define RATE 200

void(* resetFunc) (void) = 0;

LedControl disp = LedControl(11,13,10,1);

class coords {
  uint8_t x;
  uint8_t y;
  public:
  coords() : x(0), y(0) {}
  coords(uint8_t x1, uint8_t y1) : x(x1), y(y1) {}
  uint8_t getx() {return x;}
  uint8_t gety() {return y;}
  void setx(uint8_t x1) {x = x1;}
  void sety(uint8_t y1) {y = y1;}
  bool equals(coords right) {
    if(x == right.getx() && y == right.gety()) return true;
    return false;
  }
};

class node {
  public:
  coords value;
  node *next;
  node() : value(0,0) {
    next = NULL;
  }
  node(uint8_t x, uint8_t y) : value(x,y) {
    next = NULL;
  }
};

class Snake {
  node *head;
  node *tail;
  public:
  Snake(coords h, coords t) {
    head = new node(h.getx(), h.gety());
    tail = new node(t.getx(), t.gety());
    tail->next = head;
  }

  void addNode(coords loc) {
    node *temp = new node(loc.getx(), loc.gety());
    head->next = temp;
    head = temp;
  }

  void deleteTail() {
    node *temp = tail;
    tail = tail->next;
    delete temp;
  }

  coords get_new_location(int direction) {
    coords newloc;
    switch(direction) {
      case 0:
        newloc = coords((head->value.getx())?head->value.getx()-1:7, head->value.gety());
        break;
      case 1:
        newloc = coords(head->value.getx(), (head->value.gety()+1)%8);
        break;
      case 2:
        newloc = coords(((head->value.getx()+1)%8), head->value.gety());
        break;
      case 3:
        newloc = coords(head->value.getx(), (head->value.gety())?head->value.gety()-1:7);
        break;
    }
    return newloc;
  }

  void move(coords newloc, bool grow) {
    addNode(newloc);
    if(!grow)
      deleteTail();
  }

  bool is_snake_node(coords query) {
    node *ptr = tail;
    while(ptr!=NULL) {
      if(ptr->value.equals(query))
        return true;
      ptr = ptr->next;
    }
    return false;
  }

  void render() {
    disp.clearDisplay(0);
    Serial.println("New Frame");
    node *ptr = tail;
    while(ptr!=NULL) {
      coords coord = ptr->value;
      disp.setLed(0,coord.getx(),coord.gety(), true);
      Serial.print(coord.getx());
      Serial.print(":");
      Serial.println(coord.gety());
      ptr = ptr->next;
    }
  }

  void kill() {
    uint8_t i = 0;
    Serial.println("Killed Snake");
    while(i!=4) {
      delay(500);
      disp.clearDisplay(0);
      delay(500);
      render();
      i++;
    }
  }
};

class Food {
  coords location;
  public:
  Food(uint8_t x, uint8_t y) : location(x,y) {}
  coords get_location() {
    return location;
  }
  void set_new_location() {
    uint8_t x = random(0,8);
    uint8_t y = random(0,8);
    location.setx(x);
    location.sety(y);
  }
  void render() {
    disp.setLed(0, location.getx(), location.gety(), true);
  }
};

coords spawn1(1,2);
coords spawn2(1,3);
Snake snake(spawn1, spawn2);
Food food(4,4);
unsigned long currentMillis;
unsigned long prevMillis = 0;
int direction=3;

void setup() {
  Serial.begin(9600);
  pinMode(upBtn,INPUT_PULLUP);
  pinMode(downBtn,INPUT_PULLUP);
  pinMode(leftBtn,INPUT_PULLUP);
  pinMode(rightBtn,INPUT_PULLUP);
  disp.shutdown(0,false);
  disp.setIntensity(0,1);
  randomSeed(analogRead(A0));
}

void loop() {
  currentMillis = millis();
  if(!digitalRead(upBtn) && direction!=3) direction = 1;
  if(!digitalRead(downBtn) && direction != 1) direction = 3;
  if(!digitalRead(rightBtn) && direction != 0) direction = 2;
  if(!digitalRead(leftBtn) && direction != 2) direction = 0;
  if(currentMillis - prevMillis > RATE) {
    prevMillis = currentMillis;
    coords newcoords = snake.get_new_location(direction);
    if(snake.is_snake_node(newcoords)) {
      snake.kill();
      resetFunc();
    }
    if(newcoords.equals(food.get_location())) {
      snake.move(newcoords, true);
      food.set_new_location();
    } else {
      snake.move(newcoords, false);
    }
    snake.render();    
    food.render();
  }
}
