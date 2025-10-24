var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

const WEATHER_TYPES = Object.freeze({
    clear: 0,
    partly_cloudy: 1,
    cloudy: 2,
    rain: 3,
    snow: 4,
    storm: 5
})

const WEATHER_CODES = Object.freeze({
    0: WEATHER_TYPES.clear,        // 0	Clear sky
    1: WEATHER_TYPES.clear,        // 1, 2, 3	Mainly clear, partly cloudy, and overcast
    2: WEATHER_TYPES.partly_cloudy,// 1, 2, 3	Mainly clear, partly cloudy, and overcast
    3: WEATHER_TYPES.cloudy,       // 1, 2, 3	Mainly clear, partly cloudy, and overcast
    45: WEATHER_TYPES.cloudy,      // 45, 48	Fog and depositing rime fog
    48: WEATHER_TYPES.cloudy,      // 45, 48	Fog and depositing rime fog
    51: WEATHER_TYPES.rain,        // 51, 53, 55	Drizzle: Light, moderate, and dense intensity
    53: WEATHER_TYPES.rain,        // 51, 53, 55	Drizzle: Light, moderate, and dense intensity
    55: WEATHER_TYPES.rain,        // 51, 53, 55	Drizzle: Light, moderate, and dense intensity
    56: WEATHER_TYPES.snow,        // 56, 57	Freezing Drizzle: Light and dense intensity
    57: WEATHER_TYPES.snow,        // 56, 57	Freezing Drizzle: Light and dense intensity
    61: WEATHER_TYPES.rain,        // 61, 63, 65	Rain: Slight, moderate and heavy intensity
    63: WEATHER_TYPES.rain,        // 61, 63, 65	Rain: Slight, moderate and heavy intensity
    65: WEATHER_TYPES.rain,        // 61, 63, 65	Rain: Slight, moderate and heavy intensity
    66: WEATHER_TYPES.snow,        // 66, 67	Freezing Rain: Light and heavy intensity
    67: WEATHER_TYPES.snow,        // 66, 67	Freezing Rain: Light and heavy intensity
    71: WEATHER_TYPES.snow,        // 71, 73, 75	Snow fall: Slight, moderate, and heavy intensity
    73: WEATHER_TYPES.snow,        // 71, 73, 75	Snow fall: Slight, moderate, and heavy intensity
    75: WEATHER_TYPES.snow,        // 71, 73, 75	Snow fall: Slight, moderate, and heavy intensity
    77: WEATHER_TYPES.snow,        // 77	Snow grains
    80: WEATHER_TYPES.rain,        // 80, 81, 82	Rain showers: Slight, moderate, and violent
    81: WEATHER_TYPES.rain,        // 80, 81, 82	Rain showers: Slight, moderate, and violent
    82: WEATHER_TYPES.rain,        // 80, 81, 82	Rain showers: Slight, moderate, and violent
    85: WEATHER_TYPES.snow,        // 85, 86	Snow showers slight and heavy
    86: WEATHER_TYPES.snow,        // 85, 86	Snow showers slight and heavy
    95: WEATHER_TYPES.storm,       // 95 *	Thunderstorm: Slight or moderate
    96: WEATHER_TYPES.storm,       // 96, 99 *	Thunderstorm with slight and heavy hail
    99: WEATHER_TYPES.storm,       // 96, 99 *	Thunderstorm with slight and heavy hail
});

var use_current_location = false; // use GPS
var use_metric = false; // use celsius or fahrenheit
var lat = '42.36'; // latitude for weather w/o GPS
var lon = '-71.1'; // longitude for weather w/o GPS

// request data from url
var xhrRequest = function (url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(this.responseText);
	};
	xhr.open(type, url);
	xhr.send();
};

