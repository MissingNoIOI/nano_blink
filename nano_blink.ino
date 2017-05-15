#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <Bounce2.h>

#define OLED_RESET 4
#define DRAW_DELAY 118
#define D_NUM 32
#define D_SIZE 20
#define DS18B20_PIN 2
#define BUZZER_PIN 6
#define MODE_PIN 4
#define SET_PIN 5


Adafruit_SSD1306 display(OLED_RESET);
DS1307 RTC;
char* TIME;
char* DATE;
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

Bounce mode_button = Bounce();
Bounce set_button = Bounce();

struct timeStruct {
        uint16_t Year;
        uint8_t Month;
        uint8_t Day;
        uint8_t Hour;
        uint8_t Minute;
        uint8_t Second;
};

struct alarmStruct {
        uint8_t Dow;
        uint8_t Hour;
        uint8_t Minute;
        uint8_t Second;
        boolean Alarm_on;
};

alarmStruct alarm {0};

void setup()   {
        tone(BUZZER_PIN, 1000, 1000);
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

        Wire.begin();
        RTC.begin();
        Serial.begin(9600);

        TIME = (char*)malloc(D_SIZE);
        DATE = (char*)malloc(D_SIZE);

        pinMode(MODE_PIN, INPUT);
        mode_button.attach(MODE_PIN);
        mode_button.interval(1000);
        pinMode(SET_PIN, INPUT);
        set_button.attach(SET_PIN);
        set_button.interval(1000);

        sensors.begin();

        EEPROM_readAnything(0, alarm);
}

char* get_day_of_week(uint8_t dow){
        switch(dow){
        case 0: return((char*)"So"); break;
        case 1: return((char*)"Mo"); break;
        case 2: return((char*)"Di"); break;
        case 3: return((char*)"Mi"); break;
        case 4: return((char*)"Do"); break;
        case 5: return((char*)"Fr"); break;
        case 6: return((char*)"Sa"); break;
        case 7: return((char*)"WT"); break;
        case 8: return((char*)"WE"); break;
        case 9: return((char*)"* "); break;
        default: return((char*)"NA"); break;
        }

}

char* get_on_or_off(boolean state) {
        if (state) {
                return((char*)"ON");
        }
        else {
                return((char*)"OFF");
        }
}

void format_time_and_date(DateTime datetime){

        memset(TIME, '\0', D_SIZE);
        memset(DATE, '\0', D_SIZE);

        sprintf(DATE, "%s - %u.%u.%u",
                get_day_of_week(datetime.dayOfWeek()),
                datetime.day(),
                datetime.month(),
                datetime.year()%100);

        sprintf(TIME, "%02u:%02u:%02u",
                datetime.hour(),
                datetime.minute(),
                datetime.second());
}


void increment_time(timeStruct* time, int edited_field) {
        switch(edited_field) {
        case 0: time->Year++;
                if (time->Year > 2049) time->Year = 2000;
                break;
        case 1: time->Month++;
                if (time->Month > 12) time->Month = 1;
                break;
        case 2: time->Day++;
                if ((time->Month == 2) && ((time->Year % 4) == 0)) {
                        if (time->Day > 27) time->Day = 1;
                }
                else if (time->Month == 2) {
                        if (time->Day > 28) time->Day = 1;
                }
                else if ((time->Month % 2) == 0) {
                        if (time->Day > 30) time->Day = 1;
                }
                else {
                        if (time->Day > 31) time->Day = 1;
                }
                break;
        case 3: time->Hour++;
                if (time->Hour > 23) time->Hour = 0;
                break;
        case 4: time->Minute++;
                if (time->Minute > 59) time->Minute = 0;
                break;
        case 5: time->Second++;
                if(time->Second > 59) time->Second = 0;
                break;
        }
}

