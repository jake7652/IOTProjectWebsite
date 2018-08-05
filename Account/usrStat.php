<?php
session_start();
// This is how we'll know the user is logged in
echo json_encode([isset($_SESSION['logged_in'])?$_SESSION['logged_in']:"",isset($_SESSION['permissions'])?$_SESSION['permissions']:"0"]);
exit();
?>