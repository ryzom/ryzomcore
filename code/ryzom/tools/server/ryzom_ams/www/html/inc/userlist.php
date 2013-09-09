<?php

function userlist(){
    if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
        
        $pagination = new Pagination(WebUsers::getAllUsersQuery(),"web",10,"WebUsers");
        $pageResult['userlist'] = Gui_Elements::make_table($pagination->getElements() , Array("getUId","getUsername","getEmail"), Array("id","username","email"));
        $pageResult['links'] = $pagination->getLinks(5);
        $pageResult['lastPage'] = $pagination->getLast();
        $pageResult['currentPage'] = $pagination->getCurrent();
        
        $i = 0;
        foreach( $pageResult['userlist'] as $user ){
            $pageResult['userlist'][$i]['permission'] = Ticket_User::constr_ExternId($pageResult['userlist'][$i]['id'])->getPermission();
            $i++;
        }
        
        if (Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
            $pageResult['isAdmin'] = "TRUE";
        }
        global $INGAME_WEBPATH;
        $pageResult['ingame_webpath'] = $INGAME_WEBPATH;
 	global $BASE_WEBPATH;
        $pageResult['base_webpath'] = $BASE_WEBPATH;
        return $pageResult;
    }else{
        //ERROR: No access!
        $_SESSION['error_code'] = "403";
        header("Location: index.php?page=error");
        exit;
    }
}
