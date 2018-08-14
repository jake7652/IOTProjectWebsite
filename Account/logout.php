<?php
/* Log out process, unsets and destroys session variables */
session_start();
//session_unset();
session_destroy(); 
?>
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Goodbye</title>
  <?php include 'css/css.html'; ?>
</head>

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
<script src='http://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js'></script>
<script src="js/index.js"></script>
</body>
</html>
