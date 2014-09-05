<?php

require( '../../config.php' );
require_once( $AMS_LIB . '/libinclude.php' );
$id = $_POST['PHPSESSID'];
session_id($id);
session_start();

    // Set permission
    if ( isset( $_SESSION['ticket_user'] ) ) {
        $return['permission'] = unserialize( $_SESSION['ticket_user'] ) -> getPermission();
        } else {
        // default permission
        $return['permission'] = 0;
        }
        
        
    
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
    
        $ticket_id = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($ticket_id);
        if(($target_ticket->getAuthor() ==   unserialize($_SESSION['ticket_user'])->getTUserId())  ||  Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) ){

            if (!empty($_FILES)) {
                $tempFile = $_FILES['Filedata']['tmp_name'];
                
                $fileParts = pathinfo($_FILES['Filedata']['name']);
                Ticket::add_Attachment($_GET['id'],$_FILES['Filedata']['name'],$_SESSION['id'],$tempFile);
            }
        }
    }
?>
