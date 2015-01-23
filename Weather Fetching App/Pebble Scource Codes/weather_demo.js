var xhrRequest = function (url, type, callback)															//create an XMLRequest object
{
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(position)															//function to be called when the location is obtained
{
    //get weather from location
  var url="http://api.openweathermap.org/data/2.5/weather?lat=" + position.coords.latitude + "&lon=" + position.coords.longitude;
  xhrRequest(url,'GET',function(responseText)															//get the weather data
            {
                var jsonObj=JSON.parse(responseText);
                var temperature=Math.round(jsonObj.main.temp -273.15);															//convert the temp into celcius
                console.log("Temperature is  "+temperature);
                var condition=jsonObj.weather[0].main;															//read the condition
                console.log("Condition is "+condition);
                    // Assemble dictionary using our keys
                var dictionary = 
                {
                  "KEY_TEMPERATURE": temperature,
                  "KEY_CONDITIONS": condition
                };
           
                // Send to Pebble
                Pebble.sendAppMessage(dictionary,															//send the dictionary to the pebble
                  function(e) 
                  {
                    console.log("Weather info sent to Pebble successfully!");
                  },
                  function(e) 
                  {
                    console.log("Error sending weather info to Pebble!");
                  }
                );
            }
            );
}

function locationError(error)															//function to be called when the location is not obtained
{
    console.log('error in getting position');
}

function getWeather()															//function to get the weather
{
    navigator.geolocation.getCurrentPosition
    (locationSuccess,
     locationError,
     {timeout:15000,maximumAge:60000}
    );
}

Pebble.addEventListener															//add an event listener that is called when the pebble is ready
("ready",
  function(e)
  {
      console.log('pebble app has started.....');
      getWeather();															//call the function to get the wweather
  }
);

Pebble.addEventListener															//add an event listener that is called when the pebble sends an app message
("appmessage",
function(e) {
    console.log('message received...');
    getWeather();															//call the function to get the weather
  }
);
