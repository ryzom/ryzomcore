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

// Log errors server-side; never print them into the response (stack traces
// and PDO messages carry connection details).
error_reporting( E_ALL );
ini_set( 'display_errors', '0' );
ini_set( 'log_errors', '1' );

class SystemExit extends Exception {}
try {

if (!file_exists('../role_support')) {
	header("Cache-Control: max-age=1");
	header('Location: ../setup?reason=no_role_support&from=ams', true, 303);
	throw new SystemExit();
}

require( '../config.php' );

if (!empty($AMS_REDIRECT_TO_ACCOUNT)) {
	header("Cache-Control: max-age=1");
	header('Location: ../account/', true, 302);
	throw new SystemExit();
}

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

// Always try to sync on page load, ie "lazy" cron. A ?cron=true shortcut used
// to force a full sync for any anonymous caller; the dedicated cron scripts
// under cron/ are the place for an on-demand run, and they check admin.
Sync :: syncdata( false );

/**
 * Resolve a request-supplied handler name to a script inside one of our own
 * directories. Anything that is not a plain identifier, or that does not
 * resolve to an existing file directly inside $dir, is rejected: without this
 * the name is concatenated straight into require() and any file on disk (for
 * instance an uploaded ticket attachment) can be executed.
 *
 * @param $name the untrusted handler/page name
 * @param $dir the directory the script must live in, relative to this file
 * @return the path to require, or false when the name is not acceptable
 */
function ams_resolve_script( $name, $dir ) {
    if ( !is_string( $name ) || !preg_match( '/^[A-Za-z0-9_]+$/', $name ) ) {
        return false;
    }
    $filename = $dir . '/' . $name . '.php';
    if ( !is_file( $filename ) ) {
        return false;
    }
    return $filename;
}

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
        // $page ends up in the smarty template name, so only accept a plain
        // identifier here -- otherwise it can be pointed at any file on disk
        $page = is_string( $_GET["page"] ) && preg_match( '/^[A-Za-z0-9_]+$/', $_GET["page"] ) ? $_GET["page"] : 'error';
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
    $filename = ams_resolve_script( $_POST["function"], 'func' );
     if ( $filename === false ) {
        $_SESSION['error_code'] = "404";
         header("Cache-Control: max-age=1");
         header( "Location: index.php?page=error" );
         throw new SystemExit();
         }
    require( $filename );
     $return = $_POST["function"]();
    } else if ( isset( $_GET["action"] ) ) {
    $filename = ams_resolve_script( $_GET["action"], 'func' );
     if ( $filename === false ) {
        $_SESSION['error_code'] = "404";
         header("Cache-Control: max-age=1");
         header( "Location: index.php?page=error" );
         throw new SystemExit();
         }
    require( $filename );
     $return = $_GET["action"]();
    } else {
    $filename = ams_resolve_script( $page, 'inc' );
     //check if this  is a file
     if ( $filename !== false ) {
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
