<?php
//SELECT table_name FROM information_schema.tables where table_schema='TideGuage';


require 'db.php';

$query = "SELECT TABLE_NAME,TABLE_ROWS FROM information_schema.tables WHERE table_schema='" . $contents[3] . "';";
$result = $mysqli->query($query);
$tableNames = Array();
foreach ($result as $row) {
if($row["TABLE_ROWS"] > 0) {
$tableNames[] = $row["TABLE_NAME"];
}
}

echo json_encode($tableNames);
$result->close();
$mysqli->close();








?>