function fetchWeather(pos) {
	// Weather
    // https://api.open-meteo.com/v1/forecast?latitude=39.765&longitude=-83.75&current=temperature_2m,weather_code
    var url = 'https://api.open-meteo.com/v1/forecast?' +
      'latitude=' + ((pos != null) ? pos.coords.latitude : lat) +
      '&longitude=' + ((pos != null) ? pos.coords.longitude : lon) +
      '&current=temperature_2m,weather_code' + 
      '&temperature_unit=' + (use_metric ? 'celsius' : 'fahrenheit');

    console.log("Requesting " + url);
	xhrRequest(url, 'GET',
		function(responseText) {
			// responseText contains a JSON object with weather info
			var json = JSON.parse(responseText);

			var temperature = Math.round(json.current.temperature_2m);
            var weather_code = json.current.weather_code;
            var weather_type = WEATHER_CODES[weather_code];
            console.log("Conditions: " + weather_type + " Temperature: " + temperature);

			var dictionary = {
				'TEMPERATURE': temperature,
				'CONDITIONS': weather_type
			};
			Pebble.sendAppMessage(dictionary,
				function(e) {
					console.log('Weather info sent to Pebble successfully!');
				},
				function(e) {
					console.log('Error sending weather info to Pebble!');
				}
			);
		}
	);
}

function fetchMoon(pos) {
	// Moon
    // https://aa.usno.navy.mil/api/rstt/oneday?date=2025-10-20&coords=42.36,-71.09&ID=witching
    var today = new Date();
    var dd = today.getDate();
    var mm = today.getMonth() + 1; //January is 0!
    var yyyy = today.getFullYear();
    var url = 'https://aa.usno.navy.mil/api/rstt/oneday?' +
      'date=' + yyyy + '-' + mm + '-' + dd +
      '&coords=' + ((pos != null) ? pos.coords.latitude : lat) + ',' + ((pos != null) ? pos.coords.longitude : lon) +
      '&id=witching';

    console.log("Requesting " + url);
	xhrRequest(url, 'GET',
		function(responseText) {
			// responseText contains a JSON object with moon
			var json = JSON.parse(responseText);
            var fracillum_str = json.properties.data.fracillum.slice(0, -1);
            var fracillum_pct = Number(fracillum_str);
            var waning = json.properties.data.curphase.indexOf("Waning") !== -1;
			// var temperature = Math.round(json.current.temperature_2m);
            // console.log("Conditions: " + weather_type + " Temperature: " + temperature);

			var dictionary = {
                'MOON_FRACILLUM': fracillum_pct,
                'MOON_WANING': waning ? 1 : 0
			};
			Pebble.sendAppMessage(dictionary,
				function(e) {
					console.log('Moon info sent to Pebble successfully!');
				},
				function(e) {
					console.log('Error sending moon info to Pebble!');
				}
			);
		}
	);
}

function locationSuccess(pos) {
    fetchWeather(pos);
    fetchMoon(pos);
}

function locationError(err) {
	console.log('Error requesting location, retrying without geolocation');
    getWeather(true);
}

function getWeather(force_no_geo) {
	if (use_current_location == true && !force_no_geo) { // if using current location
		navigator.geolocation.getCurrentPosition(
			locationSuccess,
			locationError,
			{timeout: 15000, maximumAge: 60000}
		);
	} else {
		locationSuccess(null); // otherwise, just grab using lat and lon
	}
}

// send nothing to pebble, which will prompt weather request
function pokePebble() {
	var dictionary = {};
	Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log('Pebble poked successfully!');
		},
		function(e) {
			console.log('Error poking Pebble!');
		}
	);
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
	function(e) {
		console.log('PebbleKit JS ready!');

		// Get the initial weather
		pokePebble();
	}
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
	function(e) {
		console.log('AppMessage received!');
		var dict = e.payload;

		if ('TemperatureMetric' in dict) // use celsius/fahrenheit
			metric = dict['TemperatureMetric'] == 1;
		if ('UseCurrentLocation' in dict) // use current location
			use_current_location = dict['UseCurrentLocation'] == 1;
		if ('Latitude' in dict) // latitude if not using current location
			lat = dict['Latitude'];
		if ('Longitude' in dict) // longitude if not using current location
			lon = dict['Longitude'];

		getWeather(false);
	}
);
