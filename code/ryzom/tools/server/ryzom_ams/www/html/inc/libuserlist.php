<?php


function libuserlist(){
    if(WebUsers::isAdmin()){
        //This checks to see if there is a page number. If not, it will set it to page 1 
        if (!(isset($_GET['pagenum']))){ 
            $pagenum = 1; 
        }else{
            $pagenum = $_GET['pagenum'];
        }
        
         //Here we count the number of results
        $dbl = new DBLayer("lib");
        $rows = $dbl->executeWithoutParams("SELECT * FROM ams_querycache")->rowCount();
        
        //the array hat will contain all users
        $pageResult['liblist'] = Array();
        if($rows > 0){
            //This is the number of results displayed per page 
            $page_rows = 2;
            
            //This tells us the page number of our last page 
            $last = ceil($rows/$page_rows); 
            
            //this makes sure the page number isn't below one, or more than our maximum pages 
            if ($pagenum < 1) 
            { 
                $pagenum = 1; 
            }else if ($pagenum > $last) { 
                $pagenum = $last; 
            } 
            
            //This sets the range to display in our query 
            $max = 'limit ' .($pagenum - 1) * $page_rows .',' .$page_rows; 
            
            //This is your query again, the same one... the only difference is we add $max into it
            $data = $dbl->executeWithoutParams("SELECT * FROM ams_querycache $max"); 
            
            //This is where we put the results in a resultArray to be sent to smarty
            
            $i = 0;
            while($row = $data->fetch(PDO::FETCH_ASSOC)){
                $decode = json_decode($row['query']);
                $pageResult['liblist'][$i]['id'] = $row['SID'];
                $pageResult['liblist'][$i]['type'] = $row['type'];
                //$pageResult['liblist'][$i]['name'] = $decode[0];
                //$pageResult['liblist'][$i]['mail'] = $decode[2];
                $i++;
            }
        }
        
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