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
        $query = "SELECT t . * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User WHERE (tu.ExternId = :user_id AND t.Status = 1) OR (a.Ticket IS NULL) OR  (tu.ExternId != :user_id AND  t.Status = 1 AND   (SELECT ticket_reply.Timestamp FROM `ticket_reply` WHERE Ticket =t.TId ORDER BY TReplyId DESC LIMIT 1)  < NOW() - INTERVAL 1 DAY )";
        $values = array('user_id' => $user_id);
        $statement = $dbl->execute($query,$values);
        $rows = $statement->fetchAll();
        $this->setQueue($rows);

        
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