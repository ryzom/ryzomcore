<?php

function show_ticket_info(){
   
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){
        
        $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($result['ticket_id']);
        
        if(($target_ticket->getAuthor() ==   $_SESSION['ticket_user']->getTUserId())  || Ticket_User::isMod($_SESSION['ticket_user'] )){
            $result['ticket_title'] = $target_ticket->getTitle();
            $result['ticket_author'] = $target_ticket->getAuthor();
            
            
            $result['shard_id'] = $_GET['ShardId'];
            $result['user_position'] = $_GET['UserPosition'];
            $result['view_position'] = $_GET['ViewPosition'];
            $result['client_version'] = $_GET['ClientVersion'];
            $result['patch_version'] = $_GET['PatchVersion'];
            
            
            $result['server_tick'] = $_GET['ServerTick'];
            $result['connect_state'] = $_GET['ConnectState'];
            $result['local_address'] = $_GET['LocalAddress'];
            $result['memory'] = $_GET['Memory'];
            $result['os'] = $_GET['OS'];
            $result['processor'] = $_GET['Processor'];
            $result['cpu_id'] = $_GET['CPUID'];
            $result['cpu_mask'] = $_GET['CpuMask'];
            $result['ht'] = $_GET['HT'];
            
            $result['nel3d'] = $_GET['NeL3D'];
            
            if(Ticket_User::isMod($_SESSION['ticket_user'])){
                $result['isMod'] = "TRUE";
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