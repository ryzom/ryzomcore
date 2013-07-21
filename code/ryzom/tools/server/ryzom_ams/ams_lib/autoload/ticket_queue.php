<?php
class Ticket_Queue{
    
    protected $queueElements;
    
    public function loadAllNotAssignedTickets(){
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT ticket . * FROM ticket LEFT JOIN assigned ON ticket.TId = assigned.Ticket WHERE assigned.Ticket IS NULL");
        $rows = $statement->fetchAll();
        $this->setQueue($rows);
    }
    
    public function loadAllOpenTickets(){
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket.Status!=3");
        $rows = $statement->fetchAll();
        $this->setQueue($rows);
    }
    
    public function loadAllClosedTickets(){
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket.Status=3");
        $rows = $statement->fetchAll();
        $this->setQueue($rows);
    }
    
    public function loadToDoTickets($user_id){
        
        $dbl = new DBLayer("lib");
        
        //first: find the tickets assigned to the user with status != waiting on user reply
        $statement = $dbl->execute("SELECT ticket . * FROM ticket LEFT JOIN assigned ON ticket.TId = assigned.Ticket WHERE assigned.User = :user_id",array('user_id' => $user_id));
        $assignedTo = $statement->fetchAll();
        
        //second: find all non-assigned tickets forwarded to the support groups to which that user belongs
        //TODO
        
        //third: find all not assigned tickets that aren't forwarded yet.
        //TODO: check if not forwarded to a group!
        $statement = $dbl->executeWithoutParams("SELECT ticket . * FROM ticket LEFT JOIN assigned ON ticket.TId = assigned.Ticket WHERE assigned.Ticket IS NULL");
        $notAssigned = $statement->fetchAll();
        
        //forth: find all tickets assigned to someone else, not forwarded to a group or forwarded to a group you are a member from, with status == waiting on support and with timestamp of last reply > 1 day
        //TODO
        
        //Now let's get them all together
        $allTogether = $assignedTo + $notAssigned;

        //filter to only get unique ticket id's
        $this->setQueue(self::filter_tickets($allTogether));   
        
    }
    
    //filters the array of tickets and returns unique tickets based on there TId
    static public function filter_tickets($ticketArray){
        $tmp = array ();
        foreach ($ticketArray as $ticket1){
            $found = false;
            foreach ($tmp as $ticket2){
                if($ticket1['TId'] == $ticket2['TId']){
                    $found = true;
                }
            }
            if(! $found){
                $tmp[] = $ticket1;
            }
        }
        return $tmp;
    }
    
    public function getTickets(){
        return $this->queueElements;
    }
    
    protected function setQueue($rows){
        
        $result = Array();
        foreach($rows as $ticket){
            $instance = new Ticket();
            $instance->setTId($ticket['TId']);
            $instance->setTimestamp($ticket['Timestamp']);
            $instance->setTitle($ticket['Title']);
            $instance->setStatus($ticket['Status']);
            $instance->setQueue($ticket['Queue']);
            
            $catInstance = new Ticket_Category();
            $catInstance->load_With_TCategoryId($ticket['Ticket_Category']);
            $instance->setTicket_Category($catInstance);
            
            $userInstance = new Ticket_User();
            $userInstance->load_With_TUserId($ticket['Author']);
            $instance->setAuthor($userInstance);
            
            $result[] = $instance;
        }
        $this->queueElements =  $result; 
        
    }
    
    
    
    
    
    
    
    
    
}