<?php

function show_ticket(){
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
        
        global $cfg;
        $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT); 
        $target_ticket = new Ticket($cfg['db']['lib']);
        $target_ticket->load_With_TId($result['ticket_id']);

        if(($target_ticket->getAuthor() ==   $_SESSION['ticket_user']->getTUserId())  ||  WebUsers::isAdmin() ){
            
            $entire_ticket = Ticket::getEntireTicket( $result['ticket_id'], $cfg['db']['lib']);
            $result['ticket_tId'] = $entire_ticket['ticket_obj']->getTId();
            $result['ticket_title'] = $entire_ticket['ticket_obj']->getTitle();
            $result['ticket_replies'] = Gui_Elements::make_table($entire_ticket['reply_array'], Array("getTReplyId","getContent()->getContent","getTimestamp"), Array("tReplyId","replyContent","timestamp"));
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