<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.1b 
* Description: Exports data from the remote database for given fields, timerange, and table.
* Do note that in order for this to work you will probably have to modify php.ini in order to get * the full downloads working without issue.
END DEVELOPMENT HEADER 
*/
//setting header to json
//header('Content-Type: application/json');

require 'db.php';

$fields = ['RTCDataTime'];
//$fields = explode(',',$fields);
$min_time = 0;
$max_time = 99999999999999;
$table = "DataTable30";
$fileExtension = "csv";
$lineLimit = 100000;
if(isset($_POST['arguments'])) {
$fields = array_merge($fields,explode(',',$_POST['arguments'][0]));
$min_time = $_POST['arguments'][1];
$max_time = $_POST['arguments'][2];
$lineLimit = $_POST['arguments'][3];
$table =  $_POST['arguments'][4];
$fileExtension = $_POST['arguments'][5];

}


$query = "SELECT ";
//request the last $lines rows from the database
for($i = 0; $i < sizeof($fields)-1;$i++) {
$query = $query . "'" . $fields[$i] . "',";
}
$query = $query . "'" . $fields[sizeof($fields)-1] . "'";
$query = $query . " UNION ALL SELECT ";
for($i = 0; $i < sizeof($fields)-1;$i++) {
$query = $query . $fields[$i] . ",";
}
$query = $query . $fields[sizeof($fields)-1] . " FROM " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . "";




//echo $query;


if($lineLimit != 0) {
$linesQuery = $mysqli->query("SELECT COUNT(*) FROM " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . "");
$numLines = mysqli_fetch_array($linesQuery)[0];
$linesQuery->close();
//we will end up requesting more rows than the limit 
$interval = floor($numLines/$lineLimit);

//$query[0] = "SELECT * FROM ( SELECT @row := @row +1 AS rownum, RTCDataTime";
//$query = "RTCDataTime";
//for($i = 0; $i < $num_fields;$i++) {
//$query = $query . "," . $fields[$i];

//}
//$query[0] = $query[0] . " FROM ( SELECT @row :=0) r, " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time . " ) ranked WHERE rownum % " . $interval . " = 1";
//$query = $query . " FROM " . $table . " WHERE RTCDataTime >= " . $min_time . " AND RTCDataTime <= " . $max_time ;

if($numLines > $lineLimit){

if($interval > 1) {
$query = $query . " AND TransmissionKey % " . $interval . " = 1 ";
} 
//$query = $query . " LIMIT " . $lineLimit;
}

//echo json_encode($query[0]);
//$result[0] = $mysqli->query($query[0]);
//echo json_encode(mysqli_fetch_all($result[0]));
//echo json_encode(mysqli_fetch_array($result[0]));
//die();
//$startIndex = 0;
//echo json_encode($query[0]);

}
$filename = "/var/www/exports/export" . uniqid();
$query = $query . " INTO OUTFILE '" . $filename . ".csv' FIELDS TERMINATED BY ',' LINES TERMINATED BY '\n';";
$result = $mysqli->query($query);
//echo $query;
//exit(0);
//echo var_dump($fields);
//exit;
//
//echo json_encode(sizeof($fields));
//exit;


//include '/var/www/html/lib/PHPExcel-1.8/PHPExcel/IOFactory.php';

//$objReader = PHPExcel_IOFactory::createReader('CSV');

// If the files uses a delimiter other than a comma (e.g. a tab), then tell the reader
//$objReader->setDelimiter("\t");
// If the files uses an encoding other than UTF-8 or ASCII, then tell the reader
//$objReader->setInputEncoding('UTF-8');

//$objPHPExcel = $objReader->load($filename.'.csv');
//$objWriter = PHPExcel_IOFactory::createWriter($objPHPExcel, 'Excel5');
//$objWriter->save($filename.'.xls');



header('Content-Transfer-Encoding: binary');
header('Cache-Control: must-revalidate');
header('Pragma: public');    
header("Expires: 0");
//header("Content-type: text/x-csv");

//header("Content-Type: application/x-msexcel; name=export.xls");
//header('Content-Type: application/xml; charset=utf-8');
//header('Content-Type: application/octet-stream'); 
if($fileExtension == "xlsx") {

exec("/var/www/export ".$filename);
header("Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet;");

} else if($fileExtension == "csv") {
header("Content-type: text/csv");

}

header("Content-Disposition: attachment; filename=export.".$fileExtension);
//header("Content-Type: application/vnd.ms-excel");
header('Content-Type: application/force-download');

//header('Pragma: public');
//header('Expires: 0');
                                   
//header('Content-Description: Download'); 
//header('Content-Type: application/octet-stream'); 
//header('Content-Disposition: attachment; filename=export.xls');
//header('Content-Transfer-Encoding: binary');
//header('Content-Length: '.filesize($filename));
readfile($filename.".".$fileExtension);
unlink($filename.".".$fileExtension);


//echo $query;

//$result.free();
$mysqli->close();
exit;
?>
