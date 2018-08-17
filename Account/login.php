<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Page that actually logs a user in when the user presses the login button 
* on the index page.
END DEVELOPMENT HEADER 
*/
/* User login process, checks if user exists and password is correct */
//session_start();
// Escape email to protect against SQL injections
$email = $mysqli->escape_string($_POST['email']);
$result = $mysqli->query("SELECT * FROM Users WHERE email='$email'");

if ( $result->num_rows == 0 ){ // User doesn't exist
    $_SESSION['message'] = "User with that email doesn't exist!";
    header("location: error.php");
}
else { // User exists
    $user = $result->fetch_assoc();

    if ( password_verify($_POST['password'], $user['password']) ) {
        
        $_SESSION['email'] = $user['email'];
        $_SESSION['first_name'] = $user['first_name'];
        $_SESSION['last_name'] = $user['last_name'];
        $_SESSION['active'] = $user['active'];
	$_SESSION['permissions'] = $user['permissions'];        

        // This is how we'll know the user is logged in
        $_SESSION['logged_in'] = true;
	session_write_close();

	//echo ini_get('session.cookie_domain');
	//echo json_encode(session_id());
	//echo json_encode(session_save_path());
	//echo json_encode("I was able to start the freaking session");
	//require 'usrStat.php';
        header("location: /Content/index.html");
    }
    else {
        $_SESSION['message'] = "You have entered wrong password, try again!";
        header("location: error.php");
    }
}

