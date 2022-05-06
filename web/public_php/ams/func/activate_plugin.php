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

    // if logged in
    if ( WebUsers :: isLoggedIn() ) {

        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to activate
            $id = filter_var( $_GET['id'], FILTER_SANITIZE_FULL_SPECIAL_CHARS );
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
