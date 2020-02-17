#include <FreeRTOS_SAMD21.h> //samd21
#include <U8g2lib.h>
#include <SPI.h>
#include <Bounce2.h>

//**************************************************************************
// global variables
//**************************************************************************

TaskHandle_t Handle_monitorTask;
TaskHandle_t Handle_drawTask;
TaskHandle_t Handle_buttonTask;
TaskHandle_t Handle_gravityTask;
#define WIDTH 10
#define HEIGHT 23

byte field[WIDTH][HEIGHT];
int score = 0;
bool game_over = false;

//**************************************************************************
// Type Defines and Constants
//**************************************************************************

#define  ERROR_LED_PIN  13 //Led Pin: Typical Arduino Board

#define ERROR_LED_LIGHTUP_STATE  LOW // the state that makes the led light up on your board, either low or high

// Select the serial port the project should use and communicate over
// Sombe boards use SerialUSB, some use Serial
#define SERIAL          SerialUSB

#define NUM_BUTTONS 5
#define CS_PIN 9
#define DC_PIN 5
#define RESET_PIN 12

enum Action 
{   up = 0, 
    down = 1, 
    left = 32,
    right = 33,
    drop = 2,
    no = 100
};
enum Angle
{   d0,
    d90,
    d180,
    d270,
};
enum Type
{
  O,
  I,
  L,
  J,
  S,
  Z,
  T
};
class Locker
{
public:
  Locker() {
    taskENTER_CRITICAL(); 
  }
  ~Locker() {
    taskEXIT_CRITICAL();
  }
};
class Figure
{
private:
  int x;//top left corner
  int y;
  Angle angle;
  Type type;
  // Pieces definition
  byte figures [7 /*kind */ ][4 /* rotation */ ][5 /* horizontal blocks */ ][5 /* vertical blocks */ ] =
{
// Square
  {
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0}
    }
   },
 
// I
  {
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 1, 2, 1, 1},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0}, 
    {0, 0, 2, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {1, 1, 2, 1, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 2, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    }
   }
  ,
// L
  {
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 2, 0, 0},
    {0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 1, 2, 1, 0},
    {0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 2, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 1, 0},
    {0, 1, 2, 1, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
    }
   },
// L mirrored
  {
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 2, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 1, 2, 1, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 1, 0},
    {0, 0, 2, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 1, 2, 1, 0},
    {0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0}
    }
   },
// N
  {
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 1, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 1, 2, 0, 0},
    {0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 1, 2, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0}
    },
 
 
 
   {
    {0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
    }
   },
// N mirrored
  {
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 1, 2, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 1, 0},
    {0, 1, 2, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
    }
   },
// T
  {
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 2, 1, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 1, 2, 1, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 1, 2, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
    },
   {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 1, 2, 1, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
    }
   }
};
public:
  Figure() {
    x = 2;
    y = 0;
    angle = d0;
    type = O;
    Locker lock;
    touchCells(true);
    if (isUnder()) {
      game_over = true;
      SERIAL.println("Game over");
    }
  }
  ~Figure() {
//    shiftRows(y+5);
  }
  void shiftRows(int y) {
    Locker lock;
    int lines = 0;
    bool shift = true;
    for(int j = HEIGHT-1; j > 3; --j) {
      if(j>=HEIGHT) continue;
      shift = true;
      for(int i = 0; i < WIDTH; ++i) {
      if(field[i][j] == 0) {
        shift = false;
        break;
      }
      if(shift) {//perform shift
        SERIAL.println("Perform shift");
        for(int k = 0; k < WIDTH; ++k) {
          for(int l = j; l > 2; --l)
          field[k][l] = field[k][l-1];
        }
        lines++;
      }
    }
    switch(lines){
      case 4:
        score += 900;
      case 3:
        score += 400;
      case 2:
        score += 200;
      case 1:
        score += 100;
      }
    }
  }
  bool gravitate() {
    Locker lock;
    bool res = true;
    if(!isUnder()) { 
//      SERIAL.println("Gravitate");
      touchCells(false);
      ++y;
      touchCells(true);
    }
    return !isUnder();
  }
  void touchCells(bool enable) {
//    SERIAL.println("Touch cells");
    Locker lock;
    auto fig = figures[type][angle];
    for(int ly = 0; ly < 5; ++ly) {
      for(int lx = 0; lx < 5; ++lx) {
        if(fig[lx][ly] == 0) continue;
        if((x+lx)<0) continue;
        field[x+lx][y+ly] = (enable ? 1 : 0);
      }
    }
    
  }
  bool moveLeft() {
    auto fig = figures[type][angle];
    int offset = 0;
    while(true) {
      int s = 0;
      for(int i = 0; i < 5; ++i) s += fig[i][offset];
      if(s > 0) break;
      else offset++;
    }
    if(x-1+offset >= 0) {
      SERIAL.println("Move left");
      touchCells(false);
      --x;
      touchCells(true);
      return true;
    }
    return false;
  }
  bool moveRight() {
    auto fig = figures[type][angle];
    int offset = 4;
    while(true) {
      int s = 0;
      for(int i = 0; i < 5; ++i) s += fig[i][offset];
      if(s > 0) break;
      else offset--;
    }
    if((x+1+offset) < WIDTH) {
      SERIAL.println("Move right");
      touchCells(false);
      ++x;
      touchCells(true);
      return true;
    }
    return false;
  }
  bool drop() {
    Locker lock;
    if(isUnder()) return false;
    SERIAL.println("Drop");
    while(gravitate()){}; //implement better solution later
    return true;
  }
  bool isUnder() {
    auto fig = figures[type][angle];
    int offset = 4;
    while(true) {
      int s = 0;
      for(int i = 0; i < 5; ++i) s += fig[i][offset];
      if(s > 0) break;
      else offset--;
    }
    if(y + offset == HEIGHT-1) {
      SERIAL.println("standing on the bottom");
      return true;
    }
    Locker lock;
    for(int ly = 0; ly < 5; ++ly) {
      for(int lx = 0; lx < 5; ++lx) {
        if(fig[lx][ly] > 0 && lx+x >= 0 && field[lx+x][ly+y+1] == 1 && ly+1<5 && fig[lx][ly+1] == 0) return true;
      }
    }
    return false;
    
  }
};

