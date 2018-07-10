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

//pdo_mysql persistent connections
//$instance_pdo = new PDO('mysql:host=' . DB_HOST . ';dbname=' . DB_NAME . '', $DB_USERNAME, $DB_PASSWORD, [
//    PDO::ATTR_PERSISTENT => true
//]);
$lines = 1; //how many lines from the database we should request

//request the last $lines rows from the database
$query = sprintf("SELECT * FROM ( SELECT * FROM DataTable ORDER BY TransmissionKey DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$result = $mysqli->query($query);

//we only have one row so just fetch the associative array for that row
echo json_encode(mysqli_fetch_assoc($result));

$mysqli->close();
$result->close();



?>
