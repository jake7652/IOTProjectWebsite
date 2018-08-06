<?php
//SELECT table_name FROM information_schema.tables where table_schema='TideGuage';


require 'db.php';

$query = "SELECT table_name FROM information_schema.tables WHERE table_schema='" . $contents[3] . "';";
$result = $mysqli->query($query);
$tableNames = Array();
foreach ($result as $row) {
$tableNames[] = $row["table_name"];
}

echo json_encode($tableNames);
$result->close();
$mysqli->close();








?>