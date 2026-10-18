#include "CDM_LVGLBridge.h"
#include "CDM_AST.h"
#include "CDM_PREPROC.h"
#include "lvgl_port.h"
#include "lvgl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static char g_lv_inc_buf[16384];

static void build_lv_inc_macros(void) {
  if (g_lv_inc_buf[0] != '\0') return;

  int n = snprintf(g_lv_inc_buf, sizeof(g_lv_inc_buf),

    "#define LV_ALIGN_DEFAULT          %d\n"
    "#define LV_ALIGN_TOP_LEFT         %d\n"
    "#define LV_ALIGN_TOP_MID          %d\n"
    "#define LV_ALIGN_TOP_RIGHT        %d\n"
    "#define LV_ALIGN_BOTTOM_LEFT      %d\n"
    "#define LV_ALIGN_BOTTOM_MID       %d\n"
    "#define LV_ALIGN_BOTTOM_RIGHT     %d\n"
    "#define LV_ALIGN_LEFT_MID         %d\n"
    "#define LV_ALIGN_RIGHT_MID        %d\n"
    "#define LV_ALIGN_CENTER           %d\n"
    "#define LV_ALIGN_OUT_TOP_LEFT     %d\n"
    "#define LV_ALIGN_OUT_TOP_MID      %d\n"
    "#define LV_ALIGN_OUT_TOP_RIGHT    %d\n"
    "#define LV_ALIGN_OUT_BOTTOM_LEFT  %d\n"
    "#define LV_ALIGN_OUT_BOTTOM_MID   %d\n"
    "#define LV_ALIGN_OUT_BOTTOM_RIGHT %d\n"
    "#define LV_ALIGN_OUT_LEFT_TOP     %d\n"
    "#define LV_ALIGN_OUT_LEFT_MID     %d\n"
    "#define LV_ALIGN_OUT_LEFT_BOTTOM  %d\n"
    "#define LV_ALIGN_OUT_RIGHT_TOP    %d\n"
    "#define LV_ALIGN_OUT_RIGHT_MID    %d\n"
    "#define LV_ALIGN_OUT_RIGHT_BOTTOM %d\n"

    "#define LV_COLOR_RED      0xFF0000\n"
    "#define LV_COLOR_GREEN    0x00FF00\n"
    "#define LV_COLOR_BLUE     0x0000FF\n"
    "#define LV_COLOR_WHITE    0xFFFFFF\n"
    "#define LV_COLOR_BLACK    0x000000\n"
    "#define LV_COLOR_YELLOW   0xFFFF00\n"
    "#define LV_COLOR_CYAN     0x00FFFF\n"
    "#define LV_COLOR_MAGENTA  0xFF00FF\n"
    "#define LV_COLOR_GRAY     0x808080\n"
    "#define LV_COLOR_ORANGE   0xFFA500\n"
    "#define LV_COLOR_PURPLE   0x800080\n"
    "#define LV_COLOR_PINK     0xFFC0CB\n"
    "#define LV_COLOR_BROWN    0xA52A2A\n"
    "#define LV_COLOR_DARK     0x1A1A1A\n"
    "#define LV_COLOR_LIGHT    0xE0E0E0\n"

    "#define LV_STATE_DEFAULT   %d\n"
    "#define LV_STATE_CHECKED   %d\n"
    "#define LV_STATE_FOCUSED   %d\n"
    "#define LV_STATE_FOCUS_KEY %d\n"
    "#define LV_STATE_EDITED    %d\n"
    "#define LV_STATE_HOVERED   %d\n"
    "#define LV_STATE_PRESSED   %d\n"
    "#define LV_STATE_SCROLLED  %d\n"
    "#define LV_STATE_DISABLED  %d\n"

    "#define LV_OBJ_FLAG_HIDDEN           %d\n"
    "#define LV_OBJ_FLAG_CLICKABLE        %d\n"
    "#define LV_OBJ_FLAG_CLICK_FOCUSABLE  %d\n"
    "#define LV_OBJ_FLAG_CHECKABLE        %d\n"
    "#define LV_OBJ_FLAG_SCROLLABLE       %d\n"
    "#define LV_OBJ_FLAG_SCROLL_ELASTIC   %d\n"
    "#define LV_OBJ_FLAG_SCROLL_MOMENTUM  %d\n"
    "#define LV_OBJ_FLAG_SCROLL_CHAIN     %d\n"
    "#define LV_OBJ_FLAG_SCROLL_ON_FOCUS  %d\n"
    "#define LV_OBJ_FLAG_OVERFLOW_VISIBLE %d\n"
    "#define LV_OBJ_FLAG_FLOATING         %d\n"
    "#define LV_OBJ_FLAG_EVENT_BUBBLE     %d\n"
    "#define LV_OBJ_FLAG_GESTURE_BUBBLE   %d\n"
    "#define LV_OBJ_FLAG_ADV_HITTEST      %d\n"
    "#define LV_OBJ_FLAG_IGNORE_LAYOUT    %d\n"

    "#define LV_PART_MAIN       %d\n"
    "#define LV_PART_SCROLLBAR  %d\n"
    "#define LV_PART_INDICATOR  %d\n"
    "#define LV_PART_KNOB       %d\n"
    "#define LV_PART_SELECTED   %d\n"
    "#define LV_PART_ITEMS      %d\n"
    "#define LV_PART_CURSOR     %d\n"

    "#define LV_OPA_TRANSP  0\n"
    "#define LV_OPA_0       0\n"
    "#define LV_OPA_10      25\n"
    "#define LV_OPA_20      51\n"
    "#define LV_OPA_30      76\n"
    "#define LV_OPA_40      102\n"
    "#define LV_OPA_50      127\n"
    "#define LV_OPA_60      153\n"
    "#define LV_OPA_70      178\n"
    "#define LV_OPA_80      204\n"
    "#define LV_OPA_90      229\n"
    "#define LV_OPA_100     255\n"
    "#define LV_OPA_COVER   255\n"

    "#define LV_LABEL_LONG_WRAP            %d\n"
    "#define LV_LABEL_LONG_DOT             %d\n"
    "#define LV_LABEL_LONG_SCROLL          %d\n"
    "#define LV_LABEL_LONG_SCROLL_CIRCULAR %d\n"
    "#define LV_LABEL_LONG_CLIP            %d\n"

    "#define LV_DIR_NONE    %d\n"
    "#define LV_DIR_LEFT    %d\n"
    "#define LV_DIR_RIGHT   %d\n"
    "#define LV_DIR_TOP     %d\n"
    "#define LV_DIR_BOTTOM  %d\n"
    "#define LV_DIR_HOR     %d\n"
    "#define LV_DIR_VER     %d\n"
    "#define LV_DIR_ALL     %d\n"

    "#define LV_FONT_8   8\n"
    "#define LV_FONT_10  10\n"
    "#define LV_FONT_12  12\n"
    "#define LV_FONT_14  14\n"
    "#define LV_FONT_16  16\n"
    "#define LV_FONT_18  18\n"
    "#define LV_FONT_20  20\n"
    "#define LV_FONT_22  22\n"
    "#define LV_FONT_24  24\n"
    "#define LV_FONT_26  26\n"
    "#define LV_FONT_28  28\n"
    "#define LV_FONT_30  30\n"
    "#define LV_FONT_32  32\n"
    "#define LV_FONT_36  36\n"
    "#define LV_FONT_40  40\n"
    "#define LV_FONT_48  48\n"

    "#define LV_SIZE_CONTENT  %d\n"
    "#define LV_PCT_FLAG      %d\n"

    "#define LV_HOR_RES  %d\n"
    "#define LV_VER_RES  %d\n"

    "#define LV_TEXT_ALIGN_LEFT   %d\n"
    "#define LV_TEXT_ALIGN_CENTER %d\n"
    "#define LV_TEXT_ALIGN_RIGHT  %d\n"
    "#define LV_TEXT_ALIGN_AUTO   %d\n"

    "#define LV_ANIM_OFF %d\n"
    "#define LV_ANIM_ON  %d\n"

    "#define LV_SCROLLBAR_MODE_OFF    %d\n"
    "#define LV_SCROLLBAR_MODE_ON     %d\n"
    "#define LV_SCROLLBAR_MODE_ACTIVE %d\n"
    "#define LV_SCROLLBAR_MODE_AUTO   %d\n"

    "#define LV_GRAD_DIR_NONE %d\n"
    "#define LV_GRAD_DIR_VER  %d\n"
    "#define LV_GRAD_DIR_HOR  %d\n"

    "#define LV_BLEND_MODE_NORMAL      %d\n"
    "#define LV_BLEND_MODE_ADDITIVE    %d\n"
    "#define LV_BLEND_MODE_SUBTRACTIVE %d\n"
    "#define LV_BLEND_MODE_MULTIPLY    %d\n"

    "#define LV_BORDER_SIDE_NONE     %d\n"
    "#define LV_BORDER_SIDE_BOTTOM   %d\n"
    "#define LV_BORDER_SIDE_TOP      %d\n"
    "#define LV_BORDER_SIDE_LEFT     %d\n"
    "#define LV_BORDER_SIDE_RIGHT    %d\n"
    "#define LV_BORDER_SIDE_FULL     %d\n"
    "#define LV_BORDER_SIDE_INTERNAL %d\n"

    "#define LV_LABEL_TEXT_SELECTION_OFF %d\n"
    "#define LV_LABEL_TEXT_SELECTION_ON  %d\n"

    "#define LV_LAYOUT_NONE %d\n"
    "#define LV_LAYOUT_FLEX %d\n"
    "#define LV_LAYOUT_GRID %d\n"

    "#define LV_FLEX_FLOW_ROW                %d\n"
    "#define LV_FLEX_FLOW_COLUMN             %d\n"
    "#define LV_FLEX_FLOW_ROW_WRAP           %d\n"
    "#define LV_FLEX_FLOW_ROW_REVERSE        %d\n"
    "#define LV_FLEX_FLOW_ROW_WRAP_REVERSE   %d\n"
    "#define LV_FLEX_FLOW_COLUMN_WRAP        %d\n"
    "#define LV_FLEX_FLOW_COLUMN_REVERSE     %d\n"

    "#define LV_FLEX_ALIGN_START        %d\n"
    "#define LV_FLEX_ALIGN_END          %d\n"
    "#define LV_FLEX_ALIGN_CENTER       %d\n"
    "#define LV_FLEX_ALIGN_SPACE_EVENLY %d\n"
    "#define LV_FLEX_ALIGN_SPACE_AROUND %d\n"
    "#define LV_FLEX_ALIGN_SPACE_BETWEEN %d\n"

    "#define LV_SCALE_NONE %d\n"
    
    "#define LV_CHART_TYPE_NONE     %d\n"
    "#define LV_CHART_TYPE_LINE     %d\n"
    "#define LV_CHART_TYPE_BAR      %d\n"
    "#define LV_CHART_TYPE_SCATTER  %d\n"
    "#define LV_CHART_AXIS_PRIMARY_Y %d\n"
    "#define LV_CHART_AXIS_SECONDARY_Y %d\n"

    "#define LV_EVENT_ALL            %d\n"
    "#define LV_EVENT_PRESSED        %d\n"
    "#define LV_EVENT_PRESSING       %d\n"
    "#define LV_EVENT_PRESS_LOST     %d\n"
    "#define LV_EVENT_SHORT_CLICKED  %d\n"
    "#define LV_EVENT_LONG_PRESSED   %d\n"
    "#define LV_EVENT_LONG_PRESSED_REPEAT %d\n"
    "#define LV_EVENT_CLICKED        %d\n"
    "#define LV_EVENT_RELEASED       %d\n"
    "#define LV_EVENT_VALUE_CHANGED  %d\n"

    , (int)LV_ALIGN_DEFAULT
    , (int)LV_ALIGN_TOP_LEFT
    , (int)LV_ALIGN_TOP_MID
    , (int)LV_ALIGN_TOP_RIGHT
    , (int)LV_ALIGN_BOTTOM_LEFT
    , (int)LV_ALIGN_BOTTOM_MID
    , (int)LV_ALIGN_BOTTOM_RIGHT
    , (int)LV_ALIGN_LEFT_MID
    , (int)LV_ALIGN_RIGHT_MID
    , (int)LV_ALIGN_CENTER
    , (int)LV_ALIGN_OUT_TOP_LEFT
    , (int)LV_ALIGN_OUT_TOP_MID
    , (int)LV_ALIGN_OUT_TOP_RIGHT
    , (int)LV_ALIGN_OUT_BOTTOM_LEFT
    , (int)LV_ALIGN_OUT_BOTTOM_MID
    , (int)LV_ALIGN_OUT_BOTTOM_RIGHT
    , (int)LV_ALIGN_OUT_LEFT_TOP
    , (int)LV_ALIGN_OUT_LEFT_MID
    , (int)LV_ALIGN_OUT_LEFT_BOTTOM
    , (int)LV_ALIGN_OUT_RIGHT_TOP
    , (int)LV_ALIGN_OUT_RIGHT_MID
    , (int)LV_ALIGN_OUT_RIGHT_BOTTOM

    , (int)LV_STATE_DEFAULT
    , (int)LV_STATE_CHECKED
    , (int)LV_STATE_FOCUSED
    , (int)LV_STATE_FOCUS_KEY
    , (int)LV_STATE_EDITED
    , (int)LV_STATE_HOVERED
    , (int)LV_STATE_PRESSED
    , (int)LV_STATE_SCROLLED
    , (int)LV_STATE_DISABLED

    , (int)LV_OBJ_FLAG_HIDDEN
    , (int)LV_OBJ_FLAG_CLICKABLE
    , (int)LV_OBJ_FLAG_CLICK_FOCUSABLE
    , (int)LV_OBJ_FLAG_CHECKABLE
    , (int)LV_OBJ_FLAG_SCROLLABLE
    , (int)LV_OBJ_FLAG_SCROLL_ELASTIC
    , (int)LV_OBJ_FLAG_SCROLL_MOMENTUM
    , (int)LV_OBJ_FLAG_SCROLL_CHAIN
    , (int)LV_OBJ_FLAG_SCROLL_ON_FOCUS
    , (int)LV_OBJ_FLAG_OVERFLOW_VISIBLE
    , (int)LV_OBJ_FLAG_FLOATING
    , (int)LV_OBJ_FLAG_EVENT_BUBBLE
    , (int)LV_OBJ_FLAG_GESTURE_BUBBLE
    , (int)LV_OBJ_FLAG_ADV_HITTEST
    , (int)LV_OBJ_FLAG_IGNORE_LAYOUT

    , (int)LV_PART_MAIN
    , (int)LV_PART_SCROLLBAR
    , (int)LV_PART_INDICATOR
    , (int)LV_PART_KNOB
    , (int)LV_PART_SELECTED
    , (int)LV_PART_ITEMS
    , (int)LV_PART_CURSOR

    , (int)LV_LABEL_LONG_WRAP
    , (int)LV_LABEL_LONG_DOT
    , (int)LV_LABEL_LONG_SCROLL
    , (int)LV_LABEL_LONG_SCROLL_CIRCULAR
    , (int)LV_LABEL_LONG_CLIP

    , (int)LV_DIR_NONE
    , (int)LV_DIR_LEFT
    , (int)LV_DIR_RIGHT
    , (int)LV_DIR_TOP
    , (int)LV_DIR_BOTTOM
    , (int)LV_DIR_HOR
    , (int)LV_DIR_VER
    , (int)LV_DIR_ALL

    , (int)LV_SIZE_CONTENT
    , (int)(LV_PCT(100) & 0xFFFF)

    , (int)LV_HOR_RES
    , (int)LV_VER_RES

    , (int)LV_TEXT_ALIGN_LEFT
    , (int)LV_TEXT_ALIGN_CENTER
    , (int)LV_TEXT_ALIGN_RIGHT
    , (int)LV_TEXT_ALIGN_AUTO

    , (int)LV_ANIM_OFF
    , (int)LV_ANIM_ON

    , (int)LV_SCROLLBAR_MODE_OFF
    , (int)LV_SCROLLBAR_MODE_ON
    , (int)LV_SCROLLBAR_MODE_ACTIVE
    , (int)LV_SCROLLBAR_MODE_AUTO

    , (int)LV_GRAD_DIR_NONE
    , (int)LV_GRAD_DIR_VER
    , (int)LV_GRAD_DIR_HOR

    , (int)LV_BLEND_MODE_NORMAL
    , (int)LV_BLEND_MODE_ADDITIVE
    , (int)LV_BLEND_MODE_SUBTRACTIVE
    , (int)LV_BLEND_MODE_MULTIPLY

    , (int)LV_BORDER_SIDE_NONE
    , (int)LV_BORDER_SIDE_BOTTOM
    , (int)LV_BORDER_SIDE_TOP
    , (int)LV_BORDER_SIDE_LEFT
    , (int)LV_BORDER_SIDE_RIGHT
    , (int)LV_BORDER_SIDE_FULL
    , (int)LV_BORDER_SIDE_INTERNAL

    , 0
    , 1

    , (int)LV_LAYOUT_NONE
    , (int)LV_LAYOUT_FLEX
    , (int)LV_LAYOUT_GRID

    , (int)LV_FLEX_FLOW_ROW
    , (int)LV_FLEX_FLOW_COLUMN
    , (int)LV_FLEX_FLOW_ROW_WRAP
    , (int)LV_FLEX_FLOW_ROW_REVERSE
    , (int)LV_FLEX_FLOW_ROW_WRAP_REVERSE
    , (int)LV_FLEX_FLOW_COLUMN_WRAP
    , (int)LV_FLEX_FLOW_COLUMN_REVERSE

    , (int)LV_FLEX_ALIGN_START
    , (int)LV_FLEX_ALIGN_END
    , (int)LV_FLEX_ALIGN_CENTER
    , (int)LV_FLEX_ALIGN_SPACE_EVENLY
    , (int)LV_FLEX_ALIGN_SPACE_AROUND
    , (int)LV_FLEX_ALIGN_SPACE_BETWEEN

    , (int)LV_SCALE_NONE

    , (int)LV_CHART_TYPE_NONE
    , (int)LV_CHART_TYPE_LINE
    , (int)LV_CHART_TYPE_BAR
    , (int)LV_CHART_TYPE_SCATTER
    , (int)LV_CHART_AXIS_PRIMARY_Y
    , (int)LV_CHART_AXIS_SECONDARY_Y

    , (int)LV_EVENT_ALL
    , (int)LV_EVENT_PRESSED
    , (int)LV_EVENT_PRESSING
    , (int)LV_EVENT_PRESS_LOST
    , (int)LV_EVENT_SHORT_CLICKED
    , (int)LV_EVENT_LONG_PRESSED
    , (int)LV_EVENT_LONG_PRESSED_REPEAT
    , (int)LV_EVENT_CLICKED
    , (int)LV_EVENT_RELEASED
    , (int)LV_EVENT_VALUE_CHANGED
  );

  if (n >= (int)sizeof(g_lv_inc_buf)) {
    printf("[LVGL] warning: lv_inc_buf truncated (%d >= %d)\n",
           n, (int)sizeof(g_lv_inc_buf));
  }
}

