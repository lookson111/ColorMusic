/*
  Скетч к проекту "Светомузыка на Arduino"
  Страница проекта (схемы, описания): https://alexgyver.ru/colormusic/
  Исходники на GitHub: https://github.com/AlexGyver/ColorMusic
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  https://AlexGyver.ru/

  Как откалибровать уровень шума и как пользоваться пультом
  расписано на странице проекта! https://alexgyver.ru/colormusic/
*/

/*
   Версия 2.10
   Исправлен глюк с большим количеством светодиодов на МЕГЕ
*/


// ***************************** НАСТРОЙКИ *****************************

// ----- настройка ИК пульта
#define REMOTE_TYPE 4       // 0 - без пульта, 1 - пульт от WAVGAT, 2 - пульт от KEYES, 3 - кастомный пульт 4 - блутуз
// система может работать С ЛЮБЫМ ИК ПУЛЬТОМ (практически). Коды для своего пульта можно задать начиная со строки 160 в прошивке. Коды пультов определяются скетчем IRtest_2.0, читай инструкцию

// ----- настройки параметров
#define KEEP_SETTINGS 1     // хранить ВСЕ настройки в энергонезависимой памяти
#define KEEP_STATE 1		    // сохранять в памяти состояние вкл/выкл системы (с пульта)
#define RESET_SETTINGS 0    // сброс настроек в EEPROM памяти (поставить 1, прошиться, поставить обратно 0, прошиться. Всё)
#define SETTINGS_LOG 0      // вывод всех настроек из EEPROM в порт при запуске

// ----- настройки ленты
#define NUM_LEDS 350        // количество светодиодов (данная версия поддерживает до 350 штук)
#define CURRENT_LIMIT 3000  // лимит по току в МИЛЛИАМПЕРАХ, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
byte BRIGHTNESS = 200;      // яркость по умолчанию (0 - 255)

// ----- пины подключения
#define SOUND_R A2         // аналоговый пин вход аудио, правый канал
#define SOUND_L A4         // аналоговый пин вход аудио, левый канал
#define SOUND_R_FREQ A3    // аналоговый пин вход аудио для режима с частотами (через кондер)
#define BTN_PIN 3          // кнопка переключения режимов (PIN --- КНОПКА --- GND)

#if defined(__AVR_ATmega32U4__) // Пины для Arduino Pro Micro (смотри схему для Pro Micro на странице проекта!!!)
#define MLED_PIN 17             // пин светодиода режимов на ProMicro, т.к. обычный не выведен.
#define MLED_ON LOW
#define LED_PIN 9               // пин DI светодиодной ленты на ProMicro, т.к. обычный не выведен.
#else                           // Пины для других плат Arduino (по умолчанию)
#define MLED_PIN 13             // пин светодиода режимов
#define MLED_ON HIGH
#define LED_PIN 12              // пин DI светодиодной ленты
#endif

#define POT_GND A0              // пин земля для потенциометра
#define IR_PIN 2                // пин ИК приёмника

int  LED_WORK = A1;      // LED индикация работы
bool LED_WORK_FL = false;

// ----- настройки радуги
float RAINBOW_STEP = 5.00;         // шаг изменения цвета радуги

// ----- отрисовка
#define MODE 0                    // режим при запуске
#define MAIN_LOOP 5               // период основного цикла отрисовки (по умолчанию 5)

// ----- сигнал
#define MONO 1                    // 1 - только один канал (ПРАВЫЙ!!!!! SOUND_R!!!!!), 0 - два канала
#define EXP 1.4                   // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)
#define POTENT 0                  // 1 - используем потенциометр, 0 - используется внутренний источник опорного напряжения 1.1 В
byte EMPTY_BRIGHT = 30;           // яркость "не горящих" светодиодов (0 - 255)
#define EMPTY_COLOR HUE_PURPLE    // цвет "не горящих" светодиодов. Будет чёрный, если яркость 0

// ----- нижний порог шумов
uint16_t LOW_PASS = 100;          // нижний порог шумов режим VU, ручная настройка
uint16_t SPEKTR_LOW_PASS = 40;    // нижний порог шумов режим спектра, ручная настройка
#define AUTO_LOW_PASS 0           // разрешить настройку нижнего порога шумов при запуске (по умолч. 0)
#define EEPROM_LOW_PASS 1         // порог шумов хранится в энергонезависимой памяти (по умолч. 1)
#define LOW_PASS_ADD 13           // "добавочная" величина к нижнему порогу, для надёжности (режим VU)
#define LOW_PASS_FREQ_ADD 3       // "добавочная" величина к нижнему порогу, для надёжности (режим частот)

// ----- режим шкала громкости
float SMOOTH = 0.3;               // коэффициент плавности анимации VU (по умолчанию 0.5)
#define MAX_COEF 1.8              // коэффициент громкости (максимальное равно срднему * этот коэф) (по умолчанию 1.8)

// ----- режим цветомузыки
float SMOOTH_FREQ = 0.8;          // коэффициент плавности анимации частот (по умолчанию 0.8)
float MAX_COEF_FREQ = 1.2;        // коэффициент порога для "вспышки" цветомузыки (по умолчанию 1.5)
#define SMOOTH_STEP 20            // шаг уменьшения яркости в режиме цветомузыки (чем больше, тем быстрее гаснет)
#define LOW_COLOR HUE_RED         // цвет низких частот
#define MID_COLOR HUE_GREEN       // цвет средних
#define HIGH_COLOR HUE_YELLOW     // цвет высоких

// ----- режим стробоскопа
uint16_t STROBE_PERIOD = 140;     // период вспышек, миллисекунды
#define STROBE_DUTY 20            // скважность вспышек (1 - 99) - отношение времени вспышки ко времени темноты
#define STROBE_COLOR HUE_YELLOW   // цвет стробоскопа
#define STROBE_SAT 0              // насыщенность. Если 0 - цвет будет БЕЛЫЙ при любом цвете (0 - 255)
byte STROBE_SMOOTH = 200;         // скорость нарастания/угасания вспышки (0 - 255)

// ----- режим огня
#define HUE_GAP 21      // заброс по hue
#define FIRE_STEP 15    // шаг огня
#define HUE_START_FIRE 0     // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define MIN_BRIGHT 70   // мин. яркость огня
#define MAX_BRIGHT 255  // макс. яркость огня
#define MIN_SAT 245     // мин. насыщенность
#define MAX_SAT 255     // макс. насыщенность

