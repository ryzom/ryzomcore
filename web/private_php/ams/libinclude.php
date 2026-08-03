<?php
/**
* Base include file for library functions for AMS.
* Autoload function that loads the classes in case they aren't loaded yet.
*/

// Every entry point that starts a session requires this file first, so this
// is the one place that has to set the session cookie flags. Without
// httponly the session id is readable by any script that makes it onto a
// page; without a samesite policy the cookie rides along on cross site form
// posts. Only ask for the secure flag when the request itself arrived over
// tls, or a plain http install could never log in.
if ( session_status() === PHP_SESSION_NONE ) {
     ini_set( 'session.cookie_httponly', '1' );
     ini_set( 'session.use_only_cookies', '1' );
     ini_set( 'session.cookie_samesite', 'Lax' );
     if ( ( isset( $_SERVER['HTTPS'] ) && $_SERVER['HTTPS'] !== '' && strtolower( $_SERVER['HTTPS'] ) !== 'off' )
          || ( isset( $_SERVER['SERVER_PORT'] ) && $_SERVER['SERVER_PORT'] == 443 ) ) {
          ini_set( 'session.cookie_secure', '1' );
     }
}
function __ams_autoload( $className ){
     global $AMS_LIB;
     global $SITEBASE;
     //if the class exists in the lib's autload dir, load that one
     if(file_exists( $AMS_LIB.'/autoload/' . strtolower ( $className ) . '.php')){
          require_once 'autoload/' . strtolower ( $className ) . '.php';
     }
     //if the classname is WebUsers, use the sitebase location for the autoload dir.
     if($className == "WebUsers"){
          require_once $SITEBASE.'/autoload/' . strtolower ( $className ) . '.php';
     }
}

spl_autoload_register( '__ams_autoload' );


