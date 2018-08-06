<?php

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

/* Database connection settings */
$host = $contents[0];
$user = $contents[1];
$pass = $contents[2];
$db = $contents[3];
$mysqli = new mysqli($host,$user,$pass,$db); //or die($mysqli->error);

if(!$mysqli) {
$mysqli = new mysqli('localhost',$user,$pass,$db);
}