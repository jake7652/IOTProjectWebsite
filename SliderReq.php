<?php
//setting header to json
header('Content-Type: application/json');

//database
define('DB_HOST', '127.0.0.1');
define('DB_USERNAME', 'Tide');
define('DB_PASSWORD', 'MasterWater2025');
define('DB_NAME', 'TideGuage');

$time = '20180303030303';
if(isset($_POST['arguments'])) {
$time = $_POST['arguments'][0]; 
}

//get connection
$mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

if(!$mysqli){
	die("Connection failed: " . $mysqli->error);
}


//request first row from the table that matches or is greater than the time argument 
$query = sprintf("Select * FROM DataTable WHERE RTCDataTime >= " . $time . " LIMIT 1;");
$result = $mysqli->query($query);

//get the associative array for the result 
$res_arr = mysqli_fetch_assoc($result);
//if the result is empty then we just request the last row of the table
if($res_arr == null) {
$result->close();
$query = sprintf("SELECT * FROM ( SELECT * FROM DataTable ORDER BY RTCDataTime DESC LIMIT 1 ) sub ORDER BY TransmissionKey ASC;");
$result = $mysqli->query($query);
//we only have one row so just fetch the associative array for that row
echo json_encode(mysqli_fetch_assoc($result));
$mysqli->close();
exit(0);
} 

//we only have one row so just fetch the associative array for that row
echo json_encode($res_arr);
$result->close();
$mysqli->close()






?>