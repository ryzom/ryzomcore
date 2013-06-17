<?php

require( '../config.php' );
require( '../../ams_lib/libinclude.php' );

if ( isset( $_POST["function"] ) ){
     require( "inc/" . $_POST["function"] . ".php" );
     $return = $_POST["function"]();
     }

function loadpage ( $page ){
     require_once( 'autoload/' . $page . '.php' );
     }

$page = 'login';
if ( isset( $_GET["page"] ) ){
     $page = $_GET["page"];
     }

//Page Handling
if($page == 'login' || $page == 'register'){
     $no_visible_elements = 'TRUE';
}
// temporary set permission to 2 which = admin mode
$return = array( 'permission' => 1, 'no_visible_elements' => $no_visible_elements );

helpers :: loadTemplate( $page , $return );
