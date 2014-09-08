<?php

/**
 * Global and Local Hooks for the Domain_Management plugin
 * Global Hooks are defined with the prefix(name of the plugin)
 * Local Hooks are defined with normal function name 
 * 
 * All the Global Hooks are called during the page load
 * and Local Hooks are called according to conditions
 * 
 * Here, we request to the Domain_Management url using REST 
 * to get the contents and will display with this plugin.
 * 
 * @author shubham meena mentored by Matthew Lagoe 
 */


// Global variable to store the data which is
// returned to the templates
$domain_management_return_set = array();

// Local variable to store data during
// functionalities of the hooks
$var_set = array();

/**
 * Display hook for Domain_Management plugin
 */
function domain_management_hook_display()
 {
    global $domain_management_return_set;
     // to display plugin name in menu bar
    $domain_management_return_set['admin_menu_display'] = 'Domain Management';
    $domain_management_return_set['icon'] = 'icon-edit';
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
function domain_management_get_db_content( $data )
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
function domain_management_get_char_id( $data )
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
function domain_management_get_player_stat( $data )
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
function domain_management_variable_set()
 {
    global $domain_management_return_set;
     global $var_set;
     if ( isset( $_POST['Character'] ) && !empty( $_POST['Character'] ) )
         {
        $var_set['character'] = $_POST['Character'];
        
         // get char id from ring_open table
        if ( $var_set['character'] != 'All Characters' )
         {
            $var_set['char_id'] = domain_management_get_char_id( $var_set['character'] );
            
             } 
        
        // get db content for variable set
        $row = domain_management_get_db_content( array( 'User' => $_SESSION['user'], 'UserCharacter' => $var_set['character'] ) );
        
         // access key automatically taken from the database wrt user and character
        @$var_set['app_key'] = $row['AccessToken'];
        
         // here you can set the host where this plugin is set
        $var_set['host'] = 'localhost';
        
         // here we get the stats of the character
        $ref_set = domain_management_get_player_stat( $var_set['character'] );
        
         // here we have set items that are required to get the achivements
        // these are player stats from webig->players table
        @$var_set['items'] = json_encode( array( 'dev_shard' => $ref_set['dev_shard'] , 'name' => $ref_set['name'] , 'cid' => $ref_set['cid'] , 'lang' => 'en' , 'translater_mode' => '', 'last_played_date' => $ref_set['last_login'] ) );
        
         // url where we have to make request for domain_management
        // it sends get parameter search(what to search) and format(in which format data exchange takes place)
        $var_set['url'] = 'http://localhost6/?search=domain_management&&format=json';
         } 
    else
         {
        $domain_management_return_set['no_char'] = "Please Generate key for a character before requesting for domain_management";
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
 * @return $domain_management_return_set global array returns the template data
 */
function domain_management_hook_call_rest()
 {

    global $domain_management_return_set;
    global $WEBPATH;
    
    $domain_management_return_set['path'] = $WEBPATH;

    } 

/**
 * Global Hook to return global variables which contains
 * the content to use in the smarty templates extracted from 
 * the database
 * 
 * @return $domain_management_return_set global array returns the template data
 */
function domain_management_hook_get_db()
 {
    global $domain_management_return_set;
    
        $db = new DBLayer( 'shard' );
        
        //get all domains
        $statement = $db->executeWithoutParams("SELECT * FROM domain");
        $rows = $statement->fetchAll();   
        $domain_management_return_set['domains'] = $rows;

        if (isset($_GET['edit_domain'])){
        //get permissions
        $statement = $db->executeWithoutParams("SELECT * FROM `permission` WHERE `DomainId` = '".$rows[$_GET['edit_domain']-1]['domain_name']."'");
        $rows = $statement->fetchAll();   
        $domain_management_return_set['permissions'] = $rows;
        
        //get all users
        $pagination = new Pagination(WebUsers::getAllUsersQuery(),"web",10,"WebUsers");
        $domain_management_return_set['userlist'] = Gui_Elements::make_table($pagination->getElements() , Array("getUId","getUsername","getEmail"), Array("id","username","email"));
        
        }
        
        return $rows;
    } 

/**
 * Global Hook to return global variables which contains
 * the content to use in the smarty templates
 * 
 * @return $domain_management_return_set global array returns the template data
 */
function domain_management_hook_return_global()
 {
    global $domain_management_return_set;
     return $domain_management_return_set;
     }
