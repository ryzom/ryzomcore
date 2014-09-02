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
if (!file_exists( '../is_installed')) {
	header("Cache-Control: max-age=1");
	header('Location: ../setup', true, 303);
	die();
}

require( '../config.php' );
require_once( $AMS_LIB . '/libinclude.php' );
session_start();

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
