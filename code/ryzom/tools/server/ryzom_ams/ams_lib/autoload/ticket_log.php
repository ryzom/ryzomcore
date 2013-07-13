<?php

class Ticket_Log{
    
    private $tLogId;
    private $timestamp;
    private $query;
    private $author;
    private $ticket;
    
    /****************************************
     *Action ID's:
     * 1: User X Created Ticket
     * 2: Admin X created ticket for arg
     * 3: Read Ticket
     * 4: Added Reply ID: arg to ticket
     * 5: Changed status to arg
     * 6: Changed Priority to arg
     *
     ****************************************/
    
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
     //return all logs that are related to a ticket
    public static function getLogsOfTicket( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_log INNER JOIN ticket_user ON ticket_log.Author = ticket_user.TUserId and ticket_log.Ticket=:id ORDER BY ticket_log.TLogId DESC", array('id' => $ticket_id));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $log){
            $instanceAuthor = Ticket_User::constr_TUserId($log['Author']);
            $instanceAuthor->setExternId($log['ExternId']);
            $instanceAuthor->setPermission($log['Permission']);

            $instanceLog = new self();
            $instanceLog->setTLogId($log['TLogId']);
            $instanceLog->setTimestamp($log['Timestamp']);
            $instanceLog->setAuthor($instanceAuthor);
            $instanceLog->setTicket($ticket_id);
            $instanceLog->setQuery($log['Query']);
            
            $result[] = $instanceLog;
        }
        return $result; 
    }
    
    //Creates a log entry
    public static function createLogEntry( $ticket_id, $author_id, $action, $arg = -1) {
        global $TICKET_LOGGING;
        if($TICKET_LOGGING){
            $dbl = new DBLayer("lib");
            $query = "INSERT INTO ticket_log (Timestamp, Query, Ticket, Author) VALUES (now(), :query, :ticket, :author )";
            $values = Array('ticket' => $ticket_id, 'author' => $author_id, 'query' => json_encode(array($action,$arg)));
            $dbl->execute($query, $values);
        }
    }


    //return constructed element based on TLogId
    public static function constr_TLogId( $id) {
        $instance = new self();
        $instance->setTLogId($id);
        return $instance;
    }
    
    //returns list of all logs of a ticket
    public static function getAllLogs($ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_log INNER JOIN ticket_user ON ticket_log.Author = ticket_user.TUserId and ticket_log.Ticket=:id", array('id' => $ticket_id));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $log){
            $instance = new self();
            $instance->set($log);
            $result[] = $instance;
        }
        return $result; 
    }
     
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setTLogId($values['TLogId']);
        $this->setTimestamp($values['Timestamp']);
        $this->setQuery($values['Query']);
        $this->setTicket($values['Ticket']);
        $this->setAuthor($values['Author']);
    } 

    //Load with tlogId
    public function load_With_TLogId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_log WHERE TLogId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket_log SET Timestamp = :timestamp, Query = :query, Author = :author, Ticket = :ticket WHERE TLogId=:id";
        $values = Array('id' => $this->getTLogId(), 'timestamp' => $this->getTimestamp(), 'query' => $this->getQuery(), 'author' => $this->getAuthor(), 'ticket' => $this->getTicket() );
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getTLogId(){
        return $this->tLogId;
    }
    
    public function getTimestamp(){
        return $this->timestamp;
    }
    
    public function getQuery(){
        return $this->query;
    }
    
    public function getAuthor(){
        return $this->author;
    }
   
    public function getTicket(){
        return $this->ticket;
    }
    
    public function getAction(){
        $decodedQuery = json_decode($this->query);
        return $decodedQuery[0];
    }
    
    public function getArgument(){
        $decodedQuery = json_decode($this->query);
        return $decodedQuery[1];
    }
    
    public function getActionTextArray(){
       $variables = Helpers::handle_language();
       $result = array();
       foreach ( $variables['ticket_log'] as $key => $value ){
              $result[$key] = $value;
           }
        return $result;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    public function setTLogId($id){
        $this->tLogId = $id;
    }
    
    public function setTimestamp($t){
        $this->timestamp = $t;
    }
    
    public function setQuery($q){
        $this->query = $q;
    }
    
    public function setAuthor($a){
        $this->author = $a;
    }
    
    public function setTicket($t){
        $this->ticket = $t;
    }
   
    
}