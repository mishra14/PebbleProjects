#include<pebble.h>						//Include the pebble header file
#define KEY_INIT 0						//declare keys used for AppMessage communication
#define KEY_FETCH 1
#define KEY_SYMBOL 2
#define KEY_PRICE 3
#define KEY_CHANGE 4
#define KEY_ALERT 5

Window *window;																	//Define the layer and window handlers
TextLayer *priceLayer, *symbolLayer, *priceStatusLayer;
BitmapLayer *bitmapLayer,*bitmapLayerStatusHappy, *bitmapLayerStatusSad;
GBitmap *bitMap,*bitMapStatusHappy, *bitMapStatusSad;
DictionaryIterator *iterator;						//define the iterator used to read the dictionary
int symbolCount, first;
char *symbolTable[]={"SRPT","MSFT","AAPL","FNFG"};						//The symbol table that points to 4 different symbols
static void updateStock()
{
    DictionaryIterator *iterator;
    app_message_outbox_begin(&iterator);										//create a message to send to the phone
    Tuplet value = TupletCString(KEY_SYMBOL, symbolTable[symbolCount]);
    dict_write_tuplet(iterator, &value);														//write value into the dictionary
    app_message_outbox_send();				// a call will be made to out_sent_handler or out_failed_handler after the message is sent to the phone  
}
static void tickHandler(struct tm *tick_time, TimeUnits units_changed)								//tick handler which is called after every second's tick
{
	if(first==0)														//If the tick is called for the first time, then update the stock
	{
		updateStock();
		symbolCount=(symbolCount==3)?(0):(symbolCount+1);
		first=1;														//Update the flag to indicate that the first call has been made
	}
  if((tick_time->tm_sec%10==0))										//conditional check to ensure that the stock is refreshed every 10 seconds
  {
		updateStock();
		symbolCount=(symbolCount==3)?(0):(symbolCount+1);						//update the symbol count to point to the next symbol
  }
}

//weather app message calls

void out_sent_handler(DictionaryIterator *sentIterator, void *context)						//called when the message to the phone is successful and an ACK is received
{
	APP_LOG(APP_LOG_LEVEL_INFO,"outbox message sent ");
}

void out_failed_handler(DictionaryIterator *failedIterator, AppMessageResult reason, void *context)						//called when the message to phone is not delivered and a NACK is received
{
	APP_LOG(APP_LOG_LEVEL_ERROR,"outbox message failed ..");		//update the app log
	text_layer_set_text(symbolLayer,"Phone Connection");			//Display an error message
  text_layer_set_text(priceLayer,"Lost");						//Display an error message
  Layer *windowLayer=window_get_root_layer(window);
  layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));						//display a blank image to clear the previous image
}

