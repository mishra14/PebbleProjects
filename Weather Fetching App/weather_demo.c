#include <pebble.h>																//include pebble header file
#define KEY_TEMPERATURE 0 																//set the keys
#define KEY_CONDITIONS 1

Window *window;																//define the layer handler pointers
TextLayer *textLayer;
BitmapLayer *bitmapLayer;
GBitmap *bitMap;
GFont *weatherFont;
TextLayer *weatherTextLayer;
DictionaryIterator *iterator;


void updateTime()																//funcion to update time
{
	time_t temp=time(NULL);
	struct tm *tick_time=localtime(&temp);																//read the local time
	static char buffer[]="00:00:00";
	if(clock_is_24h_style() == true) 																//check the format of the clock
	{
		strftime(buffer, sizeof("00:00:00"), "%H:%M:%S", tick_time);
	}
	else 
	{
		strftime(buffer, sizeof("00:00:00"), "%I:%M:%S", tick_time);
	}
	text_layer_set_text(textLayer,buffer);																//add the time into the layer
}
static void tickHandler(struct tm *tick_time, TimeUnits units_changed)																//tick event handler
{
	updateTime();																//call the function to update the time
  if(tick_time->tm_sec==0)																//update weather every 10 seconds
  {
    	text_layer_set_text(weatherTextLayer, "Loading...");
    	DictionaryIterator *iterator;
      app_message_outbox_begin(&iterator);
    	dict_write_uint8(iterator,0,0);																//send the message to the phone
    	app_message_outbox_send();				// a call will be made to out_sent_handler or out_failed_handler
  }
}

//weather app message calls

void out_sent_handler(DictionaryIterator *sentIterator, void *context)																//outbox message sent handler
{
	APP_LOG(APP_LOG_LEVEL_INFO,"outbox message sent ..");
}

void out_failed_handler(DictionaryIterator *failedIterator, AppMessageResult reason, void *context)																//handler for a failed outbox message
{
	APP_LOG(APP_LOG_LEVEL_INFO,"outbox message failed ..");
}

void in_received_handler(DictionaryIterator *receivedIterator, void *context)																//handler for for a received inbox message
{
  APP_LOG(APP_LOG_LEVEL_INFO,"inbox received handler");
	static char temperatureBuffer[8];
	static char conditionBuffer[10];
	static char weatherUpdateBuffer[20];
	Tuple *tuple =dict_read_first(receivedIterator);																	//read the first tuple from the dictionary
	while(tuple!=NULL)																//check if the tuple is not NULL
	{
		switch(tuple->key)																//switch case for the key of the tuple
		{
			case(KEY_TEMPERATURE):																//temperature key check
			{
				snprintf(temperatureBuffer,sizeof(temperatureBuffer),"%dC",(int)tuple->value->int32);
				break;
			}
			case(KEY_CONDITIONS):																//conditions key check
			{
				snprintf(conditionBuffer,sizeof(conditionBuffer),"%s",tuple->value->cstring);
				break;
			}
			default:
			{
				APP_LOG(APP_LOG_LEVEL_INFO,"unknown key");
			}
		}
     tuple = dict_read_next(receivedIterator);																//read the next tuple from the dictionary
	}
	snprintf(weatherUpdateBuffer,sizeof(weatherUpdateBuffer),"%s, %s",temperatureBuffer,conditionBuffer);
	text_layer_set_text(weatherTextLayer,weatherUpdateBuffer);																//update the weather text layer
}

void in_failed_handler(AppMessageResult reason, void *context)																//inbox message drop handler
{
	APP_LOG(APP_LOG_LEVEL_INFO,"inbox message dropped!!");
}

static void windowLoad(Window *window)																//window load handler function
{
  Layer *windowLayer=window_get_root_layer(window);																//create a window
  GRect bounds=layer_get_frame(windowLayer);																//get the bounds on the window
  
  
  bitMap=gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);																//create a background bitmap
  bitmapLayer=bitmap_layer_create((GRect){.origin={0,0},.size=bounds.size});																//create a bitmaplayer
  bitmap_layer_set_bitmap(bitmapLayer,bitMap);
  layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));																//add the background bitmap into the layer
  
  weatherTextLayer=text_layer_create((GRect){.origin={0,bounds.size.h/2},.size={bounds.size.h,bounds.size.w}});																//create a layer for weather text
  text_layer_set_text(weatherTextLayer, "Loading...");
  text_layer_set_text_color(weatherTextLayer,GColorBlack);
  text_layer_set_background_color(weatherTextLayer,GColorClear);
  text_layer_set_text_alignment(weatherTextLayer,GTextAlignmentCenter);
  text_layer_set_font(weatherTextLayer,fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(windowLayer,text_layer_get_layer(weatherTextLayer));																//add the text into the window
  
  textLayer=text_layer_create((GRect){.origin={bounds.size.w/2-(sizeof(text_layer_get_text(textLayer))*10),bounds.size.h-28},.size={bounds.size.h/2,bounds.size.w}});
  text_layer_set_text(textLayer,"00:00:00");
  text_layer_set_text_color(textLayer,GColorWhite);
  text_layer_set_background_color(textLayer,GColorClear);
  text_layer_set_font(textLayer,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  layer_add_child(windowLayer,text_layer_get_layer(textLayer));
  updateTime();
}
static void windowUnload(Window *window)																			//unload window handler
{
    text_layer_destroy(textLayer);
	text_layer_destroy(weatherTextLayer);
	gbitmap_destroy(bitMap);
	bitmap_layer_destroy(bitmapLayer);
}
void init()																//initialize the system
{
  window=window_create();
  window_set_window_handlers(window,(WindowHandlers){.load=windowLoad,.unload=windowUnload,});
  tick_timer_service_subscribe(SECOND_UNIT,tickHandler);
  window_stack_push(window,true);
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
void deinit()																//deinitialize the system
{
      tick_timer_service_unsubscribe();
      window_destroy(window);
}
int main()
{
    init();
    app_event_loop();
    deinit();
	return 0;
}
