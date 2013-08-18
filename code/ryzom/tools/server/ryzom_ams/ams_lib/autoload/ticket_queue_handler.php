<?php

class Ticket_Queue_Handler{
    
    private $pagination;
    private $queue;
    
    function __construct() {
        $this->queue = new Ticket_Queue();
    }
    
    public function getTickets($input, $user_id){
     
            switch ($input){
                case "all":
                    $this->queue->loadAllTickets();
                    break;
                case "all_open":
                    $this->queue->loadAllOpenTickets();
                    break;
                case "archive":
                    $this->queue->loadAllClosedTickets();
                    break;
                case "not_assigned":
                    $this->queue->loadAllNotAssignedTickets();
                    break;
                case "todo":
                    $this->queue->loadToDoTickets($user_id);
                    break;
                case "create":
                    //set these with the createQueue function proceding the getTickets function
                    break;
                default:
                    return "ERROR";
            }
            
            $this->pagination = new Pagination($this->queue->getQuery(),"lib",10,"Ticket",$this->queue->getParams());
            $elemArray = $this->pagination->getElements();
            if(!empty($elemArray)){
                foreach( $elemArray as $element ){
                    $catInstance = new Ticket_Category();
                    $catInstance->load_With_TCategoryId($element->getTicket_Category());
                    $element->setTicket_Category($catInstance);
                    
                    $userInstance = new Ticket_User();
                    $userInstance->load_With_TUserId($element->getAuthor());
                    $element->setAuthor($userInstance);
                }
            }
            return $this->pagination->getElements();    
            
    }
    
    public function getPagination(){
        return $this->pagination;
    }
    
    public function createQueue($userid, $groupid, $what, $how, $who){     
        $this->queue->createQueue($userid, $groupid, $what, $how, $who);
    }
    
}