 <?php

/**
*
* contains the getters and setters for plugins 
* @author shubham meena mentored by Matthew Lagoe   
**/

class Plugincache{
    private $id;   
    private $plugin_name;   
    private $plugin_version;   
    private $plugin_permission;   
    private $plugin_isactive;

    /**
    * A constructor.
    * Empty constructor
    */

    public function __construct() {
    }

    public function set($values) {
        $this->setId($values['PluginId']);
        $this->setPluginName($values['PluginName']);
        $this->setPluginVersion($values['PluginVersion']);
        $this->setPluginPermission($values['PluginPermission']);
    	$this->setIsActive($values['IsActive']);
    }

    /**
    * loads the object's attributes.
    */
    public function load_With_SID( ) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM plugins");
        $row = $statement->fetch();
        $this->set($row);
    } 

    /**
    * updates the entry.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $values = Array('t' => $this->getPluginPermission(), 'q' => $this->getPluginVersion(), 'd' => $this->getIsActive());
        $dbl->update("plugins", $values, "PluginName= $this->getPluginName()");
    }
    
    public function getId(){
        return $this->Id;
    }

    /**
    * get plugin permission attribute of the object.
    */
    public function getPluginPermission(){
      return $this->plugin_permission;
    }

    /**
    * get plugin version attribute of the object.
    */
    public function getPluginVersion(){
        return $this->plugin_version;
    }

    /**
    * get plugin is active attribute of the object.
    */
    public function getIsActive(){
        return $this->plugin_isactive;
    }

    /**
    * get plugin name attribute of the object.
    */
    public function getPluginName(){
        return $this->plugin_name;
    }

    /**
    * set plugin id attribute of the object.
    * @param $s integer id 
    */

    public function setId($s){
        $this->Id = $s;
    }

    /**
    * set plugin permission attribute of the object.
    * @param $t type of the query, set permission
    */
    public function setPluginPermission($t){
        $this->plugin_permission = $t;
    }

    /**
    * set plugin version attribute of the object.
    * @param $q  string to set plugin version
    */
    public function setPluginVersion($q){
        $this->plugin_version= $q;
    }

    /**
    * set plugin is active attribute of the object.
    * @param $d tinyint to set plugin is active or not .
    */
    public function setIsActive($d){
        $this->plugin_isactive= $d;
    }
    
    /**
    * set plugin name attribute of the object.
    * @param $p_n string to set plugin name.
    */
    public function setPluginName($p_n){
        $this->plugin_name= $p_n;
    }
}