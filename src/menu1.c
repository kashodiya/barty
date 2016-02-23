#include <pebble.h>

#define TOT_STATIONS 45

static Window *s_main_window, *s_etd_window;
static MenuLayer *s_menu_layer;
static TextLayer *text_layer;
static uint16_t current_station_index = 0;
static GBitmap *s_up_bitmap, *s_down_bitmap, *s_dots_bitmap;

typedef enum {
  KeyInit = 0,
  KeyStn,
  KeyEtd,
  KeyLastStnIndex,
} MsgKey;

//static char s_etd[100];

const char *stations_abbr[45] = {
"12th", "16th", "19th", "24th", "ashb", "balb", "bayf", "cast", "civc", "cols",
"colm", "conc", "daly", "dbrk", "dubl", "deln", "plza", "embr", "frmt", "ftvl",
"glen", "hayw", "lafy", "lake", "mcar", "mlbr", "mont", "nbrk", "ncon", "oakl",
"orin", "pitt", "phil", "powl", "rich", "rock", "sbrn", "sfia", "sanl", "shay",
"ssan", "ucty", "wcrk", "wdub", "woak" };

const char *stations[45] = {
"12th St. Oakland City Center", "16th St. Mission (SF)", "19th St. Oakland",
"24th St. Mission (SF)", "Ashby (Berkeley)", "Balboa Park (SF)",
"Bay Fair (San Leandro)", "Castro Valley", "Civic Center (SF)", "Coliseum",
"Colma", "Concord", "Daly City", "Downtown Berkeley", "Dublin/Pleasanton",
"El Cerrito del Norte", "El Cerrito Plaza", "Embarcadero (SF)", "Fremont",
"Fruitvale (Oakland)", "Glen Park (SF)", "Hayward", "Lafayette",
"Lake Merritt (Oakland)", "MacArthur (Oakland)", "Millbrae",
"Montgomery St. (SF)", "North Berkeley", "North Concord/Martinez",
"Oakland Int'l Airport", "Orinda", "Pittsburg/Bay Point", "Pleasant Hill",
"Powell St. (SF)", "Richmond", "Rockridge (Oakland)", "San Bruno",
"San Francisco Int'l Airport", "San Leandro", "South Hayward",
"South San Francisco", "Union City", "Walnut Creek", "West Dublin",
"West Oakland"};


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "XSelect");
  menu_layer_set_selected_index(s_menu_layer,
    (MenuIndex){
      .section = 0,
      .row = current_station_index
    },
    MenuRowAlignCenter, true);
  window_stack_pop(true);
}

static void get_etd(){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In get_etd()");

  // text_layer_set_text(text_layer,
  //   strcat("\n", stations[current_station_index])
  // );

  text_layer_set_text(text_layer, "...");

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, KeyStn, stations_abbr[current_station_index]);
  dict_write_end(iter);
  app_message_outbox_send();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(current_station_index > 0){
    current_station_index--;
    get_etd();
  }else{
    text_layer_set_text(text_layer, "\nTop reached!");
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(current_station_index < TOT_STATIONS - 1){
    current_station_index++;
    get_etd();
  }else{
    text_layer_set_text(text_layer, "\nBottom reached!");
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
}


static void window_load_etd(Window *window) {
  Layer *window_layer = window_get_root_layer(s_etd_window);
  GRect bounds = layer_get_bounds(window_layer);

  const GEdgeInsets text_layer_insets = {.right = ACTION_BAR_WIDTH};
  text_layer = text_layer_create(grect_inset(bounds, text_layer_insets));

  ///text_layer_set_text(text_layer, stations[current_station_index] + "\n\nLoading...");
  text_layer_set_text(text_layer, stations[current_station_index]);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  text_layer_enable_screen_text_flow_and_paging(text_layer, 4);


  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UP);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOWN);
  s_dots_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOTS);

  ActionBarLayer *action_bar = action_bar_layer_create();
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, s_up_bitmap);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, s_dots_bitmap);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, s_down_bitmap);
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar,
                                               click_config_provider);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending message for data...");
  get_etd();

}

static void window_unload_etd(Window *window) {
  text_layer_destroy(text_layer);
  window_destroy(window);
  s_etd_window = NULL;
  text_layer = NULL;
}

static void etd_window_push(uint16_t station_index) {
  current_station_index = station_index;
//  if(!s_etd_window) {
    s_etd_window = window_create();
    //window_set_click_config_provider(s_etd_window, click_config_provider);
    window_set_window_handlers(s_etd_window, (WindowHandlers) {
        .load = window_load_etd,
        .unload = window_unload_etd,
    });
//  }
  window_stack_push(s_etd_window, true);

}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return TOT_STATIONS;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  menu_cell_basic_draw(ctx, cell_layer, stations[cell_index->row], NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    20);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  etd_window_push(cell_index->row);
  // switch(cell_index->row) {
  //   case 0:
  //     APP_LOG(APP_LOG_LEVEL_DEBUG, "Item 0 clicked");
  //     etd_window_push();
  //     break;
  //   case 1:
  //     APP_LOG(APP_LOG_LEVEL_DEBUG, "Item 1 clicked");
  //     etd_window_push();
  //     break;
  //   default:
  //     break;
  // }
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Received in test3.c!");

  Tuple *etdTuple = dict_find(iter, KeyEtd);

  if (etdTuple) {
    //strncpy(s_etd, etdTuple->value->cstring, strlen(etdTuple->value->cstring));
    //strncpy(s_etd, etdTuple->value->cstring, 10);
    APP_LOG(APP_LOG_LEVEL_DEBUG, etdTuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Length of etdTuple->value->cstring is %d", etdTuple->length);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, s_etd);

    text_layer_set_text(text_layer, etdTuple->value->cstring);
    //text_layer_set_text(text_layer, "OK1");
  }
  /*
  */
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}


static void window_load(Window *window) {

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, GColorBlue, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorRed, GColorWhite);
#endif
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  if(persist_exists(KeyLastStnIndex)){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "persist_exists(KeyLastStnIndex) is true");
    current_station_index = persist_read_int(KeyLastStnIndex);
    menu_layer_set_selected_index(s_menu_layer,
      (MenuIndex){
        .section = 0,
        .row = current_station_index
      },
      MenuRowAlignCenter, true);
  }


}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

static void init(void) {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(255, 255);

}

static void deinit(void) {
  persist_write_int(KeyLastStnIndex, current_station_index);
  window_destroy(s_main_window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_main_window);

  app_event_loop();
  deinit();
}