// ----- режим подсветки
byte LIGHT_COLOR = 0;             // начальный цвет подсветки
byte LIGHT_SAT = 255;             // начальная насыщенность подсветки
byte COLOR_SPEED = 100;
int RAINBOW_PERIOD = 1;
float RAINBOW_STEP_2 = 0.5;

// ----- режим бегущих частот
byte RUNNING_SPEED = 11;

// ----- режим анализатора спектра
byte HUE_START = 0;
byte HUE_STEP = 5;
byte settings[2]; //Массив настроек
#define LIGHT_SMOOTH 2

/*
  Цвета для HSV
  HUE_RED
  HUE_ORANGE
  HUE_YELLOW
  HUE_GREEN
  HUE_AQUA
  HUE_BLUE
  HUE_PURPLE
  HUE_PINK
*/

// ----- КНОПКИ ПУЛЬТА WAVGAT -----
#if REMOTE_TYPE == 1
#define BUTT_UP     0xF39EEBAD
#define BUTT_DOWN   0xC089F6AD
#define BUTT_LEFT   0xE25410AD
#define BUTT_RIGHT  0x14CE54AD
#define BUTT_OK     0x297C76AD
#define BUTT_1      0x4E5BA3AD
#define BUTT_2      0xE51CA6AD
#define BUTT_3      0xE207E1AD
#define BUTT_4      0x517068AD
#define BUTT_5      0x1B92DDAD
#define BUTT_6      0xAC2A56AD
#define BUTT_7      0x5484B6AD
#define BUTT_8      0xD22353AD
#define BUTT_9      0xDF3F4BAD
#define BUTT_0      0xF08A26AD
#define BUTT_STAR   0x68E456AD
#define BUTT_HASH   0x151CD6AD
#endif

// ----- КНОПКИ ПУЛЬТА KEYES -----
#if REMOTE_TYPE == 2
#define BUTT_UP     0xE51CA6AD
#define BUTT_DOWN   0xD22353AD
#define BUTT_LEFT   0x517068AD
#define BUTT_RIGHT  0xAC2A56AD
#define BUTT_OK     0x1B92DDAD
#define BUTT_1      0x68E456AD
#define BUTT_2      0xF08A26AD
#define BUTT_3      0x151CD6AD
#define BUTT_4      0x18319BAD
#define BUTT_5      0xF39EEBAD
#define BUTT_6      0x4AABDFAD
#define BUTT_7      0xE25410AD
#define BUTT_8      0x297C76AD
#define BUTT_9      0x14CE54AD
#define BUTT_0      0xC089F6AD
#define BUTT_STAR   0xAF3F1BAD
#define BUTT_HASH   0x38379AD
#endif

// ----- КНОПКИ СВОЕГО ПУЛЬТА -----
#if REMOTE_TYPE == 3
#define BUTT_UP     0xE51CA6AD
#define BUTT_DOWN   0xD22353AD
#define BUTT_LEFT   0x517068AD
#define BUTT_RIGHT  0xAC2A56AD
#define BUTT_OK     0x1B92DDAD
#define BUTT_1      0x68E456AD
#define BUTT_2      0xF08A26AD
#define BUTT_3      0x151CD6AD
#define BUTT_4      0x18319BAD
#define BUTT_5      0xF39EEBAD
#define BUTT_6      0x4AABDFAD
#define BUTT_7      0xE25410AD
#define BUTT_8      0x297C76AD
#define BUTT_9      0x14CE54AD
#define BUTT_0      0xC089F6AD
#define BUTT_STAR   0xAF3F1BAD  // *
#define BUTT_HASH   0x38379AD   // #
#endif

// ----- КНОПКИ СВОЕГО ПУЛЬТА -----
#if REMOTE_TYPE == 4
#define BUTT_UP     0x42
#define BUTT_DOWN   0x41
#define BUTT_LEFT   0x43
#define BUTT_RIGHT  0x44
#define BUTT_OK     0x45
#define BUTT_1      0x31
#define BUTT_2      0x32
#define BUTT_3      0x33
#define BUTT_4      0x34
#define BUTT_5      0x35
#define BUTT_6      0x36
#define BUTT_7      0x37
#define BUTT_8      0x38
#define BUTT_9      0x39
#define BUTT_0      0x30
#define BUTT_STAR   0x46  // *
#define BUTT_HASH   0x47  // #
#endif

// ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ --------------------------------
#define MODE_AMOUNT 9      // количество режимов

#define STRIPE NUM_LEDS / 5
float freq_to_stripe = NUM_LEDS / 40; // /2 так как симметрия, и /20 так как 20 частот

#define FHT_N 64         // ширина спектра х2
#define LOG_OUT 1
#include <FHT.h>         // преобразование Хартли

#include <EEPROMex.h>

#define FASTLED_ALLOW_INTERRUPTS 1
#include "FastLED.h"
CRGB leds[NUM_LEDS];

#include "GyverButton.h"
GButton butt1(BTN_PIN);

#include "IRLremote.h"
CHashIR IRLremote;
uint32_t IRdata;

// градиент-палитра от зелёного к красному
DEFINE_GRADIENT_PALETTE(soundlevel_gp) {
  0,    0,    255,  0,  // green
  100,  255,  255,  0,  // yellow
  150,  255,  100,  0,  // orange
  200,  255,  50,   0,  // red
  255,  255,  0,    0   // red
};
CRGBPalette32 myPal = soundlevel_gp;

int Rlenght, Llenght;
float RsoundLevel, RsoundLevel_f;
float LsoundLevel, LsoundLevel_f;

float averageLevel = 50;
int maxLevel = 100;
int MAX_CH = NUM_LEDS / 2;
int hue;
int counter = 0;
unsigned long main_timer, hue_timer, strobe_timer, running_timer, color_timer, rainbow_timer, eeprom_timer;
unsigned long led_timer;
float averK = 0.006;
byte count;
float index = (float)255 / MAX_CH;   // коэффициент перевода для палитры
boolean lowFlag;
byte low_pass;
int RcurrentLevel, LcurrentLevel;
int colorMusic[3];
float colorMusic_f[3], colorMusic_aver[3];
boolean colorMusicFlash[3], strobeUp_flag, strobeDwn_flag;
byte this_mode = MODE;
int thisBright[3], strobe_bright = 0;
unsigned int light_time = STROBE_PERIOD * STROBE_DUTY / 100;
volatile boolean ir_flag;
boolean settings_mode, ONstate = true;
int8_t freq_strobe_mode, light_mode;
int freq_max;
float freq_max_f, rainbow_steps;
int freq_f[32];
int this_color;
boolean running_flag[3], eeprom_flag;

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ --------------------------------

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(BRIGHTNESS);

