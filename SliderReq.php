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


$time = '20180303030303';
$table = "DataTable";
if(isset($_POST['arguments'])) {
$time = $_POST['arguments'][0];
$table =  $_POST['arguments'][1];
}

//get connection
$mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

if(!$mysqli){
	die("Connection failed: " . $mysqli->error);
}


//request first row from the table that matches or is greater than the time argument 
$query = sprintf("Select * FROM " . $table . " WHERE RTCDataTime >= " . $time . " LIMIT 1;");
$result = $mysqli->query($query);

//get the associative array for the result 
$res_arr = mysqli_fetch_assoc($result);
//if the result is empty then we just request the last row of the table
if($res_arr == null) {
$result->close();
$query = sprintf("SELECT * FROM ( SELECT * FROM " . $table . " ORDER BY RTCDataTime DESC LIMIT 1 ) sub ORDER BY TransmissionKey ASC;");
$result = $mysqli->query($query);
//we only have one row so just fetch the associative array for that row
echo json_encode(mysqli_fetch_assoc($result));
$mysqli->close();
exit(0);
} 

//we only have one row so just fetch the associative array for that row
echo json_encode($res_arr);
$result->close();
$mysqli->close()






?>
