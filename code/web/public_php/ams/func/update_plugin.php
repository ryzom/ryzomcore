<?php
/**
 * This function is used in installing updates for plugins.
 * It takes id of the plugin whose update is available using
 * $_GET global variable and then extract the update details
 * from db and then install it in the plugin.
 *
 * @author Shubham Meena, mentored by Matthew Lagoe
 */
function update_plugin() {

    // if logged in
    if ( WebUsers :: isLoggedIn() ) {

        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to update
            $id = filter_var( $_GET['id'], FILTER_SANITIZE_FULL_SPECIAL_CHARS );
             $db = new DBLayer( 'lib' );
             $sth = $db -> executeWithoutParams( "SELECT * FROM plugins INNER JOIN updates ON plugins.Id=updates.PluginId Where plugins.Id=$id" );
             $row = $sth -> fetch();

             // replacing update in the  database
            Plugincache :: rrmdir( $row['FileName'] );
             Plugincache :: zipExtraction( $row['UpdatePath'], rtrim( $row['FileName'], strtolower( $row['Name'] ) ) );

             $db -> update( "plugins", array( 'Info' => $row['UpdateInfo'] ), "Id=$row[Id]" );

             // deleting the previous update
            $db -> delete( "updates", array( 'id' => $row['s.no'] ), "s.no=:id" );

             // if update is installed succesffully redirect to show success message
             header( "Location: index.php?page=plugins&result=8" );
             die();

             }
        }
    }
