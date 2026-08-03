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

    // only the staff that can reach the plugin page may act on plugins
    if ( WebUsers :: isLoggedIn() && Ticket_User :: isMod( unserialize( $_SESSION['ticket_user'] ) ) ) {

        if ( isset( $_GET['id'] ) )
             {
            // id of plugin to delete after filtering
            $id = intval( $_GET['id'] );

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
                header("Cache-Control: max-age=1");
                     header( "Location: index.php?page=plugins&result=2" );
                     throw new SystemExit();

                     }
                else
                     {
					// if result unsuccessfull redirect and show error message
                header("Cache-Control: max-age=1");
                    header( "Location: index.php?page=plugins&result=0" );
                     throw new SystemExit();
                     }
                }
            }
        else
             {
			// if result unsuccessfull redirect and show error message
                header("Cache-Control: max-age=1");
            header( "Location: index.php?page=plugins&result=0" );
             throw new SystemExit();
             }
        }
    }
