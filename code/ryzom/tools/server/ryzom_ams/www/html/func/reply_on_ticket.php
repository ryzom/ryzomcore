<?php

function reply_on_ticket(){
    
     //if logged in
    if(WebUsers::isLoggedIn() && isset($_POST['ticket_id'])){
        
        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT); 
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($ticket_id);

        if(($target_ticket->getAuthor() ==   $_SESSION['ticket_user']->getTUserId())  ||  WebUsers::isAdmin() ){
            
            try{
                if(isset($_POST['ChangeStatus']) && $_POST['Content'] != ""){
                    $content = filter_var($_POST['Content'], FILTER_SANITIZE_STRING);
                    $author = $_SESSION['ticket_user']->getTUserId();
                    Ticket_Reply::createReply($content, $author, $ticket_id);
                }
                if(isset($_POST['ChangeStatus']) && isset($_POST['ChangePriority']) && WebUsers::isAdmin()){
                    $newStatus = filter_var($_POST['ChangeStatus'], FILTER_SANITIZE_NUMBER_INT);
                    $newPriority = filter_var($_POST['ChangePriority'], FILTER_SANITIZE_NUMBER_INT); 
                    $ticket = new Ticket();
                    $ticket->load_With_TId($ticket_id);
                    $ticket->setStatus($newStatus);
                    $ticket->setPriority($newPriority);
                    $ticket->update();
                }
                header("Location: index.php?page=show_ticket&id=".$ticket_id);
                exit;
                
            }catch (PDOException $e) {
                //ERROR: LIB DB is not online!
                header("Location: index.php");
                exit;
            }
            
        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            exit;
        }
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        exit;
    }
    
}