//Proiect: Radu Valentin, Dumitraș Dragoș-Andrei

#include <LiquidCrystal_I2C.h>
#include <DeHasTeu.h>
#include <EEPROM.h>
#define BUTTON_OK 7
#define BUTTON_CANCEL 8
#define BUTTON_PREV 4
#define BUTTON_NEXT 12
#define DHT_pin 2
#define BEC_pin 3
unsigned long uptime = -1;
LiquidCrystal_I2C LCD(0x27, 16, 2);
DHT dht(DHT_pin);
volatile int s = 0, m = 0, h = 0; 
volatile bool one_second = false;
volatile bool two_second = false;
volatile float temperatura, prev_error, setpoint = 35, suma_error, moving_sp;
volatile float kp, ki, kd;
bool flag1 = false;
bool flag2 = false;
float dt_inc = 0, dt_rac = 0;
float T1 = 0, T2 = 0;
void timer1()
{
  DDRB |= 1 << PB5;
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12) | (1 << CS12) | _BV(CS10) ;
  TIMSK1 |= (1 << OCIE1A) ;
  OCR1A = 15624;

  sei();
}

void btnInit()
{
  pinMode(4, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(BEC_pin, OUTPUT);
}
enum Buttons {
  EV_OK,
  EV_CANCEL,
  EV_NEXT,
  EV_PREV,
  EV_NONE,
  EV_MAX_NUM
};

enum Menus {
  MENU_MAIN = 0,
  MENU_START,
  MENU_KP,
  MENU_KI,
  MENU_KD,
  MENU_TEMP,
  MENU_TINCAL,
  MENU_TMEN,
  MENU_TRAC,
  MENU_MAX_NUM
};
struct Parameters
{
  float temp = 36.6;
  float kp = 20;
  float ki = 10;
  float kd = 15;
  int tIncal = 30;
  int tMen = 30;
  int tRac = 15;
};
Parameters parameter;
float temp_q = 0;
int change = 0;
Menus scroll_menu = MENU_MAIN;
Menus current_menu =  MENU_MAIN;
void state_machine(enum Menus menu, enum Buttons button);
Buttons GetButtons(void);
void print_menu(enum Menus menu);
typedef void (state_machine_handler_t)(void);
void print_menu(enum Menus menu)
{
  switch (menu)
  {
    case MENU_KP:
      LCD.setCursor(0, 0);
      LCD.print("KP = ");
      LCD.setCursor(5, 0);
      LCD.print(parameter.kp);
      break;
    case MENU_KI:
      LCD.setCursor(0, 0);
      LCD.print("KI = ");
      LCD.setCursor(5, 0);
      LCD.print(parameter.ki);
      break;
    case MENU_KD:
      LCD.setCursor(0, 0);
      LCD.print("KD = ");
      LCD.setCursor(5, 0);
      LCD.print(parameter.kd);
      break;
    case MENU_TEMP:
      LCD.setCursor(0, 0);
      LCD.print("Temp = ");
      if (parameter.temp < 50)
      { LCD.setCursor(7, 1);
        LCD.print(parameter.temp);
      }
      else
      { LCD.setCursor(7, 1);
        LCD.print("MAX!");
      }
      break;
    case MENU_TINCAL:
      LCD.setCursor(0, 0);
      LCD.print("TINCAL = ");
      LCD.setCursor(9, 0);
      LCD.print(parameter.tIncal);
      break;
    case MENU_TMEN:
      LCD.setCursor(0, 0);
      LCD.print("TMEN = ");
      LCD.setCursor(7, 0);
      LCD.print(parameter.tMen);
      break;
    case MENU_TRAC:
      LCD.setCursor(0, 0);
      LCD.print("TRAC = ");
      LCD.setCursor(7, 0);
      LCD.print(parameter.tRac);
      break;
    case MENU_START:
      LCD.setCursor(0, 0);
      LCD.print("START!");
      break;
    case MENU_MAIN:
    default:
      LCD.setCursor(5, 0);
      LCD.print("PS 2020");
      break;
  }
  if (current_menu == MENU_START)
  {
    afisare_timp();
  }
  else if (current_menu != MENU_MAIN)
  {
    LCD.setCursor(0, 0);
    LCD.print("Mod.");
 }
}

void enter_menu(void)
{
  current_menu = scroll_menu;
}

void go_home(void)
{
  scroll_menu = MENU_MAIN;
  current_menu = scroll_menu;
  change = 0;
}
void go_next(void)
{
  scroll_menu = (Menus) ((int)scroll_menu + 1);
  scroll_menu = (Menus) ((int)scroll_menu % MENU_MAX_NUM);
}
void go_prev(void)
{
  scroll_menu = (Menus) ((int)scroll_menu - 1);
  scroll_menu = (Menus) ((int)scroll_menu % MENU_MAX_NUM);
}
void save(void)
{
  EEPROM.put(0, parameter);
  go_home();
}
void inc_kp(void)
{
  parameter.kp++;
  change++;
}
void inc_ki(void)
{
  parameter.ki++;
  change++;
}
void inc_kd(void)
{
  parameter.kd++;
  change++;
}
void dec_kp(void)
{
  parameter.kp--;
  change--;
}
void dec_ki(void)
{
  parameter.ki--;
  change--;
}
void dec_kd(void)
{
  parameter.kd--;
  change--;
}
void inc_temp(void)
{
  if (parameter.temp < 50)
  {
    parameter.temp++;
    change++;
  }
}
void dec_temp(void)
{
  parameter.temp--;
  change--;
}
void cancel_Temp(void)
{
  parameter.temp -= change;
  go_home();
}
void cancel_KP(void)
{
  parameter.kp -= change;
  go_home();
}
void cancel_KI(void)
{
  parameter.ki -= change;
  go_home();
}
void cancel_KD(void)
{
  parameter.kd -= change;
  go_home();
}
void cancel_tIncal(void)
{
  parameter.tIncal -= change;
  go_home();
}
void inc_tIncal(void)
{
  parameter.tIncal++;
  change++;
}
void dec_tIncal(void)
{
  parameter.tIncal--;
  change--;
}
void cancel_tMen(void)
{
  parameter.tMen -= change;
  go_home();
}
void inc_tMen(void)
{
  parameter.tMen++;
  change++;
}
void dec_tMen(void)
{
  parameter.tMen--;
  change--;
}
void cancel_tRac(void)
{
  parameter.tRac -= change;
  go_home();
}
void inc_tRac(void)
{
  parameter.tRac++;
  change++;
}
void dec_tRac(void)
{
  parameter.tRac--;
  change--;
}
void afisare_timp (void)
{ int min = 0;
  int sec = 0;
  int remaining = 0;
  uptime ++;
  int timp_inc = parameter.tIncal;
  int timp_men = parameter.tMen;
  int timp_rac = parameter.tRac;
  float temp = parameter.temp;
  if (temperatura > 0)
  {
    LCD.setCursor(11, 0);
    LCD.print(temperatura);
    if (flag1 == false)
    {
      flag1 = true;

      dt_inc = (temp - temperatura) / (timp_inc);
      T1 = temperatura;
    }
  }
  LCD.setCursor(0, 0);
  LCD.print("P:");
  LCD.print (moving_sp);
  LCD.setCursor(0, 1);
  if (uptime < timp_inc)
  {
    LCD.print(" TInc:");
    moving_sp = dt_inc * uptime + T1;
    remaining = timp_inc - uptime;
  }
  else if (uptime <= (timp_inc + timp_men))
  { LCD.print(" TMen:");
    remaining = (timp_inc + timp_men) - uptime;
  }
  else if (uptime <= (timp_inc + timp_men + timp_rac))
  { LCD.print(" TRac:");
    if (flag2 == false)
    {
      flag2 = true;
      dt_rac = (T1 - temperatura) / (timp_rac);
      T2 = temperatura;
    }
    //Serial.println(dt_rac);
    moving_sp = dt_rac * (uptime - (timp_inc + timp_men)) + T2;
    remaining = (timp_inc + timp_men + timp_rac) - uptime;
  }
  else
    LCD.print("Oprit: ");
  min = remaining / 60;
  sec = remaining % 60;
  LCD.print(min);
  LCD.print(":");
  LCD.print (sec);
  PID();
}
bool ok()
{
  return 1;
}
void todo(void)
{
  LCD.setCursor(0, 1);
  LCD.print("To be continued...");
}
state_machine_handler_t* sm[MENU_MAX_NUM][EV_MAX_NUM] =
{   {enter_menu, go_home, go_next, go_prev},       
  {todo, go_home, todo, ok},                   
  {save, cancel_KP, inc_kp, dec_kp},             
  {save, cancel_KI, inc_ki, dec_ki},             
  {save, cancel_KD, inc_kd, dec_kd},            
  {save, cancel_Temp, inc_temp, dec_temp},        
  {save, cancel_tIncal, inc_tIncal, dec_tIncal},  
  {save, cancel_tMen, inc_tMen, dec_tMen},        
  {save, cancel_tRac, inc_tRac, dec_tRac}         
};

void state_machine(enum Menus menu, enum Buttons button)
{
  sm[menu][button]();
}
Buttons GetButtons(void)
{
  enum Buttons ret_val = EV_NONE;
  if (!digitalRead(BUTTON_OK))
  {
    ret_val = EV_OK;
  }
  else if (!digitalRead(BUTTON_CANCEL))
  {
    ret_val = EV_CANCEL;
  }
  else if (!digitalRead(BUTTON_NEXT))
  {
    ret_val = EV_NEXT;
  }
  else if (!digitalRead(BUTTON_PREV))
  {
    ret_val = EV_PREV;
  }
  return ret_val;
}
void setup() {
  LCD.init();
  LCD.backlight();
  btnInit();
  //EEPROM.put(0,parameter);
  EEPROM.get(0, parameter);
  timer1();
  Serial.begin(9600);
}
void loop() {
  if (one_second)
  {
    if (two_second)
    {
      dht.start();
      temperatura = dht.Busu();
      LCD.clear();
      two_second = false;
    }
     kp = parameter.kp;
    kd = parameter.kd;
    ki = parameter.ki;//
    volatile Buttons event = GetButtons();
    if (event != EV_NONE)
    {
      state_machine(current_menu, event);
    }
    print_menu(scroll_menu);
    one_second = false;
  }
}
ISR(TIMER1_COMPA_vect)
{
  PORTB ^= _BV(PB5);
  if (digitalRead(13) == 0)
    two_second = true;
  s++;
  if (s == 60)
  {
    m++;
    s = 0;
    if (m == 60)
    {
      h++;
      m = 0;
    }
  }
  one_second = true;
}
void PID()
{
  float error =  moving_sp - temperatura;
  if (error < 3 && error > -3)
    suma_error += error;
  suma_error = constrain(suma_error, -15, 15);
  float diff = (error - prev_error);
  float output = kp * error + ki * suma_error + kd * diff;
  output = constrain(output, 0, 100); //maxim 5.5 V
  prev_error = error;
  Serial.print(0.05 * output); //tensiune pe bec
  Serial.print(" ");
  Serial.println(temperatura);
 analogWrite(BEC_pin, int(output))