#if defined(__AVR_ATmega32U4__)   //Выключение светодиодов на Pro Micro
  TXLED1;                           //на ProMicro выключим и TXLED
  delay (1000);                     //При питании по usb от компьютера нужна задержка перед выключением RXLED. Если питать от БП, то можно убрать эту строку.
#endif
  pinMode(MLED_PIN, OUTPUT);        //Режим пина для светодиода режима на выход
  digitalWrite(MLED_PIN, !MLED_ON); //Выключение светодиода режима

  pinMode(POT_GND, OUTPUT);
  digitalWrite(POT_GND, LOW);
  butt1.setTimeout(900);
  
  pinMode(LED_WORK, OUTPUT);
  digitalWrite(LED_WORK, LOW);

  IRLremote.begin(IR_PIN);

  // для увеличения точности уменьшаем опорное напряжение,
  // выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
  // GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3
  // в данной схеме GND берётся из А0 для удобства подключения
  if (POTENT) analogReference(EXTERNAL);
  else
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1);
#else
    analogReference(INTERNAL);
#endif

  // жуткая магия, меняем частоту оцифровки до 18 кГц
  // команды на ебучем ассемблере, даже не спрашивайте, как это работает
  // поднимаем частоту опроса аналогового порта до 38.4 кГц, по теореме
  // Котельникова (Найквиста) частота дискретизации будет 19.2 кГц
  // http://yaab-arduino.blogspot.ru/2015/02/fast-sampling-from-analog-input.html
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  if (RESET_SETTINGS) EEPROM.write(100, 0);        // сброс флага настроек

  if (AUTO_LOW_PASS && !EEPROM_LOW_PASS) {         // если разрешена автонастройка нижнего порога шумов
    autoLowPass();
  }
  if (EEPROM_LOW_PASS) {                // восстановить значения шумов из памяти
    LOW_PASS = EEPROM.readInt(70);
    SPEKTR_LOW_PASS = EEPROM.readInt(72);
  }

  // в 100 ячейке хранится число 100. Если нет - значит это первый запуск системы
  if (KEEP_SETTINGS) {
    if (EEPROM.read(100) != 100) {
      //Serial.println(F("First start"));
      EEPROM.write(100, 100);
      updateEEPROM();
    } else {
      readEEPROM();
    }
  }

#if (SETTINGS_LOG == 1)
  Serial.print(F("this_mode = ")); Serial.println(this_mode);
  Serial.print(F("freq_strobe_mode = ")); Serial.println(freq_strobe_mode);
  Serial.print(F("light_mode = ")); Serial.println(light_mode);
  Serial.print(F("RAINBOW_STEP = ")); Serial.println(RAINBOW_STEP);
  Serial.print(F("MAX_COEF_FREQ = ")); Serial.println(MAX_COEF_FREQ);
  Serial.print(F("STROBE_PERIOD = ")); Serial.println(STROBE_PERIOD);
  Serial.print(F("LIGHT_SAT = ")); Serial.println(LIGHT_SAT);
  Serial.print(F("RAINBOW_STEP_2 = ")); Serial.println(RAINBOW_STEP_2);
  Serial.print(F("HUE_START = ")); Serial.println(HUE_START);
  Serial.print(F("SMOOTH = ")); Serial.println(SMOOTH);
  Serial.print(F("SMOOTH_FREQ = ")); Serial.println(SMOOTH_FREQ);
  Serial.print(F("STROBE_SMOOTH = ")); Serial.println(STROBE_SMOOTH);
  Serial.print(F("LIGHT_COLOR = ")); Serial.println(LIGHT_COLOR);
  Serial.print(F("COLOR_SPEED = ")); Serial.println(COLOR_SPEED);
  Serial.print(F("RAINBOW_PERIOD = ")); Serial.println(RAINBOW_PERIOD);
  Serial.print(F("RUNNING_SPEED = ")); Serial.println(RUNNING_SPEED);
  Serial.print(F("HUE_STEP = ")); Serial.println(HUE_STEP);
  Serial.print(F("EMPTY_BRIGHT = ")); Serial.println(EMPTY_BRIGHT);
  Serial.print(F("ONstate = ")); Serial.println(ONstate);
#endif
}

void requestParametr() {
  settings[0] = (byte)this_mode;
  settings[2] = smartConvertByte(BRIGHTNESS, 0, 255);
  settings[3] = smartConvertByte(EMPTY_BRIGHT, 0, 255);
  switch (this_mode)
  {
    case 0:
      settings[4] = smartConvertByte(SMOOTH, 0.05, 1); // Умножить на 100
      break;
    case 1:
      settings[4] = smartConvertByte(SMOOTH, 0.05, 1);     //чтобы число было минимум 0.05 и максимум 1
      settings[5] = smartConvertByte(RAINBOW_STEP, 0.5, 20);   //чтобы число было минимум 0.5 и максимум 20
      break;
    case 2:
    case 3:
      settings[4] = smartConvertByte(SMOOTH_FREQ, 0.05, 1);
      settings[5] = smartConvertByte(MAX_COEF_FREQ, 0, 5);
      break;
    case 4:
        settings[1] = (byte)light_mode;
        settings[4] = smartConvertByte(SMOOTH_FREQ, 0.05, 1);
        settings[5] = smartConvertByte(MAX_COEF_FREQ, 0, 5);
      break;
    case 5:
      break;
    case 6:
      settings[1] = (byte)light_mode;
        switch (light_mode) {
          case 0: 
            settings[4] = smartConvertByte(LIGHT_COLOR, 0, 255);
            settings[5] = smartConvertByte(LIGHT_SAT, 0, 255);
            break;
          case 1: 
            settings[4] = smartConvertByte(LIGHT_SAT, 0, 255);
            settings[5] = smartConvertByte(COLOR_SPEED, 0, 255);
            break;
          case 2: 
            settings[4] = smartConvertByte(RAINBOW_PERIOD, -20, 20);  //????????????? Написать правильный метод конвертации
            settings[5] = smartConvertByte(RAINBOW_STEP_2, 0.5, 10);
            break;
            }
      break;
    case 7:
      settings[1] = (byte)freq_strobe_mode;
      settings[4] = smartConvertByte(RUNNING_SPEED, 1, 255);
      settings[5] = smartConvertByte(MAX_COEF_FREQ, 0.0, 10);
      break;
    case 8:
      settings[4] = smartConvertByte(HUE_STEP, 1, 255);
      settings[5] = smartConvertByte(HUE_START, 0, 255);
      break;  
  }
}

