/**
 * @file rak14000-epd.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Library for RAK14000 EPD interface
 * @version 0.1
 * @date 2022-09-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>

/* uncoment the define font(s) size that you're using here */
//#define FONT8 1
//#define FONT12 1
#define FONT16 1
//#define FONT20 1
//#define FONT24 1
/* ------------------------------------------------------  */

#ifndef _EPD_H_
#define _EPD_H_
#include "src/image.h"
#include "src/epdif.h"
#include "src/epdpaint.h"
#include "src/epdfonts.h"
#include "src/epd2in13.h"

#define COLORED 0
#define UNCOLORED 1

#endif //_EPD_H_

