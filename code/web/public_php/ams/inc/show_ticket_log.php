<?php
/**
* This function is beign used to load info that's needed for the show_ticket_log page.
* This page shows the logs related to a ticket: who created the ticket, who replied on it, who viewed it, assigned or forwarded it.
* Only mods/admins are able to browse the log though. The found information is returned so it can be used by the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function show_ticket_log(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    //if logged in
    if(WebUsers::isLoggedIn() && isset($_GET['id'])){

        //only allow admins to browse the log!
        if(Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) ){

            $result['ticket_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
            $target_ticket = new Ticket();
            $target_ticket->load_With_TId($result['ticket_id']);
            $result['ticket_title'] = $target_ticket->getTitle();

            //return all logs related to a ticket.
            $ticket_logs = Ticket_Log::getLogsOfTicket( $result['ticket_id']);
            $log_action_array = Ticket_Log::getActionTextArray();
            //fetch information about each returned ticket in a format that is usable for the template
            $result['ticket_logs'] = Gui_Elements::make_table($ticket_logs, Array("getTLogId","getTimestamp","getAuthor()->getExternId","getAction","getArgument()"), Array("tLogId","timestamp","authorExtern","action","argument"));
            $i = 0;
            //for each ticket add action specific informaton to the to-be-shown text: uses the query_backpart
            foreach( $result['ticket_logs'] as $log){
                $webUser = new WebUsers($log['authorExtern']);
                $author = $webUser->getUsername();
                $result['ticket_logs'][$i]['author'] = $author;
                $query_backpart  = "";
                if($log['action'] == 2){
                    $webUser2 = new WebUsers($log['argument']);
                    $query_backpart =  $webUser2->getUsername();
                }else if($log['action'] == 4){
                    if (Helpers::check_if_game_client()) {
                        $query_backpart = "<a href='".$INGAME_WEBPATH."?page=show_reply&id=" . $log['argument'] . "'>ID#" . $log['argument'] . "</a>";
                    }else{
                        $query_backpart = "<a href='".$WEBPATH."?page=show_reply&id=" . $log['argument'] . "'>ID#" . $log['argument'] . "</a>";
                    }
                }else if($log['action'] == 5){
                    $statusArray = Ticket::getStatusArray();
                    $query_backpart = $statusArray[$log['argument'] ];
                }else if($log['action'] == 6){
                    $priorityArray = Ticket::getPriorityArray();
                    $query_backpart = $priorityArray[$log['argument'] ];
                }else if($log['action'] == 8){
                    if (Helpers::check_if_game_client()) {
                        $query_backpart = "<a href='".$INGAME_WEBPATH."?page=show_sgroupy&id=" . $log['argument'] . "'>" . Support_Group::getGroup($log['argument'])->getName() . "</a>";
                    }else{
                        $query_backpart = "<a href='".$WEBPATH."?page=show_sgroupy&id=" . $log['argument'] . "'>" . Support_Group::getGroup($log['argument'])->getName() . "</a>";
                    }
                }
                $result['ticket_logs'][$i]['query'] = $author . " " . $log_action_array[$log['action']] . " " .  $query_backpart;
                $result['ticket_logs'][$i]['timestamp_elapsed'] = Gui_Elements::time_elapsed_string($log['timestamp']);
                $i++;
            }
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
