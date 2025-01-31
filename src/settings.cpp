#include <cstdio>
#include <CRC.h>
#include <EEPROM.h>
#include "settings.h"

#define DEFAULT_AUTO_ON_DH 15
#define DEFAULT_AUTO_OFF_DH 20
#define DEFAULT_CHECK_INTERVAL 30
#define DEFAULT_ALTITUDE 0
#define DEFAULT_INSIDE_SENSOR_NAME "inside"
#define DEFAULT_OUTSIDE_SENSOR_NAME "outside"
#define DEFAULT_REVERSE_STUCK_CHECK_INTERVAL 15
#define DEFAULT_REVERSE_STUCK_MAX_TEMPERATURE 20

namespace {

void sanitize_name(char * s) {
    // ensure there's a terminator
    s[settings::SENSOR_NAME_MAX_LENGTH - 1] = '\0';

    for (char * t = s; *t; ++t) {
        // remove control chars and double quotes
        if (*t < ' ' || *t == '"')
            *t = '_';
    }
}

};

namespace settings {

Settings settings;

void sanitize() {
    if (settings.data.auto_on_dh > 100)
        settings.data.auto_on_dh = 100;

    if (settings.data.auto_off_dh > 100)
        settings.data.auto_off_dh = 100;

    // auto_on_dh must be grater or equal to auto_off_dh
    if (settings.data.auto_off_dh > settings.data.auto_on_dh) {
        const auto tmp = settings.data.auto_off_dh;
        settings.data.auto_off_dh = settings.data.auto_on_dh;
        settings.data.auto_on_dh = tmp;
    }

    if (settings.data.sensor_check_interval < 5)
        settings.data.sensor_check_interval = 5;

    if (settings.data.sensor_check_interval >= 10 * 60)
        settings.data.sensor_check_interval = 10 * 60;

    if (settings.data.altitude >= 9000)
        settings.data.altitude = 9000;

    sanitize_name(settings.data.inside_sensor_name);
    sanitize_name(settings.data.outside_sensor_name);

    if (settings.data.reverse_stuck_check_interval < 0)
        settings.data.reverse_stuck_check_interval = 0;

    if (settings.data.reverse_stuck_check_interval > 180)
        settings.data.reverse_stuck_check_interval = 180;

    if (settings.data.reverse_stuck_max_temperature < 0)
        settings.data.reverse_stuck_max_temperature = 0;

    if (settings.data.reverse_stuck_max_temperature > 50)
        settings.data.reverse_stuck_max_temperature = 50;
}

void load() {
    EEPROM.begin(sizeof(Settings));
    EEPROM.get(0, settings);

    const auto expected_checksum = CRC::crc16(&settings.data, sizeof(settings.data));
    if (settings.checksum != expected_checksum) {
        printf("Invalid CRC of settings in flash, using defaults.\n");
        settings.data.auto_on_dh = DEFAULT_AUTO_ON_DH;
        settings.data.auto_off_dh = DEFAULT_AUTO_OFF_DH;
        settings.data.sensor_check_interval = DEFAULT_CHECK_INTERVAL;
        settings.data.altitude = DEFAULT_ALTITUDE;
        strcpy(settings.data.inside_sensor_name, DEFAULT_INSIDE_SENSOR_NAME);
        strcpy(settings.data.outside_sensor_name, DEFAULT_OUTSIDE_SENSOR_NAME);
        settings.data.reverse_stuck_check_interval = DEFAULT_REVERSE_STUCK_CHECK_INTERVAL;
        settings.data.reverse_stuck_max_temperature = DEFAULT_REVERSE_STUCK_MAX_TEMPERATURE;
    } else {
        printf("Loaded settings from flash, CRC correct.\n");
    }

    sanitize();
    print();
}

void save() {
    settings.checksum = CRC::crc16(&settings.data, sizeof(settings.data));
    EEPROM.put(0, settings);
    EEPROM.commit();
}

void print() {
    printf("Settings:\n");
    printf("  auto on humidity difference:    %i\n", settings.data.auto_on_dh);
    printf("  auto off humidity difference:   %i\n", settings.data.auto_off_dh);
    printf("  sensor check interval:          %i\n", settings.data.sensor_check_interval);
    printf("  altitude:                       %i\n", settings.data.altitude);
    printf("  inside sensor name:             %s\n", settings.data.inside_sensor_name);
    printf("  outside sensor name:            %s\n", settings.data.outside_sensor_name);
    printf("  reverse stuck check interval:   %i\n", settings.data.reverse_stuck_check_interval);
    printf("  reverse stuck max temperature:  %i\n", settings.data.reverse_stuck_max_temperature);
}

}
