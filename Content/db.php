<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.1b 
* Description: Grabs all the settings for the database from the database settings file and sets them to variable and then creates a sql connection.
* This file is used by other php scripts so that this only needs to be done once.
END DEVELOPMENT HEADER 
*/

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
$database = $contents[3];

$mysqli = mysqli_init();
//uncomment all this random stuff if you want to have ssl on the mysql connection
//mysqli_options ($db, MYSQLI_OPT_SSL_VERIFY_SERVER_CERT, true);
//$mysqli->ssl_set("/var/www/certs/mysql/client-key.pem", "/var/www/certs/mysql/client-cert.pem", "///var/www/certs/mysql/ca.pem", NULL, NULL);
//$connected = mysqli_real_connect ($mysqli, $host, $user, $pass, $database, 3306, NULL,  MYSQLI_CLIENT_SSL_DONT_VERIFY_SERVER_CERT);

$connected = mysqli_real_connect ($mysqli, $host, $user, $pass, $database, 3306, NULL,  NULL);
if (!$connected)
{
    die ('Connect error (' . mysqli_connect_errno() . '): ' . mysqli_connect_error() . "\n");
}

