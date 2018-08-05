<?php
/* Database connection settings */
$host = 'localhost';
$user = 'Tide';
$pass = 'MasterWater2025';
$db = 'Accounts';
$mysqli = new mysqli($host,$user,$pass,$db) or die($mysqli->error);
