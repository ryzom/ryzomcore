<?php

/**
 * This module contains the function to install plugins
 * or check if the uploaded file is an update for a plugin.
 *
 * When user uploads a file with .zip extension(neccessary requirement)
 * steps that should perform:
 * --> Check if the file type is .zip.
 * --> Extract it to a temp folder.
 * --> Check for the .info file. If not exists throw error
 * --> Extract the information from the .info file.
 * --> Check for the plugin name already exists or not.
 * --> if Plugin Name exists it compare the version of .info and version of plugin stored in db.
 * --> if same throw error and if different it checks for UpdateInfo field in .info file.
 * --> if UpdateInfo not found throw error.
 * --> if UpdateInfo found add the update to the ryzom_ams_lib.updates table.
 * --> if it's not an update and plugin with same name already exists throw error.
 * --> if plugin with same name not present provide option to install plugin
 *
 * @author Shubham Meena, mentored by Matthew Lagoe
 *
 */


/**
 * Reduce an uploaded archive name to a plain "<name>.zip" that is safe to
 * append to a path. The name is supplied by the browser and is used to build
 * both the temp file and the plugin directory, so a directory part or a ".."
 * in it would let the upload be written anywhere the web server can reach.
 *
 * @param  $name the untrusted file name
 * @return the sanitised name, or false when it is not a usable .zip name
 */
function plugin_archive_name( $name ) {
    $name = basename( str_replace( "\\", "/", (string)$name ) );

     if ( !preg_match( '/^([A-Za-z0-9._-]+)\.zip$/i', $name, $matches ) ) {
        return false;
         }
    // no leading dots, so the name can never be "." ".." or a hidden file
    if ( $matches[1] === '' || $matches[1][0] === '.' || strpos( $matches[1], '..' ) !== false ) {
        return false;
         }
    return $matches[1] . '.zip';
    }

/**
 * This function is used in installing plugins or adding updates
 * for previously installed plugins.
 *
 */
