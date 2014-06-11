<?php
/**
 * This function is used in installing plugins
 * It performs validation check for the compressed plugin
 * then extract in plugin folder to get the info
 * 
 * @author Shubham Meena, mentored by Matthew Lagoe 
 */
function install_plugin() {
    
    // if logged in
    if ( WebUsers :: isLoggedIn() ) {
        
        if ( ( isset( $_FILES["file"] ) ) && ( $_FILES["file"]["size"] > 0 ) && ( $_FILES["file"]["type"] == 'application/zip' ) )
             {
            $fileName = $_FILES["file"]["name"]; //the files name takes from the HTML form
             $fileTmpLoc = $_FILES["file"]["tmp_name"]; //file in the PHP tmp folder
             $dir = trim( $_FILES["file"]["name"], ".zip" );
             $target_path = "../../ams_lib/plugins/$dir"; //path in which the zip extraction is to be done
             $destination = "../../ams_lib/plugins/";
            
             if ( move_uploaded_file( $fileTmpLoc, $destination . $fileName ) ) {
                // zip object to handle zip archieves
                $zip = new ZipArchive();
                 $x = $zip -> open( $destination . $fileName );
                 if ( $x === true ) {
                    $zip -> extractTo( $destination ); // change this to the correct site path
                     $zip -> close();
                    
                     // removing the uploaded zip file
                    unlink( $destination . $fileName );
                    
                     // check for the info file
                    if ( file_exists( $target_path . "/.info" ) )
                         {
                        // read the details of the plugin through the info file
                        $file_handle = fopen( $target_path . "/.info", "r" );
                         $result = array();
                         while ( !feof( $file_handle ) ) {
                            
                            $line_of_text = fgets( $file_handle );
                             $parts = array_map( 'trim', explode( '=', $line_of_text, 2 ) );
                             @$result[$parts[0]] = $parts[1];
                            
                             } 
                        
                        fclose( $file_handle );
                        
                         // sending all info to the database
                        $install_result = array();
                         $install_result['FileName'] = $target_path;
                         $install_result['Name'] = $result['PluginName'];
                         $install_result['Type'] = $result['type'];
                        
                         if ( Ticket_User :: isMod( unserialize( $_SESSION['ticket_user'] ) ) )
                             {
                            $install_result['Permission'] = 'admin';
                             } 
                        else
                             {
                            $install_result['Permission'] = 'user';
                             } 
                        
                        $install_result['Info'] = json_encode( $result );
                        
                         // connection with the database
                        $dbr = new DBLayer( "lib" );
                         $dbr -> insert( "plugins", $install_result );
                        
                         header( "Location: index.php?page=plugins&result=1" );
                         exit;
                        
                         } 
                    else
                         {
                        rmdir( $target_path );
                         header( "Location: index.php?page=install_plugin&result=2" );
                         exit;
                        
                         } 
                    
                    } 
                } 
            
            header( "Location: index.php?page=plugins" );
             exit;
             } 
        else
             {
            header( "Location: index.php?page=install_plugin&result=1" );
             exit;
             } 
        } 
    }
