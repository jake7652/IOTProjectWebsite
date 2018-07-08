<?php
//setting header to json
header('Content-Type: application/json');

//database
define('DB_HOST', '127.0.0.1');
define('DB_USERNAME', 'Tide');
define('DB_PASSWORD', 'MasterWater2025');
define('DB_NAME', 'TideGuage');


//get connection
$mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

if(!$mysqli){
	die("Connection failed: " . $mysqli->error);
}

$lines = 1; //how many lines from the database we should request

//request the last $lines rows from the database
$query = sprintf("SELECT RTCDataTime FROM DataTable LIMIT " . $lines . "");
$result = $mysqli->query($query);

//we only have one row so just fetch the associative array for that row
echo json_encode(mysqli_fetch_assoc($result));


$result->close();



?>
