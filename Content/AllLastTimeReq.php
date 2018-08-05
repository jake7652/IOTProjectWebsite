<?php
//setting header to json
header('Content-Type: application/json');

//database settings 
$contents = Array();
$handle = fopen("/var/www/databaseSettings", "r");
$lineNum = 0;
if ($handle) {
    while (($line = fgets($handle)) !== false) {
	if($line[strlen($line)-1] == "\n") {
        $contents[$lineNum] = substr($line,0,strlen( $line )-1);
	} else {
	$contents[$lineNum] = $line; 
	}
     $lineNum++;
    }

    fclose($handle);
} else {
    // error opening the file.
} 

//database
define('DB_HOST', $contents[0]);
define('DB_USERNAME', $contents[1]);
define('DB_PASSWORD', $contents[2]);
define('DB_NAME', $contents[3]);

//get connection
$mysqli = @mysqli_connect(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

//$lines = 1; //how many lines from the database we should request

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
//echo json_encode($tables[$index]);
$lastTimes[$index] = $mysqli->query("SELECT RTCDataTime FROM ( SELECT RTCDataTime FROM " . $tables[$index] . " ORDER BY TransmissionKey DESC LIMIT 1) sub ORDER BY RTCDataTime ASC;");
$index++;
}
}
$times = Array();
for($i = 0; $i < sizeof($tables); $i++) {
foreach($lastTimes[$i] as $row) {
foreach($row as $field) {
$times[$tables[$i]] = $field;
//$times[$i]=$field;
}
}
}

for($i = 0; $i < sizeof($lastTimes); $i++){
$lastTimes[$i]->close();
}

echo json_encode($times);



$result->close();
$mysqli->close();


?>