<?php

class Ticket_Queue_Handler{
    
    private $pagination;
    
    public function getTickets($input, $user_id){
     
            $queue = new Ticket_Queue();

            switch ($input){
                case "all":
                    $queue->loadAllTickets();
                    break;
                case "all_open":
                    $queue->loadAllOpenTickets();
                    break;
                case "archive":
                    $queue->loadAllClosedTickets();
                    break;
                case "not_assigned":
                    $queue->loadAllNotAssignedTickets();
                    break;
                case "todo":
                    $queue->loadToDoTickets($user_id);
                    break;
                default:
                    return "ERROR";
            }
            $this->pagination = new Pagination($queue->getQuery(),"lib",10,"Ticket",$queue->getParams());
            foreach( $this->pagination->getElements() as $element ){
                $catInstance = new Ticket_Category();
                $catInstance->load_With_TCategoryId($element->getTicket_Category());
                $element->setTicket_Category($catInstance);
                
                $userInstance = new Ticket_User();
                $userInstance->load_With_TUserId($element->getAuthor());
                $element->setAuthor($userInstance);
            }
            return $this->pagination->getElements();
            
            
            
    }
    
    public static function CreateQueue($userid, $groupid, $what, $how, $who){
        $queue = new Ticket_Queue();
        $queue->createQueue($userid, $groupid, $what, $how, $who);
        return $queue->getTickets();
    }
}