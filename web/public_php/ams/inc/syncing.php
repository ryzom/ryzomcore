<?php
/**
* This function is beign used to load info that's needed for the syncing page.
* this function is used for notifying admins that there are unsynced changes, a brief overview of the non syned changes will be shown. The entries are being loaded here
* so that they can be passed to the template itself. Only admins can browse this page, others will be redirected to an error page.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function syncing(){

    if(Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){

        //return a paginated version of all unsynced changes.
        $pagination = new Pagination("SELECT * FROM ams_querycache","lib",5,"Querycache");
        $pageResult['liblist'] = Gui_Elements::make_table($pagination->getElements() , Array("getSID","getType"), Array("id","type"));
        $pageResult['links'] = $pagination->getLinks(5);
        $pageResult['lastPage'] = $pagination->getLast();
        $pageResult['currentPage'] = $pagination->getCurrent();

        global $INGAME_WEBPATH;
        $pageResult['ingame_webpath'] = $INGAME_WEBPATH;

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
                header("Cache-Control: max-age=1");
        header("Location: index.php?page=error");
        throw new SystemExit();
    }
}
