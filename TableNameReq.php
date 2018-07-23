<?php
//SELECT table_name FROM information_schema.tables where table_schema='TideGuage';


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

if(!$mysqli){
	$mysqli = @mysqli_connect("localhost", DB_USERNAME, DB_PASSWORD, DB_NAME);
}


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
