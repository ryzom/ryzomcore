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
                    }
                }
                
                $result['tickets'] = Gui_Elements::make_table($queueArray, Array("getTId","getTitle","getTimestamp","getAuthor()->getExternId","getTicket_Category()->getName","getStatus","getStatusText","getAssigned"), Array("tId","title","timestamp","authorExtern","category","status","statusText","assigned"));
                $i = 0;
                foreach( $result['tickets'] as $ticket){
                    $result['tickets'][$i]['author'] = WebUsers::getUsername($ticket['authorExtern']);
                    $result['tickets'][$i]['assignedText'] = WebUsers::getUsername($ticket['assigned']);
                    $result['tickets'][$i]['timestamp_elapsed'] = Gui_Elements::time_elapsed_string($ticket['timestamp']);
                    $i++;
                }
                $result['user_id'] = $_SESSION['ticket_user']->getTUserId();
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