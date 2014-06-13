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
        
        // path of temporary folder for storing files
        $temp_path = "../../ams_lib/temp";
        
         // create a temp directory if not exist
        // temp folder where we first store all uploaded plugins before install
        if ( !file_exists( "$temp_path" ) )
             {
            mkdir( $temp_path );
             } 
        
        // checking the server if file is uploaded or not
        if ( ( isset( $_FILES["file"] ) ) && ( $_FILES["file"]["size"] > 0 ) )
             {
            $fileName = $_FILES["file"]["name"]; //the files name takes from the HTML form
             $fileTmpLoc = $_FILES["file"]["tmp_name"]; //file in the PHP tmp folder
             $dir = trim( $_FILES["file"]["name"], ".zip" );
             $target_path = "../../ams_lib/plugins/$dir"; //path in which the zip extraction is to be done
             $destination = "../../ams_lib/plugins/";
            
             // checking for the command to install plugin is given or not
            if ( !isset( $_POST['install_plugin'] ) )
                 {
                if ( ( $_FILES["file"]["type"] == 'application/zip' ) )
                     {
                    if ( move_uploaded_file( $fileTmpLoc, $temp_path . "/" . $fileName ) ) {
                        echo "$fileName upload is complete.";
                         exit();
                         } 
                    else
                         {
                        echo "Error in uploading file.";
                         exit();
                         } 
                    } 
                else
                     {
                    echo "Please select a file with .zip extension to upload.";
                     exit();
                     } 
                } 
            else
                 {
                
                // calling function to unzip archives
                if ( zipExtraction( $temp_path . "/" . $fileName , $destination ) )
                     {
                    if ( file_exists( $target_path . "/.info" ) )
                         {
                        $result = array();
                         $result = readPluginFile( ".info", $target_path );
                        
                         // sending all info to the database
                        $install_result = array();
                         $install_result['FileName'] = $target_path;
                         $install_result['Name'] = $result['PluginName'];
                         // $install_result['Type'] = $result['type'];
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
                        
                         // if everything is successfull redirecting to the plugin template
                        header( "Location: index.php?page=plugins&result=1" );
                         exit;
                         } 
                    else
                         {
                        // file .info not exists
                        rmdir( $target_path );
                         header( "Location: index.php?page=install_plugin&result=2" );
                         exit;
                         } 
                    
                    } else
                     {
                    // extraction failed
                    header( "Location: index.php?page=install_plugin&result=0" );
                     exit;
                     } 
                } 
            } 
        else
             {
            echo "Please Browse for a file before clicking the upload button";
             exit();
             } 
        } 
    } 

/**
 * function to unzip the zipped files
 * 
 * @param  $target_path path to the target zipped file
 * @param  $destination path to the destination
 * @return boolean 
 */
function zipExtraction( $target_path, $destination )
 {
    $zip = new ZipArchive();
     $x = $zip -> open( $target_path );
     if ( $x === true ) {
        if ( $zip -> extractTo( $destination ) )
             {
            $zip -> close();
             return true;
             } 
        else
             {
            $zip -> close();
             return false;
             } 
        } 
    } 

/**
 * function to read text files and extract
 * the information into an array
 * 
 * -----------------------------------------------------------
 * format:
 * -----------------------------------------------------------
 * PluginName = Name of the plugin 
 * Version = version of the plugin
 * Type = type of the plugin
 * Description = Description of the plugin ,it's functionality
 * -----------------------------------------------------------
 * 
 * reads only files with name .info
 * 
 * @param  $fileName file to read
 * @param  $targetPath path to the folder containing .info file
 * @return array containing above information in array(value => key)
 */
function readPluginFile( $fileName, $target_path )
 {
    $file_handle = fopen( $target_path . "/" . $fileName, "r" );
     $result = array();
     while ( !feof( $file_handle ) ) {
        $line_of_text = fgets( $file_handle );
         $parts = array_map( 'trim', explode( '=', $line_of_text, 2 ) );
         @$result[$parts[0]] = $parts[1];
         } 
    fclose( $file_handle );
     return $result;
     }
