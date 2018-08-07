<?php
//setting header to json
header('Content-Type: application/json');

require 'db.php';

$table = "DataTable";
if(isset($_POST['arguments'])) {
$table =  $_POST['arguments'][0];
}


if(!$mysqli){
	$table = "DataTable";
	
}

$lines = 1; //how many lines from the database we should request

//request the last $lines rows from the database
$query = "SELECT * FROM " . $table . " ORDER BY RTCDataTime DESC LIMIT 1;";
$result = $mysqli->query($query);

//we only have one row so just fetch the associative array for that row
echo json_encode(mysqli_fetch_assoc($result));


$result->close();
$mysqli->close();


?>