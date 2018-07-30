<?php
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
$thing = '/var/www/clients/';
$ree = scandir($thing);
for($i = 2; $i < sizeof($ree); $i++) {
$contents[$ree[$i]] = Array();
$tempFiles = scandir($thing . $ree[$i]);
for($i2 = 0; $i2 < sizeof($tempFiles); $i2++) {
if($tempFiles[$i2] != "." && $tempFiles[$i2] != "..") {
$contents[$ree[$i]][$i2-2] = file_get_contents($thing . $ree[$i] . "/" . $tempFiles[$i2]);
}
}

}


echo json_encode($contents);

?>
