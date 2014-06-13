<?php
/**
 * This function is used in deleting plugins.
 * 
 * It removes the plugin from the codebase.
 * 
 * @author Shubham Meena, mentored by Matthew Lagoe 
 */
function delete_plugin() {
    
    // if logged in
    if ( WebUsers :: isLoggedIn() ) {
        
        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to delete after filtering
            $id = filter_var( $_GET['id'], FILTER_SANITIZE_FULL_SPECIAL_CHARS );
            
             $db = new DBLayer( 'lib' );
             $sth = $db -> selectWithParameter( "FileName", "plugins", array( 'id' => $id ), "Id=:id" );
             $name = $sth -> fetch();
            
             if ( is_dir( "$name[FileName]" ) )
                 {
                // removing plugin directory from the code base
                if ( rrmdir( "$name[FileName]" ) )
                     {
                    $db -> delete( 'plugins', array( 'id' => $id ), "Id=:id" );
                    
                     header( "Location: index.php?page=plugins&result=2" );
                     exit;
                    
                     } 
                else
                     {
                    header( "Location: index.php?page=plugins&result=0" );
                     exit;
                     } 
                } 
            } 
        else
             {
            header( "Location: index.php?page=plugins&result=0" );
             exit;
             } 
        } 
    } 

/**
 * function to remove  a non empty directory
 * 
 * @param  $dir directory address
 * @return boolean 
 */
function rrmdir( $dir ) {
    if ( is_dir( $dir ) ) {
        $objects = scandir( $dir );
         foreach ( $objects as $object ) {
            if ( $object != "." && $object != ".." ) {
                if ( filetype( $dir . "/" . $object ) == "dir" ) rmdir( $dir . "/" . $object );
                else unlink( $dir . "/" . $object );
                 } 
            } 
        reset( $objects );
         return rmdir( $dir );
         } 
    } 

