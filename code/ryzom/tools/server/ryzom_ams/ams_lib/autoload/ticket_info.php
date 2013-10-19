<?php
/**
* Class that handles additional info sent by ticket creation ingame.
* If a user creates a ticket ingame, there are a lot of extra $_GET parameters being sent inside the http request that might have something todo with the ticket.
* for example the OS the user uses or the processor of it's computer, but also the current client version etc.
* This information can be stored and retrieved by using the ticket_info class.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket_Info{
    
    private $tInfoId; /**< The id of ticket_info entry */
    private $ticket;  /**< The ticket linked to this ticket_info entry */
    
    private $shardid; /**< The shard id */
    private $user_position; /**< The user's character position */
    private $view_position; /**< The view position of the character */
    private $client_version; /**< The client version in use */
    private $patch_version; /**< The patch version in use */
    private $server_tick; /**< The current server tick */
    private $connect_state; /**< The connect state */
    private $local_address;  /**< local ip */
    private $memory; /**< memory usage information */
    private $os; /**< os information */
    private $processor;  /**< processor information */
    private $cpu_id; /**< the cpu id */
    private $cpu_mask; /**< the cpu mask */
    private $ht; /**< tbh I have no idea :D */
    private $nel3d; /**< the nel3d version */
    private $user_id; /**< The users id */

    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    
    /**
    * create a ticket_info entry.
    * @param $info_array the info array (this can be the entire $_GET array being sent by the ingame browser)
    */
    public static function create_Ticket_Info($info_array) {
        $ticket_info = new self();
        $ticket_info->set($info_array);
        $ticket_info->create();
    }
     
     
    /**
    * check if a specific ticket has extra info or not.
    * Not all tickets have extra info, only tickets made ingame do. This function checks if a specific ticket does have a ticket_info entry linked to it.
    * @param $ticket_id the id of the ticket that we want to query
    * @return true or false
    */
    public static function TicketHasInfo($ticket_id) {
        $dbl = new DBLayer("lib");
        //check if ticket is already assigned
        if(  $dbl->execute(" SELECT * FROM `ticket_info` WHERE `Ticket` = :ticket_id", array('ticket_id' => $ticket_id) )->rowCount() ){
            return true;
        }else{
            return false;
        } 
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
        $this->setTicket($values['Ticket']);
        $this->setShardId($values['ShardId']);
        $this->setUser_Position($values['UserPosition']);
        $this->setView_Position($values['ViewPosition']);    
        $this->setClient_Version($values['ClientVersion']);
        $this->setPatch_Version($values['PatchVersion']);
        $this->setServer_Tick($values['ServerTick']);
        $this->setConnect_State($values['ConnectState']);
        $this->setLocal_Address($values['LocalAddress']);   
        $this->setMemory($values['Memory']);
        $this->setOS($values['OS']);
        $this->setProcessor($values['Processor']);
        $this->setCPUId($values['CPUID']);
        $this->setCPU_Mask($values['CpuMask']);
        $this->setHT($values['HT']);
        $this->setNel3D($values['NeL3D']);
        $this->setUser_Id($values['UserId']);
      
    } 

    /**
    * loads the object's attributes by using a ticket_info id.
    * loads the object's attributes by giving a ticket_info's entry id.
    * @param $id the id of the ticket_info entry that should be loaded
    */
    public function load_With_TInfoId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_info WHERE TInfoId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    
    /**
    * loads the object's attributes by using a ticket's id.
    * loads the object's attributes by giving a ticket's entry id.
    * @param $id the id of the ticket, the ticket_info entry of that ticket should be loaded.
    */
    public function load_With_Ticket( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_info WHERE Ticket=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    
    /**
    * creates a new 'ticket_info' entry.
    * this method will use the object's attributes for creating a new 'ticket_info' entry in the database.
    */
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_info ( Ticket, ShardId, UserPosition,ViewPosition, ClientVersion, PatchVersion,ServerTick, ConnectState, LocalAddress, Memory, OS, 
Processor, CPUID, CpuMask, HT, NeL3D,  UserId) VALUES ( :ticket, :shardid, :userposition, :viewposition, :clientversion, :patchversion, :servertick, :connectstate, :localaddress, :memory, :os, :processor, :cpuid, :cpu_mask, :ht, :nel3d, :user_id )";
        $values = Array('ticket' => $this->getTicket(), 'shardid' => $this->getShardId(), 'userposition' => $this->getUser_Position(), 'viewposition' => $this->getView_Position(), 'clientversion' => $this->getClient_Version(),
'patchversion' => $this->getPatch_Version(), 'servertick' => $this->getServer_Tick(), 'connectstate' => $this->getConnect_State(), 'localaddress' => $this->getLocal_Address(), 'memory' => $this->getMemory(), 'os'=> $this->getOS(), 'processor' => $this->getProcessor(), 'cpuid' => $this->getCPUId(),
'cpu_mask' => $this->getCpu_Mask(), 'ht' => $this->getHT(), 'nel3d' => $this->getNel3D(), 'user_id' => $this->getUser_Id());
        $dbl->execute($query, $values);
    }

    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get tInfoId attribute of the object.
    */
    public function getTInfoId(){
        return $this->tInfoId;
    }
    
    /**
    * get ticket attribute of the object.
    */
    public function getTicket(){
        return $this->ticket;
    }
    
    /**
    * get shardid attribute of the object.
    */
    public function getShardId(){
        return $this->shardid;
    }
    
    /**
    * get user_position attribute of the object.
    */
    public function getUser_Position(){
        return $this->user_position;
    }
   
    /**
    * get view_position attribute of the object.
    */
    public function getView_Position(){
        return $this->view_position;
    }
    
    /**
    * get client_version attribute of the object.
    */
    public function getClient_Version(){
        return $this->client_version;
    }
    
    /**
    * get patch_version attribute of the object.
    */
    public function getPatch_Version(){
       return $this->patch_version;
    }
    
    /**
    * get server_tick attribute of the object.
    */
    public function getServer_Tick(){
        return $this->server_tick;
    }
    
    /**
    * get connect_state attribute of the object.
    */
    public function getConnect_State(){
        return $this->connect_state;
    }
   
    /**
    * get local_address attribute of the object.
    */
    public function getLocal_Address(){
        return $this->local_address;
    }
    
    /**
    * get memory attribute of the object.
    */
    public function getMemory(){
        return $this->memory;
    }
    
    /**
    * get os attribute of the object.
    */
    public function getOS(){
       return $this->os;
    }
    
    /**
    * get processor attribute of the object.
    */
    public function getProcessor(){
        return $this->processor;
    }
    
    /**
    * get cpu_id attribute of the object.
    */
    public function getCPUId(){
        return $this->cpu_id;
    }
    
    /**
    * get cpu_mask attribute of the object.
    */
    public function getCPU_Mask(){
       return $this->cpu_mask;
    }
    
    /**
    * get ht attribute of the object.
    */
    public function getHT(){
       return $this->ht;
    }
    
    /**
    * get nel3d attribute of the object.
    */
    public function getNel3D(){
        return $this->nel3d;
    }
    
    /**
    * get user_id attribute of the object.
    */
    public function getUser_Id(){
       return $this->user_id;
    }
    

    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    /**
    * set tInfoId attribute of the object.
    * @param $id integer id of ticket_info object itself
    */
    public function setTInfoId($id){
        $this->tInfoId = $id;
    }   
    
    /**
    * set ticket attribute of the object.
    * @param $t integer id of the ticket linked to the info object
    */
    public function setTicket($t){
        $this->ticket = $t;
    }
    
    /**
    * set shardid attribute of the object.
    * @param $s (integer) shard id
    */
    public function setShardId($s){
        $this->shardid = $s;
    }
    
    /**
    * set user_position attribute of the object.
    * @param $u the users position
    */
    public function setUser_Position($u){
        $this->user_position = $u;
    }
    
    /**
    * set view_position attribute of the object.
    * @param $v the view position
    */
    public function setView_Position($v){
        $this->view_position = $v;
    }
    
    /**
    * set client_version attribute of the object.
    * @param $c client version number
    */
    public function setClient_Version($c){
        $this->client_version = $c;
    }
    
    /**
    * set patch_version attribute of the object.
    * @param $p patch version number
    */
    public function setPatch_Version($p){
       $this->patch_version = $p;
    }
    
    /**
    * set server_tick attribute of the object.
    * @param $s integer that resembles the server tick
    */
    public function setServer_Tick($s){
        $this->server_tick = $s;
    }
    
    /**
    * set connect_state attribute of the object.
    * @param $c string that defines the connect state.
    */
    public function setConnect_State($c){
        $this->connect_state = $c;
    }
   
    /**
    * set local_address attribute of the object.
    * @param $l local address
    */
    public function setLocal_Address($l){
        $this->local_address = $l;
    }
    
    /**
    * set memory attribute of the object.
    * @param $m memory usage 
    */
    public function setMemory($m){
        $this->memory = $m;
    }
    
    /**
    * set os attribute of the object.
    * @param $o set os version information
    */
    public function setOS($o){
       $this->os = $o;
    }
    
    /**
    * set processor attribute of the object.
    * @param $p processor information
    */
    public function setProcessor($p){
        $this->processor = $p;
    }
    
    /**
    * set cpu_id attribute of the object.
    * @param $c cpu id information
    */
    public function setCPUId($c){
        $this->cpu_id = $c;
    }
    
    /**
    * set cpu_mask attribute of the object.
    * @param $c mask of the cpu
    */
    public function setCPU_Mask($c){
       $this->cpu_mask = $c;
    }
    
    /**
    * set ht attribute of the object.
    */
    public function setHT($h){
       $this->ht = $h;
    }
    
    /**
    * set nel3d attribute of the object.
    * @param $n version information about NeL3D
    */
    public function setNel3D($n){
        $this->nel3d = $n;
    }
    
    /**
    * set user_id attribute of the object.
    * @param $u the user_id.
    */
    public function setUser_Id($u){
        $this->user_id = $u;
    }
    

}