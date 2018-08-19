<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: gets the permission level of the currently logged in user
END DEVELOPMENT HEADER 
*/
//include 'login.php';
//require 'login.php';
$logMessage = "TRY TO GOTO " . $_SERVER["HTTP_REFERER"];
include "logging.php";
session_start();
if (isset($_SESSION['LAST_ACTIVITY']) && (time() - $_SESSION['LAST_ACTIVITY'] > 1800)) {
    // last request was more than 30 minutes ago
    session_unset();     // unset $_SESSION variable for the run-time 
    session_destroy();   // destroy session data in storage
}
$_SESSION['LAST_ACTIVITY'] = time(); // update last activity time stamp
//echo($_SESSION['LAST_ACTIVITY']);
//var_dump($_SESSION);
//echo ini_get('session.cookie_domain');
//echo json_encode(session_id());
//echo json_encode(session_save_path());
// This is how we'll know the user is logged in
echo json_encode([isset($_SESSION['logged_in'])?$_SESSION['logged_in']:"",isset($_SESSION['permissions'])?$_SESSION['permissions']:"0"]);
?>
