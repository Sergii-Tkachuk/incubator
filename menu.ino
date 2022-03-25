#include <Servo.h>            //мотор
#include <LiquidCrystal_I2C.h>//дисплей 16:2
#include <Bounce2.h>          //для усунення брязкоту контактів
#include <DHT.h>              //датчик вологості
#include <EEPROM.h>           //

#define PIN_UP 8
#define PIN_DOWN 9
#define PIN_LEFT 10
#define PIN_RIGHT 11
#define PIN_SERVO 5
#define PIN_HUMIDITY_LEVEL 2
#define PIN_RELE 4


LiquidCrystal_I2C LCD(0x27, 16, 2);
DHT dht(PIN_HUMIDITY_LEVEL, DHT11);
Servo servo;
Bounce2::Button buttonU = Bounce2::Button();
Bounce2::Button buttonD = Bounce2::Button();
Bounce2::Button buttonL = Bounce2::Button();
Bounce2::Button buttonR = Bounce2::Button();

struct Settings
{
  uint8_t minTemperature;
  uint8_t maxTemperature;
  uint8_t timeRotate;
};

enum Menu
{
  HOME,
  SET_LOW_TEMP,
  SET_HIGTH_TEMP,
  SET_TIME_ROTATE,
  COUNT_OPTIONS
};

Settings set;
uint8_t menu = 0;
uint8_t t;
uint8_t h;
unsigned long lastTimeDeleyServo = 0;
unsigned long lastTimeDeleyEEPROM = 0;
bool stateServo = 0;



void setup()
{
  LCD.init();
  LCD.backlight();
  
  buttonU.attach( PIN_UP ,  INPUT_PULLUP );
  buttonD.attach( PIN_DOWN ,  INPUT_PULLUP );
  buttonL.attach( PIN_LEFT ,  INPUT_PULLUP );
  buttonR.attach( PIN_RIGHT ,  INPUT_PULLUP );

  //задаємо час усунення брязкоту контактів
  buttonU.interval(5);
  buttonD.interval(5);
  buttonL.interval(5);
  buttonR.interval(5);
  
  buttonU.setPressedState(LOW); 
  buttonD.setPressedState(LOW);
  buttonL.setPressedState(LOW);    
  buttonR.setPressedState(LOW);    
  
  pinMode(PIN_RELE, OUTPUT);

  servo.attach(PIN_SERVO);
  servo.write(0);
  
  dht.begin();

  if(EEPROM.read(0) != 'k')
  {
    EEPROM.write(0, 'k');
    set.minTemperature = 30;
    set.maxTemperature = 32;
    set.timeRotate = 20;
    EEPROM.put(1, set);
  }
  EEPROM.get(1, set);
}

void loop()
{
  buttonU.update();
  buttonD.update();
  buttonL.update();
  buttonR.update();

  int t = dht.readTemperature();
  int h = dht.readHumidity();

  
  if(t < set.minTemperature) digitalWrite(PIN_RELE, 1);
  if(t > set.maxTemperature) digitalWrite(PIN_RELE, 0);

  if (millis() - lastTimeDeleyServo > set.timeRotate * 1000)
  {
    if(stateServo)
      servo.write(0);
    else
      servo.write(80);
    stateServo = !stateServo;
    lastTimeDeleyServo = millis();
  }


  //навігація по меню
  if(buttonU.pressed())
    ++menu;
  if(buttonD.pressed())
    --menu;

  //установка значень
  switch(menu % COUNT_OPTIONS)
  {
    case SET_LOW_TEMP:
    {
      if(buttonL.pressed() && set.minTemperature > 1)
        --set.minTemperature;
      if(buttonR.pressed() && set.minTemperature < set.maxTemperature)
        ++set.minTemperature;
      break;
    }
    case SET_HIGTH_TEMP:
    {
      if(buttonL.pressed() && set.maxTemperature > set.minTemperature)
        --set.maxTemperature;
      if(buttonR.pressed() && set.maxTemperature < 50)
        ++set.maxTemperature;
      break;
    }
    case SET_TIME_ROTATE:
    {
      if(buttonL.pressed())
        --set.timeRotate;
      if(buttonR.pressed())
        ++set.timeRotate;
      break;
    }
  }

  //вивід інформації на дисплей
  LCD.setCursor(0, 0);
  switch(menu % COUNT_OPTIONS)
  {
    case HOME:
    {  
      LCD.print("Temp:");
      LCD.print(t);
      LCD.print("     ");
      LCD.setCursor(0, 1);
      LCD.print("Humu:");
      LCD.print(h);
      break;
    }
    case SET_LOW_TEMP:
    {
      LCD.print("Low temp    ");
      LCD.setCursor(0, 1);
      LCD.print("<");
      LCD.print(set.minTemperature);
      LCD.print(">   ");
      break;
    }
    case SET_HIGTH_TEMP:
    {
      LCD.print("Max temp    ");
      LCD.setCursor(0, 1);
      LCD.print("<");
      LCD.print(set.maxTemperature);
      LCD.print(">    ");
      break;
    }
    case SET_TIME_ROTATE:
    {
      LCD.print("Time rotate");
      LCD.setCursor(0, 1);
      LCD.print("<");
      LCD.print(set.timeRotate);
      LCD.print(">    ");
      break;
    }
  }
  
  if(millis() - lastTimeDeleyEEPROM > 60000)
  {
    EEPROM.put(1,set);
  }
}
