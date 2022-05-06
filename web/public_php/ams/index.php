<?php
/**
 * Core that runs the entire system.
 * The index.php page handles:
 * -# checks what page to load
 * -# if a $_POST['function'] is set try to execute that function in the matching php file located in the func folder.
 * -# else load the inc's folder matching function related to the page
 * -# set the permission and other smarty related settings
 * -# call the helper function to load the page.
 *
 * @author Daan Janssens, mentored by Matthew Lagoe
 */

// load required pages and turn error reporting on/off
error_reporting( E_ALL );
ini_set( 'display_errors', 'on' );

class SystemExit extends Exception {}
try {

if (!file_exists('../role_support')) {
	header("Cache-Control: max-age=1");
	header('Location: ../setup?reason=no_role_support&from=ams', true, 303);
	throw new SystemExit();
}

require( '../config.php' );

if ($NEL_SETUP_VERSION_CONFIGURED < $NEL_SETUP_VERSION) {
	header("Cache-Control: max-age=1");
	header('Location: ../setup?reason=upgrade&from=ams', true, 303);
	throw new SystemExit();
}

require_once( $AMS_LIB . '/libinclude.php' );
session_cache_limiter('nocache');
session_start();
header("Expires: Mon, 01 May 2000 06:00:00 GMT");
header("Last-Modified: ". gmdate("D, d M Y H:i:s") ." GMT");
header("Cache-Control: max-age=1");
header("Cache-Control: no-store, no-cache, must-revalidate");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");

// Running Cron
if ( isset( $_GET["cron"] ) ) {
    if ( $_GET["cron"] == "true" ) {
        Sync :: syncdata( false );
         }
    }

// Always try to sync on page load, ie "lazy" cron
Sync :: syncdata( false );

// Decide what page to load
if ( ! isset( $_GET["page"] ) ) {

    if ( isset( $_SESSION['user'] ) ) {
        if ( Ticket_User :: isMod( unserialize( $_SESSION['ticket_user'] ) ) ) {
            $page = 'dashboard';
             } else {
            $page = 'show_user';
             }
        } else {
        // default page
        $page = 'login';
         }
    } else {
	// if the session exists load page with $_GET requests
    if ( isset( $_SESSION['user'] ) ) {
        $page = $_GET["page"];
         } else {
        switch ( $_GET["page"] ) {
        case 'register':
             $page = 'register';
             break;
         case 'forgot_password':
             $page = 'forgot_password';
             break;
         case 'reset_password':
             $page = 'reset_password';
             break;
         case 'error':
             $page = 'error';
             break;
         default:
             $page = 'login';
             break;
             }
        }
    }

// check if ingame & page= register
// this is needed because the ingame register can't send a hidden $_POST["function"]
if ( Helpers :: check_if_game_client() && ( $page == "register" ) ) {
    require( "func/add_user.php" );
     $return = add_user();
    }

// perform an action in case one is specified
// else check if a php page is included in the inc folder, else just set page to the get param
if ( isset( $_POST["function"] ) ) {
    require( "func/" . $_POST["function"] . ".php" );
     $return = $_POST["function"]();
    } else if ( isset( $_GET["action"] ) ) {
    require( "func/" . $_GET["action"] . ".php" );
     $return = $_GET["action"]();
    } else {
    $filename = 'inc/' . $page . '.php';
     //check if this  is a file
     if ( is_file( $filename ) ) {
        require_once( $filename );
         $return = $page();
         }
    }

// add username to the return array in case logged in.
if ( isset( $_SESSION['user'] ) ) {
    $return['username'] = $_SESSION['user'];
    }

// Set permission
if ( isset( $_SESSION['ticket_user'] ) ) {
    $return['permission'] = unserialize( $_SESSION['ticket_user'] ) -> getPermission();
    } else {
    // default permission
    $return['permission'] = 0;
    }

// hide sidebar + topbar in case of login/register
if ( $page == 'login' || $page == 'register' || $page == 'logout' || $page == 'forgot_password' || $page == 'reset_password' ) {
    $return['no_visible_elements'] = 'TRUE';
    } else {
    $return['no_visible_elements'] = 'FALSE';
    }

// handle error page
if ( $page == 'error' ) {
    $return['permission'] = 0;
     $return['no_visible_elements'] = 'FALSE';
    }

// call to load hooks for the active plugins
$hook_content = Plugincache :: loadHooks();
foreach( $hook_content as $key => $value )
 {
    $return[$key] = $value;
     }

// load the template with the variables in the $return array
helpers :: loadTemplate( $page , $return );

}
catch (SystemExit $e) { /* do nothing */ }
