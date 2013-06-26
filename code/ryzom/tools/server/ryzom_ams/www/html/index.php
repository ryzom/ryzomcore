<?php
error_reporting(E_ALL);
ini_set('display_errors', 'on');
require( '../config.php' );
require( '../../ams_lib/libinclude.php' );
session_start();

//print_r($_SESSION);

//Decide what page to load
if(isset($_SESSION['user'])){
     $page = 'home';
}else{
     //default page
     $page = 'login';   
}

if ( isset( $_GET["page"] ) ){
     $page = $_GET["page"];
     }

//perform an action in case one is specified
if ( isset( $_POST["function"] ) ){
     require( "inc/" . $_POST["function"] . ".php" );
     $return = $_POST["function"]();
}


function loadpage ( $page ){
     require_once( 'autoload/' . $page . '.php' );
}

//Set permission
if(isset($_SESSION['permission'])){
     $return['permission'] = $_SESSION['permission'];
}else{
     //default permission
     $return['permission'] = 0; 
}


//hide sidebar + topbar in case of login/register
if($page == 'login' || $page == 'register'){
     $return['no_visible_elements'] = 'TRUE';
}else{
     $return['no_visible_elements'] = 'FALSE';
}
//print_r($return);
helpers :: loadTemplate( $page , $return );
