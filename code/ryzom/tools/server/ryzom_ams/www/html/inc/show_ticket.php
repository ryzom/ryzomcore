<?php

function show_ticket(){
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
        
        $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT); 
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($result['ticket_id']);

        if(($target_ticket->getAuthor() ==   $_SESSION['ticket_user']->getTUserId())  || Ticket_User::isMod($_SESSION['ticket_user'] )){
            
            $entire_ticket = Ticket::getEntireTicket( $result['ticket_id']);
            Ticket_Log::createLogEntry($result['ticket_id'],$_SESSION['ticket_user']->getTUserId(), 3);
            $result['ticket_tId'] = $entire_ticket['ticket_obj']->getTId();
            $result['ticket_title'] = $entire_ticket['ticket_obj']->getTitle();
            $result['ticket_timestamp'] = $entire_ticket['ticket_obj']->getTimestamp();
            $result['ticket_status'] = $entire_ticket['ticket_obj']->getStatus();
            $result['ticket_prioritytext'] = $entire_ticket['ticket_obj']->getPriorityText();
            $result['ticket_priorities'] = Ticket::getPriorityArray();
            $result['ticket_priority'] = $entire_ticket['ticket_obj']->getPriority();
            $result['ticket_statustext'] = $entire_ticket['ticket_obj']->getStatusText();
            $result['ticket_lastupdate'] = Gui_Elements::time_elapsed_string(Ticket::getLatestReply($result['ticket_id'])->getTimestamp());
            $result['ticket_category'] = $entire_ticket['ticket_obj']->getCategoryName();
            $result['ticket_replies'] = Gui_Elements::make_table($entire_ticket['reply_array'], Array("getTReplyId","getContent()->getContent","getTimestamp","getAuthor()->getExternId","getAuthor()->getPermission"), Array("tReplyId","replyContent","timestamp","authorExtern","permission"));
            $i = 0;
            foreach( $result['ticket_replies'] as $reply){
                $result['ticket_replies'][$i]['author'] = WebUsers::getUsername($reply['authorExtern']);
                $i++;
            }
            if(Ticket_User::isMod($_SESSION['ticket_user'])){
                $result['isMod'] = "TRUE";
                $result['statusList'] = Ticket::getStatusArray();
            }
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