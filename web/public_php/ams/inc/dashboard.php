<?php
/**
* This function is beign used to load info that's needed for the dashboard page.
* check if the person who wants to view this page is a mod/admin, if this is not the case, he will be redirected to an error page.
* next it will fetch a lot of information regarding to the status of the ticket system (eg return the total amount of tickets) and return this information so
* it can be used by the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function dashboard(){

      //if logged in
    if(WebUsers::isLoggedIn()){

        //is Mod
        if(ticket_user::isMod(unserialize($_SESSION['ticket_user']))){
            //return useful information about the status of the ticket system.
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
                header("Cache-Control: max-age=1");
            header("Location: index.php?page=error");
            throw new SystemExit();

        }

    }else{
        //ERROR: not logged in!
                header("Cache-Control: max-age=1");
        header("Location: index.php");
        throw new SystemExit();
    }


}
