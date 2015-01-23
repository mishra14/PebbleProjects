var xhrRequest = function (url, type, callback) 						//create an XMLHttpRequest object with a url, call method and a callback function when the url responds
{
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();						//send the xml request
};
var xhrRequestAlert = function (url, type, callback) 					//create an XMLHttpRequest object to hit the alert URL with a url, call method and a callback function when the url responds
{
  var xhrAlert = new XMLHttpRequest();
  xhrAlert.onload = function () {
    callback(this.responseText);
  };
  xhrAlert.open(type, url);
  xhrAlert.send();								//send the xml request
};
var changePercentage;								//variable to parse the chnage percentage
var symbol;								//symbol variable used to hit the market url
var firstSymbol="null";								//variables to store the symbols
var secondSymbol="null";
var thirdSymbol="null";
var fourthSymbol="null";
var dictionary;								//variable to store the dictionary
var firstSymbolUPAlertSent=0;								//variables to store the alert flags
var secondSymbolUPAlertSent=0;
var thirdSymbolUPAlertSent=0;
var fourthSymbolUPAlertSent=0;
var fourthSymbolDOWNAlertSent=0;
var firstSymbolDOWNAlertSent=0;
var secondSymbolDOWNAlertSent=0;
var thirdSymbolDOWNAlertSent=0;
function sendUPAlert()								//function to send the alert in case the stock is up by more than 20%
{
	var url="http://stockpebble.herobo.com/StockStatusUPAlert.php?company="+symbol+"&change="+changePercentage;								//the url variable
	xhrRequestAlert(url,'GET',function(responseText)
		{
			var jsonObj=JSON.parse(responseText);
			if(jsonObj.AlertStatus==1)
			{
				console.log("Alert sent successfully!!");
			}
			else
			{
				console.log("Alert unsuccessful");
			}
		}
		);
}
function sendDOWNAlert()								//function to send the alert if the stock is down by more than 20%
{
	var url="http://stockpebble.herobo.com/StockStatusDOWNAlert.php?company="+symbol+"&change="+changePercentage;								//the url variable
	xhrRequestAlert(url,'GET',function(responseText)
		{
			var jsonObj=JSON.parse(responseText);
			if(jsonObj.AlertStatus==1)
			{
				console.log("Alert sent successfully!!");
			}
			else
			{
				console.log("Alert unsuccessful");
			}
		}
		);
}
function getStock()										//function to get the stock details for a given symbol
{

	var url="http://dev.markitondemand.com/Api/Quote/json?" +    "symbol=" + symbol;								//the url variable
	var price;
	var change;
	var alert;
	xhrRequest(url,'GET',function(responseText)
            {
                var jsonObj=JSON.parse(responseText);
                if(jsonObj.Message)										//the website returns a MEssage field if the symbol was not found
                  {
                    console.log("symbol not found");										//display that the symbol not found error in the log
                    dictionary = 
                    {
						"KEY_FETCH": "Invalid Symbol",										//add the fetch key if the symbol was not found
						"KEY_SYMBOL":symbol													//adding the symbol which was not found
                    };
               
                    // Send to Pebble
                    Pebble.sendAppMessage(dictionary,										//send the dictionary to the pebble as an appmessage
                      function(e) 										//function for a successful send
                      {
                        console.log("Invalid Symbol message sent to Pebble successfully!");
                      },
                      function(e) 										//function for an unsuccessful send
                      {
                        console.log("Error sending the Invalid Symbol message info to Pebble!");
                      }
                    );
                  }
                else if(jsonObj.Data)													//the json object has the Data field if the symbol was found and the data was obtained
                {
                    
                    price=jsonObj.Data.LastPrice;										//parse the LastPrice field from the JSon object
                    console.log("Price is "+price);
					if(jsonObj.Data.ChangePercent>0.0)										//check if the change percentage is more than 0
						{
							change="UP";										//set the change key field as UP
							if(jsonObj.Data.ChangePercent>20.0)										//check if the change percentage is more than 20%
							{
								alert="SELL"; 										//set the alert field as SELL if the chnage percent is more than 20%
								changePercentage=jsonObj.Data.ChangePercent;
								if(symbol==firstSymbol)										// check if the symbol is equal to the first symbol
								{
									if(firstSymbolUPAlertSent==0)							//check if an alert has already been sent or not
									{
										sendUPAlert();										//send alert for the first symbol
										firstSymbolUPAlertSent=1;							//set the fag to indicate that the alert has been sent or not
									}
								}
								else if(symbol==secondSymbol)								//check if the symbol is equal to the second symbol
								{
									if(secondSymbolUPAlertSent==0)							//check if an alert has been sent or not
									{
										sendUPAlert();										//send the alert
										secondSymbolUPAlertSent=1;							//set the flag to indicate that the alert has been sent or not 
									}
								}
								else if(symbol==thirdSymbol)
								{
									if(thirdSymbolUPAlertSent==0)
									{
										sendUPAlert();
										thirdSymbolUPAlertSent=1;
									}
								}
								else if(symbol==fourthSymbol)
								{
									if(fourthSymbolUPAlertSent==0)
									{
										sendUPAlert();
										fourthSymbolUPAlertSent=1;
									}
								}
							}
							else
							{
								alert="NO SELL";
							}
						}
					else
						{
							change="DOWN";
							if(jsonObj.Data.ChangePercent<-20.0)								//check if the percentage change was below -20%
							{
								alert="BUY";													//set the alert field as BUY
								changePercentage=jsonObj.Data.ChangePercent;					//parse the change percentage
								if(symbol==firstSymbol)
								{
									if(firstSymbolDOWNAlertSent==0)
									{
										sendDOWNAlert();
										firstSymbolDOWNAlertSent=1;
									}
								}
								else if(symbol==secondSymbol)
								{
									if(secondSymbolDOWNAlertSent==0)
									{
										sendDOWNAlert();
										secondSymbolDOWNAlertSent=1;
									}
								}
								else if(symbol==thirdSymbol)
								{
									if(thirdSymbolDOWNAlertSent==0)
									{
										sendDOWNAlert();
										thirdSymbolDOWNAlertSent=1;
									}
								}
								else if(symbol==fourthSymbol)
								{
									if(fourthSymbolDOWNAlertSent==0)
									{
										sendDOWNAlert();
										fourthSymbolDOWNAlertSent=1;
									}
								}
							}
							else
							{
								alert="NO BUY";									//set the alert field as NO BUY if the chnage percentage is not more than 20%
							}
						}
					console.log("change percent is "+change);
                        // Assemble dictionary using our keys
                    dictionary = 								//create a dictionary
                    {
						"KEY_PRICE": price.toString(),
						"KEY_SYMBOL": symbol,
						"KEY_CHANGE": change,
						"KEY_ALERT":alert
                    };
               
                    // Send to Pebble
                    Pebble.sendAppMessage(dictionary,								//send the dictionary to the pebble
                      function(e) 
                      {
                        console.log("Stock Price info sent to Pebble successfully!");
                      },
                      function(e) 
                      {
                        console.log("Error sending Stock Price info to Pebble!");
                      }
                    );
                }
            }
            );
}

Pebble.addEventListener								//add an event listener that is called when the pebble  is ready
("ready",
  function(e)
  {
      console.log('pebble app has started.....');
  }
);

Pebble.addEventListener								//add an event listener that is called when the pebble sends an app message to the phone
("appmessage",
function(e) 
 {
    console.log('message received...');
     if(e.payload)								//check if the message has a payload
      {
          if(e.payload.KEY_SYMBOL)								//check if the message has a symbol field
            {
				console.log('symbol is '+e.payload.KEY_SYMBOL);
				symbol=e.payload.KEY_SYMBOL;
				if(firstSymbol=="null")								//check if the first symbol has been recorded or not
				{	
					firstSymbol=symbol;								//record the first symbol
				}
				else if(secondSymbol=="null")								//check if the second symbol has been recorded or not
				{
					secondSymbol=symbol;								//record the first symbol
				}
				else if(thirdSymbol=="null")								//check if the third symbol has been recorded or not
				{
					thirdSymbol=symbol;								//record the first symbol
				}
				else if(fourthSymbol=="null")								//check if the fourth symbol has been recorded or not
				{
					fourthSymbol=symbol;								//record the first symbol
				}
				getStock();								//check if the first symbol has been recorded or not
            }
      }
  }
);
