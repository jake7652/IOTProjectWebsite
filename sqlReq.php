<?php
//setting header to json
header('Content-Type: application/json');

//database
define('DB_HOST', '127.0.0.1');
define('DB_USERNAME', 'Tide');
define('DB_PASSWORD', 'MasterWater2025');
define('DB_NAME', 'TideGuage');

$field = "PressureTemperature";
$lines = 10;

if(isset($_POST['arguments'])) {
$field = $_POST['arguments'][0]; 
$lines = $_POST['arguments'][1]; 
}

//get connection
$mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

if(!$mysqli){
	die("Connection failed: " . $mysqli->error);
}

if($lines == 0) {
$query = sprintf("SELECT " . $field . " FROM DataTable");
$time_query = sprintf("SELECT RTCDataTime FROM DataTable");
} else {
//query to get data from the table
$query = sprintf("SELECT " . $field . " FROM ( SELECT * FROM DataTable ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$time_query = sprintf("SELECT RTCDataTime FROM ( SELECT * FROM DataTable ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
}
//execute query
$result = $mysqli->query($query);
$time_result = $mysqli->query($time_query);
//loop through the returned data
$data = array();
foreach ($result as $row) {
foreach($row as $entry) {
$data[] = $entry;
}
}

//loop through the returned data
$time_data = array();
foreach ($time_result as $row) {
foreach($row as $entry) {
$time_data[] = $entry;
}
}


$ret = array();

$ret[data] = $data;
$ret[time] = $time_data;
//free memory associated with result
$result->close();
$time_result->close();
//close connection
$mysqli->close();

//now print the data
echo json_encode($ret);
?>