void increment_alarm(int edited_field){
        switch(edited_field) {
        case 0: alarm.Dow++;
                if (alarm.Dow > 9) alarm.Dow = 0;
                break;
        case 1: alarm.Hour++;
                if (alarm.Hour > 23) alarm.Hour = 0;
                break;
        case 2: alarm.Minute++;
                if (alarm.Minute > 59) alarm.Minute = 0;
                break;
        case 3: alarm.Second++;
                if(alarm.Second > 59) alarm.Second = 0;
                break;
        case 4: alarm.Alarm_on = !alarm.Alarm_on;
        }
}

void draw_settings_screen(timeStruct* time, uint8_t edited_field, boolean blink) {
        char output[30];
        const char* time_hint = "YYYY-MM-DD HH:MM:SS";

        if (blink) {
                sprintf(output,
                        "%04u-%02u-%02u %02u:%02u:%02u",
                        time->Year,
                        time->Month,
                        time->Day,
                        time->Hour,
                        time->Minute,
                        time->Second);
        }
        else {
                switch(edited_field) {
                case 0: sprintf(output,
                                "    -%02u-%02u %02u:%02u:%02u",
                                time->Month,
                                time->Day,
                                time->Hour,
                                time->Minute,
                                time->Second);
                        break;
                case 1: sprintf(output,
                                "%04u-  -%02u %02u:%02u:%02u",
                                time->Year,
                                time->Day,
                                time->Hour,
                                time->Minute,
                                time->Second);
                        break;
                case 2: sprintf(output,
                                "%04u-%02u-   %02u:%02u:%02u",
                                time->Year,
                                time->Month,
                                time->Hour,
                                time->Minute,
                                time->Second);
                        break;
                case 3: sprintf(output,
                                "%04u-%02u-%02u   :%02u:%02u",
                                time->Year,
                                time->Month,
                                time->Day,
                                time->Minute,
                                time->Second);
                        break;
                case 4: sprintf(output,
                                "%04u-%02u-%02u %02u:  :%02u",
                                time->Year,
                                time->Month,
                                time->Day,
                                time->Hour,
                                time->Second);
                        break;
                case 5: sprintf(output,
                                "%04u-%02u-%02u %02u:%02u:  ",
                                time->Year,
                                time->Month,
                                time->Day,
                                time->Hour,
                                time->Minute);
                        break;
                }
        }


        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(1,10);
        display.println(output);
        display.println(time_hint);
        display.display();

}

void draw_alarm_screen(uint8_t edited_field, boolean blink) {
        char output[18] = {'\0'};
        if (blink) {
                sprintf(output,
                        "%s %02u:%02u:%02u %s",
                        get_day_of_week(alarm.Dow),
                        alarm.Hour,
                        alarm.Minute,
                        alarm.Second,
                        get_on_or_off(alarm.Alarm_on));
        }
        else {
                switch(edited_field) {
                case 0: sprintf(output,
                                "   %02u:%02u:%02u %s",
                                alarm.Hour,
                                alarm.Minute,
                                alarm.Second,
                                get_on_or_off(alarm.Alarm_on));
                        break;
                case 1: sprintf(output,
                                "%s   :%02u:%02u %s",
                                get_day_of_week(alarm.Dow),
                                alarm.Minute,
                                alarm.Second,
                                get_on_or_off(alarm.Alarm_on));
                        break;
                case 2: sprintf(output,
                                "%s %02u:  :%02u %s",
                                get_day_of_week(alarm.Dow),
                                alarm.Hour,
                                alarm.Second,
                                get_on_or_off(alarm.Alarm_on));
                        break;
                case 3: sprintf(output,
                                "%s %02u:%02u:   %s",
                                get_day_of_week(alarm.Dow),
                                alarm.Hour,
                                alarm.Minute,
                                get_on_or_off(alarm.Alarm_on));
                        break;
                case 4: sprintf(output,
                                "%s %02u:%02u:%02u   ",
                                get_day_of_week(alarm.Dow),
                                alarm.Hour,
                                alarm.Minute,
                                alarm.Second);

                        break;
                }
        }

        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(15,10);
        display.println(output);
        display.display();
}

