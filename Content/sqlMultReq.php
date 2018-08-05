<?php
//setting header to json
header('Content-Type: application/json');

//database settings 
$contents = Array();
$handle = fopen("/var/www/databaseSettings", "r");
$lineNum = 0;
if ($handle) {
    while (($line = fgets($handle)) !== false) {
	if($line[strlen($line)-1] == "\n") {
        $contents[$lineNum] = substr($line,0,strlen( $line )-1);
	} else {
	$contents[$lineNum] = $line; 
	}
     $lineNum++;
    }

    fclose($handle);
} else {
    // error opening the file.
} 


//database
define('DB_HOST', $contents[0]);
define('DB_USERNAME', $contents[1]);
define('DB_PASSWORD', $contents[2]);
define('DB_NAME', $contents[3]);


$fields = array("RTCTemperature");
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

//get connection
$mysqli = @mysqli_connect(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);



if(!$mysqli){
	$table = "DataTable";
	$mysqli = @mysqli_connect("localhost", DB_USERNAME, DB_PASSWORD, DB_NAME);
}

$query = array();
$num_fields = sizeof($fields);
$result = array();
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

for($i = 0; $i < $num_fields;$i++) {
$query[$i] = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, " . $fields[$i] . " FROM ( SELECT @row :=0) r, " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . " ) ranked WHERE rownum % " . $interval . " = 1";
//echo json_encode($query[$i]);
$result[$i] = $mysqli->query($query[$i]);
//echo json_encode(mysqli_num_rows($result[$i]));
}
$time_query = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, RTCDataTime FROM ( SELECT @row :=0) r, " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . ") ranked WHERE rownum % " . $interval . " = 1";

} else {
for($i = 0; $i < $num_fields;$i++) {
$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM " . $table . "");
$result[$i] = $mysqli->query($query[$i]);
}
$time_query = sprintf("SELECT RTCDataTime FROM " . $table . "");

}

} else {
//query to get data from the table
for($i = 0; $i < $num_fields; $i++) {
$query[$i] = sprintf("SELECT " . $fields[$i] . " FROM ( SELECT * FROM " . $table . " ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$result[$i] = $mysqli->query($query[$i]);
}
$time_query = sprintf("SELECT RTCDataTime FROM ( SELECT * FROM " . $table . " ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
}

//echo json_encode(mysqli_num_rows($result[0]));
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