void in_received_handler(DictionaryIterator *receivedIterator, void *context)
{
  APP_LOG(APP_LOG_LEVEL_INFO,"inbox received handler");
	static char priceBuffer[20];
  static char priceDisplayBuffer[20];
	static char symbolBuffer[10];
	static char symbolDisplayBuffer[20];
  Tuple *symbolTuple = dict_find(receivedIterator, KEY_SYMBOL);						//Search for the Symbol Key tuple
  Tuple *fetchTuple = dict_find(receivedIterator, KEY_FETCH);						//Search for the Fetch Key tuple
  Tuple *priceTuple = dict_find(receivedIterator, KEY_PRICE);						//Search for the Price Key tuple
	Tuple *changeTuple = dict_find(receivedIterator, KEY_CHANGE);						//Search for the Change Key tuple
	Tuple *alertTuple = dict_find(receivedIterator, KEY_ALERT);						//Search for the Alert Key tuple
	if(symbolTuple!=NULL)						//If the symbol tuple was present in the dictionary
	{
			strcpy(symbolBuffer,symbolTuple->value->cstring);						//copy the tuple value into the symbol buffer
			snprintf(symbolDisplayBuffer,sizeof(symbolDisplayBuffer),"Company : \n%s",symbolBuffer);
			text_layer_set_text(symbolLayer,symbolDisplayBuffer);						//display the symbol
	}
  if(fetchTuple!=NULL)						//If the fetch tuple was present in the dictionary
  {
      if(strcmp(fetchTuple->value->cstring,"Invalid Symbol")==0)
      {
          strcpy(priceBuffer,fetchTuple->value->cstring);						//If the fetch key was present then the symbol was not found by the JS
      }
      else
      {
          APP_LOG(APP_LOG_LEVEL_ERROR, "KEY_FETCH present in dictionary with invalid content");
      }
  }
  else if(priceTuple!=NULL)						//If the purple tuple was present in the dictionary
  {
      strncpy(priceBuffer,priceTuple->value->cstring,20);
			if(changeTuple!=NULL)						//If the change tuple was present in the dictionary
			{
					Layer *windowLayer=window_get_root_layer(window);						//retrieve the window layer
					if(strcmp(changeTuple->value->cstring,"UP")==0)						//check if the stock was up
					{
						if(alertTuple!=NULL)						//If the alert tuple was present in the dictionary
						{
							if(strcmp(alertTuple->value->cstring,"SELL")==0)						//check if the alert was set to SELL
							{
								text_layer_set_text(priceStatusLayer,alertTuple->value->cstring);
								layer_add_child(windowLayer,text_layer_get_layer(priceStatusLayer));						//add the alert to the window
							}
							else
							{
								layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));
								layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayerStatusHappy));						//add the UP image to the window
							}
						}
						else
						{
							layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));
							layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayerStatusHappy));						//add the UP image to the window
						}
					}
					else if(strcmp(changeTuple->value->cstring,"DOWN")==0)						//check if  the change tuple was set to DOWN
					{
						if(alertTuple!=NULL)						//check if the alert tuple was present in the dictionary or not
						{
							if(strcmp(alertTuple->value->cstring,"BUY")==0)						//check if the alert tuple is set to BUY
							{
								text_layer_set_text(priceStatusLayer,alertTuple->value->cstring);
								layer_add_child(windowLayer,text_layer_get_layer(priceStatusLayer));						//add the alert into the window
							}
							else
							{
								layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));
								layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayerStatusSad));						//add the DOWN image into the window
							}
						}
						else
						{
							layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));
							layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayerStatusSad));						//add the down image into the window
						}

					}
					else
					{
						APP_LOG(APP_LOG_LEVEL_ERROR, "Change Tuple contains invalid data");
						layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));						//if no conditions are met then add a clear image to the window
					}
			}
  }
  else
  {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Dictionary contains invalid key");
  }
	snprintf(priceDisplayBuffer,sizeof(priceDisplayBuffer),"Price :  \n%s",priceBuffer);
  //APP_LOG(APP_LOG_LEVEL_INFO,weatherUpdateBuffer);
	text_layer_set_text(priceLayer,priceDisplayBuffer);							//display the stock price on the window
}

void in_failed_handler(AppMessageResult reason, void *context)						//handler for an incoming message drop
{
	APP_LOG(APP_LOG_LEVEL_ERROR,"inbox message dropped!!");
}

