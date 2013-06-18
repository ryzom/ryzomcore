<?php

require( '../config.php' );
require( '../../ams_lib/libinclude.php' );
//default page
$page = 'login';

 
if ( isset( $_POST["function"] ) ){
     require( "inc/" . $_POST["function"] . ".php" );
     $tempReturn = $_POST["function"]();
     $functionReturn = array_merge($tempReturn,$_POST);
     if ( isset($_POST["callBack"])){
          $page = $_POST["callBack"];
          
     }
}

function loadpage ( $page ){
     require_once( 'autoload/' . $page . '.php' );
     }

if ( isset( $_GET["page"] ) ){
     $page = $_GET["page"];
     }

//Page Handling
if($page == 'login' || $page == 'register'){
     $no_visible_elements = 'TRUE';
}

if ( isset($functionReturn) ){
     $return = array_merge(array( 'permission' => 1, 'no_visible_elements' => $no_visible_elements ),$functionReturn);
}else{
     $return = array( 'permission' => 1, 'no_visible_elements' => $no_visible_elements );
}
//print_r($return);

helpers :: loadTemplate( $page , $return );
