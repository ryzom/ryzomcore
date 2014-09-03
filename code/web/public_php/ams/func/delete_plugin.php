<?php
/**
 * This function is used in deleting plugins.
 * It removes the plugin from the codebase as well as
 * from the Database. When user request to delete a plugin
 * id of that plugin is sent in $_GET global variable.
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
                if ( Plugincache::rrmdir( "$name[FileName]" ) )
                     {
                    $db -> delete( 'plugins', array( 'id' => $id ), "Id=:id" );

                    //if result	successfull redirect and show success message
                     header( "Location: index.php?page=plugins&result=2" );
                     throw new SystemExit();

                     }
                else
                     {
					// if result unsuccessfull redirect and show error message
                    header( "Location: index.php?page=plugins&result=0" );
                     throw new SystemExit();
                     }
                }
            }
        else
             {
			// if result unsuccessfull redirect and show error message
            header( "Location: index.php?page=plugins&result=0" );
             throw new SystemExit();
             }
        }
    }
