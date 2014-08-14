<?php

error_reporting( E_ALL ^ E_NOTICE );
ini_set( "display_errors", "1" );

define( 'APP_NAME', 'app_achievements' );

require_once( '../config.php' );
include_once( '../lang.php' );
include_once( 'lang.php' );
require_once( 'conf.php' );
require_once( "class/RyzomUser_class.php" );

require_once( "include/ach_render_common.php" );

require_once( "class/DLL_class.php" );
// require_once("class/InDev_trait.php");
require_once( "class/Node_abstract.php" );
require_once( "class/AVLTree_class.php" );
require_once( "class/Parentum_abstract.php" );
require_once( "class/AchList_abstract.php" );
require_once( "class/Tieable_inter.php" );
require_once( "class/NodeIterator_class.php" );


require_once( "class/AchMenu_class.php" );
require_once( "class/AchMenuNode_class.php" );
require_once( "class/AchSummary_class.php" );
require_once( "class/AchCategory_class.php" );
require_once( "class/AchAchievement_class.php" );
require_once( "class/AchTask_class.php" );
require_once( "class/AchObjective_class.php" );



// Update user acces on Db
// $DBc = ryDB::getInstance(APP_NAME."_test");
$DBc = ryDB :: getInstance( APP_NAME );


// if getting request using REST
if ( isset( $_GET['search'] ) && isset( $_GET['format'] ) )
     {
    // if the format is json
    if ( $_GET['format'] == 'json' )
     {
        // getting the headers when the request is sent
        $header = getallheaders();
        
         // this block is to get the posted data
        $fp = fopen( 'php://input', 'r' );
         $rawData = stream_get_contents( $fp );
         $userd = json_decode( $rawData, true );
        
         // authenticate the user using data we get from server
        appAuthenticateRest( $user, $userd );
        
         // create a ryzom user object whose achievements we have to send in response
        $_USER = new RyzomUser( $user );
        
         require_once( "include/ach_render_web.php" );
         $c .= ach_render();
         $response = $c;
         // sending the response
        echo( $response );
         exit;
        
         } 
    } 
else
     {
    echo 'Invalid response';
     exit;
     } 




// Ask to authenticate user (using ingame or session method) and fill $user with all information
ryzom_app_authenticate( $user, true );


// echo var_export($user,true);
// $user['id'] = $user['char_id'];
// $user['name'] = $user['char_name'];
/**
 * $user = array();
 * $user['cid'] = 1;
 * $user['lang'] = 'en';
 * $user['name'] = 'Talvela';
 * $user['race'] = "r_matis";
 * $user['civilization'] = "c_neutral";
 * $user['cult'] = "c_neutral";
 * $user['ig'] = ($_REQUEST['ig']==1);
 * #$user['ig'] = true;
 */

$_USER = new RyzomUser( $user );


if ( $_USER -> isIG() ) {
    require_once( "include/ach_render_ig.php" );
    } 
else {
    require_once( "include/ach_render_web.php" );
    } 

// require_once("fb/facebook.php");

$c = "";
if ( !$_USER -> isIG() ) {
    /**
     * $facebook = new Facebook(array(
     * 'appId' => $_CONF['fb_id'],
     * 'secret' => $_CONF['fb_secret'],
     * 'cookie' => true
     * ));
     * 
     * #code taken from facebook tutorial
     * 
     * // Get the url to redirect for login to facebook
     * // and request permission to write on the user's wall.
     * $login_url = $facebook->getLoginUrl(
     * array('scope' => 'publish_stream')
     * );
     * 
     * // If not authenticated, redirect to the facebook login dialog.
     * // The $login_url will take care of redirecting back to us
     * // after successful login.
     * if (! $facebook->getUser()) {
     * $c .= '<script type="text/javascript">
     * top.location.href = "'.$login_url.'";
     * </script>;';
     * }
     * else {
     * $DBc->sqlQuery("INSERT INTO ach_fb_token (aft_player,aft_token,aft_date,aft_allow) VALUES ('".$_USER->getID()."','".$DBc->sqlEscape($facebook->getAccessToken())."','".time()."','1') ON DUPLICATE KEY UPDATE aft_token='".$DBc->sqlEscape($facebook->getAccessToken())."', aft_date='".time()."'");
     * }
     */
    
    
    } 

if ( !$_USER -> isIG && $_CONF['enable_webig'] == false ) {
    $c .= ach_render_forbidden( false );
    
    } 
elseif ( $_USER -> isIG && $_CONF['enable_offgame'] == false ) {
    $c .= ach_render_forbidden( true );
    
    } 
else {
    $c .= ach_render();
    } 


echo ryzom_app_render( strtoupper( get_translation( 'ach_app_name', $_USER -> getLang() ) ), $c, $_USER -> isIG() );

?>