function install_plugin() {

    $result = array();

     // Installing a plugin puts new php code on the server, so this is staff
    // only -- the same permission that guards the plugin page itself.
    if ( WebUsers :: isLoggedIn() && Ticket_User :: isMod( unserialize( $_SESSION['ticket_user'] ) ) ) {

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
            // the name comes from the browser: strip any directory part and
            // anything that is not a plain plugin archive name, otherwise the
            // upload below can be made to land anywhere on the server
            $fileName = plugin_archive_name( $_FILES["file"]["name"] );
             if ( $fileName === false )
                 {
                echo "Please select a file with .zip extension to upload.";
                 throw new SystemExit();
                 }
            $fileTmpLoc = $_FILES["file"]["tmp_name"]; //file in the PHP tmp folder
             $dir = substr( $fileName, 0, -4 ); // the name without ".zip"
             $target_path = "../../ams_lib/plugins/$dir"; //path in which the zip extraction is to be done
             $destination = "../../ams_lib/plugins/";

             // scanning plugin folder if plugin with same name is already exists or not
            $x = checkForUpdate( $dir, $destination, $fileTmpLoc, $temp_path );
             if ( $x == '1' )
             {
                echo "update found";
                 throw new SystemExit();
                 }
            else if ( $x == '2' )
             {
                echo "Plugin already exists with same name .";
                 throw new SystemExit();
                 }
            else if ( $x == '3' )
             {
                echo "Update info is not present in the update";
                 throw new SystemExit();
                 }


            // checking for the command to install plugin is given or not
            if ( !isset( $_POST['install_plugin'] ) )
                 {
                // the browser supplied content type is not evidence of
                // anything, so check that the upload really is an archive
                $zip_probe = new ZipArchive();
                 if ( $zip_probe -> open( $fileTmpLoc ) === true )
                     {
                    $zip_probe -> close();
                     if ( move_uploaded_file( $fileTmpLoc, $temp_path . "/" . $fileName ) ) {
                        echo htmlspecialchars( $fileName, ENT_QUOTES ) . " upload is complete.</br>" . "<button type='submit' class='btn btn-primary' style='margin-left:5px; margin-top:10px;' name='install_plugin'>Install Plugin</button></br>";
                         throw new SystemExit();
                         }
                    else
                         {
                        echo "Error in uploading file.";
                         throw new SystemExit();
                         }
                    }
                else
                     {
                    echo "Please select a file with .zip extension to upload.";
                     throw new SystemExit();
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
                header("Cache-Control: max-age=1");
                        header( "Location: index.php?page=plugins&result=1" );
                         throw new SystemExit();
                         }
                    else
                         {
                        // file .info not exists
                        rmdir( $target_path );
                header("Cache-Control: max-age=1");
                         header( "Location: index.php?page=install_plugin&result=2" );
                         throw new SystemExit();
                         }

                    } else
                     {
                    // extraction failed
                header("Cache-Control: max-age=1");
                    header( "Location: index.php?page=install_plugin&result=0" );
                     throw new SystemExit();
                     }
                }
            }
        else
             {
            echo "Please Browse for a file before clicking the upload button";
             throw new SystemExit();
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
    // Plugincache does the same job but refuses archives whose entries would
    // be written outside of $destination
    return Plugincache :: zipExtraction( $target_path, $destination );
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
 * TemplatePath = path to the template
 * Description = Description of the plugin ,it's functionality
 * -----------------------------------------------------------
 *
 * reads only files with name .info
 *
 * @param  $fileName file to read
 * @param  $target_path path to the folder containing .info file
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
 * also, if the update founds ,check for the UpdateInfo in the .info file.
 * Update is saved in the temp directory with pluginName_version.zip
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

            // extracting the update
            if ( zipExtraction( $tempFile, $tempPath . "/test/" ) )
                 {
                $result = readPluginFile( ".info", $tempPath . "/test/" . $fileName );

                 // check for the version for the plugin
                $db = new DBLayer( "lib" );
                 $sth = $db -> select( "plugins", array( 'Name' => $result['PluginName'] ), "Name = :Name" );
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
                if ( !array_diff( $new_version , $pre_version ) )
                     {
                    // removing the uploaded file
                    Plugincache :: rrmdir( $tempPath . "/test/" . $fileName );
                     return '2'; //plugin already exists
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
                        // check if update already exists
                        if ( pluginUpdateExists( $info['Id'], $tempPath . "/" . trim( $fileName, ".zip" ) . "_" . $result['Version'] . ".zip" ) )
                             {
                            echo "Update already exists";
                             throw new SystemExit();
                             }
                        else {
                            // removing the preivous update
                            $dbr = new DBLayer( "lib" );
                             $dbr -> delete( "updates", array( 'id' => $info['Id'] ), "PluginId=:id" );
                             // storing update in the temp directory
                            // format of update save
                            if ( move_uploaded_file( $tempFile, $tempPath . "/" . trim( $fileName, ".zip" ) . "_" . $result['Version'] . ".zip" ) ) {
                                // setting update information in the database
                                $update['PluginId'] = $info['Id'];
                                 $update['UpdatePath'] = $tempPath . "/" . trim( $fileName, ".zip" ) . "_" . $result['Version'] . ".zip";
                                 $update['UpdateInfo'] = json_encode( $result );
                                 $dbr -> insert( "updates", $update );
                header("Cache-Control: max-age=1");
                                 header( "Location: index.php?page=plugins&result=7" );
                                 throw new SystemExit();
                                 }
                            }
                        }
                    }
                }
            }
        }
    }

/**
 * Function to check for the update of a plugin already exists
 *
 * @param  $pluginId id of the plugin for which update is available
 * @param  $updatePath path of the new update
 * @return boolean True if update already exists else False
 *
 */
function PluginUpdateExists( $pluginId, $updatePath )
 {
    $db = new DBLayer( 'lib' );
     $sth = $db -> selectWithParameter( "UpdatePath", "updates", array( 'pluginid' => $pluginId ), "PluginId=:pluginid" );
     $row = $sth -> fetch();
     if ( $updatePath == $row['UpdatePath'] )
     {
        return true;
         }
    else
         {
        rmdir( $row['UpdatePath'] );
         return false;
         }
    }
