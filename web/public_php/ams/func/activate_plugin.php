<?php
/**
 * This function is used in activating plugins.
 * This can be done by providing id using $_GET global variable of the plugin which
 * we want to activate. After getting id we update the respective plugin with status
 * activate which here means '1' .
 *
 * @author Shubham Meena, mentored by Matthew Lagoe
 */
function activate_plugin() {

    // only the staff that can reach the plugin page may act on plugins
    // Enabling a plugin runs its hooks on every request; admin only.
    if ( WebUsers :: isLoggedIn() && Ticket_User :: isAdmin( unserialize( $_SESSION['ticket_user'] ) ) ) {

        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to activate -- this goes into the WHERE clause
            // unquoted, so it has to be a number and nothing else
            $id = intval( $_GET['id'] );
             $db = new DBLayer( 'lib' );
             $result = $db -> update( "plugins", array( 'Status' => '1' ), "Id = $id" );
             if ( $result )
             {
				 // if result is successfull it redirects and shows success message
                header("Cache-Control: max-age=1");
                header( "Location: index.php?page=plugins&result=3" );
                 throw new SystemExit();
                 }
            else
                 {
				//if result is unsuccessfull it redirects and throws error
                header("Cache-Control: max-age=1");
                header( "Location: index.php?page=plugins&result=4" );
                 throw new SystemExit();
                 }
            }
        else
             {
			//if $_GET variable is not set it redirects and shows error
                header("Cache-Control: max-age=1");
            header( "Location: index.php?page=plugins&result=4" );
             throw new SystemExit();
             }
        }
    }
