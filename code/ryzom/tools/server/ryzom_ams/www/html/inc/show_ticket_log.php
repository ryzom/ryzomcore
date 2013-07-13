<?php

function show_ticket_log(){
   
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
        
        $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT); 
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($result['ticket_id']);

        if(($target_ticket->getAuthor() ==   $_SESSION['ticket_user']->getTUserId())  ||  WebUsers::isAdmin() ){
            
            $result['ticket_title'] = $target_ticket->getTitle();
            $ticket_logs = Ticket_Log::getLogsOfTicket( $result['ticket_id']);
            $log_action_array = Ticket_Log::getActionTextArray();
            $result['ticket_logs'] = Gui_Elements::make_table($ticket_logs, Array("getTLogId","getTimestamp","getAuthor()->getExternId","getAction","getArgument()"), Array("tLogId","timestamp","authorExtern","action","argument"));
            $i = 0;
            foreach( $result['ticket_logs'] as $log){
                $author = WebUsers::getUsername($log['authorExtern']);
                $result['ticket_logs'][$i]['author'] = $author;
                $query_backpart  = "";
                if($log['action'] == 2){
                    $query_backpart =  WebUsers::getUsername($log['argument']);
                }else if($log['action'] == 4){
                    $query_backpart = "<a href='index.php?page=show_reply&id=" . $log['argument'] . "'>" . $log['argument'] . "</a>";
                }else if($log['action'] == 5){
                    $statusArray = Ticket::getStatusArray();
                    $query_backpart = $statusArray[$log['argument'] ];
                }else if($log['action'] == 6){
                    $priorityArray = Ticket::getPriorityArray();
                    $query_backpart = $priorityArray[$log['argument'] ];
                }
                $result['ticket_logs'][$i]['query'] = $author . " " . $log_action_array[$log['action']] . " " .  $query_backpart;
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