void loop() {
  buttonTick();     // опрос и обработка кнопки
#if REMOTE_TYPE != 0
  remoteTick();     // опрос пульта
#endif
  mainLoop();       // главный цикл обработки и отрисовки
  eepromTick();     // проверка не пора ли сохранить настройки
#if REMOTE_TYPE == 4
  ledBlink();	    // мигание светодиода
#endif
}

void mainLoop() {
  // главный цикл отрисовки
  if (ONstate) {
    if (millis() - main_timer > MAIN_LOOP) {
      // сбрасываем значения
      RsoundLevel = 0;
      LsoundLevel = 0;

      // перваые два режима - громкость (VU meter)
      if (this_mode == 0 || this_mode == 1) {
        for (byte i = 0; i < 100; i ++) {                                 // делаем 100 измерений
          RcurrentLevel = analogRead(SOUND_R);                            // с правого
          if (!MONO) LcurrentLevel = analogRead(SOUND_L);                 // и левого каналов

          if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;   // ищем максимальное
          if (!MONO) if (LsoundLevel < LcurrentLevel) LsoundLevel = LcurrentLevel;   // ищем максимальное
        }

        // фильтруем по нижнему порогу шумов
        RsoundLevel = map(RsoundLevel, LOW_PASS, 1023, 0, 500);
        if (!MONO)LsoundLevel = map(LsoundLevel, LOW_PASS, 1023, 0, 500);

        // ограничиваем диапазон
        RsoundLevel = constrain(RsoundLevel, 0, 500);
        if (!MONO)LsoundLevel = constrain(LsoundLevel, 0, 500);

        // возводим в степень (для большей чёткости работы)
        RsoundLevel = pow(RsoundLevel, EXP);
        if (!MONO)LsoundLevel = pow(LsoundLevel, EXP);

        // фильтр
        RsoundLevel_f = RsoundLevel * SMOOTH + RsoundLevel_f * (1 - SMOOTH);
        if (!MONO)LsoundLevel_f = LsoundLevel * SMOOTH + LsoundLevel_f * (1 - SMOOTH);

        if (MONO) LsoundLevel_f = RsoundLevel_f;  // если моно, то левый = правому

        // заливаем "подложку", если яркость достаточная
        if (EMPTY_BRIGHT > 5) {
          for (int i = 0; i < NUM_LEDS; i++)
            leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        }

        // если значение выше порога - начинаем самое интересное
        if (RsoundLevel_f > 15 && LsoundLevel_f > 15) {

          // расчёт общей средней громкости с обоих каналов, фильтрация.
          // Фильтр очень медленный, сделано специально для автогромкости
          averageLevel = (float)(RsoundLevel_f + LsoundLevel_f) / 2 * averK + averageLevel * (1 - averK);

          // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
          maxLevel = (float)averageLevel * MAX_COEF;

          // преобразуем сигнал в длину ленты (где MAX_CH это половина количества светодиодов)
          Rlenght = map(RsoundLevel_f, 0, maxLevel, 0, MAX_CH);
          Llenght = map(LsoundLevel_f, 0, maxLevel, 0, MAX_CH);

          // ограничиваем до макс. числа светодиодов
          Rlenght = constrain(Rlenght, 0, MAX_CH);
          Llenght = constrain(Llenght, 0, MAX_CH);

          animation();       // отрисовать
        }
      }

      // 3-5 режим - цветомузыка
      if (this_mode == 2 || this_mode == 3 || this_mode == 4 || this_mode == 7 || this_mode == 8) {
        analyzeAudio();
        colorMusic[0] = 0;
        colorMusic[1] = 0;
        colorMusic[2] = 0;
        for (int i = 0 ; i < 32 ; i++) {
          if (fht_log_out[i] < SPEKTR_LOW_PASS) fht_log_out[i] = 0;
        }
        // низкие частоты, выборка со 2 по 5 тон (0 и 1 зашумленные!)
        for (byte i = 2; i < 6; i++) {
          if (fht_log_out[i] > colorMusic[0]) colorMusic[0] = fht_log_out[i];
        }
        // средние частоты, выборка с 6 по 10 тон
        for (byte i = 6; i < 11; i++) {
          if (fht_log_out[i] > colorMusic[1]) colorMusic[1] = fht_log_out[i];
        }
        // высокие частоты, выборка с 11 по 31 тон
        for (byte i = 11; i < 32; i++) {
          if (fht_log_out[i] > colorMusic[2]) colorMusic[2] = fht_log_out[i];
        }
        freq_max = 0;
        for (byte i = 0; i < 30; i++) {
          if (fht_log_out[i + 2] > freq_max) freq_max = fht_log_out[i + 2];
          if (freq_max < 5) freq_max = 5;

          if (freq_f[i] < fht_log_out[i + 2]) freq_f[i] = fht_log_out[i + 2];
          if (freq_f[i] > 0) freq_f[i] -= LIGHT_SMOOTH;
          else freq_f[i] = 0;
        }
        freq_max_f = freq_max * averK + freq_max_f * (1 - averK);
        for (byte i = 0; i < 3; i++) {
          colorMusic_aver[i] = colorMusic[i] * averK + colorMusic_aver[i] * (1 - averK);  // общая фильтрация
          colorMusic_f[i] = colorMusic[i] * SMOOTH_FREQ + colorMusic_f[i] * (1 - SMOOTH_FREQ);      // локальная
          if (colorMusic_f[i] > ((float)colorMusic_aver[i] * MAX_COEF_FREQ)) {
            thisBright[i] = 255;
            colorMusicFlash[i] = true;
            running_flag[i] = true;
          } else colorMusicFlash[i] = false;
          if (thisBright[i] >= 0) thisBright[i] -= SMOOTH_STEP;
          if (thisBright[i] < EMPTY_BRIGHT) {
            thisBright[i] = EMPTY_BRIGHT;
            running_flag[i] = false;
          }
        }
        animation();
      }
      if (this_mode == 5) {
        if ((long)millis() - strobe_timer > STROBE_PERIOD) {
          strobe_timer = millis();
          strobeUp_flag = true;
          strobeDwn_flag = false;
        }
        if ((long)millis() - strobe_timer > light_time) {
          strobeDwn_flag = true;
        }
        if (strobeUp_flag) {                    // если настало время пыхнуть
          if (strobe_bright < 255)              // если яркость не максимальная
            strobe_bright += STROBE_SMOOTH;     // увелчить
          if (strobe_bright > 255) {            // если пробили макс. яркость
            strobe_bright = 255;                // оставить максимум
            strobeUp_flag = false;              // флаг опустить
          }
        }

        if (strobeDwn_flag) {                   // гаснем
          if (strobe_bright > 0)                // если яркость не минимальная
            strobe_bright -= STROBE_SMOOTH;     // уменьшить
          if (strobe_bright < 0) {              // если пробили мин. яркость
            strobeDwn_flag = false;
            strobe_bright = 0;                  // оставить 0
          }
        }
        animation();
      }
      if (this_mode == 6) animation();

      if (!IRLremote.receiving())    // если на ИК приёмник не приходит сигнал (без этого НЕ РАБОТАЕТ!)
        FastLED.show();         // отправить значения на ленту

      if (this_mode != 7)       // 7 режиму не нужна очистка!!!
        FastLED.clear();          // очистить массив пикселей
      main_timer = millis();    // сбросить таймер
    }
  }
}

