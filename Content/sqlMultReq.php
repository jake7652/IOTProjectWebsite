<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Requests any number of sensor fields from the database(first argument)
* for any number of rows of the database(second argument) 
* for a specific timerange(third and fourth argument)
* for a specific table in the database(5th argument).
* Only requests every other rows when the database has more rows in the timerange than a set value.
END DEVELOPMENT HEADER 
*/
require 'db.php';

//setting header to json
header('Content-Type: application/json');

$fields = array("RTCTemperature","PressurePressure");
$lines = 0;
$table = "DataTable2";
$min_time = "0";
$max_time = "99999999999999";
if(isset($_POST['arguments'])) {
$fields = $_POST['arguments'][0]; 
$lines = $_POST['arguments'][1];
$min_time = $_POST['arguments'][2];
$max_time = $_POST['arguments'][3];
$table = $_POST['arguments'][4];
}

if($min_time == null) {

$min_time = "0";
$max_time = "99999999999999";
}


if(!$mysqli){
	$table = "DataTable";
}

$query = array();
$num_fields = sizeof($fields);
$result = array();
$startIndex = 0;
//$query[0] = sprintf();
//$mysqli->query("SELECT * FROM ( SELECT @row := @row +1 AS rownum, RTCDataTime FROM ( SELECT @row :=0) r, " . $table . " ) ranked WHERE rownum % 2 = 1");
//print($num_lines);
//print($num_lines);
//print("\n");
if($lines == 0) {
$lines_query = $mysqli->query("SELECT COUNT(*) FROM " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . "");
$num_lines = mysqli_fetch_array($lines_query)[0];
$lines_query->close();
$line_limit = 3600;
$interval = ceil($num_lines/$line_limit);
if($num_lines > $line_limit) {

$query[0] = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, RTCDataTime";
for($i = 0; $i < $num_fields;$i++) {
$query[0] = $query[0] . "," . $fields[$i];

}
$query[0] = $query[0] . " FROM ( SELECT @row :=0) r, " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . " ) ranked WHERE rownum % " . $interval . " = 1";
//echo json_encode($query[0]);
$result[0] = $mysqli->query($query[0]);
//echo json_encode(mysqli_fetch_all($result[0]));
//echo json_encode(mysqli_fetch_array($result[0]));
//die();
$startIndex = 1;
} else {
$query[0] = "SELECT RTCDataTime";
for($i = 0; $i < $num_fields;$i++) {
//$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM " . $table . "");
$query[0] = $query[0] . "," . $fields[$i];
}
$query[0] = $query[0] . " FROM " . $table . ";";
$result[0] = $mysqli->query($query[0]);
}

} else {

$query[0] = "SELECT RTCDataTime"; 
//query to get data from the table
for($i = 0; $i < $num_fields; $i++) {
$query[0] = $query[0] . "," . $fields[$i];
//$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM ( SELECT * FROM " . $table . " ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");

}
$query[0] = $query[0] . " FROM ( SELECT * FROM " . $table . " ORDER BY RTCDataTime DESC LIMIT " . $lines . ") sub ORDER BY RTCDataTime ASC;";
$result[0] = $mysqli->query($query[0]);
//$time_query = sprintf("SELECT RTCDataTime FROM ( SELECT * FROM " . $table . " ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
}

//echo json_encode(mysqli_num_rows($result[0]));
$increment = 1;
//if(mysql_num_rows($result[0]) > 1000) {
//$increment = round(mysql_num_rows($result[0])/1000); 
//}

//execute query


$resArr = mysqli_fetch_all($result[0]);
//loop through the returned data
$time_data = array();
$data = array();
//the first index in the resArr is the rownum, so we don't have to look at it 
for($i = $startIndex; $i < sizeof($resArr[0]); $i++) {
	if($i != $startIndex) {
		$data[$i-$startIndex-1] = array();
	}
	for($i2 = 0; $i2 < sizeof($resArr); $i2++){
		if($i != $startIndex) {
			$data[$i-$startIndex-1][$i2] = $resArr[$i2][$i];
		} else {
			$time_data[$i2] = $resArr[$i2][$i];
		}
	
	}
	
}



//echo json_encode($data);
//echo json_encode($time_data);
$ret = array();

$ret[data] = $data;
$ret[time] = $time_data;
//free memory associated with result

$result[0]->close();

//close connection
$mysqli->close();

//now print the data
echo json_encode($ret);
?>
