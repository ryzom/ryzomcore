<?php

function show_queue(){
    
     //if logged in  & queue id is given
    if(WebUsers::isLoggedIn() && isset($_GET['get'])){
        
        if( Ticket_User::isMod($_SESSION['ticket_user'])){
            $result['queue_view'] = filter_var($_GET['get'], FILTER_SANITIZE_STRING);
   
            $user_id = $_SESSION['ticket_user']->getTUserId();
            $queueArray = Ticket_Queue_Handler::getTickets($result['queue_view'], $user_id);
            
            //if queue_view is a valid parameter value
            if ($queueArray != "ERROR"){
                
                
                
                if(isset($_POST['action'])){
                    switch($_POST['action']){
                        case "assignTicket":
                            $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                            $result['ACTION_RESULT'] = Ticket::assignTicket($user_id, $ticket_id);
                            break;
                       
                        case "unAssignTicket":
                            $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                            $result['ACTION_RESULT'] = Ticket::unAssignTicket($user_id, $ticket_id);
                            break;
                        
                            case "create_queue":
                            $userid = filter_var($_POST['userid'], FILTER_SANITIZE_NUMBER_INT);
                            $groupid = filter_var($_POST['groupid'], FILTER_SANITIZE_NUMBER_INT);
                            $what = filter_var($_POST['what'], FILTER_SANITIZE_STRING);
                            $how = filter_var($_POST['how'], FILTER_SANITIZE_STRING);
                            $who = filter_var($_POST['who'], FILTER_SANITIZE_STRING);
                            $result['ACTION_RESULT'] = Ticket_Queue_Handler::CreateQueue($userid, $groupid, $what, $how, $who);
                            if ($result['ACTION_RESULT'] != "ERROR"){
                               $queueArray = $result['ACTION_RESULT'];
                            }
                            break;
                        
                    }
                }
                
                $result['tickets'] = Gui_Elements::make_table($queueArray, Array("getTId","getTitle","getTimestamp","getAuthor()->getExternId","getTicket_Category()->getName","getStatus","getStatusText","getAssigned","getForwardedGroupName","getForwardedGroupId"), Array("tId","title","timestamp","authorExtern","category","status","statusText","assigned","forwardedGroupName","forwardedGroupId"));
                $i = 0;
                foreach( $result['tickets'] as $ticket){
                    $result['tickets'][$i]['author'] = WebUsers::getUsername($ticket['authorExtern']);
                    $result['tickets'][$i]['assignedText'] = WebUsers::getUsername($ticket['assigned']);
                    $result['tickets'][$i]['timestamp_elapsed'] = Gui_Elements::time_elapsed_string($ticket['timestamp']);
                    $i++;
                }
                $result['user_id'] = $_SESSION['ticket_user']->getTUserId();
                
                //Queue creator field info   
                $result['grouplist'] = Gui_Elements::make_table(Support_Group::getGroups(), Array("getSGroupId","getName"), Array("sGroupId","name"));
                $result['teamlist'] = Gui_Elements::make_table(Ticket_User::getModsAndAdmins(), Array("getTUserId","getExternId"), Array("tUserId","externId"));
                $i = 0;
                foreach( $result['teamlist'] as $member){
                    $result['teamlist'][$i]['name'] = WebUsers::getUsername($member['externId']);
                    $i++;
                }
                return $result;
            
            }else{
                
                //ERROR: Doesn't exist!
                $_SESSION['error_code'] = "404";
                header("Location: index.php?page=error");
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