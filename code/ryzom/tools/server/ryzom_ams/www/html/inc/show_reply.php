<?php

function show_reply(){
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
        
        $result['reply_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
        $reply = new Ticket_Reply();
        $reply->load_With_TReplyId($result['reply_id']);
        
        
        $ticket = new Ticket();
        $ticket->load_With_TId($reply->getTicket());
        
        if(( $ticket->getAuthor() ==   unserialize($_SESSION['ticket_user'])->getTUserId() && ! $reply->getHidden())  ||  Ticket_User::isMod(unserialize($_SESSION['ticket_user']) )){
            $content = new Ticket_Content();
            $content->load_With_TContentId($reply->getContent());
            
            $author = new Ticket_User();
            $author->load_With_TUserId($reply->getAuthor());
            
            $result['hidden'] = $reply->getHidden();
            $result['ticket_id'] = $reply->getTicket();
            $result['reply_timestamp'] = $reply->getTimestamp();
            $result['author_permission'] = $author->getPermission();
            $result['reply_content'] = $content->getContent();
            $result['author'] = $author->getExternId();
            $webUser = new WebUsers($author->getExternId());
            $result['authorName'] = $webUser->getUsername();
            if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
                $result['isMod'] = "TRUE";
            }
            global $INGAME_WEBPATH;
            $result['ingame_webpath'] = $INGAME_WEBPATH;
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