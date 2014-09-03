<?php
/**
* This function is beign used to load info that's needed for the show_queue page.
* check if the person who wants to view this page is a mod/admin, if this is not the case, he will be redirected to an error page.
* if an action is set (this is done by $_GET['action']) it will try to execute it first, actions are: assign a ticket, unassign a ticket an create a queue.
* There are a few predefined queues which is the 'all tickets' queue, 'archive' queue, 'todo' queue, .. these are passed by $_GET['get'].
* if  $_GET['get'] = create; then it's a custom made queue, this will call the createQueue function which builds the query that we will later use to get the tickets.
* The tickets fetched will be returned and used in the template. Now why use POST and GET params here and have a createQueue function twice? Well the first time someone creates
* a queue the POST variables will be used, however after going to the next page it will use the GET params.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function show_queue(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
     //if logged in  & queue id is given
    if(WebUsers::isLoggedIn() && isset($_GET['get'])){

        if( Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){

            //the  queue you want to see.
            $result['queue_view'] = filter_var($_GET['get'], FILTER_SANITIZE_STRING);
            $user_id = unserialize($_SESSION['ticket_user'])->getTUserId();
            $queueArray = array();
            $queue_handler = new  Ticket_Queue_handler();

            //Pagination Base Links
            if (Helpers::check_if_game_client()) {
                $result['pagination_base_link'] = $INGAME_WEBPATH."?page=show_queue&get=".$result['queue_view'] ;
            }else{
                $result['pagination_base_link'] = $WEBPATH."?page=show_queue&get=".$result['queue_view'] ;
            }

            //form url to keep the getters constant
            if (Helpers::check_if_game_client()) {
                $result['getURL'] = $INGAME_WEBPATH."?page=show_queue&get=" . $result['queue_view'];
            }else{
                $result['getURL'] = $WEBPATH."?page=show_queue&get=" . $result['queue_view'];
            }

            if(isset($_GET['pagenum'])){
                $result['getURL'] = $result['getURL'] . "&pagenum=".$_GET['pagenum'];
            }

            if(isset($_GET['get']) && ($_GET['get'] == "create") && isset($_GET['userid']) && isset($_GET['groupid']) && isset($_GET['what']) && isset($_GET['how']) && isset($_GET['who'])){
                $userid = filter_var($_GET['userid'], FILTER_SANITIZE_NUMBER_INT);
                $groupid = filter_var($_GET['groupid'], FILTER_SANITIZE_NUMBER_INT);
                $what = filter_var($_GET['what'], FILTER_SANITIZE_STRING);
                $how = filter_var($_GET['how'], FILTER_SANITIZE_STRING);
                $who = filter_var($_GET['who'], FILTER_SANITIZE_STRING);
                //create the custom queue
                $queue_handler->CreateQueue($userid, $groupid, $what, $how, $who);

                if (Helpers::check_if_game_client()) {
                    $result['pagination_base_link'] = $INGAME_WEBPATH."?page=show_queue&get=create&userid=".$userid."&groupid=".$groupid."&what=".$what."&how=".$how."&who=".$who;
                }else{
                    $result['pagination_base_link'] = $WEBPATH."?page=show_queue&get=create&userid=".$userid."&groupid=".$groupid."&what=".$what."&how=".$how."&who=".$who;
                }


                $result['prev_created_userid'] = $userid;
                $result['prev_created_groupid'] = $groupid;
                $result['prev_created_what'] = $what;
                $result['prev_created_how'] = $how;
                $result['prev_created_who'] = $who;

                $result['getURL'] = $result['getURL'] . "&userid=".$userid."&groupid=".$groupid."&what=".$what."&how=".$how."&who=".$who;

            }

            //if an action is set
            if(isset($_POST['action'])){
                switch($_POST['action']){
                    case "assignTicket":
                        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                        $result['ACTION_RESULT'] = Ticket::assignTicket($user_id, $ticket_id);
                        break;

                    case "unAssignTicket":
                        $ticket_id = filter_var($_POST['ticket_id'], FILTER_SANITIZE_NUMBER_INT);
                        $result['ACTION_RESULT'] = Ticket::unAssignTicket($user_id, $ticket_id);
                        break;

                        case "create_queue":
                        $userid = filter_var($_POST['userid'], FILTER_SANITIZE_NUMBER_INT);
                        if (isset($_POST['groupid'])) {
                            $groupid = filter_var($_POST['groupid'], FILTER_SANITIZE_NUMBER_INT);
                        } else {
                            $groupid = 0;
                        }
                        $what = filter_var($_POST['what'], FILTER_SANITIZE_STRING);
                        $how = filter_var($_POST['how'], FILTER_SANITIZE_STRING);
                        $who = filter_var($_POST['who'], FILTER_SANITIZE_STRING);
                        //create the custom queue
                        $queue_handler->CreateQueue($userid, $groupid, $what, $how, $who);
                        if (Helpers::check_if_game_client()) {
                            $result['pagination_base_link'] = $INGAME_WEBPATH."?page=show_queue&get=create&userid=".$userid."&groupid=".$groupid."&what=".$what."&how=".$how."&who=".$who;
                        }else{
                           $result['pagination_base_link'] = $WEBPATH."?page=show_queue&get=create&userid=".$userid."&groupid=".$groupid."&what=".$what."&how=".$how."&who=".$who;
                        }
                        $result['prev_created_userid'] = $userid;
                        $result['prev_created_groupid'] = $groupid;
                        $result['prev_created_what'] = $what;
                        $result['prev_created_how'] = $how;
                        $result['prev_created_who'] = $who;
                        $result['getURL'] = $result['getURL'] . "&userid=".$userid."&groupid=".$groupid."&what=".$what."&how=".$how."&who=".$who;

                        break;

                }
            }

            $queueArray = $queue_handler->getTickets($result['queue_view'], $user_id);

            //pagination
            $result['links'] = $queue_handler->getPagination()->getLinks(5);
            $result['lastPage'] = $queue_handler->getPagination()->getLast();
            $result['currentPage'] = $queue_handler->getPagination()->getCurrent();


            //if queue_view is a valid parameter value
            if ($queueArray != "ERROR"){
                $result['tickets'] = Gui_Elements::make_table($queueArray, Array("getTId","getTitle","getTimestamp","getAuthor()->getExternId","getTicket_Category()->getName","getStatus","getStatusText","getAssigned","getForwardedGroupName","getForwardedGroupId"), Array("tId","title","timestamp","authorExtern","category","status","statusText","assigned","forwardedGroupName","forwardedGroupId"));
                $i = 0;
                foreach( $result['tickets'] as $ticket){
                    $web_author = new WebUsers($ticket['authorExtern']);
                    $result['tickets'][$i]['author'] = $web_author->getUsername();
                    $web_assigned = new WebUsers($ticket['assigned']);
                    $result['tickets'][$i]['assignedText'] = $web_assigned->getUsername();
                    $result['tickets'][$i]['timestamp_elapsed'] = Gui_Elements::time_elapsed_string($ticket['timestamp']);
                    $i++;
                }
                $result['user_id'] = unserialize($_SESSION['ticket_user'])->getTUserId();

                //Queue creator field info
                $result['grouplist'] = Gui_Elements::make_table(Support_Group::getGroups(), Array("getSGroupId","getName"), Array("sGroupId","name"));
                $result['teamlist'] = Gui_Elements::make_table(Ticket_User::getModsAndAdmins(), Array("getTUserId","getExternId"), Array("tUserId","externId"));
                $i = 0;
                foreach( $result['teamlist'] as $member){
                    $web_teammember = new Webusers($member['externId']);
                    $result['teamlist'][$i]['name'] = $web_teammember->getUsername();
                    $i++;
                }
                global $INGAME_WEBPATH;
                $result['ingame_webpath'] = $INGAME_WEBPATH;
                return $result;

            }else{

                //ERROR: Doesn't exist!
                $_SESSION['error_code'] = "404";
                header("Location: ams?page=error");
                die();
            }

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
