#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/eventinput.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "at.h"
#include "curl_module.h"

#define DISP_BUF_SIZE (128 * 1024)

char *endpoint_url;

lv_obj_t *label_gps;
lv_obj_t *ta;
static void sendbtn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        char *str = lv_textarea_get_text(ta);
        double lon = 0, lat = 0;
        int fix = 0;
        char requrl[256];
        time_t t = time(NULL);

        at_gpslocget(&lat, &lon, &fix);

        char *msgbuf = curl_urlenc(str);
        sprintf(requrl, "%s?msg=%s&lat=%.6f&lon=%.6f&time=%ld", endpoint_url, msgbuf, lat, lon, t);
        free(msgbuf);
        printf("req param url %s\n", requrl);
        curl_req(requrl);
        lv_textarea_set_text(ta, "");
    }
}

static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_user_data(e);
    if (code == LV_EVENT_FOCUSED)
    {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_DEFOCUSED)
    {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}
static void makeScreen(void)
{
    lv_coord_t width = lv_disp_get_hor_res(lv_disp_get_default());
    lv_coord_t height = lv_disp_get_ver_res(lv_disp_get_default());

    // Label
    lv_obj_t *cont = lv_obj_create(lv_scr_act());

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_flex_flow(&style, LV_FLEX_FLOW_ROW_WRAP);
    lv_style_set_flex_main_place(&style, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_style_set_layout(&style, LV_LAYOUT_FLEX);
    // Container
    lv_obj_set_size(cont, width, LV_SIZE_CONTENT);
    lv_obj_center(cont);
    lv_obj_set_align(cont, LV_ALIGN_TOP_MID);
    lv_obj_add_style(cont, &style, 0);

    // Title Label
    static lv_style_t style_title;
    lv_obj_t *label_title;
    label_title = lv_label_create(cont);
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_20);

    lv_obj_add_style(label_title, &style_title, 0);
    lv_label_set_text(label_title, "Logging with GPS");

    // Send Button
    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 60, LV_SIZE_CONTENT);
    lv_obj_add_event_cb(btn, sendbtn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Send");
    lv_obj_center(label);

    // Textarea & Keyboard
    /*Create a text area. The keyboard will write here*/

    ta = lv_textarea_create(cont);
    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t *kb = lv_keyboard_create(lv_scr_act());

    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_set_size(ta, width * 0.8, height / 4);
    lv_keyboard_set_textarea(kb, ta);

    // GPS Label
    label_gps = lv_label_create(cont);
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_12);

    lv_obj_add_style(label_gps, &style_title, 0);
    lv_label_set_text(label_gps, "Lat:x Lon:x");
}

int main(int argc, char **argv)
{
    double lon, lat;
    int fix;
    char strbuf[256];
    if (argc != 2)
    {
        printf("Usage %s <Endpoint URL to send the log>\n", argv[0]);
        return -1;
    }
    endpoint_url = argv[1];
    printf("Endpoint URL=%s\n", endpoint_url);

    at_open("/dev/ttyAT");
    at_gpsinit();

    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 320;
    lv_disp_drv_register(&disp_drv);

    /* Linux input device init */
    eventinput_init();

    /* Set up touchpad input device interface */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = eventinput_read;
    lv_indev_drv_register(&indev_drv);

    makeScreen();
    /*Handle LitlevGL tasks (tickless mode)*/
    int tick = 0;
    while (1)
    {
        if (tick > 200)
        {
            if (at_gpslocget(&lat, &lon, &fix) == 0 && fix != 0)
            {
                sprintf(strbuf, "Lat:%10.6f Lon:%9.6f", lat, lon);
                lv_label_set_text(label_gps, strbuf);
            }
            tick = 0;
        }
        lv_tick_inc(5);
        lv_task_handler();
        usleep(5000);
        tick++;
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}