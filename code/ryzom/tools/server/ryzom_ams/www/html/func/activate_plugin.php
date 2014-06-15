<?php
/**
 * This function is used in activating plugins.
 * 
 * @author Shubham Meena, mentored by Matthew Lagoe 
 */
function activate_plugin() {
    
    // if logged in
    if ( WebUsers :: isLoggedIn() ) {
        
        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to delete
            $id = filter_var( $_GET['id'], FILTER_SANITIZE_FULL_SPECIAL_CHARS );
             $db = new DBLayer( 'lib' );
             $result = $db -> update( "plugins", array( 'Status' => '1' ), "Id = $id" );
             if ( $result )
             {
                header( "Location: index.php?page=plugins&result=3" );
                 exit;
                 } 
            else
                 {
                header( "Location: index.php?page=plugins&result=4" );
                 exit;
                 } 
            } 
        else
             {
            header( "Location: index.php?page=plugins&result=4" );
             exit;
             } 
        } 
    }
