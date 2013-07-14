<?php

class Ticket_Queue_Handler{
    
    public static function getTickets($input,$permission){
            switch($permission){
                case 2:
                    $queue = new Ticket_Queue_Csr();
                    break;
                
                case 3:
                   // $queue = new Ticket_Queue_Dev();
                    break;   
            }
            
            switch ($input){
                case "all_open":
                    $queue->loadAllOpenTickets();
                    break;
                case "archive":
                    $queue->loadAllClosedTickets();
                    break;
                default:
                    return "ERROR";
            }
   
            return $queue->getTickets();
    }
}