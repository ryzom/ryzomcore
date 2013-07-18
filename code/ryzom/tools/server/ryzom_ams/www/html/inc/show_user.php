<?php

function show_user(){
     //if logged in
    if(WebUsers::isLoggedIn()){
        
        if( !isset($_GET['id']) ||  Ticket_User::isMod($_SESSION['ticket_user']) || $_GET['id'] == $_SESSION['id'] ){
            
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
            
            $ticket_user = Ticket_User::constr_ExternId($result['target_id']);
            $ticketlist = Ticket::getTicketsOf($ticket_user->getTUserId());
            
            $result['ticketlist'] = Gui_Elements::make_table($ticketlist, Array("getTId","getTimestamp","getTitle","getStatus","getStatusText","getStatusText","getCategoryName"), Array("tId","timestamp","title","status","statustext","statusText","category"));
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