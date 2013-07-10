<?php

function show_ticket(){
    //if logged in
    if(WebUsers::isLoggedIn()){
        
        //if( !isset($_GET['id']) ||  WebUsers::isAdmin() || $_GET['id'] == $_SESSION['id'] ){
            
            if(isset($_GET['id'])){
                $result['target_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);        
            }else{
                $result['target_id'] = $_SESSION['id']; 
            }
            global $cfg;
            $entire_ticket = Ticket::getEntireTicket( $result['target_id'], $cfg['db']['lib']);
            $result['ticket_title'] = $entire_ticket['ticket_obj']->getTitle();
            $result['ticket_replies'] = Gui_Elements::make_table($entire_ticket['reply_array'], Array("getTReplyId","getContent()->getContent","getTimestamp"), Array("tReplyId","replyContent","timestamp"));
            //$result['ticket_replies'][0]['replyContent'] = nl2br($result['ticket_replies'][0]['replyContent']);
            return $result;
            
        /*}else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            exit;
        }*/
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        exit;
    }
}        