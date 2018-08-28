<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Gets the first entry of the RTCDataTime field for the table defined in the first argument.
END DEVELOPMENT HEADER 
*/
//setting header to json
//header('Content-Type: application/json');

require 'db.php';


$lines = 1; //how many lines from the database we should request
//$fields = 'PressureTemperature,PressurePressure,Humidity,HumidityHeatIndex,HumidityTemperature,Temperature1,Temperature2';
$fields = 'PressureTemperature';
$fields = explode(',',$fields);
$min_time = 0;
$max_time = 99999999999999;
$table = "DataTable";
$filename = "export.xls";
if(isset($_POST['arguments'])) {
$fields = explode(',',$_POST['arguments'][0]);
$min_time = $_POST['arguments'][1];
$max_time = $_POST['arguments'][2];
$table =  $_POST['arguments'][3];

}

//echo var_dump($fields);
//exit;
//
//echo json_encode(sizeof($fields));
//exit;
$query = "SELECT ";
//request the last $lines rows from the database
for($i = 0; $i < sizeof($fields)-1;$i++) {
$query = $query . $fields[$i] . ",";
}


$query = $query . $fields[sizeof($fields)-1] . " FROM " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . ";";
$result = $mysqli->query($query);
header("Content-Disposition: attachment; filename=export.xls");
header("Content-Type: application/vnd.ms-excel; charset=UTF-16");
header('Expires: 0');
header('Cache-Control: must-revalidate');
header('Pragma: public');
//header("Pragma: no-cache"); 
//header("Expires: 0");
$user_query = $result;
// Write data to file
$flag = false;
while ($row = mysqli_fetch_assoc($user_query)) {
    if (!$flag) {
        // display field/column names as first row
        echo implode("\t", array_keys($row)) . "\r\n";
        $flag = true;
    }
    echo implode("\t", array_values($row)) . "\r\n";
}
$result->close();
$mysqli->close();
exit;
?>
