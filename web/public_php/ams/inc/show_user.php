<?php
/**
* This function is beign used to load info that's needed for the show_user page.
* Users can only browse their own user page, while mods/admins can browse all user pages. The current settings of the user being browsed will be loaded, as also their created tickets
* and this info will be returned so it can be used by the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function show_user(){
     //if logged in
    if(WebUsers::isLoggedIn()){

        //Users can only browse their own user page, while mods/admins can browse all user pages
        if( !isset($_GET['id']) ||  Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) || $_GET['id'] == $_SESSION['id'] ){

            if(isset($_GET['id'])){
                $result['target_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
            }else{
                $result['target_id'] = $_SESSION['id'];
            }
            $webUser = new WebUsers($result['target_id']);
            $result['target_name'] = $webUser->getUsername();
            $result['mail'] = $webUser->getEmail();
            $info = $webUser->getInfo();
            $result['firstName'] = $info['FirstName'];
            $result['lastName'] = $info['LastName'];
            $result['country'] = $info['Country'];
            $result['gender'] = $info['Gender'];

            $ticket_user = Ticket_User::constr_ExternId($result['target_id']);
            $result['userPermission'] = $ticket_user->getPermission();
            if(Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
                $result['isAdmin'] = "TRUE";
            }
            $ticketlist = Ticket::getTicketsOf($ticket_user->getTUserId());

            $result['ticketlist'] = Gui_Elements::make_table($ticketlist, Array("getTId","getTimestamp","getTitle","getStatus","getStatusText","getStatusText","getCategoryName"), Array("tId","timestamp","title","status","statustext","statusText","category"));
            global $INGAME_WEBPATH;
            $result['ingame_webpath'] = $INGAME_WEBPATH;
            return $result;

        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
                header("Cache-Control: max-age=1");
            header("Location: index.php?page=error");
            throw new SystemExit();
        }
    }else{
        //ERROR: not logged in!
                header("Cache-Control: max-age=1");
        header("Location: index.php");
        throw new SystemExit();
    }
}
