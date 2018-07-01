<?php
//setting header to json
header('Content-Type: application/json');

//database
define('DB_HOST', '127.0.0.1');
define('DB_USERNAME', 'Tide');
define('DB_PASSWORD', 'MasterWater2025');
define('DB_NAME', 'TideGuage');

//default values for $fields and $lines when no arguments are passed in. Mainly used for testing 
$fields = array("RTCTemperature");
$lines = 0;

//if we have some arguments then set the $fields and $lines variables to whatever these are instead of their default values
if(isset($_POST['arguments'])) {
$fields = $_POST['arguments'][0]; 
$lines = $_POST['arguments'][1]; 
}

//get connection
$mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

//if we cannot connect to the database then kill the script and print out the error
if(!$mysqli){
	die("Connection failed: " . $mysqli->error);
}

//we are probably doing multiple queries so it has to be a array
$query = array();
//used for loops over the fields
$num_fields = sizeof($fields);
//array of the results of the queries
$result = array();
//if the number of lines we are getting is 0 then we fetch from the total range of the database 
//with a upper limiter to prevent the dataset from getting too large
if($lines == 0) {
//get number of lines to use for the limiter 
$lines_query = $mysqli->query("SELECT COUNT(*) FROM DataTable");
$num_lines = mysqli_fetch_array($lines_query)[0];
$lines_query->close();
//max number of lines to get from each field 
$line_limit = 10000;
//interval determines the gap between each row we get so if there was 50000 lines we would interval would be 5
//meaning we would only get every 5th line in the database
$interval = round($num_lines/$line_limit);
//if we have more than line_limit lines we have to use a more complex query 
if($num_lines > $line_limit) {
for($i = 0; $i < $num_fields;$i++) {
//query that gets every interval lines from a field in fields
$query[$i] = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, " . $fields[$i] . " FROM ( SELECT @row :=0) r, DataTable ) ranked WHERE rownum % " . $interval . " = 1";
//get the result of the query
$result[$i] = $mysqli->query($query[$i]);
}
//get every interval lines from the time field
$time_query = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, RTCDataTime FROM ( SELECT @row :=0) r, DataTable ) ranked WHERE rownum % " . $interval . " = 1";

} else { //else if $lines != 0
//if we have less than line_limit lines then we just get all lines from each field and all lines from the time field
for($i = 0; $i < $num_fields;$i++) {
$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM DataTable");
$result[$i] = $mysqli->query($query[$i]);
}
$time_query = sprintf("SELECT RTCDataTime FROM DataTable");

}

} else {
//If we requested a specific number of lines we use a different query and no line limit 
for($i = 0; $i < $num_fields; $i++) {
//get the last $lines lines from the database for each field  
$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM ( SELECT * FROM DataTable ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$result[$i] = $mysqli->query($query[$i]);
}
//get the last $lines lines from the database for the time field
$time_query = sprintf("SELECT RTCDataTime FROM ( SELECT * FROM DataTable ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
}


//actually do the time query outside of the previous if else statement
$time_result = $mysqli->query($time_query);
//loop through the returned data
$data = array();

//parse all the results of the field queries into a array of arrays where each index is all of the results from one field
for($i = 0; $i < $num_fields; $i++) {
foreach ($result[$i] as $row) {
foreach($row as $entry) {
$data[$i][] = $entry;
}
}
}

//parse the time results into a array where each index is one line from the database
$time_data = array();
foreach ($time_result as $row) {
foreach($row as $entry) {
$time_data[] = $entry;

}
}


//return array containing the data results and the time result
$ret = array();

//make the data index = data and the time index = time
$ret[data] = $data;
$ret[time] = $time_data;
//free memory associated with the data results 
for($i=0; $i < $num_fields; $i++) {

$result[$i]->close();

}
//free data associated with the time results
$time_result->close();

//close connection
$mysqli->close();

//now print/return the data
echo json_encode($ret);
?>
