<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Writes a command value defined by the second argument for the sensor platform file defined by the first argument.
END DEVELOPMENT HEADER 
*/
$platform = 'DataTable';
$commandFile = 'commands';
$value = "200";
if(isset($_POST['arguments'])) {
$platform =  $_POST['arguments'][0];
$value =  $_POST['arguments'][1];
}


ini_set('track_errors', 1);
$filename =  "/var/www/clients/" . $platform . "/" . $commandFile;
$file = fopen($filename,"w");
fwrite($file,$value);
if( $file == false ) {
            echo ( "Error in opening file \n" );
	    echo 'fopen failed. reason: ', $php_errormsg;
            exit();
         }
fclose($file);




?>