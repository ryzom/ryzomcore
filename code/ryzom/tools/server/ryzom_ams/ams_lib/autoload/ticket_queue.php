<?php
class Ticket_Queue{
    
    protected $queueElements;
    
    public function loadAllNotAssignedTickets(){
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT ticket . * FROM ticket LEFT JOIN assigned ON ticket.TId = assigned.Ticket WHERE assigned.Ticket IS NULL");
        $rows = $statement->fetchAll();
        $this->setQueue($rows);
    }
    
    public function loadAllTickets(){
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM `ticket`");
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
    
    public function createQueue($userid, $groupid, $what, $how, $who){
        $dbl = new DBLayer("lib");
        
        if($who == "user"){
            $selectfrom = "SELECT * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User";
            if ($how == "assigned"){
                $assign = "tu.TUserId = :id" ;
            }else if ($how == "not_assigned"){
                $assign = "(tu.TUserId != :id OR a.Ticket IS NULL)";
            }
        }else if ($who == "support_group"){
            $selectfrom = "SELECT * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User LEFT JOIN `forwarded` f ON t.TId = f.Ticket";
            if ($how == "assigned"){
                $assign = "f.Group = :id";
            }else if ($how == "not_assigned"){
                $assign = "(f.Group != :id  OR f.Ticket IS NULL)" ;
            }
        
        }
        
        if ($what == "waiting_for_support"){
            $status = "t.Status = 1";
        }else if ($what == "waiting_for_user"){
            $status = "t.Status = 0";
        }else if ($what == "closed"){
            $status = "t.Status = 3";
        }
        
        $query = $selectfrom ." WHERE " . $assign;
        if(isset($status)){
            $query = $query . " AND " . $status;
        }
        if($who == "user"){
            $params = array('id' => $userid);
        }else if ($who == "support_group"){
            $params = array('id' => $groupid);
        }  
        $statement = $dbl->execute($query, $params);
        $rows = $statement->fetchAll();
        $this->setQueue($rows);
    }
    
    public function loadToDoTickets($user_id){
        
        $dbl = new DBLayer("lib");
        //first: find the tickets assigned to the user with status = waiting on support
        //second find all not assigned tickets that aren't forwarded yet.        
        //find all tickets assigned to someone else witht status waiting on support,  with timestamp of last reply > 1 day
        //find all non-assigned tickets forwarded to the support groups to which that user belongs
        $query = "SELECT * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User LEFT JOIN `forwarded` f ON t.TId = f.Ticket 
        WHERE (tu.ExternId = :user_id AND t.Status = 1) 
        OR (a.Ticket IS NULL AND f.Group IS NULL)
        OR (tu.ExternId != :user_id AND  t.Status = 1 AND (SELECT ticket_reply.Timestamp FROM `ticket_reply` WHERE Ticket =t.TId ORDER BY TReplyId DESC LIMIT 1)  < NOW() - INTERVAL 1 DAY )
        OR (a.Ticket IS NULL AND EXISTS (SELECT * FROM  `in_support_group` isg JOIN `ticket_user` tu2 ON isg.User = tu2.TUserId WHERE isg.Group = f.Group))
        ";
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