<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Gets the name of all the fields in a table defined by the first argument from the database.
END DEVELOPMENT HEADER 
*/
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


//request the last $lines rows from the database
$query = sprintf("DESCRIBE " . $table . ";");
$result = $mysqli->query($query);

$fields = Array();
foreach($result as $row) {
$fields[] = $row['Field'];

}

//we only have one row so just fetch the associative array for that row
echo json_encode($fields);


$result->close();
$mysqli->close();


?>
