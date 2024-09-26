<?php

/**
 * Global and Local Hooks for the Achievements plugin
 * Global Hooks are defined with the prefix(name of the plugin)
 * Local Hooks are defined with normal function name 
 * 
 * All the Global Hooks are called during the page load
 * and Local Hooks are called according to conditions
 * 
 * Here, we request to the Achievements url using REST 
 * to get the contents and will display with this plugin.
 * 
 * @author shubham meena mentored by Matthew Lagoe 
 */


// Global variable to store the data which is
// returned to the templates
$achievements_return_set = array();

// Local variable to store data during
// functionalities of the hooks
$var_set = array();

/**
 * Display hook for Achievements plugin
 */
function achievements_hook_display()
 {
    global $achievements_return_set;
     // to display plugin name in menu bar
    $achievements_return_set['menu_display'] = 'Achievements';
    $achievements_return_set['icon'] = 'icon-certificate';
     } 

/**
 * Local Hook to get database content
 * which is called by the global hook 
 * by passing a parameter
 * 
 * This hook returns the api keys registerd with 
 * the logged in user
 * 
 * @param  $data array with respective information
 * @return $row extracted db content wrt $data
 */
function hook_get_db_content( $data )
 {
    $db = new DBLayer( 'lib' );
     $sth = $db -> select( 'ams_api_keys', $data , 'User = :User AND UserCharacter = :UserCharacter' );
     $row = $sth -> fetchAll();
     return $row;
     } 

/**
 * Local Hook to get database content
 * which is called by the global hook 
 * by passing a parameter
 * 
 * This hook returns the id of the character 
 * whose achivements we have to get
 * 
 * @param  $data array with respective information
 * @return $row extracted db content wrt $data
 */
function hook_get_char_id( $data )
 {
    // returns the character id with respect to the character name  in the ring_tool->characters
    $db = new DBLayer( 'ring' );
     $sth = $db -> selectWithParameter( 'char_id', 'characters' , array( 'char_name' => $data ), 'char_name=:char_name' );
     $row = $sth -> fetch();
     return $row['char_id'];
     } 

/**
 * Local Hook to get database content
 * which is called by the global hook 
 * by passing a parameter
 * 
 * Hook to get the player stats of the character
 * 
 * @param  $data array with respective information
 * @return $row extracted db content wrt $data
 */
function hook_get_player_stat( $data )
 {
    // returns the character id with respect to the character name  in the ring_tool->characters
    $db = new DBLayer( 'webig' );
     $sth = $db -> select( 'players' , array( 'name' => $data ), 'name=:name' );
     $row = $sth -> fetch();
     return $row;
     } 

/**
 * Local Hook to set variables which contains
 * the content to use during the plugin functionality.
 */
function hook_variable_set()
 {
    global $achievements_return_set;
     global $var_set;
     if ( isset( $_POST['Character'] ) && !empty( $_POST['Character'] ) )
         {
        $var_set['character'] = $_POST['Character'];
        
         // get char id from ring_open table
        if ( $var_set['character'] != 'All Characters' )
         {
            $var_set['char_id'] = hook_get_char_id( $var_set['character'] );
            
             } 
        
        // get db content for variable set
        $row = hook_get_db_content( array( 'User' => $_SESSION['user'], 'UserCharacter' => $var_set['character'] ) );
        
         // access key automatically taken from the database wrt user and character
        @$var_set['app_key'] = $row['AccessToken'];
        
         // here you can set the host where this plugin is set
        $var_set['host'] = 'localhost';
        
         // here we get the stats of the character
        $ref_set = hook_get_player_stat( $var_set['character'] );
        
         // here we have set items that are required to get the achivements
        // these are player stats from webig->players table
        @$var_set['items'] = json_encode( array( 'dev_shard' => $ref_set['dev_shard'] , 'name' => $ref_set['name'] , 'cid' => $ref_set['cid'] , 'lang' => 'en' , 'translater_mode' => '', 'last_played_date' => $ref_set['last_login'] ) );
        
         // url where we have to make request for achievements
        // it sends get parameter search(what to search) and format(in which format data exchange takes place)
        $var_set['url'] = 'http://localhost6/?search=achievements&&format=json';
         } 
    else
         {
        $achievements_return_set['no_char'] = "Please Generate key for a character before requesting for achievements";
         } 
    } 

/**
 * Global Hook to interact with the REST api
 * Pass the variables in the REST object to 
 * make request 
 * 
 * variables REST object expects
 * url --> on which request is to be made
 * appkey --> app key for authentication
 * host --> host from which request have been sent
 * 
 * @return $achievements_return_set global array returns the template data
 */
function achievements_hook_call_rest()
 {
    // defined the variables
    global $var_set;
     global $achievements_return_set;
    
     if ( isset( $_POST['get_data'] ) )
         {
        hook_variable_set();
         // here we make the REST connection
        $rest_api = new Rest_Api();
         $ach_data = $rest_api -> request( $var_set['url'], $var_set['app_key'], $var_set['host'], $var_set['items'] );
         // here we store the response we get from the server
        $achievements_return_set['char_achievements'] = $ach_data ;
         } 
    } 

/**
 * Global Hook to return global variables which contains
 * the content to use in the smarty templates extracted from 
 * the database
 * 
 * @return $achievements_return_set global array returns the template data
 */
function achievements_hook_get_db()
 {
    global $achievements_return_set;
    
     if ( isset( $_SESSION['user'] ) )
         {
        $db = new DBLayer( 'lib' );
        
         // getting content for selecting characters
        $sth = $db -> selectWithParameter( 'UserCharacter', 'ams_api_keys', array( 'User' => $_SESSION['user'] ) , 'User = :User' );
         $row = $sth -> fetch();
         $achievements_return_set['Character'] = $row;
         } 
    } 

/**
 * Global Hook to return global variables which contains
 * the content to use in the smarty templates
 * 
 * @return $achievements_return_set global array returns the template data
 */
function achievements_hook_return_global()
 {
    global $achievements_return_set;
     return $achievements_return_set;
     }
