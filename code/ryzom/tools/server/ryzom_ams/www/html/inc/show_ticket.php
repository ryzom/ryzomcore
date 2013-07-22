<?php

function show_ticket(){
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
        
        $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT); 
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($result['ticket_id']);
        
        if(Ticket_User::isMod($_SESSION['ticket_user'] )){    
            if(isset($_POST['action'])){
                switch($_POST['action']){
                    case "forward":
                        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                        $group_id = filter_var($_POST['group'], FILTER_SANITIZE_NUMBER_INT);
                        $result['ACTION_RESULT'] = Ticket::forwardTicket($_SESSION['ticket_user']->getTUserId(), $ticket_id, $group_id);
                        break;
                }
            }
        }      


        if(($target_ticket->getAuthor() ==   $_SESSION['ticket_user']->getTUserId())  || Ticket_User::isMod($_SESSION['ticket_user'] )){
            
            $show_as_admin = false;
            if(Ticket_User::isMod($_SESSION['ticket_user'])){
                $show_as_admin = true;
            }
            $entire_ticket = Ticket::getEntireTicket( $result['ticket_id'],$show_as_admin);
            Ticket_Log::createLogEntry($result['ticket_id'],$_SESSION['ticket_user']->getTUserId(), 3);
            $result['ticket_tId'] = $entire_ticket['ticket_obj']->getTId();
            $result['ticket_forwardedGroupName'] = $entire_ticket['ticket_obj']->getForwardedGroupName();
            $result['ticket_forwardedGroupId'] = $entire_ticket['ticket_obj']->getForwardedGroupId();
            $result['ticket_title'] = $entire_ticket['ticket_obj']->getTitle();
            $result['ticket_timestamp'] = $entire_ticket['ticket_obj']->getTimestamp();
            $result['ticket_status'] = $entire_ticket['ticket_obj']->getStatus();
            $result['ticket_prioritytext'] = $entire_ticket['ticket_obj']->getPriorityText();
            $result['ticket_priorities'] = Ticket::getPriorityArray();
            $result['ticket_priority'] = $entire_ticket['ticket_obj']->getPriority();
            $result['ticket_statustext'] = $entire_ticket['ticket_obj']->getStatusText();
            $result['ticket_lastupdate'] = Gui_Elements::time_elapsed_string(Ticket::getLatestReply($result['ticket_id'])->getTimestamp());
            $result['ticket_category'] = $entire_ticket['ticket_obj']->getCategoryName();
            $result['ticket_replies'] = Gui_Elements::make_table($entire_ticket['reply_array'], Array("getTReplyId","getContent()->getContent","getTimestamp","getAuthor()->getExternId","getAuthor()->getPermission","getHidden"), Array("tReplyId","replyContent","timestamp","authorExtern","permission","hidden"));
            $i = 0;
            foreach( $result['ticket_replies'] as $reply){
                $result['ticket_replies'][$i]['author'] = WebUsers::getUsername($reply['authorExtern']);
                $i++;
            }
            if(Ticket_User::isMod($_SESSION['ticket_user'])){
                $result['isMod'] = "TRUE";
                $result['statusList'] = Ticket::getStatusArray();
                $result['sGroups'] = Gui_Elements::make_table_with_key_is_id(Support_Group::getAllSupportGroups(), Array("getName"), "getSGroupId" );
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