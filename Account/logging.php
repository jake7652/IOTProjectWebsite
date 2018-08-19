<?php 
$logDir = '/var/www/users/';
session_start();
$doneLogging = false;
if(isset($_SESSION['email'])){
	$logFile = fopen($logDir . $_SESSION['email'] . ".txt","a+");
	if(isset($logMessage)) {
		fwrite($logFile, $logMessage . " " . $_SERVER['REMOTE_ADDR'] . " "  . date("Y-m-d H:i:s T") . "\n");
		//echo ($logMessage . " "  . date("Y-m-d H:i:s T") . "\n");
		
	} 
	fclose($logFile);
	
} 

$doneLogging = true;



?>
