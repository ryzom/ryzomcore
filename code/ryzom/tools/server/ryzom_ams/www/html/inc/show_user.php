<?php

function show_user(){
     //if logged in
    if(WebUsers::isLoggedIn()){
        
        if( !isset($_GET['id']) ||  WebUsers::isAdmin() || $_GET['id'] == $_SESSION['id'] ){
            
            if(isset($_GET['id'])){
                $result['target_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);        
            }else{
                $result['target_id'] = $_SESSION['id']; 
            }
            $result['target_name'] = WebUsers::getUsername( $result['target_id']);
            $result['mail'] = WebUsers::getEmail( $result['target_id']);
            $info = WebUsers::getInfo($result['target_id']);
            $result['firstName'] = $info['FirstName'];
            $result['lastName'] = $info['LastName'];
            $result['country'] = $info['Country'];
            $result['gender'] = $info['Gender'];
            
            global $cfg;
            $ticket_user = Ticket_User::constr_ExternId($result['target_id'],$cfg['db']['lib']);
            $ticketlist = Ticket::getTicketsOf($ticket_user->getTUserId(),$cfg['db']['lib']);
            $i = 0;
            $result['ticketlist'] = Array();
            foreach($ticketlist as $ticket){
                $result['ticketlist'][$i]['tId'] = $ticket->getTId();
                $result['ticketlist'][$i]['timestamp'] = $ticket->getTimestamp();
                $result['ticketlist'][$i]['title'] = $ticket->getTitle();
                
                //get the status
                $statusId = $ticket->getStatus();
                if ($statusId == 0){
                    $status = "Waiting on support..";
                }else if($statusId == 1){
                    $status = "Being handled..";
                }else if($statusId == 2){
                    $status = "Closed";
                }
                
                $result['ticketlist'][$i]['statusText'] = $status;
                $result['ticketlist'][$i]['status'] = $statusId;
                //get the category
                $category = Ticket_Category::constr_TCategoryId($ticket->getTicket_Category(), $cfg['db']['lib']);
                $result['ticketlist'][$i]['category'] = $category->getName();
                $i++;
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