Figure* fig = nullptr;


//**************************************************************************
// Can use these function for RTOS delays
// Takes into account procesor speed
//**************************************************************************
void myDelayUs(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );  
}

void myDelayMs(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );  
}

void myDelayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, (ms * 1000) / portTICK_PERIOD_US );  
}



//*****************************************************************
// Task will periodicallt print out useful information about the tasks running
// Is a useful tool to help figure out stack sizes being used
//*****************************************************************
void taskMonitor(void *pvParameters)
{
    int x;
    int measurement;
    
    SERIAL.println("Task Monitor: Started");

    // run this task afew times before exiting forever
    for(x=0; x<10; x)
    {

      SERIAL.println("");
      SERIAL.println("******************************");
      SERIAL.println("[Stacks Free Bytes Remaining] ");

      
      measurement = uxTaskGetStackHighWaterMark( Handle_monitorTask );
      SERIAL.print("Monitor Stack: ");
      SERIAL.println(measurement);

      SERIAL.println("******************************");
      measurement = uxTaskGetStackHighWaterMark( Handle_drawTask );
      SERIAL.print("Draw Stack: ");
      SERIAL.println(measurement);

      SERIAL.println("******************************");
      measurement = uxTaskGetStackHighWaterMark( Handle_buttonTask );
      SERIAL.print("Button Stack: ");
      SERIAL.println(measurement);

      SERIAL.println("******************************");
      measurement = uxTaskGetStackHighWaterMark( Handle_gravityTask );
      SERIAL.print("Gravity Stack: ");
      SERIAL.println(measurement);

      SERIAL.println("******************************");

      myDelayMs(10000); // print every 10 seconds
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    SERIAL.println("Task Monitor: Deleting");
    vTaskDelete( NULL );

}
//*****************************************************************
static void threadButton( void *pvParameters ) 
{
  SERIAL.println("Button thread started");
  Bounce buttons[NUM_BUTTONS];
  const Action BUTTON_ACTIONS[NUM_BUTTONS] = {up, down, left, right, drop};
  for (int i = 0; i < NUM_BUTTONS; i++) {
  buttons[i].attach( BUTTON_ACTIONS[i] , INPUT_PULLUP  );
  buttons[i].interval(25);
  }
  
  while(!game_over)
  {
    if(fig != nullptr) {
      Action a = no;
      for (int i = 0; i < NUM_BUTTONS; i++)  {
        buttons[i].update();
        if ( buttons[i].fell() ) {
          a = BUTTON_ACTIONS[i];
        }
      }
      if(a != no) SERIAL.println("Button event");
      switch(a) {
        case up:
    //      str = "up";
          break;
        case down:
    //      str = "down";
          break;
        case left:
          fig->moveLeft();
          break;
        case right:
          fig->moveRight();
          break;
        case drop:
          if(fig->drop()) {
            delete fig;
            fig = nullptr;
            SERIAL.println("Delete figure after drop");            
          }
          break;
        case no:
    //      str = "";
          break;
      }
    }
    myDelayMs(10);
  }
  SERIAL.println("Button thread deleted");
  vTaskDelete( NULL );
  
}

//*****************************************************************
static void threadDraw( void *pvParameters ) 
{
  SERIAL.println("Draw thread started");
  U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R1, /* cs=*/ CS_PIN, /* dc=*/ DC_PIN, /* reset=*/ RESET_PIN);
  u8g2.begin();
  while(true)
  {
    u8g2.clearBuffer();
    {//draw field
      u8g2.drawFrame(0,0,63,128);
      int bw = 5;
      int gw = 1;
      if(!game_over) {
        for(int i = 3; i < HEIGHT; ++i) {
          for(int j = 0; j < WIDTH; ++j) {
            if(field[j][i] == 1) {
              u8g2.drawBox(2 + j*(bw+gw),2 + 5 + (i-3)*(bw+gw),bw,bw);
              //u8g2.drawFrame(2 + j*(bw+gw),2 + 5 +(i-2)*(bw+gw),bw,bw);
              //u8g2.drawPixel(2 + j*(bw+gw) + 2,2 + 5 + (i-2)*(bw+gw) + 2);
            }
          }
        }
      } else {
        u8g2.setFont(u8g2_font_courB10_tf); // choose a suitable font
        char buf[] = "Game";
        u8g2.drawStr(15,56,buf);
        char buf1[] = "Over";
        u8g2.drawStr(15,67,buf1);
      }
    }
    {// draw score
      u8g2.setFont(u8g2_font_pxplusibmcgathin_8n); // choose a suitable font
      char buf[7];
      sprintf(buf, "%06d", score);
      u8g2.drawStr(0, 7, buf);
      
    }
    u8g2.sendBuffer();
    if(game_over) break;
    myDelayMs(100);
  }
  SERIAL.println("Draw thread deleted");
  vTaskDelete( NULL );
  

}
//*****************************************************************
static void threadGravity( void *pvParameters ) 
{
  SERIAL.println("Gravity thread started");
  while(!game_over)
  {
    if(fig == nullptr) {
      fig = new Figure;
      if(game_over) {
        SERIAL.println("Game over, break");
        break;
      }
      SERIAL.println("Create figure");
    } else {
      if(!fig->gravitate()) {
        delete fig;
        fig = nullptr;
        SERIAL.println("Delete figure");
      }
    }
    myDelayMs(1000);
  }
  SERIAL.println("Gravity thread deleted");
  vTaskDelete( NULL );
  
}


