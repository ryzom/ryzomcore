<?php
/**
* This function is beign used to load info that's needed for the show_ticket page.
* check if the person browsing this page is a mod/admin or the ticket creator himself, if not he'll be redirected to an error page.
* if the $_GET['action'] var is set and the user executing is a mod/admin, it will try to execute the action. The actions here are: forwarding of a ticket,
* assigning a ticket and unassigning a ticket. This function returns a lot of information that will be used by the template to show the ticket. Mods/admins will be able to
* also see hidden replies to a ticket.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function show_ticket(){
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){

        $result['user_id'] = unserialize($_SESSION['ticket_user'])->getTUserId();
        $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($result['ticket_id']);

        if(Ticket_User::isMod(unserialize($_SESSION['ticket_user'] ))){
            if(isset($_POST['action'])){
                switch($_POST['action']){
                    case "forward":
                        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                        $group_id = filter_var($_POST['group'], FILTER_SANITIZE_NUMBER_INT);
                        $result['ACTION_RESULT'] = Ticket::forwardTicket($result['user_id'], $ticket_id, $group_id);
                        break;
                    case "assignTicket":
                        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                        $result['ACTION_RESULT'] = Ticket::assignTicket($result['user_id'] , $ticket_id);
                        break;
                    case "unAssignTicket":
                        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                        $result['ACTION_RESULT'] = Ticket::unAssignTicket($result['user_id'], $ticket_id);
                        break;

                }
            }
        }

        if(($target_ticket->getAuthor() ==   unserialize($_SESSION['ticket_user'])->getTUserId())  || Ticket_User::isMod(unserialize($_SESSION['ticket_user']) )){

            $show_as_admin = false;
            if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
                $show_as_admin = true;
            }

            $entire_ticket = Ticket::getEntireTicket( $result['ticket_id'],$show_as_admin);
            Ticket_Log::createLogEntry($result['ticket_id'],unserialize($_SESSION['ticket_user'])->getTUserId(), 3);
            $result['ticket_tId'] = $entire_ticket['ticket_obj']->getTId();
            $result['ticket_forwardedGroupName'] = $entire_ticket['ticket_obj']->getForwardedGroupName();
            $result['ticket_forwardedGroupId'] = $entire_ticket['ticket_obj']->getForwardedGroupId();
            $result['ticket_title'] = $entire_ticket['ticket_obj']->getTitle();
            $result['ticket_timestamp'] = $entire_ticket['ticket_obj']->getTimestamp();
            $result['ticket_status'] = $entire_ticket['ticket_obj']->getStatus();
            $result['ticket_author'] = $entire_ticket['ticket_obj']->getAuthor();
            $result['ticket_prioritytext'] = $entire_ticket['ticket_obj']->getPriorityText();
            $result['ticket_priorities'] = Ticket::getPriorityArray();
            $result['ticket_priority'] = $entire_ticket['ticket_obj']->getPriority();
            $result['ticket_statustext'] = $entire_ticket['ticket_obj']->getStatusText();
            $result['ticket_lastupdate'] = Gui_Elements::time_elapsed_string(Ticket::getLatestReply($result['ticket_id'])->getTimestamp());
            $result['ticket_category'] = $entire_ticket['ticket_obj']->getCategoryName();
            $webUser = new WebUsers(Assigned::getUserAssignedToTicket($result['ticket_tId']));
            $result['ticket_assignedToText'] = $webUser->getUsername();
            $result['ticket_assignedTo'] = Assigned::getUserAssignedToTicket($result['ticket_tId']);
            $result['ticket_replies'] = Gui_Elements::make_table($entire_ticket['reply_array'], Array("getTReplyId","getContent()->getContent","getTimestamp","getAuthor()->getExternId","getAuthor()->getPermission","getHidden"), Array("tReplyId","replyContent","timestamp","authorExtern","permission","hidden"));
            $i = 0;
            foreach( $result['ticket_replies'] as $reply){
                $webReplyUser = new WebUsers($reply['authorExtern']);
                $result['ticket_replies'][$i]['author'] = $webReplyUser->getUsername();
                $i++;
            }
            if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
                $result['isMod'] = "TRUE";
                $result['statusList'] = Ticket::getStatusArray();
                $result['sGroups'] = Gui_Elements::make_table_with_key_is_id(Support_Group::getAllSupportGroups(), Array("getName"), "getSGroupId" );
            }
            $result['hasInfo'] = $target_ticket->hasInfo();
            global $INGAME_WEBPATH;
            $result['ingame_webpath'] = $INGAME_WEBPATH;
            return $result;

        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            throw new SystemExit();
        }
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        throw new SystemExit();
    }
}
