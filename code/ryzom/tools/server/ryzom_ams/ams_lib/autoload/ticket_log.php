<?php
/**
* Class that handles the logging. The logging will be used when a ticket is created, a reply is added, if someone views a ticket,
* if someone assigns a ticket to him or if someone forwards a ticket. This class provides functions to get retrieve those logs and also make them.
* 
*-the Action IDs being used are:
* -# User X Created ticket
* -# Admin X created ticket for arg
* -# Read ticket
* -# Added Reply ID: arg to ticket
* -# Changed status to arg
* -# Changed Priority to arg
* -# assigned to the ticket
* -# forwarded ticket to support group arg
* -# unassigned to the ticket
*
* @author Daan Janssens, mentored by Matthew Lagoe
*/

class Ticket_Log{
    
    private $tLogId; /**< The id of the log entry */ 
    private $timestamp; /**< The timestamp of the log entry */ 
    private $query; /**< The query (json encoded array containing action id & argument) */ 
    private $author; /**< author of the log */ 
    private $ticket; /**< the id of the ticket related to the log entry */ 
    
    /****************************************
     *Action ID's:
     * 1: User X Created Ticket
     * 2: Admin X created ticket for arg
     * 3: Read Ticket
     * 4: Added Reply ID: arg to ticket
     * 5: Changed status to arg
     * 6: Changed Priority to arg
     * 7: assigned to the ticket
     * 8: Forwarded ticket to support group arg
     * 9: unassigned to the ticket
     *
     ****************************************/
    
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    /**
    * return all log entries related to a ticket.
    * @param $ticket_id the id of the ticket of which we want all related log entries returned.
    * @return an array of ticket_log objects, be aware that the author in the ticket_log object is a ticket_user object on its own (so not a simple integer).
    */
    public static function getLogsOfTicket( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_log INNER JOIN ticket_user ON ticket_log.Author = ticket_user.TUserId and ticket_log.Ticket=:id ORDER BY ticket_log.TLogId ASC", array('id' => $ticket_id));
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
    
    
    /**
    * create a new log entry.
    * It will check if the $TICKET_LOGGING global var is true, this var is used to turn logging on and off. In case it's on, the log message will be stored.
    * the action id and argument (which is -1 by default), will be json encoded and stored in the query field in the db.
    * @param $ticket_id the id of the ticket related to the new log entry
    * @param $author_id the id of the user that instantiated the logging.
    * @param $action the action id (see the list in the class description)
    * @param $arg argument for the action (default = -1)
    */
    public static function createLogEntry( $ticket_id, $author_id, $action, $arg = -1) {
        global $TICKET_LOGGING;
        if($TICKET_LOGGING){
            $dbl = new DBLayer("lib");
            $query = "INSERT INTO ticket_log (Timestamp, Query, Ticket, Author) VALUES (now(), :query, :ticket, :author )";
            $values = Array('ticket' => $ticket_id, 'author' => $author_id, 'query' => json_encode(array($action,$arg)));
            $dbl->execute($query, $values);
        }
    }


    /**
    * return constructed element based on TLogId
    * @param $id ticket_log id of the entry that we want to load into our object.
    * @return constructed ticket_log object.
    */
    public static function constr_TLogId( $id) {
        $instance = new self();
        $instance->setTLogId($id);
        return $instance;
    }
    
    /**
    * return all log entries related to a ticket.
    * @param $ticket_id the id of the ticket of which we want all related log entries returned.
    * @return an array of ticket_log objects, here the author is an integer.
    * @todo only use one of the 2 comparable functions in the future and make the other depricated.
    */
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
     
    /**
    * A constructor.
    * Empty constructor
    */
    public function __construct() {
    }
    
    /**
    * sets the object's attributes.
    * @param $values should be an array.
    */
    public function set($values) {
        $this->setTLogId($values['TLogId']);
        $this->setTimestamp($values['Timestamp']);
        $this->setQuery($values['Query']);
        $this->setTicket($values['Ticket']);
        $this->setAuthor($values['Author']);
    } 

    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a ticket_log entries ID (TLogId).
    * @param id the id of the ticket_log entry that should be loaded
    */
    public function load_With_TLogId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_log WHERE TLogId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    
    /**
    * update attributes of the object to the DB.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket_log SET Timestamp = :timestamp, Query = :query, Author = :author, Ticket = :ticket WHERE TLogId=:id";
        $values = Array('id' => $this->getTLogId(), 'timestamp' => $this->getTimestamp(), 'query' => $this->getQuery(), 'author' => $this->getAuthor(), 'ticket' => $this->getTicket() );
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get tLogId attribute of the object.
    */
    public function getTLogId(){
        return $this->tLogId;
    }
    
    /**
    * get timestamp attribute of the object.
    */
    public function getTimestamp(){
        return Helpers::outputTime($this->timestamp);
    }
    
    /**
    * get query attribute of the object.
    */
    public function getQuery(){
        return $this->query;
    }
    
    /**
    * get author attribute of the object.
    */
    public function getAuthor(){
        return $this->author;
    }
   
    /**
    * get ticket attribute of the object.
    */
    public function getTicket(){
        return $this->ticket;
    }
    
    /**
    * get the action id out of the query by decoding it.
    */
    public function getAction(){
        $decodedQuery = json_decode($this->query);
        return $decodedQuery[0];
    }
    
    /**
    * get the argument out of the query by decoding it.
    */
    public function getArgument(){
        $decodedQuery = json_decode($this->query);
        return $decodedQuery[1];
    }
    
    /**
    * get the action text(string) array.
    * this is being read from the language .ini files.
    */
    public static function getActionTextArray(){
       $variables = Helpers::handle_language();
       $result = array();
       foreach ( $variables['ticket_log'] as $key => $value ){
              $result[$key] = $value;
           }
        return $result;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    /**
    * set tLogId attribute of the object.
    * @param $id integer id of the log entry
    */
    public function setTLogId($id){
        $this->tLogId = $id;
    }
    
    /**
    * set timestamp attribute of the object.
    * @param $t timestamp of the log entry
    */
    public function setTimestamp($t){
        $this->timestamp = $t;
    }
    
    /**
    * set query attribute of the object.
    * @param $q the encoded query
    */
    public function setQuery($q){
        $this->query = $q;
    }
    
    /**
    * set author attribute of the object.
    * @param $a integer id of the user who created the log entry
    */
    public function setAuthor($a){
        $this->author = $a;
    }
    
    /**
    * set ticket attribute of the object.
    * @param $t integer id of ticket of which the log entry is related to.
    */
    public function setTicket($t){
        $this->ticket = $t;
    }
   
    
}