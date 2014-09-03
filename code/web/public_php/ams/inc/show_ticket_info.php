<?php
/**
* This function is beign used to load info that's needed for the show_ticket_info page.
* check if the person browsing this page is a mod/admin or the ticket creator himself, if not he'll be redirected to an error page.
* not all tickets have this page related to it, only tickets created ingame will have additional information. The returned info will be used by the template to show the show_ticket_info page.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function show_ticket_info(){

    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){

        $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
        $target_ticket = new Ticket();
        $target_ticket->load_With_TId($result['ticket_id']);

        if( $target_ticket->hasInfo() && (($target_ticket->getAuthor() ==   unserialize($_SESSION['ticket_user'])->getTUserId())  || Ticket_User::isMod(unserialize($_SESSION['ticket_user']) ))){
            $result['ticket_title'] = $target_ticket->getTitle();
            $result['ticket_author'] = $target_ticket->getAuthor();

            $ticket_info = new Ticket_Info();
            $ticket_info->load_With_Ticket($result['ticket_id']);
            $result['shard_id'] = $ticket_info->getShardId();
            $result['user_position'] = $ticket_info->getUser_Position();
            $result['view_position'] = $ticket_info->getView_Position();
            $result['client_version'] = $ticket_info->getClient_Version();
            $result['patch_version'] = $ticket_info->getPatch_Version();
            $result['server_tick'] = $ticket_info->getServer_Tick();
            $result['connect_state'] = $ticket_info->getConnect_State();
            $result['local_address'] = $ticket_info->getLocal_Address();
            $result['memory'] = $ticket_info->getMemory();
            $result['os'] = $ticket_info->getOS();
            $result['processor'] = $ticket_info->getProcessor();
            $result['cpu_id'] = $ticket_info->getCPUId();
            $result['cpu_mask'] = $ticket_info->getCPU_Mask();
            $result['ht'] = $ticket_info->getHT();
            $result['nel3d'] = $ticket_info->getNel3D();
            $result['user_id'] = $ticket_info->getUser_Id();
            global $IMAGELOC_WEBPATH;
            $result['IMAGELOC_WEBPATH'] = $IMAGELOC_WEBPATH;

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
            die();
        }
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        die();
    }
}
