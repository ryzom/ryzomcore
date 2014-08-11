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


// Global variables to store the data
$return_set = array();
$var_set = array();


/**
 * Display hook for Achievements plugin
 */
function achievements_hook_display()
 {
    global $return_set;
     // to display plugin name in menu bar
    $return_set['menu_display'] = 'Achievements';
    } 

/**
 * Local Hook to get database content
 * which is called by the global hook 
 * by passing a parameter
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
 * Local Hook to set variables which contains
 * the content to use during the plugin functionality.
 */
function hook_variable_set()
 {
    global $var_set;
     $var_set['character'] = $_POST['Character'];
    
     // get db content for variable set
    $row = hook_get_db_content( array( 'User' => $_SESSION['user'], 'UserCharacter' => $var_set['character'] ) );
    
     // access key automatically taken from the database wrt user and character
    @$var_set['app_key'] = $row['AccessToken'];
    
     // here you can set the host where this plugin is set
    $var_set['host'] = 'localhost';
    
     // here you can set what you are looking for
    // when you are requesting encoded in json
    @$var_set['items'] = json_encode( array( 'Task' => 'Achievements', 'Character' => $var_set['character'] ) );
    
     // url where we have to make request for achievements
    // it sends get parameter search(what to search) and format(in which format data exchange takes place)
    $var_set['url'] = 'app.domain.org?search=' . $var_set['items'] . '&&format=json';
    
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
 * @return $return_set global array returns the template data
 */
function achievements_hook_call_rest()
 {
    // defined the variables
    global $var_set;
     global $return_set;
    
     if ( isset( $_POST['get_data'] ) )
         {
        hook_variable_set();
        
         $rest_api = new Rest_Api();
         $ach_data = $rest_api -> request( $var_set['url'], $var_set['app_key'], $var_set['host'] );
         print_r( $ach_data );
        
         $return_set['char_achievements'] = json_decode( $ach_data );
         } 
    } 

/**
 * Global Hook to return global variables which contains
 * the content to use in the smarty templates extracted from 
 * the database
 * 
 * @return $return_set global array returns the template data
 */
function achievements_hook_get_db()
 {
    global $return_set;
    
     $db = new DBLayer( 'lib' );
    
     // getting content for selecting characters
    $sth = $db -> selectWithParameter( 'UserCharacter', 'ams_api_keys', array( 'User' => $_SESSION['user'] ) , 'User = :User' );
     $row = $sth -> fetchAll();
     $retur_set['Character'] = $row;
    
     } 

/**
 * Global Hook to return global variables which contains
 * the content to use in the smarty templates
 * 
 * @return $return_set global array returns the template data
 */
function achievements_hook_return_global()
 {
    global $return_set;
     return $return_set;
    
     } 
