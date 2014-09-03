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

    // if logged in
    if ( WebUsers :: isLoggedIn() ) {


        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to deactivate
            $id = filter_var( $_GET['id'], FILTER_SANITIZE_FULL_SPECIAL_CHARS );
             $db = new DBLayer( 'lib' );
             $result = $db -> update( "plugins", array( 'Status' => '0' ), "Id = $id" );
             if ( $result )
             {
				// if result is successfull it redirects and shows success message
                header( "Location: index.php?page=plugins&result=5" );
                 die();
                 }
            else
                 {
				// if result is unsuccessfull it redirects and shows success message
                header( "Location: index.php?page=plugins&result=6" );
                 die();

                 }
            }
        else
             {
			//if $_GET variable is not set it redirects and shows error
            header( "Location: index.php?page=plugins&result=6" );
             die();
             }
        }
    }
