<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Gets the first entry of the RTCDataTime field for the table defined in the first argument.
END DEVELOPMENT HEADER 
*/
//setting header to json
header('Content-Type: application/json');

require 'db.php';


$lines = 1; //how many lines from the database we should request

$table = "DataTable";
if(isset($_POST['arguments'])) {
$table =  $_POST['arguments'][0];
}

//request the last $lines rows from the database
$query = sprintf("SELECT RTCDataTime FROM " . $table . " LIMIT " . $lines . "");
$result = $mysqli->query($query);

//we only have one row so just fetch the associative array for that row
echo json_encode(mysqli_fetch_assoc($result));


$result->close();
$mysqli->close();


?>
