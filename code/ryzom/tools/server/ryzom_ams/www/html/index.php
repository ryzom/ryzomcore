<?php
error_reporting(E_ALL);
ini_set('display_errors', 'on');
require( '../config.php' );
require( '../../ams_lib/libinclude.php' );
session_start();

//Decide what page to load
if(isset($_SESSION['user'])){
     $page = 'home';
     $return['username'] = $_SESSION['user'];
}else{
     //default page
     $page = 'login';   
}

//perform an action in case one is specified
//else check if a php page is included in the inc folder, else just set page to the get param
if ( isset( $_POST["function"] ) ){
     require( "func/" . $_POST["function"] . ".php" );
     $return = $_POST["function"]();
}else if ( isset( $_GET["page"] ) ){
     $filename = 'inc/' . $_GET["page"] . '.php';
     if(is_file($filename)){
          require_once($filename);
     }
     $page = $_GET["page"];
}


/*function loadpage ( $page ){
     $filename = 'autoload/' . $page . '.php';
     if(is_file($filename)){
          require_once($filename);
     }
}

loadpage($page);*/

//Set permission
if(isset($_SESSION['permission'])){
     $return['permission'] = $_SESSION['permission'];
}else{
     //default permission
     $return['permission'] = 0; 
}


//hide sidebar + topbar in case of login/register
if($page == 'login' || $page == 'register' || $page == 'logout'){
     $return['no_visible_elements'] = 'TRUE';
}else{
     $return['no_visible_elements'] = 'FALSE';
}
//print_r($return);
helpers :: loadTemplate( $page , $return );
