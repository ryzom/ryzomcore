<?php

class Ticket_Queue_Handler{
    
    public static function getTickets($input){
     
            $queue = new Ticket_Queue();
             
            
            switch ($input){
                case "all_open":
                    $queue->loadAllOpenTickets();
                    break;
                case "archive":
                    $queue->loadAllClosedTickets();
                    break;
                case "not_assigned":
                    $queue->loadAllNotAssignedTickets();
                    break;
                default:
                    return "ERROR";
            }
   
            return $queue->getTickets();
    }
}