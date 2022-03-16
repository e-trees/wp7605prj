/**
 * @file eventinput.h
 *
 */

#ifndef EVENTINPUT_H
#define EVENTINPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_EVENTINPUT
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the mouse
 */
void eventinput_init(void);

/**
 * Get the current position and state of the mouse
 * @param indev_drv pointer to the related input device driver
 * @param data store the mouse data here
 */
bool eventinput_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);


/**********************
 *      MACROS
 **********************/

#endif /* USE_EVENTINPUT */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EVENTINPUT_H */
