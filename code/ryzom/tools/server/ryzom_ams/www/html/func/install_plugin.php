<?php
/**
 * This function is used in installing plugins
 * It performs validation check for the compressed plugin
 * then extract in plugin folder to get the info
 * 
 * @author Shubham Meena, mentored by Matthew Lagoe 
 */
function install_plugin() {
    
    $result = array();
    
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
            
             // scanning plugin folder if plugin with same name is already exists or not
            $x = checkForUpdate( $dir, $destination, $fileTmpLoc, $temp_path );
             if ( $x == '1' )
             {
                echo "update found";
                 exit();
                 } 
            else if ( $x == '2' )
             {
                echo "Plugin already exists with same name .";
                 exit();
                 } 
            else if ( $x == '3' )
             {
                echo "Update info is not present in the update";
                 exit();
                 } 
            
            
            // checking for the command to install plugin is given or not
            if ( !isset( $_POST['install_plugin'] ) )
                 {
                if ( ( $_FILES["file"]["type"] == 'application/zip' ) )
                     {
                    if ( move_uploaded_file( $fileTmpLoc, $temp_path . "/" . $fileName ) ) {
                        echo "$fileName upload is complete.</br>" . "<button type='submit' class='btn btn-primary' style='margin-left:5px; margin-top:10px;' name='install_plugin'>Install Plugin</button></br>";
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
                        $result = readPluginFile( ".info", $target_path );
                        
                         // sending all info to the database
                        $install_result = array();
                         $install_result['FileName'] = $target_path;
                         $install_result['Name'] = $result['PluginName'];
                         $install_result['Type'] = $result['Type'];
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

/**
 * function to check for updates or 
 * if the same plugin already exists
 * also, if the update founds ,check for the update info in the .info file. 
 * Update is saved in the temp direcotry with pluginName_version.zip
 * 
 * @param  $fileName file which is uploaded in .zip extension
 * @param  $findPath where we have to look for the installed plugins
 * @param  $tempFile path for the temporary file
 * @param  $tempPath path where we have to store the update
 * @return 2 if plugin already exists and update not found
 * @return 3 if update info tag not found in .info file
 */
function checkForUpdate( $fileName, $findPath, $tempFile, $tempPath )
 {
    // check for plugin if exists
    $file = scandir( $findPath );
     foreach( $file as $key => $value )
     {
        if ( strcmp( $value, $fileName ) == 0 )
             {
            if ( !file_exists( $tempPath . "/test" ) )
                 {
                mkdir( $tempPath . "/test" );
                 } 
            if ( zipExtraction( $tempFile, $tempPath . "/test/" ) )
                 {
                $result = readPluginFile( ".info", $tempPath . "/test/" . $fileName );
                
                 // check for the version for the plugin
                $db = new DBLayer( "lib" );
                 $sth = $db -> select( "plugins", array( ':name' => $result['PluginName'] ), "Name = :name" );
                 $info = $sth -> fetch();
                 $info['Info'] = json_decode( $info['Info'] );
                
                 // the two versions from main plugin and the updated part
                $new_version = explode( '.', $result['Version'] );
                 $pre_version = explode( '.', $info['Info'] -> Version );
                
                 // For all plugins we have used semantic versioning
                // Format: X.Y.Z ,X->Major, Y->Minor, Z->Patch
                // change in the X Y & Z values refer the type of change in the plugin.
                // for initial development only Minor an Patch MUST be 0.
                // if there is bug fix then there MUST be an increment in the Z value.
                // if there is change in the functionality or addition of new functionality
                // then there MUST be an increment in the Y value.
                // When there is increment in the X value , Y and Z MUST be 0.
                // comparing if there is some change
                if ( !array_intersect( $new_version , $pre_version ) )
                     {
                    // removing the uploaded file
                    Plugincache :: rrmdir( $tempPath . "/test/" . $fileName );
                     return '2';
                     } 
                else
                     {
                    // check for update info if exists
                    if ( !array_key_exists( 'UpdateInfo', $result ) )
                         {
                        return '3'; //update info tag not found
                         } 
                    else
                         {
                        // storing update in the temp directory
                        // format of update save
                        if ( move_uploaded_file( $tempFile, $tempPath . "/" . trim( $fileName, ".zip" ) . "_" . $result['Version'] . ".zip" ) ) {
                            // setting update information in the database
                            $dbr = new DBLayer( "lib" );
                             $update['PluginId'] = $info['Id'];
                             $update['UpdatePath'] = $tempPath . "/" . trim( $fileName, ".zip" ) . "_" . $result['Version'] . ".zip";
                             $update['UpdateInfo'] = json_encode( $result );
                             $dbr -> insert( "updates", $update );
                             header( "Location: index.php?page=plugins&result=7" );
                             exit;      
                             } 
                        } 
                    } 
                } 
            } 
        } 
    }