void animation() {
  // согласно режиму
  switch (this_mode) {
    case 0:
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
        count++;
      }
      if (EMPTY_BRIGHT > 0) {
        CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
          leds[i] = this_dark;
        for (int i = MAX_CH + Llenght; i < NUM_LEDS; i++)
          leds[i] = this_dark;
      }
      break;
    case 1:
      if (millis() - rainbow_timer > 30) {
        rainbow_timer = millis();
        hue = floor((float)hue + RAINBOW_STEP);
      }
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue);  // заливка по палитре радуга
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue); // заливка по палитре радуга
        count++;
      }
      if (EMPTY_BRIGHT > 0) {
        CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
          leds[i] = this_dark;
        for (int i = MAX_CH + Llenght; i < NUM_LEDS; i++)
          leds[i] = this_dark;
      }
      break;
    case 2:
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < STRIPE)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < STRIPE * 2) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIPE * 3) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
        else if (i < STRIPE * 4) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIPE * 5) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
      }
      break;
    case 3:
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < NUM_LEDS / 3)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < NUM_LEDS * 2 / 3) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < NUM_LEDS)         leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
      }
      break;
    case 4:
      switch (freq_strobe_mode) {
        case 0:
          if (colorMusicFlash[2]) HIGHS();
          else if (colorMusicFlash[1]) MIDS();
          else if (colorMusicFlash[0]) LOWS();
          else SILENCE();
          break;
        case 1:
          if (colorMusicFlash[2]) HIGHS();
          else SILENCE();
          break;
        case 2:
          if (colorMusicFlash[1]) MIDS();
          else SILENCE();
          break;
        case 3:
          if (colorMusicFlash[0]) LOWS();
          else SILENCE();
          break;
      }
      break;
    case 5:
      /*if (strobe_bright > 0)
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(STROBE_COLOR, STROBE_SAT, strobe_bright);
      else
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);*/
    //Режим стробоскопа заменён на режим огня
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = getFireColor((inoise8(i * FIRE_STEP, counter)));
      }
      counter += 20;
      break;
    case 6:
	     switch (light_mode) {
        case 0: for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LIGHT_COLOR, LIGHT_SAT, 255);
          break;
        case 1:
          if (millis() - color_timer > COLOR_SPEED) {
            color_timer = millis();
            if (++this_color > 255) this_color = 0;
          }
          for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(this_color, LIGHT_SAT, 255);
          break;
        case 2:
          if (millis() - rainbow_timer > 30) {
            rainbow_timer = millis();
            this_color += RAINBOW_PERIOD;
            if (this_color > 255) this_color = 0;
            if (this_color < 0) this_color = 255;
          }
          rainbow_steps = this_color;
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV((int)floor(rainbow_steps), 255, 255);
            rainbow_steps += RAINBOW_STEP_2;
            if (rainbow_steps > 255) rainbow_steps = 0;
            if (rainbow_steps < 0) rainbow_steps = 255;
          }
          break;
      }
      break;
    case 7:
      switch (freq_strobe_mode) {
        case 0:
          if (running_flag[2]) leds[NUM_LEDS / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
          else if (running_flag[1]) leds[NUM_LEDS / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
          else if (running_flag[0]) leds[NUM_LEDS / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 1:
          if (running_flag[2]) leds[NUM_LEDS / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 2:
          if (running_flag[1]) leds[NUM_LEDS / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 3:
          if (running_flag[0]) leds[NUM_LEDS / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
      }
      leds[(NUM_LEDS / 2) - 1] = leds[NUM_LEDS / 2];
      if (millis() - running_timer > RUNNING_SPEED) {
        running_timer = millis();
        for (int i = 0; i < NUM_LEDS / 2 - 1; i++) {
          leds[i] = leds[i + 1];
          leds[NUM_LEDS - i - 1] = leds[i];
        }
      }
      break;
    case 8:
      byte HUEindex = HUE_START;
      for (int i = 0; i < NUM_LEDS / 2; i++) {
        byte this_bright = map(freq_f[(int)floor((NUM_LEDS / 2 - i) / freq_to_stripe)], 0, freq_max_f, 0, 255);
        this_bright = constrain(this_bright, 0, 255);
        leds[i] = CHSV(HUEindex, 255, this_bright);
        leds[NUM_LEDS - i - 1] = leds[i];
        HUEindex += HUE_STEP;
        if (HUEindex > 255) HUEindex = 0;
      }
      break;
  }
}

void HIGHS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
}
void MIDS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
}
void LOWS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
}
void SILENCE() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
}

// вспомогательная функция, изменяет величину value на шаг incr в пределах minimum.. maximum
int smartIncr(int value, int incr_step, int mininmum, int maximum) {
  int val_buf = value + incr_step;
  val_buf = constrain(val_buf, mininmum, maximum);
  return val_buf;
}

float smartIncrFloat(float value, float incr_step, float mininmum, float maximum) {
  float val_buf = value + incr_step;
  val_buf = constrain(val_buf, mininmum, maximum);
  return val_buf;
}

