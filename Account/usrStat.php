<?php
//include 'login.php';
//require 'login.php';
session_start();
//var_dump($_SESSION);
//echo ini_get('session.cookie_domain');
//echo json_encode(session_id());
//echo json_encode(session_save_path());
// This is how we'll know the user is logged in
echo json_encode([isset($_SESSION['logged_in'])?$_SESSION['logged_in']:"",isset($_SESSION['permissions'])?$_SESSION['permissions']:"0"]);
?>