//*****************************************************************

void setup() 
{

  SERIAL.begin(115200);

  vNopDelayMS(1000); // prevents usb driver crash on startup, do not omit this
  while (!SERIAL) ;  // Wait for serial terminal to open port before starting program

  SERIAL.println("");
  SERIAL.println("******************************");
  SERIAL.println("        Program start         ");
  SERIAL.println("******************************");

  // Set the led the rtos will blink when we have a fatal rtos error
  // RTOS also Needs to know if high/low is the state that turns on the led.
  // Error Blink Codes:
  //    3 blinks - Fatal Rtos Error, something bad happened. Think really hard about what you just changed.
  //    2 blinks - Malloc Failed, Happens when you couldn't create a rtos object. 
  //               Probably ran out of heap.
  //    1 blink  - Stack overflow, Task needs more bytes defined for its stack! 
  //               Use the taskMonitor thread to help gauge how much more you need
  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);
    

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
  xTaskCreate(taskMonitor, "Task Monitor", 256, NULL, tskIDLE_PRIORITY + 4, &Handle_monitorTask);
  xTaskCreate(threadDraw,     "Task Draw",       256, NULL, tskIDLE_PRIORITY + 1, &Handle_drawTask);
  xTaskCreate(threadButton,     "Task Button",       256, NULL, tskIDLE_PRIORITY + 2, &Handle_buttonTask);
  xTaskCreate(threadGravity,     "Task Gravity",       256, NULL, tskIDLE_PRIORITY + 3, &Handle_gravityTask);
  // Start the RTOS, this function will never return and will schedule the tasks.
  vTaskStartScheduler();

}

//*****************************************************************
// This is now the rtos idle loop
// No rtos blocking functions allowed!
//*****************************************************************
void loop() 
{
    // Optional commands, can comment/uncomment below
    
    vNopDelayMS(100);
}


//*****************************************************************
