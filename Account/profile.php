<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Displays user name, email, and permission level for the site for their account.
END DEVELOPMENT HEADER 
*/
$logMessage = "PROFILE";
include "logging.php";

/* Displays user information and some useful messages */
session_start();

if (isset($_SESSION['LAST_ACTIVITY']) && (time() - $_SESSION['LAST_ACTIVITY'] > 1800)) {
    // last request was more than 30 minutes ago
    session_unset();     // unset $_SESSION variable for the run-time 
    session_destroy();   // destroy session data in storage
}
$_SESSION['LAST_ACTIVITY'] = time(); // update last activity time stamp

// Check if user is logged in using the session variable
if ( $_SESSION['logged_in'] != 1 ) {
  $_SESSION['message'] = "You must log in before viewing your profile page!";
  header("location: error.php");    
}
else {
    // Makes it easier to read
    $first_name = $_SESSION['first_name'];
    $last_name = $_SESSION['last_name'];
    $email = $_SESSION['email'];
    $active = $_SESSION['active'];
    $permissions = $_SESSION['permissions'];
}
?>
<!DOCTYPE html>
<html >
<head>
  <meta charset="UTF-8">
  <title>Welcome <?= $first_name.' '.$last_name ?></title>
  <?php include 'css/css.html'; ?>
</head>

<body>
  <div class="form">

          <h1>Account Profile</h1>
          
          <p>
          <?php 
     
          // Display message about account verification link only once
          if ( isset($_SESSION['message']) )
          {
              echo $_SESSION['message'];
              
              // Don't annoy the user with more messages upon page refresh
              unset( $_SESSION['message'] );
          }
          
          ?>
          </p>
          
          <?php
          
          // Keep reminding the user this account is not active, until they activate
          if ( !$active ){
              echo
              '<div class="info">
              Account is unverified, please confirm your email by clicking
              on the email link!
              </div>';
          }
          
          ?>
          
          <h2>Name: <?php echo $first_name.' '.$last_name; ?></h2>
          <div> 
	  <p>Email: <?= $email ?> <br> Permission Level: <?= $permissions ?></p>
          </div>
          <a href="logout.php"><button class="button button-block" name="logout"/>Log Out</button></a>
	  <br>
	  <a href="/Content/index.html"><button class="button button-block" name="indexButton"/>Back to index</button></a>
	  <br>
	  <footer>
	  Website last updated 2018-08-14 9:43
	  </footer>    
</div>
    
<script src='http://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js'></script>
<script src="js/index.js"></script>

</body>
</html>
