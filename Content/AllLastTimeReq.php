<?php
//setting header to json
header('Content-Type: application/json');

require 'db.php';
//database settings 

$lines = 1; //how many lines from the database we should request

//request the last $lines rows from the database
$query = sprintf("show tables");
//$query = sprintf("SELECT * FROM ( SELECT * FROM " . $table . " ORDER BY RTCDataTime DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$result = $mysqli->query($query);




$tables = Array();
$index = 0;
$lastTimes = Array();
foreach($result as $row) {
foreach($row as $field) {
$tables[$index] = $field;
$index ++;
}
}

$result->close();

$query = "SELECT ";
for($i = 0; $i < (sizeof($tables)-1); $i++) {
$query = $query . "(SELECT RTCDataTime FROM " . $tables[$i] . " ORDER BY RTCDataTime DESC LIMIT 1) AS " . $tables[$i] . ",";


}

$query = $query . "(SELECT RTCDataTime FROM " . $tables[$i] . " ORDER BY RTCDataTime DESC LIMIT 1) AS " . $tables[$i] . ";";

$result = $mysqli->query($query);

echo json_encode(mysqli_fetch_assoc($result));










$result->close();
$mysqli->close();


?>