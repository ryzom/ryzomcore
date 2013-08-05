<?php


function libuserlist(){
    if(Ticket_User::isAdmin($_SESSION['ticket_user'])){
        
        $pagination = new Pagination("SELECT * FROM ams_querycache","lib",1,"Querycache");
        print "<font color='red'>1 elements / page </font><br/>";
        print "<font color='green'>7 links max</font>";
        print "<br/><br/>";
        print "last page=";
        print_r($pagination->getLast());
        print "<br/>----------------------------------------------<br/>";
        print "elements:";
        print_r($pagination->getElements());
        print "<br/>----------------------------------------------<br/>";
        print "links:";
        print_r($pagination->getLinks(7));
        exit;
        
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