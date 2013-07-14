<?php

function show_queue(){
    
     //if logged in  & queue id is given
    if(WebUsers::isLoggedIn() && isset($_GET['get'])){
        if( WebUsers::isAdmin()){
            $result['queue_action'] = filter_var($_GET['get'], FILTER_SANITIZE_STRING);
            
            $queue = new Ticket_Queue();
            
            switch ($result['queue_action']){
                case "all_open":
                    $queue->loadAllOpenTickets();
                    break;
                case "archive":
                    $queue->loadAllClosedTickets();
                    break;
            }
   
            $queueArray = $queue->getTickets();
            $result['tickets'] = Gui_Elements::make_table($queueArray, Array("getTId","getTitle","getTimestamp","getAuthor()->getExternId","getTicket_Category()->getName","getStatus","getStatusText"), Array("tId","title","timestamp","authorExtern","category","status","statusText"));
         
            $i = 0;
            foreach( $result['tickets'] as $ticket){
                $result['tickets'][$i]['author'] = WebUsers::getUsername($ticket['authorExtern']);
                $i++;
            }
            if(WebUsers::isAdmin()){
                $result['isAdmin'] = "TRUE";
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