float smartConvertFloat(float value, float mininmum, float maximum, boolean invert) {
  if (invert) value = (value - 100) * (-1);
  float val_buf = value / (100/maximum);
  val_buf = constrain(val_buf, mininmum, maximum);
  return val_buf;
}

byte smartConvertByte(float value, float mininmum, float maximum) {
  float val_buf = value / (maximum/100);
  //val_buf = constrain(val_buf, mininmum, maximum);
  return (byte)val_buf;
}

#if REMOTE_TYPE != 0

void remoteTick() {
#if REMOTE_TYPE <=3
  if (IRLremote.available())  {
    auto data = IRLremote.read();
    IRdata = data.command;
    ir_flag = true;
  }
#else
  
  
  if (Serial.available() > 0) {  //если пришли данные
    int sum = 0;
    for (int i = 0; i < 2; ++i) {
      if (Serial.available() <= 0) {
        ir_flag = false;
        break;
      }
      settings[i] = Serial.read();
      sum += settings[i];
      delay(5);
      ir_flag = true;
    }
    if (sum <= 1) {
        ir_flag = false;
    }
      

    //IRdata =  Serial.read();    //получаем ххх
    
    
  }
#endif
  

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (ir_flag) { // если данные пришли
    eeprom_timer = millis();
    eeprom_flag = true;
    LED_WORK_FL = true;
    //byte settt[] = {2,5,44,77,33,55,66,77};
    //if (settings[0] == 10)
    //{
    //   requestParametr();
    //   for (int i = 0; i < 8; ++i) {
    //   Serial.write(settings[i]);
    //   //Serial.print(" ");
    //   }
    //}

    //if (settings[6] == 1) //Если пришла команда выключения или включения
    //{
    //  ONstate = !ONstate; 
    //  FastLED.clear(); 
    //  FastLED.show(); 
    //  updateEEPROM();
    //}
    //if (settings[7] == 1) //Если пришла команда настройки шума
    //{
    //   fullLowPass();
    //}

    // if (settings[0] <= 8)
    // {
    //  this_mode = settings[0];
    //  FastLED.setBrightness(smartConvertFloat(settings[2], 0, 255));
    // EMPTY_BRIGHT = smartConvertFloat(settings[3], 0, 255);
    // }

///////////////////////////////////////////////////////////////////////////////////////////////////////////

    switch (settings[0]) {
      case 0: break;
      case 1: 
        if (settings[1] == 1) {
            ONstate = !ONstate; 
            FastLED.clear(); 
            FastLED.show(); 
            updateEEPROM();
        }
        break;
      case 2:
        if (settings[1] == 1) {
            fullLowPass();
        }
        break;
      case 3:
        if (settings[1] <= 8) {
            this_mode = settings[1];
        }
        break;
      case 4:
        switch (this_mode) {
          case 4:
          case 6:
            light_mode = (int8_t)settings[1];
            break;
          case 7:
            freq_strobe_mode = (int8_t)settings[1];
            break;
        }
        break;
      case 5:
        FastLED.setBrightness(smartConvertFloat(settings[1], 0, 255, false));
        break;
      case 6:
        EMPTY_BRIGHT = smartConvertFloat(settings[1], 0, 255, false);
        break;
      case 7:
        SMOOTH = smartConvertFloat(settings[1], 0.05, 1, true);
        break;
      case 8:
        RAINBOW_STEP = smartConvertFloat(settings[1], 0.5, 20, false);   //чтобы число было минимум 0.5 и максимум 20
        break;
      case 9:
        SMOOTH_FREQ = smartConvertFloat(settings[1], 0.05, 1, true);
        break;
      case 10:
        MAX_COEF_FREQ = smartConvertFloat(settings[1], 0, 5, true);
        break;
      case 11:
        LIGHT_COLOR = smartConvertFloat(settings[1], 0, 255, false);
        break;
      case 12:
        LIGHT_SAT = smartConvertFloat(settings[1], 0, 255, false);
        break;
      case 13:
        COLOR_SPEED = smartConvertFloat(settings[1], 0, 255, true);
        break;
      case 14:
        RAINBOW_PERIOD = smartConvertFloat(settings[1], -20, 20, false);  //????????????? Написать правильный метод конвертации
        break;
      case 15:
        RAINBOW_STEP_2 = smartConvertFloat(settings[1], 0.5, 10, true);
        break;
      case 16:
        RUNNING_SPEED = smartConvertFloat(settings[1], 1, 255, true);
        break;
      case 17:
        HUE_STEP = smartConvertFloat(settings[1], 1, 255, false);
        break;
      case 18:
        HUE_START = smartConvertFloat(settings[1], 0, 255, false);
        break;
      default: eeprom_flag = false;   // если не распознали кнопку, не обновляем настройки!
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////


    // switch (settings[0]) {
    //   case 0: 
    //     SMOOTH = smartConvertFloat(settings[4], 0.05, 1);
    //     break;
    //   case 1: 
    //     SMOOTH = smartConvertFloat(settings[4], 0.05, 1);     //чтобы число было минимум 0.05 и максимум 1
    //     RAINBOW_STEP = smartConvertFloat(settings[5], 0.5, 20);   //чтобы число было минимум 0.5 и максимум 20
    //     break;
    //   case 2:
    //   case 3: 
    //     SMOOTH_FREQ = smartConvertFloat(settings[4], 0.05, 1);
    //     MAX_COEF_FREQ = smartConvertFloat(settings[5], 0, 5);
    //     break;
    //   case 4: 
    //     light_mode = (int8_t)settings[1];
    //     SMOOTH_FREQ = smartConvertFloat(settings[4], 0.05, 1);
    //     MAX_COEF_FREQ = smartConvertFloat(settings[5], 0, 5);
    //     break;
    //   case 5: 
    //     break;
    //   case 6: 
    //     light_mode = (int8_t)settings[1];
    //     switch (settings[1]) {
    //       case 0: 
    //         LIGHT_COLOR = smartConvertFloat(settings[4], 0, 255);
    //         LIGHT_SAT = smartConvertFloat(settings[5], 0, 255);
    //         break;
    //       case 1: 
    //         LIGHT_SAT = smartConvertFloat(settings[4], 0, 255);
    //         COLOR_SPEED = smartConvertFloat(settings[5], 0, 255);
    //         break;
    //       case 2: 
    //         RAINBOW_PERIOD = smartConvertFloat(settings[4], -20, 20);  //????????????? Написать правильный метод конвертации
    //         RAINBOW_STEP_2 = smartConvertFloat(settings[5], 0.5, 10);
    //         break;
    //           }
    //     break;
    //   case 7: 
    //     freq_strobe_mode = (int8_t)settings[1];
    //     RUNNING_SPEED = smartConvertFloat(settings[4], 1, 255);
    //     MAX_COEF_FREQ = smartConvertFloat(settings[5], 0.0, 10);
    //     break;
    //   case 8: 
    //     HUE_STEP = smartConvertFloat(settings[4], 1, 255);
    //     HUE_START = smartConvertFloat(settings[5], 0, 255);
    //     break;
    //   default: eeprom_flag = false;   // если не распознали кнопку, не обновляем настройки!
    //     break;
    // }
    // Нужны кнопки настройки шумов и включения и отключения
    ir_flag = false;
    for (int j = 0; j < 2; ++j)
    {
      Serial.print(F("SETTING "));
      Serial.print(j);
      Serial.print(F(" = "));
      Serial.println(settings[j]);
    }/*
    
    switch (IRdata) {
      // режимы
      case BUTT_1: this_mode = 0;
        break;
      case BUTT_2: this_mode = 1;
        break;
      case BUTT_3: this_mode = 2;
        break;
      case BUTT_4: this_mode = 3;
        break;
      case BUTT_5: this_mode = 4;
        break;
      case BUTT_6: this_mode = 5;
        break;
      case BUTT_7: this_mode = 6;
        break;
      case BUTT_8: this_mode = 7;
        break;
      case BUTT_9: this_mode = 8;
        break;
      case BUTT_0: fullLowPass();
        break;
      case BUTT_STAR: ONstate = !ONstate; FastLED.clear(); FastLED.show(); updateEEPROM();
        break;
      case BUTT_HASH:
        switch (this_mode) {
          case 4:
          case 7: if (++freq_strobe_mode > 3) freq_strobe_mode = 0;
            break;
          case 6: if (++light_mode > 2) light_mode = 0;
            break;
        }
        break;
      case BUTT_OK: digitalWrite(MLED_PIN, settings_mode ^ MLED_ON); settings_mode = !settings_mode;
        break;
      case BUTT_UP:
        if (settings_mode) {
          // ВВЕРХ общие настройки
          EMPTY_BRIGHT = smartIncr(EMPTY_BRIGHT, 5, 0, 255);
        } else {
          switch (this_mode) {
            case 0:
              break;
            case 1: RAINBOW_STEP = smartIncrFloat(RAINBOW_STEP, 0.5, 0.5, 20);
              break;
            case 2:
            case 3:
            case 4: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, 0.1, 0, 5);
              break;
            case 5: STROBE_PERIOD = smartIncr(STROBE_PERIOD, 20, 1, 1000);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);
                  break;
                case 1: LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);
                  break;
                case 2: RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, 0.5, 0.5, 10);
                  break;
              }
              break;
            case 7: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, 0.1, 0.0, 10);
              break;
            case 8: HUE_START = smartIncr(HUE_START, 10, 0, 255);
              break;
          }
        }
        break;
      case BUTT_DOWN:
        if (settings_mode) {
          // ВНИЗ общие настройки
          EMPTY_BRIGHT = smartIncr(EMPTY_BRIGHT, -5, 0, 255);
        } else {
          switch (this_mode) {
            case 0:
              break;
            case 1: RAINBOW_STEP = smartIncrFloat(RAINBOW_STEP, -0.5, 0.5, 20);
              break;
            case 2:
            case 3:
            case 4: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, -0.1, 0, 5);
              break;
            case 5: STROBE_PERIOD = smartIncr(STROBE_PERIOD, -20, 1, 1000);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_SAT = smartIncr(LIGHT_SAT, -20, 0, 255);
                  break;
                case 1: LIGHT_SAT = smartIncr(LIGHT_SAT, -20, 0, 255);
                  break;
                case 2: RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, -0.5, 0.5, 10);
                  break;
              }
              break;
            case 7: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, -0.1, 0.0, 10);
              break;
            case 8: HUE_START = smartIncr(HUE_START, -10, 0, 255);
              break;
          }
        }
        break;
      case BUTT_LEFT:
        if (settings_mode) {
          // ВЛЕВО общие настройки
          BRIGHTNESS = smartIncr(BRIGHTNESS, -20, 0, 255);
          FastLED.setBrightness(BRIGHTNESS);
        } else {
          switch (this_mode) {
            case 0:
            case 1: SMOOTH = smartIncrFloat(SMOOTH, -0.05, 0.05, 1);
              break;
            case 2:
            case 3:
            case 4: SMOOTH_FREQ = smartIncrFloat(SMOOTH_FREQ, -0.05, 0.05, 1);
              break;
            case 5: STROBE_SMOOTH = smartIncr(STROBE_SMOOTH, -20, 0, 255);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_COLOR = smartIncr(LIGHT_COLOR, -10, 0, 255);
                  break;
                case 1: COLOR_SPEED = smartIncr(COLOR_SPEED, -10, 0, 255);
                  break;
                case 2: RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, -1, -20, 20);
                  break;
              }
              break;
            case 7: RUNNING_SPEED = smartIncr(RUNNING_SPEED, -10, 1, 255);
              break;
            case 8: HUE_STEP = smartIncr(HUE_STEP, -1, 1, 255);
              break;
          }
        }
        break;
      case BUTT_RIGHT:
        if (settings_mode) {
          // ВПРАВО общие настройки
          BRIGHTNESS = smartIncr(BRIGHTNESS, 20, 0, 255);
          FastLED.setBrightness(BRIGHTNESS);
        } else {
          switch (this_mode) {
            case 0:
            case 1: SMOOTH = smartIncrFloat(SMOOTH, 0.05, 0.05, 1);
              break;
            case 2:
            case 3:
            case 4: SMOOTH_FREQ = smartIncrFloat(SMOOTH_FREQ, 0.05, 0.05, 1);
              break;
            case 5: STROBE_SMOOTH = smartIncr(STROBE_SMOOTH, 20, 0, 255);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_COLOR = smartIncr(LIGHT_COLOR, 10, 0, 255);
                  break;
                case 1: COLOR_SPEED = smartIncr(COLOR_SPEED, 10, 0, 255);
                  break;
                case 2: RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, 1, -20, 20);
                  break;
              }
              break;
            case 7: RUNNING_SPEED = smartIncr(RUNNING_SPEED, 10, 1, 255);
              break;
            case 8: HUE_STEP = smartIncr(HUE_STEP, 1, 1, 255);
              break;
          }
        }
        break;
      default: eeprom_flag = false;   // если не распознали кнопку, не обновляем настройки!
        break;
    }*/
    
    /////////////////////////////////////////////////////////////////////////////////////////////////
  }
}
#endif

void autoLowPass() {
  // для режима VU
  delay(10);                                // ждём инициализации АЦП
  int thisMax = 0;                          // максимум
  int thisLevel;
  for (byte i = 0; i < 200; i++) {
    thisLevel = analogRead(SOUND_R);        // делаем 200 измерений
    if (thisLevel > thisMax)                // ищем максимумы
      thisMax = thisLevel;                  // запоминаем
    delay(4);                               // ждём 4мс
  }
  LOW_PASS = thisMax + LOW_PASS_ADD;        // нижний порог как максимум тишины + некая величина

  // для режима спектра
  thisMax = 0;
  for (byte i = 0; i < 100; i++) {          // делаем 100 измерений
    analyzeAudio();                         // разбить в спектр
    for (byte j = 2; j < 32; j++) {         // первые 2 канала - хлам
      thisLevel = fht_log_out[j];
      if (thisLevel > thisMax)              // ищем максимумы
        thisMax = thisLevel;                // запоминаем
    }
    delay(4);                               // ждём 4мс
  }
  SPEKTR_LOW_PASS = thisMax + LOW_PASS_FREQ_ADD;  // нижний порог как максимум тишины
  if (EEPROM_LOW_PASS && !AUTO_LOW_PASS) {
    EEPROM.updateInt(70, LOW_PASS);
    EEPROM.updateInt(72, SPEKTR_LOW_PASS);
  }
}

void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(SOUND_R_FREQ);
    fht_input[i] = sample; // put real data into bins
  }
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht
}

