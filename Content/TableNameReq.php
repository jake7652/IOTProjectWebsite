<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Requests all table names and the rows number in each table
* Then returns all non empty tables.
END DEVELOPMENT HEADER 
*/
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