static char* lvgl_inc_resolver(const char* filename) {
  if (!filename) return NULL;

  if (strcmp(filename, "lvgl.h") == 0) {
    build_lv_inc_macros();
    size_t n = strlen(g_lv_inc_buf);
    char* buf = (char*)malloc(n + 1);
    if (!buf) return NULL;
    memcpy(buf, g_lv_inc_buf, n);
    buf[n] = '\0';
    return buf;
  }

  if (strcmp(filename, "lv_conf.h") == 0) {
    const char* stub = "// lv_conf.h stub\n";
    size_t n = strlen(stub);
    char* buf = (char*)malloc(n + 1);
    if (!buf) return NULL;
    memcpy(buf, stub, n);
    buf[n] = '\0';
    return buf;
  }

  return NULL;
}

static lv_obj_t* arg_obj(CDM_Value_t* argv, int argc, int idx) {
  if (idx >= argc) return NULL;
  CDM_Value_t v = CDM_Deref(argv[idx]);
  if (v.type == CDM_TYPE_PTR && v.v.p) return (lv_obj_t*)v.v.p;
  if (v.type == CDM_TYPE_INT && v.v.i == 0) return lv_screen_active();
  return NULL;
}

static int arg_int(CDM_Value_t* argv, int argc, int idx, int def) {
  if (idx >= argc) return def;
  CDM_Value_t v = CDM_Deref(argv[idx]);
  if (v.type == CDM_TYPE_INT)   return (int)v.v.i;
  if (v.type == CDM_TYPE_FLOAT) return (int)v.v.f;
  return def;
}

