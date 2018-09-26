<?php
/*
DEVELOPMENT HEADER
* Author: Jacob Barnett
* Version: 1.0.0b 
* Description: Page to log a user out when the user presses a logout button.
END DEVELOPMENT HEADER 
*/
/* Log out process, unsets and destroys session variables */
require 'db.php';
session_start();
//only update that the user is logged put if related session in the database is the same as this machine's given session
$setLogin = "UPDATE Users SET loggedin=0 WHERE email='" . $_SESSION['email'] . "' AND sessionid='". $_SESSION['ID'] . "';";
$mysqli->query($setLogin);
$logMessage = "LOGOUT";
include "logging.php";
while(!$doneLogging) {
	
}
session_unset();
session_destroy();
 
?>
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Goodbye</title>
  <?php include 'css/css.html'; ?>
</head>
<script>

window.location.href = "/Account/index.php";
</script>
<body>
<div class="form">
    <div class="form">
          <h1>Thanks for stopping by</h1>
              
          <p><?= 'You have been logged out!'; ?></p>
          
          <a href="../index.php"><button class="button button-block"/>Home</button></a>

    </div>
    <footer>
    Website last updated 2018-08-14 9:44
    </footer>  
</div>
<script src='https://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js'></script>
<script src="js/index.js"></script>
</body>
</html>
