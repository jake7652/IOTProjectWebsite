<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Requests the row from the database for a specific table(second argument)
* that has a time value closest to the value passed in(first argument).
END DEVELOPMENT HEADER 
*/
//setting header to json
header('Content-Type: application/json');

require 'db.php';

$time = '20180303030303';
$table = "DataTable";
if(isset($_POST['arguments'])) {
$time = $_POST['arguments'][0];
$table =  $_POST['arguments'][1];
}

if(!$mysqli){
	$table = "DataTable";
}


//request first row from the table that matches or is greater than the time argument 
$query = sprintf("Select * FROM " . $table . " WHERE RTCDataTime >= " . $time . " LIMIT 1;");
$result = $mysqli->query($query);

//get the associative array for the result 
$res_arr = mysqli_fetch_assoc($result);
//if the result is empty then we just request the last row of the table
if($res_arr == null) {
$result->close();
$query = sprintf("SELECT * FROM ( SELECT * FROM " . $table . " ORDER BY RTCDataTime DESC LIMIT 1 ) sub ORDER BY TransmissionKey ASC;");
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
