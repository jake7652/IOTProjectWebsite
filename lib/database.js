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
		"SoftwareVersion",
		"HardwareVersion",
		"RTCDataTime"
	],
}