void buttonTick() {
  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  if (butt1.isSingle())                              // если единичное нажатие
    if (++this_mode >= MODE_AMOUNT) this_mode = 0;   // изменить режим

  if (butt1.isHolded()) {     // кнопка удержана
    fullLowPass();
  }
}
void fullLowPass() {
  digitalWrite(MLED_PIN, MLED_ON);   // включить светодиод
  FastLED.setBrightness(0); // погасить ленту
  FastLED.clear();          // очистить массив пикселей
  FastLED.show();           // отправить значения на ленту
  delay(500);               // подождать чутка
  autoLowPass();            // измерить шумы
  delay(500);               // подождать
  FastLED.setBrightness(BRIGHTNESS);  // вернуть яркость
  digitalWrite(MLED_PIN, !MLED_ON);    // выключить светодиод
}
void updateEEPROM() {
  EEPROM.updateByte(1, this_mode);
  EEPROM.updateByte(2, freq_strobe_mode);
  EEPROM.updateByte(3, light_mode);
  EEPROM.updateInt(4, RAINBOW_STEP);
  EEPROM.updateFloat(8, MAX_COEF_FREQ);
  EEPROM.updateInt(12, STROBE_PERIOD);
  EEPROM.updateInt(16, LIGHT_SAT);
  EEPROM.updateFloat(20, RAINBOW_STEP_2);
  EEPROM.updateInt(24, HUE_START);
  EEPROM.updateFloat(28, SMOOTH);
  EEPROM.updateFloat(32, SMOOTH_FREQ);
  EEPROM.updateInt(36, STROBE_SMOOTH);
  EEPROM.updateInt(40, LIGHT_COLOR);
  EEPROM.updateInt(44, COLOR_SPEED);
  EEPROM.updateInt(48, RAINBOW_PERIOD);
  EEPROM.updateInt(52, RUNNING_SPEED);
  EEPROM.updateInt(56, HUE_STEP);
  EEPROM.updateInt(60, EMPTY_BRIGHT);
  if (KEEP_STATE) EEPROM.updateByte(64, ONstate);
}
void readEEPROM() {
  this_mode = EEPROM.readByte(1);
  freq_strobe_mode = EEPROM.readByte(2);
  light_mode = EEPROM.readByte(3);
  RAINBOW_STEP = EEPROM.readInt(4);
  MAX_COEF_FREQ = EEPROM.readFloat(8);
  STROBE_PERIOD = EEPROM.readInt(12);
  LIGHT_SAT = EEPROM.readInt(16);
  RAINBOW_STEP_2 = EEPROM.readFloat(20);
  HUE_START = EEPROM.readInt(24);
  SMOOTH = EEPROM.readFloat(28);
  SMOOTH_FREQ = EEPROM.readFloat(32);
  STROBE_SMOOTH = EEPROM.readInt(36);
  LIGHT_COLOR = EEPROM.readInt(40);
  COLOR_SPEED = EEPROM.readInt(44);
  RAINBOW_PERIOD = EEPROM.readInt(48);
  RUNNING_SPEED = EEPROM.readInt(52);
  HUE_STEP = EEPROM.readInt(56);
  EMPTY_BRIGHT = EEPROM.readInt(60);
  if (KEEP_STATE) ONstate = EEPROM.readByte(64);
}
void eepromTick() {
  if (eeprom_flag)
    if (millis() - eeprom_timer > 30000) {  // 30 секунд после последнего нажатия с пульта
      eeprom_flag = false;
      eeprom_timer = millis();
      updateEEPROM();
    }
}
#if REMOTE_TYPE == 4
void ledBlink() {
  if (LED_WORK_FL) {
    digitalWrite(LED_WORK, HIGH);
    LED_WORK_FL = false;
    led_timer = millis();
  }
  if (!LED_WORK_FL && ((millis() - led_timer) >= 100)) {
    digitalWrite(LED_WORK, LOW);
    led_timer = 0xFFFFFFFFUL;    
  }
}
#endif

// возвращает цвет огня для одного пикселя
CHSV getFireColor(int val) {
  // чем больше val, тем сильнее сдвигается цвет, падает насыщеность и растёт яркость
  return CHSV(
           HUE_START_FIRE + map(val, 0, 255, 0, HUE_GAP),                    // H
           constrain(map(val, 0, 255, MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(val, 0, 255, MIN_BRIGHT, MAX_BRIGHT), 0, 255)  // V
         );
}
