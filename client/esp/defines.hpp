
#ifndef DEFINES_H
#define DEFINES_H

#include "log.h"
#include <string>
#include <vector>


typedef void (*f_callback)(std::string type, const int status, std::vector <std::string> data);


void setup_button (
        const f_callback f);

// init sensors and start the task
void setup_sensors (
        const f_callback f);

void setup_pump (
        const f_callback f);

int get_soil_moisture_level (
        void);

int get_liquid_level (
        void);

void do_watering (
        void);

#endif // DEFINES_H