static const char* arg_str(CDM_Value_t* argv, int argc, int idx) {
  if (idx >= argc) return "";
  CDM_Value_t v = CDM_Deref(argv[idx]);
  if (v.type == CDM_TYPE_STRING && v.v.p) return (const char*)v.v.p;
  return "";
}

static CDM_Value_t ret_ptr(void* p) { return CDM_MakePtr(p, 0); }

static CDM_Value_t ret_int(int64_t x) {
  CDM_Value_t v;
  v.type = CDM_TYPE_INT;
  v.struct_id = -1;
  v.array_len = 0; v.array_dim = 0;
  v.array_sizes[0]=v.array_sizes[1]=v.array_sizes[2]=v.array_sizes[3]=0;
  v.array_elem_type = CDM_TYPE_INT;
  v.array_elem_struct_id = -1;
  v.owned = 0; v.from_new = 0;
  v.v.i = x;
  return v;
}

static CDM_Value_t ret_str(const char* s) {
  return CDM_MakeString(s ? s : "");
}

static lv_color_t hex_to_color(int hex) {
  return lv_color_make((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

static const lv_font_t* pick_font(int size) {
  if (size <= 0) return NULL;

#if defined(LV_FONT_MONTSERRAT_8) && LV_FONT_MONTSERRAT_8
  if (size <= 8) return &lv_font_montserrat_8;
#endif
#if defined(LV_FONT_MONTSERRAT_10) && LV_FONT_MONTSERRAT_10
  if (size <= 10) return &lv_font_montserrat_10;
#endif
#if defined(LV_FONT_MONTSERRAT_12) && LV_FONT_MONTSERRAT_12
  if (size <= 12) return &lv_font_montserrat_12;
#endif
#if defined(LV_FONT_MONTSERRAT_14) && LV_FONT_MONTSERRAT_14
  if (size <= 14) return &lv_font_montserrat_14;
#endif
#if defined(LV_FONT_MONTSERRAT_16) && LV_FONT_MONTSERRAT_16
  if (size <= 16) return &lv_font_montserrat_16;
#endif
#if defined(LV_FONT_MONTSERRAT_18) && LV_FONT_MONTSERRAT_18
  if (size <= 18) return &lv_font_montserrat_18;
#endif
#if defined(LV_FONT_MONTSERRAT_20) && LV_FONT_MONTSERRAT_20
  if (size <= 20) return &lv_font_montserrat_20;
#endif
#if defined(LV_FONT_MONTSERRAT_22) && LV_FONT_MONTSERRAT_22
  if (size <= 22) return &lv_font_montserrat_22;
#endif
#if defined(LV_FONT_MONTSERRAT_24) && LV_FONT_MONTSERRAT_24
  if (size <= 24) return &lv_font_montserrat_24;
#endif
#if defined(LV_FONT_MONTSERRAT_26) && LV_FONT_MONTSERRAT_26
  if (size <= 26) return &lv_font_montserrat_26;
#endif
#if defined(LV_FONT_MONTSERRAT_28) && LV_FONT_MONTSERRAT_28
  if (size <= 28) return &lv_font_montserrat_28;
#endif
#if defined(LV_FONT_MONTSERRAT_30) && LV_FONT_MONTSERRAT_30
  if (size <= 30) return &lv_font_montserrat_30;
#endif
#if defined(LV_FONT_MONTSERRAT_32) && LV_FONT_MONTSERRAT_32
  if (size <= 32) return &lv_font_montserrat_32;
#endif
#if defined(LV_FONT_MONTSERRAT_34) && LV_FONT_MONTSERRAT_34
  if (size <= 34) return &lv_font_montserrat_34;
#endif
#if defined(LV_FONT_MONTSERRAT_36) && LV_FONT_MONTSERRAT_36
  if (size <= 36) return &lv_font_montserrat_36;
#endif
#if defined(LV_FONT_MONTSERRAT_38) && LV_FONT_MONTSERRAT_38
  if (size <= 38) return &lv_font_montserrat_38;
#endif
#if defined(LV_FONT_MONTSERRAT_40) && LV_FONT_MONTSERRAT_40
  if (size <= 40) return &lv_font_montserrat_40;
#endif
#if defined(LV_FONT_MONTSERRAT_42) && LV_FONT_MONTSERRAT_42
  if (size <= 42) return &lv_font_montserrat_42;
#endif
#if defined(LV_FONT_MONTSERRAT_44) && LV_FONT_MONTSERRAT_44
  if (size <= 44) return &lv_font_montserrat_44;
#endif
#if defined(LV_FONT_MONTSERRAT_46) && LV_FONT_MONTSERRAT_46
  if (size <= 46) return &lv_font_montserrat_46;
#endif
#if defined(LV_FONT_MONTSERRAT_48) && LV_FONT_MONTSERRAT_48
  if (size <= 48) return &lv_font_montserrat_48;
#endif

  return NULL;
}

static CDM_Value_t LV_PCT_w(int argc, CDM_Value_t* argv) {
  int x = arg_int(argv, argc, 0, 100);
  if (x < 0) x = 0;
  if (x > 100) x = 100;
  lv_coord_t r = LV_PCT(x);
  return ret_int((int64_t)(uint16_t)r);
}

static CDM_Value_t LV_PCT_NEG_w(int argc, CDM_Value_t* argv) {
  int x = arg_int(argv, argc, 0, 100);
  if (x < 0) x = 0;
  if (x > 100) x = 100;
  lv_coord_t r = LV_PCT(-x);
  return ret_int((int64_t)(uint16_t)r);
}

static CDM_Value_t lv_color_hex_w(int argc, CDM_Value_t* argv) {
  int hex = arg_int(argv, argc, 0, 0);
  return ret_int((int64_t)hex);
}

static CDM_Value_t lv_screen_active_w(int argc, CDM_Value_t* argv) {
  (void)argc; (void)argv;
  return ret_ptr(lvgl_port_screen_get());
}

static CDM_Value_t lv_screen_get_w(int argc, CDM_Value_t* argv) {
  (void)argc; (void)argv;
  return ret_ptr(lvgl_port_screen_get());
}

static CDM_Value_t lv_screen_clear_w(int argc, CDM_Value_t* argv) {
  int bg = arg_int(argv, argc, 0, 0x1A1A1A);
  lvgl_port_screen_clear((uint32_t)bg);
  return ret_int(0);
}

static CDM_Value_t lv_refresh_now_w(int argc, CDM_Value_t* argv) {
  (void)argc; (void)argv;
  lvgl_port_refresh_now();
  return ret_int(0);
}

static CDM_Value_t lv_obj_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_obj_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_label_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_label_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_btn_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_button_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_bar_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_bar_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_slider_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_slider_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_switch_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_switch_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_arc_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_arc_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_line_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_line_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_checkbox_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_checkbox_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_dropdown_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_dropdown_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_roller_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_roller_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_textarea_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_textarea_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_table_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_table_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_led_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_led_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_spinner_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_spinner_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_image_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_image_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_tabview_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_tabview_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_tabview_add_tab_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_ptr(NULL);
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* tv = arg_obj(argv, argc, 0);
  const char* name = arg_str(argv, argc, 1);
  lv_obj_t* tab = NULL;
  if (tv) tab = lv_tabview_add_tab(tv, name);
  lvgl_port_unlock();
  return ret_ptr(tab);
}

static CDM_Value_t lv_chart_create_w(int argc, CDM_Value_t* argv) {
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* parent = arg_obj(argv, argc, 0);
  if (!parent) parent = lv_screen_active();
  lv_obj_t* obj = lv_chart_create(parent);
  lvgl_port_unlock();
  return ret_ptr(obj);
}

static CDM_Value_t lv_label_set_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* text = arg_str(argv, argc, 1);
  if (obj) lv_label_set_text(obj, text);
  lvgl_port_unlock();
  return ret_int(obj ? 0 : -1);
}

static CDM_Value_t lv_label_get_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_str("");
  if (!lvgl_port_lock(100)) return ret_str("");
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* t = obj ? lv_label_get_text(obj) : "";
  CDM_Value_t r = ret_str(t ? t : "");
  lvgl_port_unlock();
  return r;
}

