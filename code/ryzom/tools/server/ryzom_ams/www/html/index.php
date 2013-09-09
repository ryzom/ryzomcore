<?php
error_reporting(E_ALL);
ini_set('display_errors', 'on');
require( '../config.php' );
require( '../../ams_lib/libinclude.php' );
session_start();

//Decide what page to load
if ( ! isset( $_GET["page"]) ){
     if(isset($_SESSION['user'])){
          if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
               $page = 'dashboard';
          }else{
               $page = 'show_user';
          }
     }else{
          //default page
          $page = 'login';   
     }
}else{
     $page = $_GET["page"];
}

//perform an action in case one is specified
//else check if a php page is included in the inc folder, else just set page to the get param
if ( isset( $_POST["function"] ) ){
     require( "func/" . $_POST["function"] . ".php" );
     $return = $_POST["function"]();
}else{
     $filename = 'inc/' . $page . '.php';
     if(is_file($filename)){
          require_once($filename);
          $return = $page();
     }
}

//add username to the return array in case logged in.
if(isset($_SESSION['user'])){
     $return['username'] = $_SESSION['user'];
}
     
     


//Set permission
if(isset($_SESSION['ticket_user'])){
     $return['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
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

//handle error page
if($page == 'error'){
     $return['permission'] = 0;
     $return['no_visible_elements'] = 'FALSE';
}

helpers :: loadTemplate( $page , $return );
