<?php

function dashboard(){
    
      //if logged in
    if(WebUsers::isLoggedIn()){
       
        //is Mod
        if(ticket_user::isMod(unserialize($_SESSION['ticket_user']))){
            $result['user_id'] = unserialize($_SESSION['ticket_user'])->getTUserId();
            $result['nrToDo'] = Ticket_Queue_Handler::getNrOfTicketsToDo(unserialize($_SESSION['ticket_user'])->getTUserId());
            $result['nrAssignedWaiting'] = Ticket_Queue_Handler::getNrOfTicketsAssignedWaiting(unserialize($_SESSION['ticket_user'])->getTUserId());
            $result['nrTotalTickets'] = Ticket_Queue_Handler::getNrOfTickets();
            $ticket = Ticket_Queue_Handler::getNewestTicket();
            $result['newestTicketId'] = $ticket->getTId();
            $result['newestTicketTitle'] = $ticket->getTitle();
            $result['newestTicketAuthor'] = Ticket_User::get_username_from_id($ticket->getAuthor());
            global $INGAME_WEBPATH;
            $result['ingame_webpath'] = $INGAME_WEBPATH;
            return $result;
        
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