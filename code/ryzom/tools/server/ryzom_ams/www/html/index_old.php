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

$page = 'home';
if ( isset( $_GET["page"] ) ){
     $page = $_GET["page"];
    }
    
//temporary set permission to 1 which = admin mode
$return =  array('permission' => 1, 'no_visible_elements' => 'TRUE');

helpers :: loadTemplate( $page , $return );
