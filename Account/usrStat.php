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
include "db.php";
$result = $mysqli->query("SELECT * FROM Users WHERE email='". $_SESSION['email'] ."'");
if ( $result->num_rows == 0 ){ // User doesn't exist
    $_SESSION['message'] = "No user logged in!";
    header("location: error.php");
}
$user = $result->fetch_assoc();
//var_dump($_SESSION);

if (isset($_SESSION['LAST_ACTIVITY']) && (time() - $_SESSION['LAST_ACTIVITY'] > 120)) {
    
    
    unset($_SESSION['LAST_ACTIVITY']);
    // last request was more than 2 minutes ago
    include 'logout.php';
    session_unset();     // unset $_SESSION variable for the run-time 
    session_destroy();   // destroy session data in storage
    session_write_close();
    
} else if(isset($_SESSION['LAST_ACTIVITY']) ) {
$_SESSION['LAST_ACTIVITY'] = time(); // update last activity time stamp
$mysqli->query("UPDATE Users SET lastactivity=". $_SESSION['LAST_ACTIVITY'] . " WHERE email='" . $_SESSION['email']."'");
}

//echo "UPDATE Users SET lastactivity=". $_SESSION['LAST_ACTIVITY'] . " WHERE email='" . $_SESSION['email']."'";
//echo($_SESSION['LAST_ACTIVITY']);
//var_dump($_SESSION);
//echo ini_get('session.cookie_domain');
//echo json_encode(session_id());
//echo json_encode(session_save_path());
// This is how we'll know the user is logged in
$mysqli->close();
$result->free();
echo json_encode([isset($_SESSION['logged_in'])?$_SESSION['logged_in']:"",isset($_SESSION['permissions'])?$_SESSION['permissions']:"0"]);
?>
