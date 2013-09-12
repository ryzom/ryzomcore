<?php
/**
* Data class that holds a lot of queries that load specific tickets.
* These queries are being used by the ticket_queue_handler class. An object of this class holds 2 attributes: the query and the params used for the query.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket_Queue{
    
    private $query; /**< The query that loads specific tickets */ 
    private $params; /**< The parameter array that's being needed by the query */ 
    
    /**
    * loads the not yet assigned tickets query into the objects attributes.
    */
    public function loadAllNotAssignedTickets(){
        $this->query =  "SELECT ticket . * FROM ticket LEFT JOIN assigned ON ticket.TId = assigned.Ticket WHERE assigned.Ticket IS NULL";
        $this->params = array();
    }
    
    /**
    * loads the 'all' tickets query into the objects attributes.
    */
    public function loadAllTickets(){
        $this->query = "SELECT * FROM `ticket`";
        $this->params = array();
    }
    
    /**
    * loads the 'all open' tickets query into the objects attributes.
    */
    public function loadAllOpenTickets(){
        $this->query = "SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket.Status!=3";
        $this->params = array();
    }
    
    /**
    * loads the 'closed' tickets query into the objects attributes.
    */
    public function loadAllClosedTickets(){
        $this->query = "SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket.Status=3";
        $this->params = array();
    }
    
    /**
    * loads the 'todo' tickets query & params into the objects attributes.
    * first: find the tickets assigned to the user with status = waiting on support, 
    * second find all not assigned tickets that aren't forwarded yet.
    * find all tickets assigned to someone else witht status waiting on support,  with timestamp of last reply > 1 day,
    * find all non-assigned tickets forwarded to the support groups to which that user belongs
    * @param $user_id the user's id to whom the tickets should be assigned
    */
    public function loadToDoTickets($user_id){
        
        $this->query = "SELECT * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User LEFT JOIN `forwarded` f ON t.TId = f.Ticket 
        WHERE (tu.ExternId = :user_id AND t.Status = 1) 
        OR (a.Ticket IS NULL AND f.Group IS NULL)
        OR (tu.ExternId != :user_id AND  t.Status = 1 AND (SELECT ticket_reply.Timestamp FROM `ticket_reply` WHERE Ticket =t.TId ORDER BY TReplyId DESC LIMIT 1)  < NOW() - INTERVAL 1 DAY )
        OR (a.Ticket IS NULL AND EXISTS (SELECT * FROM  `in_support_group` isg JOIN `ticket_user` tu2 ON isg.User = tu2.TUserId WHERE isg.Group = f.Group))
        ";
        $this->params = array('user_id' => $user_id);     
    }
    
    /**
    * loads the 'tickets asssigned to a user and waiting on support' query & params into the objects attributes.
    * @param $user_id the user's id to whom the tickets should be assigned
    */
    public function loadAssignedandWaiting($user_id){
        $this->query = "SELECT * FROM `ticket` t LEFT JOIN `assigned` a ON t.TId = a.Ticket LEFT JOIN `ticket_user` tu ON tu.TUserId = a.User
        WHERE (tu.ExternId = :user_id AND t.Status = 1)";
        $this->params = array('user_id' => $user_id);     
    }
    
    
    /**
    * loads the 'created' query & params into the objects attributes.
    * This function creates dynamically a query based on the selected features.
    * @param $who specifies if we want to user the user_id or group_id to form the query.
    * @param $user_id the user's id to whom the tickets should be assigned/not assigned
    * @param $group_id  the group's id to whom the tickets should be forwarded/not forwarded
    * @param $what specifies what kind of tickets we want to return: waiting for support, waiting on user, closed
    * @param $how specifies if the tickets should be or shouldn't be assigned/forwarded to the group/user selected.
    */
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
    
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get query attribute of the object.
    */
    public function getQuery(){
        return $this->query;
    }
    
    /**
    * get params attribute of the object.
    */
    public function getParams(){
        return $this->params;
    }
}