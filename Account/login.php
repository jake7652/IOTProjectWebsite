<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Page that actually logs a user in when the user presses the login button 
* on the index page.
END DEVELOPMENT HEADER 
*/

//require "db.php";
//session_start();
/* User login process, checks if user exists and password is correct */
session_start();
// Escape email to protect against SQL injections
$email = $mysqli->escape_string($_POST['email']);
$result = $mysqli->query("SELECT * FROM Users WHERE email='$email'");
$timeout = 120;
echo var_dump($_SESSION);
if ( $result->num_rows == 0 ){ // User doesn't exist
    $_SESSION['message'] = "User with that email doesn't exist!";
    header("location: error.php");
}

else { // User exists
    $user = $result->fetch_assoc();
    isset($_SESSION['ID']) ? "" : $_SESSION['ID'] = 0; 
    if($user['loggedin'] == 1 && $_SESSION['ID'] != $user['sessionid'] && time() - $user['lastactivity'] < $timeout) {
	$_SESSION['message'] = "User is already logged in on the site. Please log it out or wait for it to timeout";
    	header("location: error.php");
    } else if($user['loggedin'] == 1 && $_SESSION['ID'] == $user['sessionid'] && time() - $user['lastactivity'] < $timeout) {
	$_SESSION['message'] = "You are already logged into the site";
    	header("location: error.php");
    }
    //else the user session was timed out or was logged out manually
    else if ( password_verify($_POST['password'], $user['password']) ) {
        
        $_SESSION['email'] = $user['email'];
        $_SESSION['first_name'] = $user['first_name'];
        $_SESSION['last_name'] = $user['last_name'];
        $_SESSION['active'] = $user['active'];
	$_SESSION['permissions'] = $user['permissions'];
	//apache_note('user',$_SESSION['email']);
	$logMessage = "LOGIN";
	//include "logging.php";        
	$_SESSION['LAST_ACTIVITY'] = time();
	$_SESSION['ID'] = md5(uniqid());
	$setLogin = "UPDATE Users SET loggedin=1,lastactivity=". $_SESSION['LAST_ACTIVITY'] . ",sessionid='" . $_SESSION['ID'] . "' WHERE email='" . $user['email'] . "';";	
	$mysqli->query($setLogin);
	echo($setLogin);
        // This is how we'll know the user is logged in
        $_SESSION['logged_in'] = true;
	session_write_close();
	//echo var_dump($_SESSION);
	//echo ini_get('session.cookie_domain');
	//echo json_encode(session_id());
	//echo json_encode(session_save_path());
	//echo json_encode("I was able to start the freaking session");
	require 'usrStat.php';
        header("location: /Content/index.html");
    }
    else {
        $_SESSION['message'] = "You have entered wrong password, try again!";
        header("location: error.php");
    }
}

