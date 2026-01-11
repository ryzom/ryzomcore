<?php
/**
* returns tickets (queues) that are related in some way.
* This class handles the creation and returning of existing ticket queues. Normally a $_GET['get'] parameter is being used to identify what kind of tickets should be shown.
* the getTickets() function uses this parameter($input) and uses the ticket_queue class to load the specific query.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket_Queue_Handler{
    
    private $pagination; /**< Pagination object, this way only a few tickets (related to that pagenumber) will be shown */ 
    private $queue; /**< The queue object, being used to get the queries and parameters. */ 
    
    /**
    * A constructor.
    * Instantiates the queue object.
    */
    function __construct() {
        $this->queue = new Ticket_Queue();
    }
    
    /**
    * returns the tickets that are related in someway defined by $input.
    * The $input parameter should be a string that defines what kind of queue should be loaded. A new pagination object will be instantiated and will load 10 entries,
    * related to the $_GET['pagenum'] variable.
    * @param $input identifier that defines what queue to load.
    * @param $user_id the id of the user that browses the queues, some queues can be depending on this.
    * @return an array consisting of ticket objects, beware, the author & category of a ticket, are objects on their own (no integers are used this time).
    */
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
    
    
    /**
    * get pagination attribute of the object.
    */
    public function getPagination(){
        return $this->pagination;
    }
    
    /**
    * creates the queue.
    * afterwards the getTickets function should be called, else a lot of extra parameters had to be added to the getTickets function..
    */
    public function createQueue($userid, $groupid, $what, $how, $who){     
        $this->queue->createQueue($userid, $groupid, $what, $how, $who);
    }
    
    
    ////////////////////////////////////////////Info retrievers about ticket statistics////////////////////////////////////////////////////
    
    /**
    * get the number of tickets in the todo queue for a specific user.
    * @param $user_id the user being queried
    */
    public static function getNrOfTicketsToDo($user_id){
        $queueHandler = new Ticket_Queue_Handler();
        $queueHandler->queue->loadToDoTickets($user_id);
        $query = $queueHandler->queue->getQuery();
        $params = $queueHandler->queue->getParams();
        $dbl = new DBLayer("lib");
        return $dbl->execute($query,$params)->rowCount();
    }
    
    /**
    * get the number of tickets assigned to a specific user and waiting for support.
    * @param $user_id the user being queried
    */
    public static function getNrOfTicketsAssignedWaiting($user_id){
        $queueHandler = new Ticket_Queue_Handler();
        $queueHandler->queue->loadAssignedandWaiting($user_id);
        $query = $queueHandler->queue->getQuery();
        $params = $queueHandler->queue->getParams();
        $dbl = new DBLayer("lib");
        return $dbl->execute($query,$params)->rowCount();
    }
    
    /**
    * get the total number of tickets.
    */
    public static function getNrOfTickets(){
        $queueHandler = new Ticket_Queue_Handler();
        $queueHandler->queue->loadAllTickets();
        $query = $queueHandler->queue->getQuery();
        $params = $queueHandler->queue->getParams();
        $dbl = new DBLayer("lib");
        return $dbl->execute($query,$params)->rowCount();
    }
    
    /**
    * get the ticket object of the latest added ticket.
    */
    public static function getNewestTicket(){
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM `ticket` ORDER BY `TId` DESC LIMIT 1 ");
        $ticket = new Ticket();
        $ticket->set($statement->fetch());
        return $ticket;
    }
}