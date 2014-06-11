<?php

/**
 * function plugins to get
 * plugins from the Database using pagination object
 * 
 * @author shubham meena mentored by Matthew Lagoe 
 */

function plugins()
 {
    if ( Ticket_User :: isMod( unserialize( $_SESSION['ticket_user'] ) ) ) {
        
        $pagination = new Pagination( "SELECT * FROM plugins", "lib", 5, "Plugincache" );
         $pageResult['plug'] = Gui_Elements :: make_table( $pagination -> getElements(), Array( "getId", "getPluginName", "getPluginType", "getPluginPermission", "getPluginStatus", "getPluginInfo" ), Array( "id", "plugin_name", "plugin_type", "plugin_permission", "plugin_status", "plugin_info" ) );
         $pageResult['links'] = $pagination -> getLinks( 5 );
         $pageResult['lastPage'] = $pagination -> getLast();
         $pageResult['currentPage'] = $pagination -> getCurrent();
        
         global $INGAME_WEBPATH;
         $pageResult['ingame_webpath'] = $INGAME_WEBPATH;
        
         // check if shard is online
        try {
            $dbs = new DBLayer( "shard" );
             $pageResult['shard'] = "online";
             } 
        catch( PDOException $e ) {
            $pageResult['shard'] = "offline";
             } 
        return( $pageResult );
         } else {
        // ERROR: No access!
        $_SESSION['error_code'] = "403";
         header( "Location: index.php?page=error" );
         exit;
         } 
    
    }
