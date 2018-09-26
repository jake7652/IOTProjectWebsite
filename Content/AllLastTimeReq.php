<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Requests the last entry of the RTCDataTime field from the database defined in db.php for all tables in one mysql query.
END DEVELOPMENT HEADER 
*/
//setting header to json
header('Content-Type: application/json');

require 'db.php';
//database settings 

$lines = 1; //how many lines from the database we should request

//request the last $lines rows from the database
//$query = sprintf("show tables");
$query = sprintf("SELECT TABLE_NAME,TABLE_ROWS FROM information_schema.tables WHERE table_schema='" . $contents[3] . "';");
//$query = sprintf("SELECT * FROM ( SELECT * FROM " . $table . " ORDER BY RTCDataTime DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$result = $mysqli->query($query);


//var_dump($result);
//exit(0);
$tables = Array();
$index = 0;
$lastTimes = Array();
foreach($result as $row) {
if($row["TABLE_ROWS"] > 0) {
$tables[$index] = $row["TABLE_NAME"];
$index ++;
}
}
//var_dump($tables);
//exit(0);
$result->close();

$query = "SELECT ";
for($i = 0; $i < (sizeof($tables)-1); $i++) {
$query = $query . "(SELECT RTCDataTime FROM " . $tables[$i] . " ORDER BY RTCDataTime DESC LIMIT 1) AS " . $tables[$i] . ",";


}

$query = $query . "(SELECT RTCDataTime FROM " . $tables[$i] . " ORDER BY RTCDataTime DESC LIMIT 1) AS " . $tables[$i] . ";";


$result = $mysqli->query($query);
echo json_encode($query);
//echo json_encode(mysqli_fetch_assoc($result));










$result->close();
$mysqli->close();


?>
