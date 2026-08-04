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
 * True when the current request comes from a logged in admin. The hooks in
 * this file are called for every request, so anything that writes has to ask.
 */
function domain_management_caller_is_admin()
 {
    if ( !isset( $_SESSION['ticket_user'] ) ) return false;
     return Ticket_User :: isAdmin( unserialize( $_SESSION['ticket_user'] ) );
     }

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

     // This hook runs on every page load, including for visitors who are not
    // logged in, and it rewrites the domain row -- patch urls and login
    // address included. Only an admin may do that. Note the comparisons
    // below were assignments, so the flag was always considered set.
    if ( domain_management_caller_is_admin()
         && isset( $_GET['ModifyDomain'] ) && $_GET['ModifyDomain'] == '1' && isset($_POST['domain_name'])) {
        try {

            $dbs = new DBLayer( 'shard' );
            $dbs->update("domain", Array(
                'domain_name' => $_POST['domain_name'],
                'status' => $_POST['status'],
                'patch_version' => $_POST['patch_version'],
                'backup_patch_url' => $_POST['backup_patch_url'],
                'patch_urls' => $_POST['patch_urls'],
                'login_address' => $_POST['login_address'],
                'session_manager_address' => $_POST['session_manager_address'],
                'ring_db_name' => $_POST['ring_db_name'],
                'web_host' => $_POST['web_host'],
                'web_host_php' => $_POST['web_host_php'],
                'description' => $_POST['description'],
            ), 'domain_id = :domain_id', array('domain_id' => (int)$_GET['edit_domain']));

            }
        catch ( Exception $e ) {
            return null;
             }
        }     
        
        if ( domain_management_caller_is_admin()
             && isset( $_GET['ModifyPermission'] ) && $_GET['ModifyPermission'] == '1' && isset($_POST['user'])) {
        try {
        
            $dbl = new DBLayer("lib");
        
            $statement = $dbl->execute("SELECT * FROM `settings` WHERE `Setting` = :setting", Array('setting' => 'Domain_Auto_Add'));
            $json = $statement->fetch();
            $json = json_decode($json['Value'],true);
            
            $domainKey = (int)$_GET['edit_domain'];
            $json[$domainKey]['1'] = $_POST['user'];
            $json[$domainKey]['2'] = $_POST['moderator'];
            $json[$domainKey]['3'] = $_POST['admin'];   
            
            $update = json_encode($json);

            $dbl->update("settings", Array( 'Value' => $update), "Setting = :Setting", array('Setting' => 'Domain_Auto_Add'));

            }
        catch ( Exception $e ) {
            return null;
             }
        }

    try {

        $db = new DBLayer( 'shard' );

         // get all domains
        $statement = $db -> executeWithoutParams( "SELECT * FROM domain" );
         $rows = $statement -> fetchAll();
         $domain_management_return_set['domains'] = $rows;

         if ( isset( $_GET['edit_domain'] ) ) {
            // get permissions
            $statement = $db -> execute( "SELECT * FROM `domain` WHERE `domain_id` = :domain_id", array('domain_id' => intval($_GET['edit_domain'])) );
             $rows = $statement -> fetchAll();
             $domain_management_return_set['domains'] = $rows;

             $statement = $db -> execute( "SELECT * FROM `permission` WHERE `DomainId` = :domain_id", array('domain_id' => intval($_GET['edit_domain'])) );
             $rows = $statement -> fetchAll();
             $domain_management_return_set['permissions'] = $rows;

             // get all users
            $pagination = new Pagination( WebUsers :: getAllUsersQuery(), "web", 10, "WebUsers" );
             $domain_management_return_set['userlist'] = Gui_Elements :: make_table( $pagination -> getElements() , Array( "getUId", "getUsername", "getEmail" ), Array( "id", "username", "email" ) );

            $dbl = new DBLayer("lib");
        
            $statement = $dbl->execute("SELECT * FROM `settings` WHERE `Setting` = :setting", Array('setting' => 'Domain_Auto_Add'));
            $json = $statement->fetch();
            $json = json_decode($json['Value'],true);
            
            $domain_management_return_set['Domain_Auto_Add'] = $json[$_GET['edit_domain']];
                         
             }

        return $rows;

         }
    catch ( Exception $e ) {
        return null;
         }
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

     
function domain_management_hook_activate()
 {
    $dbl = new DBLayer( "lib" );
     $sql = "INSERT INTO `settings` (Setting, Value) 
            SELECT 'Domain_Auto_Add', 0 FROM DUAL
            WHERE NOT EXISTS 
            (SELECT Setting FROM settings WHERE Setting='Domain_Auto_Add');";

     $dbl -> executeWithoutParams( $sql );
     }     