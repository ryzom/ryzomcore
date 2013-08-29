<?php
class Ticket_Queue{
    
    private $query;
    private $params;
    
    public function loadAllNotAssignedTickets(){
        $this->query =  "SELECT ticket . * FROM ticket LEFT JOIN assigned ON ticket.TId = assigned.Ticket WHERE assigned.Ticket IS NULL";
        $this->params = array();
    }
    
    public function loadAllTickets(){
        $this->query = "SELECT * FROM `ticket`";
        $this->params = array();
    }
    
    public function loadAllOpenTickets(){
        $this->query = "SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket.Status!=3";
        $this->params = array();
    }
    
    public function loadAllClosedTickets(){
        $this->query = "SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket.Status=3";
        $this->params = array();
    }
    
    public function loadToDoTickets($user_id){
        
        //first: find the tickets assigned to the user with status = waiting on support
        //second find all not assigned tickets that aren't forwarded yet.        
        //find all tickets assigned to someone else witht status waiting on support,  with timestamp of last reply > 1 day
        //find all non-assigned tickets forwarded to the support groups to which that user belongs
        $this->query = "SELECT * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User LEFT JOIN `forwarded` f ON t.TId = f.Ticket 
        WHERE (tu.ExternId = :user_id AND t.Status = 1) 
        OR (a.Ticket IS NULL AND f.Group IS NULL)
        OR (tu.ExternId != :user_id AND  t.Status = 1 AND (SELECT ticket_reply.Timestamp FROM `ticket_reply` WHERE Ticket =t.TId ORDER BY TReplyId DESC LIMIT 1)  < NOW() - INTERVAL 1 DAY )
        OR (a.Ticket IS NULL AND EXISTS (SELECT * FROM  `in_support_group` isg JOIN `ticket_user` tu2 ON isg.User = tu2.TUserId WHERE isg.Group = f.Group))
        ";
        $this->params = array('user_id' => $user_id);     
    }
    
    public function loadAssignedandWaiting($user_id){
        $this->query = "SELECT * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User
        WHERE (tu.ExternId = :user_id AND t.Status = 1)";
        $this->params = array('user_id' => $user_id);     
    }
    
    public function createQueue($userid, $groupid, $what, $how, $who){
        
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
        $this->query = $query;
        $this->params = $params;
    }    
    
    public function getQuery(){
        return $this->query;
    }
    
    public function getParams(){
        return $this->params;
    }
}