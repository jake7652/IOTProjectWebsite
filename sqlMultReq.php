<?php
//setting header to json
header('Content-Type: application/json');

//database
define('DB_HOST', '127.0.0.1');
define('DB_USERNAME', 'Tide');
define('DB_PASSWORD', 'MasterWater2025');
define('DB_NAME', 'TideGuage');

$fields = array("RTCTemperature");
$lines = 0;

if(isset($_POST['arguments'])) {
$fields = $_POST['arguments'][0]; 
$lines = $_POST['arguments'][1]; 
}

//get connection
$mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

if(!$mysqli){
	die("Connection failed: " . $mysqli->error);
}

$query = array();
$num_fields = sizeof($fields);
$result = array();
//$query[0] = sprintf();
//$mysqli->query("SELECT * FROM ( SELECT @row := @row +1 AS rownum, RTCDataTime FROM ( SELECT @row :=0) r, DataTable ) ranked WHERE rownum % 2 = 1");
//print($num_lines);
//print($num_lines);
//print("\n");
if($lines == 0) {
$lines_query = $mysqli->query("SELECT COUNT(*) FROM DataTable");
$num_lines = mysqli_fetch_array($lines_query)[0];
$lines_query->close();
$line_limit = 10000;
$interval = round($num_lines/$line_limit);
if($num_lines > $line_limit) {

for($i = 0; $i < $num_fields;$i++) {
$query[$i] = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, " . $fields[$i] . " FROM ( SELECT @row :=0) r, DataTable ) ranked WHERE rownum % " . $interval . " = 1";
$result[$i] = $mysqli->query($query[$i]);
}
$time_query = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, RTCDataTime FROM ( SELECT @row :=0) r, DataTable ) ranked WHERE rownum % " . $interval . " = 1";

} else {
for($i = 0; $i < $num_fields;$i++) {
$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM DataTable");
$result[$i] = $mysqli->query($query[$i]);
}
$time_query = sprintf("SELECT RTCDataTime FROM DataTable");

}

} else {
//query to get data from the table
for($i = 0; $i < $num_fields; $i++) {
$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM ( SELECT * FROM DataTable ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$result[$i] = $mysqli->query($query[$i]);
}
$time_query = sprintf("SELECT RTCDataTime FROM ( SELECT * FROM DataTable ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
}

$increment = 1;
//if(mysql_num_rows($result[0]) > 1000) {
//$increment = round(mysql_num_rows($result[0])/1000); 
//}

//execute query

$time_result = $mysqli->query($time_query);
//loop through the returned data
$data = array();

for($i = 0; $i < $num_fields; $i++) {
foreach ($result[$i] as $row) {
foreach($row as $entry) {
$data[$i][] = $entry;
//print("\n");
}
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
for($i=0; $i < $num_fields; $i++) {

$result[$i]->close();

}
$time_result->close();
//close connection
$mysqli->close();

//now print the data
echo json_encode($ret);
?>
