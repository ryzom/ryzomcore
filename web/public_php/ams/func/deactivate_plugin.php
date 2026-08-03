<?php
/**
 * This function is used in deactivating plugins.
 * This can be done by providing id using $_GET global variable of the plugin which
 * we want to activate. After getting id we update the respective plugin with status
 * deactivate which here means '0'.
 *
 * @author Shubham Meena, mentored by Matthew Lagoe
 */
function deactivate_plugin() {

    // only the staff that can reach the plugin page may act on plugins
    if ( WebUsers :: isLoggedIn() && Ticket_User :: isMod( unserialize( $_SESSION['ticket_user'] ) ) ) {


        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to deactivate -- this goes into the WHERE clause
            // unquoted, so it has to be a number and nothing else
            $id = intval( $_GET['id'] );
             $db = new DBLayer( 'lib' );
             $result = $db -> update( "plugins", array( 'Status' => '0' ), "Id = $id" );
             if ( $result )
             {
				// if result is successfull it redirects and shows success message
                header("Cache-Control: max-age=1");
                header( "Location: index.php?page=plugins&result=5" );

                 throw new SystemExit();
                 }
            else
                 {
				// if result is unsuccessfull it redirects and shows success message
                header("Cache-Control: max-age=1");
                header( "Location: index.php?page=plugins&result=6" );
                 throw new SystemExit();

                 }
            }
        else
             {
			//if $_GET variable is not set it redirects and shows error
                header("Cache-Control: max-age=1");
            header( "Location: index.php?page=plugins&result=6" );
             throw new SystemExit();
             }
        }
    }
