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

$table = "DataTable";
if(isset($_POST['arguments'])) {
$table =  $_POST['arguments'][0];
}


//get connection
$mysqli = @mysqli_connect(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

if(!$mysqli){
	$table = "DataTable";
	$mysqli = @mysqli_connect("localhost", DB_USERNAME, DB_PASSWORD, DB_NAME);
}

$lines = 1; //how many lines from the database we should request

//request the last $lines rows from the database
$query = sprintf("SELECT * FROM ( SELECT * FROM " . $table . " ORDER BY RTCDataTime DESC LIMIT " . $lines . ") sub ORDER BY TransmissionKey ASC;");
$result = $mysqli->query($query);

//we only have one row so just fetch the associative array for that row
echo json_encode(mysqli_fetch_assoc($result));


$result->close();
$mysqli->close();


?>
