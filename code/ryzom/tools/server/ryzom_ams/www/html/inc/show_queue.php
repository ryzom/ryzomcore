<?php

function show_queue(){
    
     //if logged in  & queue id is given
    if(WebUsers::isLoggedIn() && isset($_GET['get'])){
        if( Ticket_User::isMod($_SESSION['ticket_user'])){
            $result['queue_action'] = filter_var($_GET['get'], FILTER_SANITIZE_STRING);
   
            $queueArray = Ticket_Queue_Handler::getTickets($result['queue_action']);
            if ($queueArray != "ERROR"){    
                $result['tickets'] = Gui_Elements::make_table($queueArray, Array("getTId","getTitle","getTimestamp","getAuthor()->getExternId","getTicket_Category()->getName","getStatus","getStatusText","getAssigned"), Array("tId","title","timestamp","authorExtern","category","status","statusText","assigned"));
             
                $i = 0;
                foreach( $result['tickets'] as $ticket){
                    $result['tickets'][$i]['author'] = WebUsers::getUsername($ticket['authorExtern']);
                    $result['tickets'][$i]['assignedText'] = WebUsers::getUsername($ticket['assigned']);
                    $i++;
                }
                if(Ticket_User::isMod($_SESSION['ticket_user'])){
                    $result['isMod'] = "TRUE";
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