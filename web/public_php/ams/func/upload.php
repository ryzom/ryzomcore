<?php

require( '../../config.php' );
require_once( $AMS_LIB . '/libinclude.php' );

// The flash uploader cannot send our session cookie, so it posts the session
// id instead. Only honour that when the request really has no session cookie
// of its own, and only when the value looks like a session id -- it is handed
// straight to the session handler, which uses it to build a file name.
if ( !isset( $_COOKIE[session_name()] ) && isset( $_POST['PHPSESSID'] )
     && preg_match( '/^[A-Za-z0-9,-]{1,128}$/', $_POST['PHPSESSID'] ) ) {
    session_id( $_POST['PHPSESSID'] );
}
session_start();

    // Set permission
    if ( isset( $_SESSION['ticket_user'] ) ) {
        $return['permission'] = unserialize( $_SESSION['ticket_user'] ) -> getPermission();
        } else {
        // default permission
        $return['permission'] = 0;
        }
        
        
    
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
    
        // the very same id has to be used for the permission check below and
        // for the attachment itself, or the check guards a different ticket
        $ticket_id = intval($_GET['id']);
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($ticket_id);
        if(($target_ticket->getAuthor() ==   unserialize($_SESSION['ticket_user'])->getTUserId())  ||  Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) ){

            if (!empty($_FILES)) {
                $tempFile = $_FILES['Filedata']['tmp_name'];
                
                $fileParts = pathinfo($_FILES['Filedata']['name']);
                Ticket::add_Attachment($ticket_id,$_FILES['Filedata']['name'],$_SESSION['id'],$tempFile);
                echo "Uploaded :".htmlspecialchars($_FILES['Filedata']['name'], ENT_QUOTES);
            } else {
                echo "Upload Failed!";
            }
            echo "Upload Failed!";
        }
        echo "Upload Failed!";
    }
    echo "Upload Failed!";
?>