static CDM_Value_t lv_label_set_long_mode_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int m = arg_int(argv, argc, 1, 0);
  if (obj) lv_label_set_long_mode(obj, (lv_label_long_mode_t)m);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_font_size_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int size = arg_int(argv, argc, 1, 14);
  const lv_font_t* f = pick_font(size);
  if (obj && f) {
    lv_obj_set_style_text_font(obj, f, LV_PART_MAIN);
  }
  lvgl_port_unlock();
  return ret_int(f ? 0 : -1);
}

static CDM_Value_t lv_obj_set_font_size_part_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int size = arg_int(argv, argc, 1, 14);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  const lv_font_t* f = pick_font(size);
  if (obj && f) {
    lv_obj_set_style_text_font(obj, f, (lv_style_selector_t)part);
  }
  lvgl_port_unlock();
  return ret_int(f ? 0 : -1);
}

static CDM_Value_t lv_obj_set_pos_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_set_pos(obj, arg_int(argv, argc, 1, 0), arg_int(argv, argc, 2, 0));
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_size_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_set_size(obj, arg_int(argv, argc, 1, 100), arg_int(argv, argc, 2, 50));
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_x_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_set_x(obj, arg_int(argv, argc, 1, 0));
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_y_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_set_y(obj, arg_int(argv, argc, 1, 0));
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_set_width(obj, arg_int(argv, argc, 1, 100));
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_height_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_set_height(obj, arg_int(argv, argc, 1, 50));
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_center_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_center(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_align_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) {
    int align = arg_int(argv, argc, 1, 0);
    int x_ofs = arg_int(argv, argc, 2, 0);
    int y_ofs = arg_int(argv, argc, 3, 0);
    lv_obj_align(obj, (lv_align_t)align, x_ofs, y_ofs);
  }
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_align_to_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj  = arg_obj(argv, argc, 0);
  lv_obj_t* base = arg_obj(argv, argc, 1);
  int align      = arg_int(argv, argc, 2, 0);
  int x_ofs      = arg_int(argv, argc, 3, 0);
  int y_ofs      = arg_int(argv, argc, 4, 0);
  if (obj && base) lv_obj_align_to(obj, base, (lv_align_t)align, x_ofs, y_ofs);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_get_x_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_obj_get_x(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_obj_get_y_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_obj_get_y(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_obj_get_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_obj_get_width(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_obj_get_height_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_obj_get_height(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_obj_get_child_count_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_obj_get_child_count(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_obj_get_child_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_ptr(NULL);
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int idx = arg_int(argv, argc, 1, 0);
  lv_obj_t* child = obj ? lv_obj_get_child(obj, idx) : NULL;
  lvgl_port_unlock();
  return ret_ptr(child);
}

static CDM_Value_t lv_obj_get_parent_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_ptr(NULL);
  if (!lvgl_port_lock(100)) return ret_ptr(NULL);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  lv_obj_t* p = obj ? lv_obj_get_parent(obj) : NULL;
  lvgl_port_unlock();
  return ret_ptr(p);
}

static CDM_Value_t lv_obj_move_foreground_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_move_foreground(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_move_background_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_move_background(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_scroll_to_view_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int anim = arg_int(argv, argc, 1, LV_ANIM_ON);
  if (obj) lv_obj_scroll_to_view(obj, (lv_anim_enable_t)anim);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_scroll_to_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int x = arg_int(argv, argc, 1, 0);
  int y = arg_int(argv, argc, 2, 0);
  int anim = arg_int(argv, argc, 3, LV_ANIM_OFF);
  if (obj) lv_obj_scroll_to(obj, x, y, (lv_anim_enable_t)anim);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_layout_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int layout = arg_int(argv, argc, 1, 0);
  if (obj) lv_obj_set_layout(obj, (uint32_t)layout);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_flex_flow_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int flow = arg_int(argv, argc, 1, 0);
  if (obj) lv_obj_set_flex_flow(obj, (lv_flex_flow_t)flow);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_flex_align_w(int argc, CDM_Value_t* argv) {
  if (argc < 4) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int main_align = arg_int(argv, argc, 1, 0);
  int cross_align = arg_int(argv, argc, 2, 0);
  int track_align = arg_int(argv, argc, 3, 0);
  if (obj) lv_obj_set_flex_align(obj,
                                 (lv_flex_align_t)main_align,
                                 (lv_flex_align_t)cross_align,
                                 (lv_flex_align_t)track_align);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_bg_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_bg_color(obj, hex_to_color(hex), (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_bg_grad_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_bg_grad_color(obj, hex_to_color(hex), (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_bg_grad_dir_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int dir = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_bg_grad_dir(obj, (lv_grad_dir_t)dir, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_bg_opa_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int o = arg_int(argv, argc, 1, 255);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_bg_opa(obj, (lv_opa_t)o, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_text_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_text_color(obj, hex_to_color(hex), (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_radius_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int r = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_radius(obj, r, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_border_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int w = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_border_width(obj, w, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_border_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_border_color(obj, hex_to_color(hex), (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_border_side_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int side = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_border_side(obj, (lv_border_side_t)side, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_pad_all_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int p = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_pad_all(obj, p, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_pad_top_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int p = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_pad_top(obj, p, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_pad_bottom_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int p = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_pad_bottom(obj, p, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_pad_left_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int p = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_pad_left(obj, p, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_pad_right_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int p = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_pad_right(obj, p, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_margin_all_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int m = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_margin_all(obj, m, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_margin_top_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int m = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_margin_top(obj, m, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_margin_bottom_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int m = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_margin_bottom(obj, m, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_margin_left_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int m = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_margin_left(obj, m, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_margin_right_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int m = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_margin_right(obj, m, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_opa_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int o = arg_int(argv, argc, 1, 255);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_opa(obj, (lv_opa_t)o, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_text_align_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int a = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_text_align(obj, (lv_text_align_t)a, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_text_letter_space_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int s = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_text_letter_space(obj, s, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_text_line_space_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int s = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_text_line_space(obj, s, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_outline_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int w = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_outline_width(obj, w, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_outline_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_outline_color(obj, hex_to_color(hex), (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_outline_opa_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int o = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_outline_opa(obj, (lv_opa_t)o, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_outline_pad_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int p = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_outline_pad(obj, p, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_shadow_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int w = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_shadow_width(obj, w, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_shadow_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_shadow_color(obj, hex_to_color(hex), (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_shadow_opa_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int o = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_shadow_opa(obj, (lv_opa_t)o, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_shadow_ofs_x_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int x = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_shadow_offset_x(obj, x, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_shadow_ofs_y_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int y = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_shadow_offset_y(obj, y, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_transform_scale_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int scale = arg_int(argv, argc, 1, 256);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_transform_scale(obj, scale, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_transform_angle_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int angle = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_transform_rotation(obj, angle, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_line_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int w = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_line_width(obj, w, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_line_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_line_color(obj, hex_to_color(hex), (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_line_rounded_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int r = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_line_rounded(obj, r != 0, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_min_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int w = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_min_width(obj, w, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_min_height_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int h = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_min_height(obj, h, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_max_width_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int w = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_max_width(obj, w, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_max_height_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int h = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_max_height(obj, h, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_style_clip_corner_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int c = arg_int(argv, argc, 1, 0);
  int part = arg_int(argv, argc, 2, LV_PART_MAIN);
  if (obj) lv_obj_set_style_clip_corner(obj, c != 0, (lv_style_selector_t)part);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_bar_set_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = arg_int(argv, argc, 1, 0);
  if (obj) lv_bar_set_value(obj, v, LV_ANIM_OFF);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_bar_get_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_bar_get_value(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_bar_set_range_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int mn = arg_int(argv, argc, 1, 0);
  int mx = arg_int(argv, argc, 2, 100);
  if (obj) lv_bar_set_range(obj, mn, mx);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_slider_set_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = arg_int(argv, argc, 1, 0);
  if (obj) lv_slider_set_value(obj, v, LV_ANIM_OFF);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_slider_get_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_slider_get_value(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_slider_set_range_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int mn = arg_int(argv, argc, 1, 0);
  int mx = arg_int(argv, argc, 2, 100);
  if (obj) lv_slider_set_range(obj, mn, mx);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_arc_set_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = arg_int(argv, argc, 1, 0);
  if (obj) lv_arc_set_value(obj, v);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_arc_get_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_arc_get_value(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_arc_set_range_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int mn = arg_int(argv, argc, 1, 0);
  int mx = arg_int(argv, argc, 2, 100);
  if (obj) lv_arc_set_range(obj, mn, mx);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_arc_set_angles_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int a1 = arg_int(argv, argc, 1, 0);
  int a2 = arg_int(argv, argc, 2, 360);
  if (obj) lv_arc_set_angles(obj, a1, a2);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_add_state_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int s = arg_int(argv, argc, 1, 0);
  if (obj) lv_obj_add_state(obj, (lv_state_t)s);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_clear_state_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int s = arg_int(argv, argc, 1, 0);
  if (obj) lv_obj_clear_state(obj, (lv_state_t)s);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_has_state_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int s = arg_int(argv, argc, 1, 0);
  int r = obj ? (lv_obj_has_state(obj, (lv_state_t)s) ? 1 : 0) : 0;
  lvgl_port_unlock();
  return ret_int(r);
}

static CDM_Value_t lv_obj_add_flag_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int f = arg_int(argv, argc, 1, 0);
  if (obj) lv_obj_add_flag(obj, (lv_obj_flag_t)f);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_clear_flag_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int f = arg_int(argv, argc, 1, 0);
  if (obj) lv_obj_clear_flag(obj, (lv_obj_flag_t)f);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_delete_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_obj_delete(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_clean_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (!obj) obj = lv_screen_active();
  lv_obj_clean(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_checkbox_set_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* t = arg_str(argv, argc, 1);
  if (obj) lv_checkbox_set_text(obj, t);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_checkbox_get_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_str("");
  if (!lvgl_port_lock(100)) return ret_str("");
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* t = obj ? lv_checkbox_get_text(obj) : "";
  CDM_Value_t r = ret_str(t ? t : "");
  lvgl_port_unlock();
  return r;
}

static CDM_Value_t lv_dropdown_set_options_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* opts = arg_str(argv, argc, 1);
  if (obj) lv_dropdown_set_options(obj, opts);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_dropdown_set_selected_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int sel = arg_int(argv, argc, 1, 0);
  if (obj) lv_dropdown_set_selected(obj, sel);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_dropdown_get_selected_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_dropdown_get_selected(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_roller_set_options_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* opts = arg_str(argv, argc, 1);
  int mode = arg_int(argv, argc, 2, 0);
  if (obj) lv_roller_set_options(obj, opts, (lv_roller_mode_t)mode);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_roller_set_selected_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int sel = arg_int(argv, argc, 1, 0);
  if (obj) lv_roller_set_selected(obj, sel, LV_ANIM_OFF);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_roller_get_selected_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(0);
  if (!lvgl_port_lock(100)) return ret_int(0);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = obj ? (int)lv_roller_get_selected(obj) : 0;
  lvgl_port_unlock();
  return ret_int(v);
}

static CDM_Value_t lv_textarea_set_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* t = arg_str(argv, argc, 1);
  if (obj) lv_textarea_set_text(obj, t);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_textarea_get_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_str("");
  if (!lvgl_port_lock(100)) return ret_str("");
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* t = obj ? lv_textarea_get_text(obj) : "";
  CDM_Value_t r = ret_str(t ? t : "");
  lvgl_port_unlock();
  return r;
}

static CDM_Value_t lv_textarea_set_placeholder_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* t = arg_str(argv, argc, 1);
  if (obj) lv_textarea_set_placeholder_text(obj, t);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_textarea_set_one_line_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int v = arg_int(argv, argc, 1, 0);
  if (obj) lv_textarea_set_one_line(obj, v != 0);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_textarea_set_max_length_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int n = arg_int(argv, argc, 1, 128);
  if (obj) lv_textarea_set_max_length(obj, (uint32_t)n);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_textarea_add_text_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  const char* t = arg_str(argv, argc, 1);
  if (obj) lv_textarea_add_text(obj, t);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_table_set_cell_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 4) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int row = arg_int(argv, argc, 1, 0);
  int col = arg_int(argv, argc, 2, 0);
  const char* t = arg_str(argv, argc, 3);
  if (obj) lv_table_set_cell_value(obj, row, col, t);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_table_get_cell_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 3) return ret_str("");
  if (!lvgl_port_lock(100)) return ret_str("");
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int row = arg_int(argv, argc, 1, 0);
  int col = arg_int(argv, argc, 2, 0);
  const char* t = obj ? lv_table_get_cell_value(obj, row, col) : "";
  CDM_Value_t r = ret_str(t ? t : "");
  lvgl_port_unlock();
  return r;
}

static CDM_Value_t lv_table_set_row_count_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int n = arg_int(argv, argc, 1, 1);
  if (obj) lv_table_set_row_count(obj, (uint32_t)n);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_table_set_column_count_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int n = arg_int(argv, argc, 1, 1);
  if (obj) lv_table_set_column_count(obj, (uint32_t)n);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_led_set_color_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int hex = arg_int(argv, argc, 1, 0);
  if (obj) lv_led_set_color(obj, hex_to_color(hex));
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_led_set_brightness_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int b = arg_int(argv, argc, 1, 255);
  if (obj) lv_led_set_brightness(obj, (uint8_t)b);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_led_on_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_led_on(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_led_off_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_led_off(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_led_toggle_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) lv_led_toggle(obj);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_image_set_src_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  CDM_Value_t src = CDM_Deref(argv[1]);
  const void* s = NULL;
  if (src.type == CDM_TYPE_PTR) s = src.v.p;
  else if (src.type == CDM_TYPE_STRING) s = src.v.p;
  if (obj && s) lv_image_set_src(obj, s);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_image_set_scale_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int s = arg_int(argv, argc, 1, 256);
  if (obj) lv_image_set_scale(obj, (uint32_t)s);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_image_set_rotation_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int a = arg_int(argv, argc, 1, 0);
  if (obj) lv_image_set_rotation(obj, a);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_obj_set_ext_click_area_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int area = arg_int(argv, argc, 1, 0);
  if (obj) lv_obj_set_ext_click_area(obj, area);
  lvgl_port_unlock();
  return ret_int(0);
}

static lv_obj_t* g_last_event_obj = NULL;
static int g_last_event_code = 0;
static volatile bool g_has_event = false;

static void global_event_cb(lv_event_t * e) {
    // ★ 修复1：LVGL v9 的 lv_event_get_target 返回 void*，需要显式转 lv_obj_t*
    g_last_event_obj = (lv_obj_t*)lv_event_get_target(e);
    g_last_event_code = (int)lv_event_get_code(e);
    g_has_event = true;
}

static CDM_Value_t lv_obj_add_event_cb_w(int argc, CDM_Value_t* argv) {
  if (argc < 1) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  if (obj) {
    lv_obj_add_event_cb(obj, global_event_cb, LV_EVENT_ALL, NULL);
  }
  lvgl_port_unlock();
  return ret_int(obj ? 0 : -1);
}

static CDM_Value_t lv_event_pending_w(int argc, CDM_Value_t* argv) {
  (void)argc; (void)argv;
  return ret_int(g_has_event ? 1 : 0);
}

static CDM_Value_t lv_event_get_obj_w(int argc, CDM_Value_t* argv) {
  (void)argc; (void)argv;
  return ret_ptr(g_last_event_obj);
}

static CDM_Value_t lv_event_get_code_w(int argc, CDM_Value_t* argv) {
  (void)argc; (void)argv;
  return ret_int(g_last_event_code);
}

static CDM_Value_t lv_event_clear_w(int argc, CDM_Value_t* argv) {
  (void)argc; (void)argv;
  g_has_event = false;
  g_last_event_obj = NULL;
  g_last_event_code = 0;
  return ret_int(0);
}

static CDM_Value_t lv_chart_set_type_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int type = arg_int(argv, argc, 1, 0);
  if (obj) lv_chart_set_type(obj, (lv_chart_type_t)type);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_chart_set_point_count_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int cnt = arg_int(argv, argc, 1, 10);
  if (obj) lv_chart_set_point_count(obj, cnt);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_chart_set_range_w(int argc, CDM_Value_t* argv) {
  if (argc < 4) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int axis = arg_int(argv, argc, 1, 0);
  int mn = arg_int(argv, argc, 2, 0);
  int mx = arg_int(argv, argc, 3, 100);
  if (obj) lv_chart_set_range(obj, (lv_chart_axis_t)axis, mn, mx);
  lvgl_port_unlock();
  return ret_int(0);
}

static CDM_Value_t lv_chart_set_next_value_w(int argc, CDM_Value_t* argv) {
  if (argc < 2) return ret_int(-1);
  if (!lvgl_port_lock(100)) return ret_int(-1);
  lv_obj_t* obj = arg_obj(argv, argc, 0);
  int val = arg_int(argv, argc, 1, 0);
  if (obj) {
    lv_chart_series_t* ser = lv_chart_get_series_next(obj, NULL);
    if (!ser) {
      ser = lv_chart_add_series(obj, lv_color_hex(0x00FFFF), LV_CHART_AXIS_PRIMARY_Y);
    }
    lv_chart_set_next_value(obj, ser, val);
  }
  lvgl_port_unlock();
  return ret_int(0);
}

#define MAX_STYLES 16

typedef struct {
    lv_style_t* style;
    int         in_use;
    char        name[32];
} StyleSlot_t;

static StyleSlot_t g_styles[MAX_STYLES];

static lv_style_t* arg_style(CDM_Value_t* argv, int argc, int idx) {
    if (idx >= argc) return NULL;
    CDM_Value_t v = CDM_Deref(argv[idx]);
    if (v.type != CDM_TYPE_PTR || !v.v.p) return NULL;
    return (lv_style_t*)v.v.p;
}

static CDM_Value_t lv_style_create_w(int argc, CDM_Value_t* argv) {
    (void)argc; (void)argv;
    if (!lvgl_port_lock(100)) return ret_ptr(NULL);

    int idx = -1;
    for (int i = 0; i < MAX_STYLES; i++) {
        if (!g_styles[i].in_use) { idx = i; break; }
    }
    if (idx < 0) {
        printf("[LVGL style] table full (%d)\n", MAX_STYLES);
        lvgl_port_unlock();
        return ret_ptr(NULL);
    }

    lv_style_t* st = (lv_style_t*)calloc(1, sizeof(lv_style_t));
    if (!st) { lvgl_port_unlock(); return ret_ptr(NULL); }
    lv_style_init(st);

    g_styles[idx].style = st;
    g_styles[idx].in_use = 1;
    g_styles[idx].name[0] = '\0';

    lvgl_port_unlock();
    return ret_ptr(st);
}

static CDM_Value_t lv_style_delete_w(int argc, CDM_Value_t* argv) {
    if (argc < 1) return ret_int(-1);
    if (!lvgl_port_lock(100)) return ret_int(-1);
    lv_style_t* st = arg_style(argv, argc, 0);
    if (!st) { lvgl_port_unlock(); return ret_int(-1); }

    int idx = -1;
    for (int i = 0; i < MAX_STYLES; i++) {
        if (g_styles[i].in_use && g_styles[i].style == st) { idx = i; break; }
    }
    if (idx < 0) { lvgl_port_unlock(); return ret_int(-1); }

    lv_style_reset(st);
    free(st);
    g_styles[idx].in_use = 0;
    g_styles[idx].style = NULL;
    g_styles[idx].name[0] = '\0';

    lvgl_port_unlock();
    return ret_int(0);
}

static CDM_Value_t lv_style_reset_w(int argc, CDM_Value_t* argv) {
    if (argc < 1) return ret_int(-1);
    if (!lvgl_port_lock(100)) return ret_int(-1);
    lv_style_t* st = arg_style(argv, argc, 0);
    if (st) lv_style_reset(st);
    lvgl_port_unlock();
    return ret_int(st ? 0 : -1);
}

static CDM_Value_t lv_styles_clear_w(int argc, CDM_Value_t* argv) {
    (void)argc; (void)argv;
    if (!lvgl_port_lock(100)) return ret_int(-1);
    int freed = 0;
    for (int i = 0; i < MAX_STYLES; i++) {
        if (g_styles[i].in_use && g_styles[i].style) {
            lv_style_reset(g_styles[i].style);
            free(g_styles[i].style);
            g_styles[i].style = NULL;
            g_styles[i].in_use = 0;
            g_styles[i].name[0] = '\0';
            freed++;
        }
    }
    lvgl_port_unlock();
    return ret_int(freed);
}

static CDM_Value_t lv_style_set_name_w(int argc, CDM_Value_t* argv) {
    if (argc < 2) return ret_int(-1);
    if (!lvgl_port_lock(100)) return ret_int(-1);
    lv_style_t* st = arg_style(argv, argc, 0);
    const char* name = arg_str(argv, argc, 1);
    int ok = 0;
    if (st) {
        for (int i = 0; i < MAX_STYLES; i++) {
            if (g_styles[i].in_use && g_styles[i].style == st) {
                strncpy(g_styles[i].name, name, 31);
                g_styles[i].name[31] = '\0';
                ok = 1;
                break;
            }
        }
    }
    lvgl_port_unlock();
    return ret_int(ok ? 0 : -1);
}

static CDM_Value_t lv_style_find_w(int argc, CDM_Value_t* argv) {
    if (argc < 1) return ret_ptr(NULL);
    if (!lvgl_port_lock(100)) return ret_ptr(NULL);
    const char* name = arg_str(argv, argc, 0);
    lv_style_t* found = NULL;
    for (int i = 0; i < MAX_STYLES; i++) {
        if (g_styles[i].in_use && strcmp(g_styles[i].name, name) == 0) {
            found = g_styles[i].style;
            break;
        }
    }
    lvgl_port_unlock();
    return ret_ptr(found);
}

#define STYLE_SET_INT(fname, lvfn) \
static CDM_Value_t style_set_##fname##_w(int argc, CDM_Value_t* argv) { \
    if (argc < 2) return ret_int(-1); \
    if (!lvgl_port_lock(100)) return ret_int(-1); \
    lv_style_t* st = arg_style(argv, argc, 0); \
    int v = arg_int(argv, argc, 1, 0); \
    if (st) lvfn(st, v); \
    lvgl_port_unlock(); \
    return ret_int(st ? 0 : -1); \
}

// ★ 修复2：为需要显式转换枚举类型的函数增加专属宏
#define STYLE_SET_INT_CAST(fname, lvfn, type) \
static CDM_Value_t style_set_##fname##_w(int argc, CDM_Value_t* argv) { \
    if (argc < 2) return ret_int(-1); \
    if (!lvgl_port_lock(100)) return ret_int(-1); \
    lv_style_t* st = arg_style(argv, argc, 0); \
    int v = arg_int(argv, argc, 1, 0); \
    if (st) lvfn(st, (type)v); \
    lvgl_port_unlock(); \
    return ret_int(st ? 0 : -1); \
}

#define STYLE_SET_COLOR(fname, lvfn) \
static CDM_Value_t style_set_##fname##_w(int argc, CDM_Value_t* argv) { \
    if (argc < 2) return ret_int(-1); \
    if (!lvgl_port_lock(100)) return ret_int(-1); \
    lv_style_t* st = arg_style(argv, argc, 0); \
    lv_color_t c = hex_to_color(arg_int(argv, argc, 1, 0)); \
    if (st) lvfn(st, c); \
    lvgl_port_unlock(); \
    return ret_int(st ? 0 : -1); \
}

STYLE_SET_COLOR(bg_color,           lv_style_set_bg_color)
STYLE_SET_INT  (bg_opa,             lv_style_set_bg_opa)
STYLE_SET_COLOR(bg_grad_color,      lv_style_set_bg_grad_color)
STYLE_SET_INT_CAST(bg_grad_dir,     lv_style_set_bg_grad_dir, lv_grad_dir_t)
STYLE_SET_COLOR(text_color,         lv_style_set_text_color)
STYLE_SET_INT_CAST(text_align,      lv_style_set_text_align, lv_text_align_t)
STYLE_SET_INT  (text_letter_space,  lv_style_set_text_letter_space)
STYLE_SET_INT  (text_line_space,    lv_style_set_text_line_space)
STYLE_SET_INT  (radius,             lv_style_set_radius)
STYLE_SET_INT  (border_width,       lv_style_set_border_width)
STYLE_SET_COLOR(border_color,       lv_style_set_border_color)
STYLE_SET_INT_CAST(border_side,     lv_style_set_border_side, lv_border_side_t)
STYLE_SET_INT  (pad_all,            lv_style_set_pad_all)
STYLE_SET_INT  (pad_top,            lv_style_set_pad_top)
STYLE_SET_INT  (pad_bottom,         lv_style_set_pad_bottom)
STYLE_SET_INT  (pad_left,           lv_style_set_pad_left)
STYLE_SET_INT  (pad_right,          lv_style_set_pad_right)
STYLE_SET_INT  (margin_all,         lv_style_set_margin_all)
STYLE_SET_INT  (margin_top,         lv_style_set_margin_top)
STYLE_SET_INT  (margin_bottom,      lv_style_set_margin_bottom)
STYLE_SET_INT  (margin_left,        lv_style_set_margin_left)
STYLE_SET_INT  (margin_right,       lv_style_set_margin_right)
STYLE_SET_INT  (opa,                lv_style_set_opa)
STYLE_SET_INT  (shadow_width,       lv_style_set_shadow_width)
STYLE_SET_COLOR(shadow_color,       lv_style_set_shadow_color)
STYLE_SET_INT  (shadow_opa,         lv_style_set_shadow_opa)
STYLE_SET_INT  (shadow_ofs_x,       lv_style_set_shadow_offset_x)
STYLE_SET_INT  (shadow_ofs_y,       lv_style_set_shadow_offset_y)
STYLE_SET_INT  (outline_width,      lv_style_set_outline_width)
STYLE_SET_COLOR(outline_color,      lv_style_set_outline_color)
STYLE_SET_INT  (outline_opa,        lv_style_set_outline_opa)
STYLE_SET_INT  (outline_pad,        lv_style_set_outline_pad)
STYLE_SET_INT  (line_width,         lv_style_set_line_width)
STYLE_SET_COLOR(line_color,         lv_style_set_line_color)
STYLE_SET_INT  (min_width,          lv_style_set_min_width)
STYLE_SET_INT  (min_height,         lv_style_set_min_height)
STYLE_SET_INT  (max_width,          lv_style_set_max_width)
STYLE_SET_INT  (max_height,         lv_style_set_max_height)
STYLE_SET_INT  (clip_corner,        lv_style_set_clip_corner)
STYLE_SET_INT  (transform_scale,    lv_style_set_transform_scale)

static CDM_Value_t style_set_text_font_w(int argc, CDM_Value_t* argv) {
    if (argc < 2) return ret_int(-1);
    if (!lvgl_port_lock(100)) return ret_int(-1);
    lv_style_t* st = arg_style(argv, argc, 0);
    int size = arg_int(argv, argc, 1, 14);
    const lv_font_t* f = pick_font(size);
    if (st && f) lv_style_set_text_font(st, f);
    lvgl_port_unlock();
    return ret_int((st && f) ? 0 : -1);
}

static CDM_Value_t lv_obj_add_style_w(int argc, CDM_Value_t* argv) {
    if (argc < 2) return ret_int(-1);
    if (!lvgl_port_lock(100)) return ret_int(-1);
    lv_obj_t* obj = arg_obj(argv, argc, 0);
    lv_style_t* st = arg_style(argv, argc, 1);
    int part = arg_int(argv, argc, 2, LV_PART_MAIN);
    if (obj && st) lv_obj_add_style(obj, st, (lv_style_selector_t)part);
    lvgl_port_unlock();
    return ret_int((obj && st) ? 0 : -1);
}

static CDM_Value_t lv_obj_remove_style_w(int argc, CDM_Value_t* argv) {
    if (argc < 2) return ret_int(-1);
    if (!lvgl_port_lock(100)) return ret_int(-1);
    lv_obj_t* obj = arg_obj(argv, argc, 0);
    lv_style_t* st = arg_style(argv, argc, 1);
    int part = arg_int(argv, argc, 2, LV_PART_MAIN);
    if (obj && st) lv_obj_remove_style(obj, st, (lv_style_selector_t)part);
    lvgl_port_unlock();
    return ret_int((obj && st) ? 0 : -1);
}

static CDM_Value_t lv_obj_remove_style_all_w(int argc, CDM_Value_t* argv) {
    if (argc < 1) return ret_int(-1);
    if (!lvgl_port_lock(100)) return ret_int(-1);
    lv_obj_t* obj = arg_obj(argv, argc, 0);
    if (obj) lv_obj_remove_style_all(obj);
    lvgl_port_unlock();
    return ret_int(obj ? 0 : -1);
}

void CDM_LVGLBridge_Init(void) {
  build_lv_inc_macros();
  CDM_SetIncludeResolver(lvgl_inc_resolver);

  CDM_RegisterHostFuncEx("lv_screen_active", lv_screen_active_w);
  CDM_RegisterHostFuncEx("lv_screen_get",    lv_screen_get_w);
  CDM_RegisterHostFuncEx("lv_screen_clear",  lv_screen_clear_w);
  CDM_RegisterHostFuncEx("lv_refresh_now",   lv_refresh_now_w);

  CDM_RegisterHostFuncEx("LV_PCT",     LV_PCT_w);
  CDM_RegisterHostFuncEx("LV_PCT_NEG", LV_PCT_NEG_w);
  CDM_RegisterHostFuncEx("lv_color_hex", lv_color_hex_w);

  CDM_RegisterHostFuncEx("lv_obj_create",       lv_obj_create_w);
  CDM_RegisterHostFuncEx("lv_label_create",     lv_label_create_w);
  CDM_RegisterHostFuncEx("lv_btn_create",       lv_btn_create_w);
  CDM_RegisterHostFuncEx("lv_bar_create",       lv_bar_create_w);
  CDM_RegisterHostFuncEx("lv_slider_create",    lv_slider_create_w);
  CDM_RegisterHostFuncEx("lv_switch_create",    lv_switch_create_w);
  CDM_RegisterHostFuncEx("lv_arc_create",       lv_arc_create_w);
  CDM_RegisterHostFuncEx("lv_chart_create",     lv_chart_create_w);
  CDM_RegisterHostFuncEx("lv_line_create",      lv_line_create_w);
  CDM_RegisterHostFuncEx("lv_checkbox_create",  lv_checkbox_create_w);
  CDM_RegisterHostFuncEx("lv_dropdown_create",  lv_dropdown_create_w);
  CDM_RegisterHostFuncEx("lv_roller_create",    lv_roller_create_w);
  CDM_RegisterHostFuncEx("lv_textarea_create",  lv_textarea_create_w);
  CDM_RegisterHostFuncEx("lv_table_create",     lv_table_create_w);
  CDM_RegisterHostFuncEx("lv_led_create",       lv_led_create_w);
  CDM_RegisterHostFuncEx("lv_spinner_create",   lv_spinner_create_w);
  CDM_RegisterHostFuncEx("lv_image_create",     lv_image_create_w);
  CDM_RegisterHostFuncEx("lv_tabview_create",   lv_tabview_create_w);
  CDM_RegisterHostFuncEx("lv_tabview_add_tab",  lv_tabview_add_tab_w);

  CDM_RegisterHostFuncEx("lv_label_set_text",       lv_label_set_text_w);
  CDM_RegisterHostFuncEx("lv_label_get_text",       lv_label_get_text_w);
  CDM_RegisterHostFuncEx("lv_label_set_long_mode",  lv_label_set_long_mode_w);

  CDM_RegisterHostFuncEx("lv_obj_set_font_size",       lv_obj_set_font_size_w);
  CDM_RegisterHostFuncEx("lv_obj_set_font_size_part",  lv_obj_set_font_size_part_w);

  CDM_RegisterHostFuncEx("lv_obj_set_pos",    lv_obj_set_pos_w);
  CDM_RegisterHostFuncEx("lv_obj_set_size",   lv_obj_set_size_w);
  CDM_RegisterHostFuncEx("lv_obj_set_x",      lv_obj_set_x_w);
  CDM_RegisterHostFuncEx("lv_obj_set_y",      lv_obj_set_y_w);
  CDM_RegisterHostFuncEx("lv_obj_set_width",  lv_obj_set_width_w);
  CDM_RegisterHostFuncEx("lv_obj_set_height", lv_obj_set_height_w);
  CDM_RegisterHostFuncEx("lv_obj_center",     lv_obj_center_w);
  CDM_RegisterHostFuncEx("lv_obj_align",      lv_obj_align_w);
  CDM_RegisterHostFuncEx("lv_obj_align_to",   lv_obj_align_to_w);

  CDM_RegisterHostFuncEx("lv_obj_get_x",           lv_obj_get_x_w);
  CDM_RegisterHostFuncEx("lv_obj_get_y",           lv_obj_get_y_w);
  CDM_RegisterHostFuncEx("lv_obj_get_width",       lv_obj_get_width_w);
  CDM_RegisterHostFuncEx("lv_obj_get_height",      lv_obj_get_height_w);
  CDM_RegisterHostFuncEx("lv_obj_get_child_count", lv_obj_get_child_count_w);
  CDM_RegisterHostFuncEx("lv_obj_get_child",       lv_obj_get_child_w);
  CDM_RegisterHostFuncEx("lv_obj_get_parent",      lv_obj_get_parent_w);

  CDM_RegisterHostFuncEx("lv_obj_move_foreground", lv_obj_move_foreground_w);
  CDM_RegisterHostFuncEx("lv_obj_move_background", lv_obj_move_background_w);
  CDM_RegisterHostFuncEx("lv_obj_scroll_to_view",  lv_obj_scroll_to_view_w);
  CDM_RegisterHostFuncEx("lv_obj_scroll_to",       lv_obj_scroll_to_w);

  CDM_RegisterHostFuncEx("lv_obj_set_layout",     lv_obj_set_layout_w);
  CDM_RegisterHostFuncEx("lv_obj_set_flex_flow",  lv_obj_set_flex_flow_w);
  CDM_RegisterHostFuncEx("lv_obj_set_flex_align", lv_obj_set_flex_align_w);

  CDM_RegisterHostFuncEx("lv_obj_set_style_bg_color",        lv_obj_set_style_bg_color_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_bg_grad_color",   lv_obj_set_style_bg_grad_color_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_bg_grad_dir",     lv_obj_set_style_bg_grad_dir_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_bg_opa",          lv_obj_set_style_bg_opa_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_text_color",      lv_obj_set_style_text_color_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_text_align",      lv_obj_set_style_text_align_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_text_letter_space", lv_obj_set_style_text_letter_space_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_text_line_space", lv_obj_set_style_text_line_space_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_radius",          lv_obj_set_style_radius_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_border_width",    lv_obj_set_style_border_width_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_border_color",    lv_obj_set_style_border_color_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_border_side",     lv_obj_set_style_border_side_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_pad_all",         lv_obj_set_style_pad_all_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_pad_top",         lv_obj_set_style_pad_top_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_pad_bottom",      lv_obj_set_style_pad_bottom_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_pad_left",        lv_obj_set_style_pad_left_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_pad_right",       lv_obj_set_style_pad_right_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_margin_all",      lv_obj_set_style_margin_all_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_margin_top",      lv_obj_set_style_margin_top_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_margin_bottom",   lv_obj_set_style_margin_bottom_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_margin_left",     lv_obj_set_style_margin_left_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_margin_right",    lv_obj_set_style_margin_right_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_opa",             lv_obj_set_style_opa_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_outline_width",   lv_obj_set_style_outline_width_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_outline_color",   lv_obj_set_style_outline_color_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_outline_opa",     lv_obj_set_style_outline_opa_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_outline_pad",     lv_obj_set_style_outline_pad_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_shadow_width",    lv_obj_set_style_shadow_width_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_shadow_color",    lv_obj_set_style_shadow_color_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_shadow_opa",      lv_obj_set_style_shadow_opa_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_shadow_ofs_x",    lv_obj_set_style_shadow_ofs_x_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_shadow_ofs_y",    lv_obj_set_style_shadow_ofs_y_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_transform_scale", lv_obj_set_style_transform_scale_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_transform_angle", lv_obj_set_style_transform_angle_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_line_width",      lv_obj_set_style_line_width_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_line_color",      lv_obj_set_style_line_color_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_line_rounded",    lv_obj_set_style_line_rounded_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_min_width",       lv_obj_set_style_min_width_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_min_height",      lv_obj_set_style_min_height_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_max_width",       lv_obj_set_style_max_width_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_max_height",      lv_obj_set_style_max_height_w);
  CDM_RegisterHostFuncEx("lv_obj_set_style_clip_corner",     lv_obj_set_style_clip_corner_w);

  CDM_RegisterHostFuncEx("lv_bar_set_value",     lv_bar_set_value_w);
  CDM_RegisterHostFuncEx("lv_bar_get_value",     lv_bar_get_value_w);
  CDM_RegisterHostFuncEx("lv_bar_set_range",     lv_bar_set_range_w);
  CDM_RegisterHostFuncEx("lv_slider_set_value",  lv_slider_set_value_w);
  CDM_RegisterHostFuncEx("lv_slider_get_value",  lv_slider_get_value_w);
  CDM_RegisterHostFuncEx("lv_slider_set_range",  lv_slider_set_range_w);
  CDM_RegisterHostFuncEx("lv_arc_set_value",     lv_arc_set_value_w);
  CDM_RegisterHostFuncEx("lv_arc_get_value",     lv_arc_get_value_w);
  CDM_RegisterHostFuncEx("lv_arc_set_range",     lv_arc_set_range_w);
  CDM_RegisterHostFuncEx("lv_arc_set_angles",    lv_arc_set_angles_w);
  CDM_RegisterHostFuncEx("lv_obj_add_state",     lv_obj_add_state_w);
  CDM_RegisterHostFuncEx("lv_obj_clear_state",   lv_obj_clear_state_w);
  CDM_RegisterHostFuncEx("lv_obj_has_state",     lv_obj_has_state_w);

  CDM_RegisterHostFuncEx("lv_obj_add_flag",   lv_obj_add_flag_w);
  CDM_RegisterHostFuncEx("lv_obj_clear_flag", lv_obj_clear_flag_w);
  CDM_RegisterHostFuncEx("lv_obj_delete",     lv_obj_delete_w);
  CDM_RegisterHostFuncEx("lv_obj_clean",      lv_obj_clean_w);

  CDM_RegisterHostFuncEx("lv_checkbox_set_text", lv_checkbox_set_text_w);
  CDM_RegisterHostFuncEx("lv_checkbox_get_text", lv_checkbox_get_text_w);

  CDM_RegisterHostFuncEx("lv_dropdown_set_options",  lv_dropdown_set_options_w);
  CDM_RegisterHostFuncEx("lv_dropdown_set_selected", lv_dropdown_set_selected_w);
  CDM_RegisterHostFuncEx("lv_dropdown_get_selected", lv_dropdown_get_selected_w);

  CDM_RegisterHostFuncEx("lv_roller_set_options",  lv_roller_set_options_w);
  CDM_RegisterHostFuncEx("lv_roller_set_selected", lv_roller_set_selected_w);
  CDM_RegisterHostFuncEx("lv_roller_get_selected", lv_roller_get_selected_w);

  CDM_RegisterHostFuncEx("lv_textarea_set_text",             lv_textarea_set_text_w);
  CDM_RegisterHostFuncEx("lv_textarea_get_text",             lv_textarea_get_text_w);
  CDM_RegisterHostFuncEx("lv_textarea_set_placeholder_text", lv_textarea_set_placeholder_text_w);
  CDM_RegisterHostFuncEx("lv_textarea_set_one_line",         lv_textarea_set_one_line_w);
  CDM_RegisterHostFuncEx("lv_textarea_set_max_length",       lv_textarea_set_max_length_w);
  CDM_RegisterHostFuncEx("lv_textarea_add_text",             lv_textarea_add_text_w);

  CDM_RegisterHostFuncEx("lv_table_set_cell_value",   lv_table_set_cell_value_w);
  CDM_RegisterHostFuncEx("lv_table_get_cell_value",   lv_table_get_cell_value_w);
  CDM_RegisterHostFuncEx("lv_table_set_row_count",    lv_table_set_row_count_w);
  CDM_RegisterHostFuncEx("lv_table_set_column_count", lv_table_set_column_count_w);

  CDM_RegisterHostFuncEx("lv_led_set_color",      lv_led_set_color_w);
  CDM_RegisterHostFuncEx("lv_led_set_brightness", lv_led_set_brightness_w);
  CDM_RegisterHostFuncEx("lv_led_on",             lv_led_on_w);
  CDM_RegisterHostFuncEx("lv_led_off",            lv_led_off_w);
  CDM_RegisterHostFuncEx("lv_led_toggle",         lv_led_toggle_w);

  CDM_RegisterHostFuncEx("lv_image_set_src",      lv_image_set_src_w);
  CDM_RegisterHostFuncEx("lv_image_set_scale",    lv_image_set_scale_w);
  CDM_RegisterHostFuncEx("lv_image_set_rotation", lv_image_set_rotation_w);

  CDM_RegisterHostFuncEx("lv_obj_set_ext_click_area", lv_obj_set_ext_click_area_w);

  CDM_RegisterHostFuncEx("lv_obj_add_event_cb",     lv_obj_add_event_cb_w);
  CDM_RegisterHostFuncEx("lv_event_pending",        lv_event_pending_w);
  CDM_RegisterHostFuncEx("lv_event_get_obj",        lv_event_get_obj_w);
  CDM_RegisterHostFuncEx("lv_event_get_code",       lv_event_get_code_w);
  CDM_RegisterHostFuncEx("lv_event_clear",          lv_event_clear_w);

  CDM_RegisterHostFuncEx("lv_chart_set_type",       lv_chart_set_type_w);
  CDM_RegisterHostFuncEx("lv_chart_set_point_count",lv_chart_set_point_count_w);
  CDM_RegisterHostFuncEx("lv_chart_set_range",      lv_chart_set_range_w);
  CDM_RegisterHostFuncEx("lv_chart_set_next_value", lv_chart_set_next_value_w);

  CDM_RegisterHostFuncEx("lv_style_create",       lv_style_create_w);
  CDM_RegisterHostFuncEx("lv_style_delete",       lv_style_delete_w);
  CDM_RegisterHostFuncEx("lv_style_reset",        lv_style_reset_w);
  CDM_RegisterHostFuncEx("lv_styles_clear",       lv_styles_clear_w);
  CDM_RegisterHostFuncEx("lv_style_set_name",     lv_style_set_name_w);
  CDM_RegisterHostFuncEx("lv_style_find",         lv_style_find_w);

  CDM_RegisterHostFuncEx("lv_style_set_bg_color",           style_set_bg_color_w);
  CDM_RegisterHostFuncEx("lv_style_set_bg_opa",             style_set_bg_opa_w);
  CDM_RegisterHostFuncEx("lv_style_set_bg_grad_color",      style_set_bg_grad_color_w);
  CDM_RegisterHostFuncEx("lv_style_set_bg_grad_dir",        style_set_bg_grad_dir_w);
  CDM_RegisterHostFuncEx("lv_style_set_text_color",         style_set_text_color_w);
  CDM_RegisterHostFuncEx("lv_style_set_text_font",          style_set_text_font_w);
  CDM_RegisterHostFuncEx("lv_style_set_text_align",         style_set_text_align_w);
  CDM_RegisterHostFuncEx("lv_style_set_text_letter_space",  style_set_text_letter_space_w);
  CDM_RegisterHostFuncEx("lv_style_set_text_line_space",    style_set_text_line_space_w);
  CDM_RegisterHostFuncEx("lv_style_set_radius",             style_set_radius_w);
  CDM_RegisterHostFuncEx("lv_style_set_border_width",       style_set_border_width_w);
  CDM_RegisterHostFuncEx("lv_style_set_border_color",       style_set_border_color_w);
  CDM_RegisterHostFuncEx("lv_style_set_border_side",        style_set_border_side_w);
  CDM_RegisterHostFuncEx("lv_style_set_pad_all",            style_set_pad_all_w);
  CDM_RegisterHostFuncEx("lv_style_set_pad_top",            style_set_pad_top_w);
  CDM_RegisterHostFuncEx("lv_style_set_pad_bottom",         style_set_pad_bottom_w);
  CDM_RegisterHostFuncEx("lv_style_set_pad_left",           style_set_pad_left_w);
  CDM_RegisterHostFuncEx("lv_style_set_pad_right",          style_set_pad_right_w);
  CDM_RegisterHostFuncEx("lv_style_set_margin_all",         style_set_margin_all_w);
  CDM_RegisterHostFuncEx("lv_style_set_margin_top",         style_set_margin_top_w);
  CDM_RegisterHostFuncEx("lv_style_set_margin_bottom",      style_set_margin_bottom_w);
  CDM_RegisterHostFuncEx("lv_style_set_margin_left",        style_set_margin_left_w);
  CDM_RegisterHostFuncEx("lv_style_set_margin_right",       style_set_margin_right_w);
  CDM_RegisterHostFuncEx("lv_style_set_opa",                style_set_opa_w);
  CDM_RegisterHostFuncEx("lv_style_set_shadow_width",       style_set_shadow_width_w);
  CDM_RegisterHostFuncEx("lv_style_set_shadow_color",       style_set_shadow_color_w);
  CDM_RegisterHostFuncEx("lv_style_set_shadow_opa",         style_set_shadow_opa_w);
  CDM_RegisterHostFuncEx("lv_style_set_shadow_ofs_x",       style_set_shadow_ofs_x_w);
  CDM_RegisterHostFuncEx("lv_style_set_shadow_ofs_y",       style_set_shadow_ofs_y_w);
  CDM_RegisterHostFuncEx("lv_style_set_outline_width",      style_set_outline_width_w);
  CDM_RegisterHostFuncEx("lv_style_set_outline_color",      style_set_outline_color_w);
  CDM_RegisterHostFuncEx("lv_style_set_outline_opa",        style_set_outline_opa_w);
  CDM_RegisterHostFuncEx("lv_style_set_outline_pad",        style_set_outline_pad_w);
  CDM_RegisterHostFuncEx("lv_style_set_line_width",         style_set_line_width_w);
  CDM_RegisterHostFuncEx("lv_style_set_line_color",         style_set_line_color_w);
  CDM_RegisterHostFuncEx("lv_style_set_min_width",          style_set_min_width_w);
  CDM_RegisterHostFuncEx("lv_style_set_min_height",         style_set_min_height_w);
  CDM_RegisterHostFuncEx("lv_style_set_max_width",          style_set_max_width_w);
  CDM_RegisterHostFuncEx("lv_style_set_max_height",         style_set_max_height_w);
  CDM_RegisterHostFuncEx("lv_style_set_clip_corner",        style_set_clip_corner_w);
  CDM_RegisterHostFuncEx("lv_style_set_transform_scale",    style_set_transform_scale_w);

  CDM_RegisterHostFuncEx("lv_obj_add_style",         lv_obj_add_style_w);
  CDM_RegisterHostFuncEx("lv_obj_remove_style",      lv_obj_remove_style_w);
  CDM_RegisterHostFuncEx("lv_obj_remove_style_all",  lv_obj_remove_style_all_w);
}