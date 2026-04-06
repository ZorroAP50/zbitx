#include <Arduino.h>
#include <TFT_eSPI.h>
#include "zbitx.h"

#define WIFI_LIST_MAX   20
#define WIFI_SSID_LEN   33
#define WIFI_ROW_H      18

static char wifi_ssids[WIFI_LIST_MAX][WIFI_SSID_LEN];
static int  wifi_count    = 0;
static int  wifi_top      = 0;   // first visible row index
static int  wifi_selected = -1;  // highlighted row (-1 = none)

void wifi_list_init(){
    memset(wifi_ssids, 0, sizeof(wifi_ssids));
    wifi_count    = 0;
    wifi_top      = 0;
    wifi_selected = -1;
}

/* Called from field_set when label == "WIFI_LIST"
   value format: "ssid1|ssid2|ssid3|..."   (pipe-separated, no spaces around) */
void wifi_list_update(const char *value){
    wifi_list_init();
    char buf[WIFI_LIST_MAX * WIFI_SSID_LEN];
    strncpy(buf, value, sizeof(buf)-1);
    buf[sizeof(buf)-1] = 0;

    char *p = strtok(buf, "|");
    while (p && wifi_count < WIFI_LIST_MAX){
        strncpy(wifi_ssids[wifi_count], p, WIFI_SSID_LEN-1);
        wifi_ssids[wifi_count][WIFI_SSID_LEN-1] = 0;
        wifi_count++;
        p = strtok(NULL, "|");
    }
}

void wifi_list_draw(struct field *f){
    screen_fill_rect(f->x, f->y, f->w, f->h, TFT_BLACK);

    if (wifi_count == 0){
        screen_draw_text("No networks — press SCAN", -1, f->x+4, f->y+4, TFT_DARKGREY, 2);
        return;
    }

    int visible = f->h / WIFI_ROW_H;
    // clamp top
    if (wifi_top > wifi_count - visible && wifi_count >= visible)
        wifi_top = wifi_count - visible;
    if (wifi_top < 0)
        wifi_top = 0;

    for (int i = 0; i < visible && (wifi_top + i) < wifi_count; i++){
        int idx = wifi_top + i;
        int y   = f->y + i * WIFI_ROW_H;
        uint16_t bg  = (idx == wifi_selected) ? TFT_NAVY   : TFT_BLACK;
        uint16_t fg  = (idx == wifi_selected) ? TFT_WHITE  : TFT_CYAN;
        screen_fill_rect(f->x, y, f->w, WIFI_ROW_H-1, bg);
        screen_draw_text(wifi_ssids[idx], -1, f->x+4, y+1, fg, 2);
    }

    // scroll indicator
    if (wifi_count > visible){
        int bar_h = (f->h * visible) / wifi_count;
        int bar_y = f->y + (f->h * wifi_top) / wifi_count;
        screen_fill_rect(f->x + f->w - 4, f->y,    4, f->h,  TFT_DARKGREY);
        screen_fill_rect(f->x + f->w - 4, bar_y,   4, bar_h, TFT_WHITE);
    }
}

/* Returns index of SSID under touch coordinates, or -1 */
int wifi_list_touch(struct field *f, uint16_t tx, uint16_t ty){
    if (tx < f->x || tx > f->x + f->w) return -1;
    if (ty < f->y || ty > f->y + f->h) return -1;
    int row = (ty - f->y) / WIFI_ROW_H;
    int idx = wifi_top + row;
    if (idx >= 0 && idx < wifi_count)
        return idx;
    return -1;
}

/* Called when user taps on the list; fills WIFI_SSID field */
void wifi_list_select(struct field *f, uint16_t tx, uint16_t ty){
    int idx = wifi_list_touch(f, tx, ty);
    if (idx < 0) return;

    wifi_selected = idx;
    f->redraw = true;

    // Copy selected SSID to WIFI_SSID field
    struct field *f_ssid = field_get("WIFI_SSID");
    if (f_ssid){
        strncpy(f_ssid->value, wifi_ssids[idx], FIELD_TEXT_MAX_LENGTH-1);
        f_ssid->value[FIELD_TEXT_MAX_LENGTH-1] = 0;
        f_ssid->redraw = true;
    }

    // Move focus to password field and show keyboard
    struct field *f_pass = field_get("WIFI_PASS");
    if (f_pass){
        extern struct field *f_selected;
        f_selected = f_pass;
        f_pass->redraw = true;
        keyboard_show(EDIT_STATE_ALPHA);
    }
}

/* Scroll the list */
void wifi_list_scroll(struct field *f, int direction){
    if (direction > 0 && wifi_top + 1 < wifi_count)
        wifi_top++;
    else if (direction < 0 && wifi_top > 0)
        wifi_top--;
    f->redraw = true;
}
