<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Requests all directories in the clients directory and creates a associative array that lists all the contents of the files in the each client directory
END DEVELOPMENT HEADER 
*/
function getDirContents($dir, &$results = array()){
    $files = scandir($dir);

    foreach($files as $key => $value){
        $path = realpath($dir.DIRECTORY_SEPARATOR.$value);
        if(!is_dir($path)) {
            $results[] = $path;
        } else if($value != "." && $value != "..") {
            getDirContents($path, $results);
            $results[] = $path;
        }
    }

    return $results;
}
$contents = Array();
$clientDir = '/var/www/clients/';
$clientDirInner = scandir($clientDir);
for($i = 2; $i < sizeof($clientDirInner); $i++) {
//$contents[$clientDirInner[$i]] = Array();
$contents[$i-2] = Array();
$contents[$i-2][0] = $clientDirInner[$i];
$tempFiles = scandir($clientDir . $clientDirInner[$i]);
for($i2 = 1; $i2 < sizeof($tempFiles); $i2++) {
if($tempFiles[$i2] != "." && $tempFiles[$i2] != "..") {
$contents[$i-2][$i2-1] = file_get_contents($clientDir . $clientDirInner[$i] . "/" . $tempFiles[$i2]);
//$contents[$clientDirInner[$i]][$i2-1] = file_get_contents($clientDir . $clientDirInner[$i] . "/" . $tempFiles[$i2]);
}
}

}


echo json_encode($contents);

?>
