<?php

/**
 * Global and Local Hooks for the API key Management plugin
 * Global Hooks are defined with the prefix(name of the plugin)
 * Local Hooks are defined with normal function name 
 * 
 * All the Global Hooks are called during the page load
 * and Local Hooks are called according to conditions
 * 
 * @author shubham meena mentored by Matthew Lagoe 
 */

// Global variable to store the data which is
// returned to the templates
$return_set = array();

// Local variable to store data during
// functionalities of the hooks
$var_set = array();

/**
 * Display hook for api key management
 */
function api_key_management_hook_display()
 {
    global $return_set;
     // to display plugin name in menu bar
    $return_set['menu_display'] = 'API Key Management';
     } 

/**
 * Local Hook to validate the posted data
 */
function hook_validate( $var )
 {
    if ( isset( $var ) && !empty( $var ) )
         {
        return true;
         } 
    else
         {
        return false;
         } 
    } 

/**
 * Local Hook to set the POST variables and validate them
 */
function hook_variables()
 {
    global $var_set;
     global $return_set;
    
     if ( hook_validate( $_POST['expDate'] ) && hook_validate( $_POST['sp_name'] ) && hook_validate( $_POST['api_type'] )
             && hook_validate( $_POST['character_name'] ) )
         {
        $var_set['ExpiryDate'] = $_POST['expDate'];
         $var_set['FrName'] = $_POST['sp_name'];
         $var_set['UserType'] = $_POST['api_type'];
         $var_set['UserCharacter'] = $_POST['character_name'];
         $var_set['User'] = $_SESSION['user'];
         $var_set['AddedOn'] = date( "Y-m-d H:i:s" );
         $var_set['Items'] = '';
         $return_set['gen_key_validate'] = 'true';
         } 
    else
         {
        $return_set['gen_key_validate'] = 'false';
         } 
    } 

/**
 * Global Hook to create table of the API_key_management
 * if not created.
 * Contains the sql code
 */
function api_key_management_hook_create_tb()
 {
    $dbl = new DBLayer( "lib" );
     $sql = "
                        --
                        -- Database: `ryzom_ams_lib`
                        --

                        -- --------------------------------------------------------

                        --
                        -- Table structure for table `ams_api_keys`
                        --

                        CREATE TABLE IF NOT EXISTS `ams_api_keys` (
                          `SNo` int(10) NOT NULL AUTO_INCREMENT,
                          `User` varchar(50) COLLATE utf8_unicode_ci DEFAULT NULL,
                          `FrName` varchar(50) COLLATE utf8_unicode_ci DEFAULT NULL,
                          `UserType` varchar(10) COLLATE utf8_unicode_ci DEFAULT NULL,
                          `UserCharacter` varchar(50) COLLATE utf8_unicode_ci DEFAULT NULL,
                          `ExpiryDate` date DEFAULT NULL,
                          `AccessToken` text COLLATE utf8_unicode_ci DEFAULT NULL,
                          `AddedOn` datetime DEFAULT NULL,
                          `Items` text COLLATE utf8_unicode_ci,
                          PRIMARY KEY (`SNo`),
                          KEY `User` (`User`)
                        ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

                        --
                        -- Constraints for table `ams_api_keys`
                        --
                        ALTER TABLE `ams_api_keys`
                          ADD CONSTRAINT `ams_api_keys_ibfk_1` FOREIGN KEY (`User`) REFERENCES `ryzom_ams`.`ams_user` (`Login`);";
    
     $dbl -> executeWithoutParams( $sql );
     } 

/**
 * Hook to store data to database which is sent as post 
 * method from the forms in this plugin
 * It also calls the local hook
 */
function api_key_management_hook_store_db()
 {
    global $var_set;
     global $return_set;
    
     // if the form been submited move forward
    if ( @hook_validate( $_POST['gen_key'] ) ) {
        
        // local hook to validate the POST variables
        hook_variables();
        
         // if validation successfull move forward
        if ( $return_set['gen_key_validate'] == 'true' && $_GET['plugin_action'] == 'generate_key' )
         {
            // this part generated the access token
            include 'generate_key.php';
             $var_set['AccessToken'] = generate_key :: randomToken( 56, false, true, false );
            
             // database connection
            $db = new DBLayer( 'lib' );
             // insert the form data to the database
            $db -> insert( 'ams_api_keys', $var_set );
            
             // redirect to the the main page with success code
            // 1 refers to the successfull addition of key to the database
            header( "Location: index.php?page=layout_plugin&&name=API_key_management&&success=1" );
             exit;
             } 
        } 
    } 

/**
 * Global Hook to load the data from db and set it 
 * into the global array to return it to the template
 */
function api_key_management_hook_load_db()
 {
    global $var_set;
     global $return_set;
    
     $db = new DBLayer( 'lib' );
    
     if ( isset( $_SESSION['user'] ) )
         {
        // returns the registered keys
        $sth = $db -> select( 'ams_api_keys', array( 'user' => $_SESSION['user'] ), 'User = :user' );
         $row = $sth -> fetchAll();
         $return_set['api_keys'] = $row;
        
         // fetch the character from the array to compare
        $com = array_column( $return_set['api_keys'], 'UserCharacter' );
        
         // returns the characters with respect to the user id in the ring_tool->characters
        $db = new DBLayer( 'ring' );
         $sth = $db -> selectWithParameter( 'char_name', 'characters' , array(), '1' );
         $row = $sth -> fetch();
        
         // loop through the character list and remove the character if already have an api key
        $return_set['characters'] = array_diff( $row, $com );
         } 
    } 

/**
 * Global Hook to update or delete the data from db
 */
function api_key_management_hook_update_db()
 {
    global $var_set;
     global $return_set;
    
     $db = new DBLayer( 'lib' );
     if ( isset( $_GET['delete_id'] ) )
         {
        // removes the registered key using get variable which contains the id of the registered key
        $db -> delete( 'ams_api_keys', array( 'SNo' => $_GET['delete_id'] ), 'SNo = :SNo' );
        
         // redirecting to the API_key_management plugins template with success code
        // 2 refers to the succssfull delete condition
        header( "Location: index.php?page=layout_plugin&&name=API_key_management&&success=2" );
         exit;
         } 
    } 

/**
 * Global Hook to return global variables which contains
 * the content to use in the smarty templates
 * 
 * @return $return_set global array returns the template data
 */
function api_key_management_hook_return_global()
 {
    global $return_set;
     return $return_set;
     }