void settings(boolean alarm_menu){
        DateTime now = RTC.now();
        timeStruct time = {now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second()};
        uint8_t edited_field = 0;

        unsigned long blink_time = millis();

        boolean blink = true;

        mode_button.interval(10);
        set_button.interval(10);

        while (true) {

                set_button.update();
                mode_button.update();

                if (mode_button.rose()) {
                        edited_field++;
                        if (edited_field > 5 && !alarm_menu) {
                                break;
                        }
                        else if (edited_field > 4 && alarm_menu) {
                                break;
                        }
                }

                if (set_button.rose()) {
                        if (!alarm_menu) {
                                increment_time(&time, edited_field);
                        }
                        else {
                                increment_alarm(edited_field);
                        }
                }

                if ((millis() - blink_time) > 300) {
                        blink = !blink;
                        blink_time = millis();
                }

                if (!alarm_menu) {
                        draw_settings_screen(&time, edited_field, blink);
                }
                else {
                        draw_alarm_screen(edited_field, blink);
                }

        }

        if (!alarm_menu) {
                RTC.adjust(DateTime(time.Year, time.Month, time.Day, time.Hour, time.Minute, time.Second));
        }
        else {
                EEPROM_writeAnything(0, alarm);
        }

        mode_button.interval(1000);
        set_button.interval(1000);
}

void alarm_screen() {
        unsigned long alarm_begin = millis();
        uint8_t counter = 0;
        mode_button.interval(10);
        set_button.interval(10);
        tone(BUZZER_PIN, 1000);
        while(true) {
                counter++;
                if (counter == 25 ) {
                        noTone(BUZZER_PIN);
                }
                else if (counter == 50 ) {
                        tone(BUZZER_PIN, 1000);
                        counter = 0;
                }
                display.clearDisplay();
                display.setTextColor(WHITE);
                display.setTextSize(2);
                display.setCursor(30,10);
                display.println("ALARM!");
                display.display();

                if (millis() - alarm_begin > 600000) {
                        noTone(BUZZER_PIN);
                        break;
                }

                set_button.update();
                mode_button.update();

                if (mode_button.rose()) {
                        noTone(BUZZER_PIN);
                        break;
                }
                if (set_button.rose()) {
                        noTone(BUZZER_PIN);
                        break;
                }
        }
        mode_button.interval(1000);
        set_button.interval(1000);
}

void check_alarm(DateTime time) {
        boolean right_dow = false;
        if (alarm.Alarm_on) {
                switch(alarm.Dow) {
                case 7: if (constrain(time.dayOfWeek(), 1, 5) == time.dayOfWeek()) right_dow = true;
                        break;
                case 8: if (constrain(time.dayOfWeek(), 6, 7) == time.dayOfWeek()) right_dow = true;
                        break;
                case 9: right_dow = true;
                        break;
                default: if (alarm.Dow == time.dayOfWeek()) right_dow = true;

                }

                if (right_dow) {
                        if (time.hour() == alarm.Hour && time.minute() == alarm.Minute && time.second() == alarm.Second) alarm_screen();
                }
        }
}

void loop() {

        DateTime now = RTC.now();
        format_time_and_date(now);

        sensors.requestTemperatures();

        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(1,0);
        display.print(DATE);
        display.print(" ");
        display.print(sensors.getTempCByIndex(0));
        display.print((char)247);
        display.println('C');
        display.setTextSize(2);
        display.setCursor(20,15);
        display.println(TIME);
        display.display();

        if (set_button.update()) {
                if (set_button.rose()) {
                        settings(true);
                }
        }
        if (mode_button.update()) {
                if (mode_button.rose()) {
                        settings(false);
                }
        }

        check_alarm(now);
}