static void windowLoad(Window *window)						//intializing the pebble window function
{
  Layer *windowLayer=window_get_root_layer(window);						//get the root window layer
  GRect bounds=layer_get_frame(windowLayer);						//get the bounds of the window layer
  
  bitmapLayer=bitmap_layer_create((GRect){.origin={0,(2*bounds.size.h)/3},.size={bounds.size.w,bounds.size.h/3}});						//create a blank bitmapLayer to display a blank image if needed
  bitmap_layer_set_background_color(bitmapLayer, GColorWhite);						//set the background colour as white
  //bitmap_layer_set_bitmap(bitmapLayer,bitMap);
  layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayer));						//add the clear image onto the window
  
  bitMapStatusHappy=gbitmap_create_with_resource(RESOURCE_ID_PANDA_HAPPY);						//create a bitmap to display the UP image
  bitmapLayerStatusHappy=bitmap_layer_create((GRect){.origin={0,(2*bounds.size.h)/3},.size={bounds.size.w,bounds.size.h/3}});						//create a layer to display the UP Image
  bitmap_layer_set_bitmap(bitmapLayerStatusHappy,bitMapStatusHappy);						//Add the image into the layer
  //layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayerStatusHappy));
	
	bitMapStatusSad=gbitmap_create_with_resource(RESOURCE_ID_PANDA_SAD);						//create a bitmap to display the DOWN image
  bitmapLayerStatusSad=bitmap_layer_create((GRect){.origin={0,(2*bounds.size.h)/3},.size={bounds.size.w,bounds.size.h/3}});						//create a layer to display the down image
  bitmap_layer_set_bitmap(bitmapLayerStatusSad,bitMapStatusSad);						//add the DOWN image to the layer
  //layer_add_child(windowLayer,bitmap_layer_get_layer(bitmapLayerStatusSad));
  
	symbolLayer=text_layer_create((GRect){.origin={0,0},.size={bounds.size.w,bounds.size.h/3}});						//create a symbol layer to display the company symbol
  text_layer_set_text(symbolLayer, "Company : ...");						//Set the company display
	text_layer_set_text_color(symbolLayer,GColorBlack);
	text_layer_set_text_alignment(symbolLayer,GTextAlignmentCenter);
	text_layer_set_font(symbolLayer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(windowLayer,text_layer_get_layer(symbolLayer));						//add the text to the window
	
  priceLayer=text_layer_create((GRect){.origin={0,bounds.size.h/3},.size={bounds.size.w,bounds.size.h/3}});						//create a layer to display theprices
  text_layer_set_text(priceLayer, "Price : ...");						//set the initial text for the layer
  text_layer_set_text_color(priceLayer,GColorBlack);
  //text_layer_set_background_color(priceLayer,GColorBlack);
  text_layer_set_text_alignment(priceLayer,GTextAlignmentCenter);
  text_layer_set_font(priceLayer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(windowLayer,text_layer_get_layer(priceLayer));						//add the text to the window
  
  priceStatusLayer=text_layer_create((GRect){.origin={0,(2*bounds.size.h)/3},.size={bounds.size.w,bounds.size.h/3}});						//create a text layer to display the BUY/SELL alerts
  text_layer_set_text(priceStatusLayer,"$$$$");							//set the initial text for the layer
  text_layer_set_text_color(priceStatusLayer,GColorBlack);
  text_layer_set_text_alignment(priceStatusLayer,GTextAlignmentCenter);
  //text_layer_set_background_color(priceStatusLayer,GColorClear);
  text_layer_set_font(priceStatusLayer,fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  //layer_add_child(windowLayer,text_layer_get_layer(priceStatusLayer));
}
static void windowUnload(Window *window)						//function to destroy any created resources
{
	text_layer_destroy(priceLayer);						//destroy the text layers
	text_layer_destroy(priceStatusLayer);
	text_layer_destroy(symbolLayer);
      gbitmap_destroy(bitMapStatusHappy);						//destroy the bit maps
      gbitmap_destroy(bitMapStatusSad);
      bitmap_layer_destroy(bitmapLayerStatusHappy);						//destroy the bitmaplayers
      bitmap_layer_destroy(bitmapLayerStatusSad);
      bitmap_layer_destroy(bitmapLayer);
}
void init()						//function to intialize the system
{
  window=window_create();						//reate a window which will contain the whole app
  window_set_window_handlers(window,(WindowHandlers){.load=windowLoad,.unload=windowUnload,});						//define the window handlers
  tick_timer_service_subscribe(SECOND_UNIT,tickHandler);						//register for a time tick event for each second
  window_stack_push(window,true);						//push the main window onto the window stack of the pebble
  app_message_register_inbox_received(in_received_handler);						//register the handler functions for App Messages
  app_message_register_inbox_dropped(in_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());						//start the app messages with the maximum buffers
}
void deinit()						//function to destroy the window an unsubscribe from the tick event service
{
      tick_timer_service_unsubscribe();						//turn off the tick events
      window_destroy(window);						//destroy the main window
}
int main()
{
    init();						//initialize the system
    app_event_loop();						//create a loop for events
    deinit();						//clear the window and the event service
	return 0;
}
