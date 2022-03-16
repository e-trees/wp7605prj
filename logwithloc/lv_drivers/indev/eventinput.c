/**
 * @file eventinput.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "eventinput.h"
#if USE_EVENTINPUT != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

int16_t calcX(int16_t raw);
int16_t calcY(int16_t raw);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool left_button_down = false;
static int16_t last_x = 0;
static int16_t last_y = 0;
struct input_event event;

int eventfd;

/**********************
 *      MACROS
 **********************/

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the mouse
 */
void eventinput_init(void)
{
    eventfd = open(LIBINPUT_NAME, O_RDWR | O_NONBLOCK);
    if(eventfd < 0)
    {
        perror("mouse open error\n");
    }
}

/**
 * Get the current position and state of the mouse
 * @param indev_drv pointer to the related input device driver
 * @param data store the mouse data here
 */
bool eventinput_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/
    int len;
    while((len = read(eventfd, &event, sizeof(event)))>0)
    {
        switch(event.type) {
        case EV_KEY:
            switch (event.code)
            {
            case BTN_TOUCH:
                left_button_down = event.value;
                break;
            }
            break;
        case EV_ABS:
            switch(event.code) {
            case ABS_X:
                last_x = calcX(event.value);
                break;
            case ABS_Y:
                last_y = calcY(event.value);
                break;
            case ABS_PRESSURE:
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    /*Store the collected data*/
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = left_button_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    return false;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

int16_t calcX(int16_t raw)
{
    double tmp = 1.0 * MIN(MAX(raw,EVENTINPUT_X_MIN),EVENTINPUT_X_MAX) - EVENTINPUT_X_MIN;
    tmp = tmp / (EVENTINPUT_X_MAX-EVENTINPUT_X_MIN) * EVENTINPUT_HOR_RES;
    return (EVENTINPUT_X_FLIP)?(EVENTINPUT_HOR_RES-tmp):tmp;
}

int16_t calcY(int16_t raw)
{
    double tmp = 1.0 * MIN(MAX(raw,EVENTINPUT_Y_MIN),EVENTINPUT_Y_MAX) - EVENTINPUT_Y_MIN;
    tmp= tmp / (EVENTINPUT_Y_MAX-EVENTINPUT_Y_MIN) * EVENTINPUT_VER_RES;
    return (EVENTINPUT_Y_FLIP)?(EVENTINPUT_VER_RES-tmp):tmp;
}

#endif