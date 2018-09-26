//A library mostly used for holding information about the fields in the databases
var database = {
	//min and max value for each sensor field 
	MIN_MAX: { 
		RTCTemperature: [0,120],
		PressureTemperature: [0,120],
		Humidity: [0,100],
		HumidityHeatIndex: [0,120],
		HumidityTemperature: [0,120],
		Transducer: [0,4000],
		PressurePressure: [26,33], // conversion of the 300-1100 hPa range that the datasheet lists for the sensor
		PressureAlititude: [-500, 2000], //can go up to 9000 but i don't think we'll be going that high
		Temperature1: [0,120],
		Temperature2: [0,120],
	},
	//display labels associated with each field
	LABELS: { 
		RTCTemperature: "Air Temperature",
		HumidityTemperature: "Temperature",
		PressureTemperature: "Air Temperature 2",
		Humidity: "Humidity",
		HumidityHeatIndex: "Heat Index",
		Transducer: "Distance",
		PressurePressure: "Pressure",
		PressureAlititude: "Altitude", 
		Temperature1: "Water Temperature",
		Temperature2: "Water Temperature 2",
		RTCDataTime: "Realtime Clock",

	},
	//units associated with each field
	UNITS: { 
		RTCTemperature: "F",
		HumidityTemperature: "F",
		PressureTemperature: "F",
		Humidity: "% Saturation",
		HumidityHeatIndex: "F",
		Transducer: "mm",
		PressurePressure: "Hg/mm",
		PressureAlititude: "ft", 
		Temperature1: "F",
		Temperature2: "F",
		RTCDataTime: "Time",
	},
	//fields to ignore displaying/displaying options for
	IGNORED_FIELDS: [
		"TransmissionKey", //transmission key is not needed since it's just the transmission time mostly
		"SystemID",//system ID is not needed since we usually display the table in SQL as the name of the platform
		"SoftwareVersion", // Don't give option for software version to the user since this does not change over time 
		"HardwareVersion", // Don't give option for hardware version since this does not change over time
		"RTCDataTime" //Don't give option for RTCDataTime since this is usually needed for most queries anyways
	],
}

//a user activity monitor thing
function userActive() {
	//just run the usrStat script which will update the last active time 
	$.ajax({ url: "../Account/usrStat.php",
		async: true,
		type: "GET",
		dataType: 'json',
		data: {arguments: []}, //no arguments needed for this
		success: function(output) {
			
		}
	});
}

//The following code is for user activity monitoring
var wasActive = false; //variable to check if user was active during the last second
var lastActive = Math.ceil((new Date()).getTime() / 1000); //unix time in seconds
var timeout = 121; //timeout in seconds. I just make this one second higher than the php timeout because of rounding
var userWarned = false; //variable used to prevent displaying multiple warnings
var activityLoop = setInterval(timerIncrement, 31000); // Check user active every 31 seconds ;

function startActivityMonitor() {
	clearInterval(activityLoop);
	lastActive = Math.ceil((new Date()).getTime() / 1000); //unix time in seconds
	activityLoop = setInterval(timerIncrement, 31000); // Check user active every 31 seconds 

}

function stopActivityMonitor() {
	clearInterval(activityLoop);

}

$(document).ready(function () {

    //Set was active to 1 whenever there was mouse activity during the past second
    $(this).mousemove(function (e) {
        wasActive = true;
    });
    $(this).keypress(function (e) {
        wasActive = true;
    });
});

function timerIncrement() {
		if(wasActive) {
			lastActive = lastActive + 31;
			userActive(); // just call the activity thing anyways since the user will eventually be logged out when their timeout is up anyways 
			userWarned = false;
		} else if(Math.floor((new Date()).getTime() / 1000) - lastActive > timeout) {
			setTimeout("alert('You have been inactive too long and will be taken back to the login page');", 1);		
			window.location.href = '/Account/logout.php';
		} else if(Math.floor((new Date()).getTime() / 1000) - lastActive > timeout -60 && userWarned == false) {
			userWarned = true;	
			setTimeout("alert('You will be taken back to the login page if you continue to be inactive');", 1);
		}
	wasActive = false;
}




