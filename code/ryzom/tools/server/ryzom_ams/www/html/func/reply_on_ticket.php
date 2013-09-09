<?php

function reply_on_ticket(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_POST['ticket_id'])){
        
        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT); 
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($ticket_id);

        if(($target_ticket->getAuthor() ==   unserialize($_SESSION['ticket_user'])->getTUserId())  ||  Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) ){
            
            try{
                $author = unserialize($_SESSION['ticket_user'])->getTUserId();
                if(isset($_POST['Content'])){
                    $content = $_POST['Content'];
                }else{
                    $content="";
                }
                $hidden = 0;
                if(isset($_POST['hidden']) &&  Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
                    $hidden = 1;
                }
                Ticket::createReply($content, $author, $ticket_id,  $hidden);
                
                if(isset($_POST['ChangeStatus']) && isset($_POST['ChangePriority']) && Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
                    $newStatus = filter_var($_POST['ChangeStatus'], FILTER_SANITIZE_NUMBER_INT);
                    $newPriority = filter_var($_POST['ChangePriority'], FILTER_SANITIZE_NUMBER_INT); 
                    Ticket::updateTicketStatusAndPriority($ticket_id,$newStatus, $newPriority, $author);
                }
                if (Helpers::check_if_game_client()) {
                    header("Location: ".$INGAME_WEBPATH."?page=show_ticket&id=".$ticket_id);
                }else{
                    header("Location: ".$WEBPATH."?page=show_ticket&id=".$ticket_id);
                }
                exit;
                
            }catch (PDOException $e) {
                //ERROR: LIB DB is not online!
                print_r($e);
                //header("Location: index.php");
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