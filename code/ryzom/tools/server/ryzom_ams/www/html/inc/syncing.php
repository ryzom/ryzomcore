<?php


function syncing(){
    if(Ticket_User::isAdmin($_SESSION['ticket_user'])){
        
        $pagination = new Pagination("SELECT * FROM ams_querycache","lib",5,"Querycache");
        $pageResult['liblist'] = Gui_Elements::make_table($pagination->getElements() , Array("getSID","getType"), Array("id","type"));
        $pageResult['links'] = $pagination->getLinks(5);
        $pageResult['lastPage'] = $pagination->getLast();
        $pageResult['currentPage'] = $pagination->getCurrent();
        
        
        //check if shard is online
        try{
            $dbs = new DBLayer("shard");
            $pageResult['shard'] = "online";
        }catch(PDOException $e) {
            $pageResult['shard'] = "offline";
        }
        return $pageResult;
    }else{
        //ERROR: No access!
        $_SESSION['error_code'] = "403";
        header("Location: index.php?page=error");
        exit;
    }
}