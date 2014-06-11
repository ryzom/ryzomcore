<?php

/**
 * API for loading and interacting with plugins
 *   contains getters and setters
 * 
 * @author shubham meena mentored by Matthew Lagoe 
 */

class Plugincache {
    private $id;
     private $plugin_name;
     private $plugin_type;
     private $plugin_permission;
     private $plugin_status;
     private $plugin_info = array();
    
    /**
     * A constructor.
     * Empty constructor
     */
    
     public function __construct() {
        } 
    
    public function set( $values ) {
        $this -> setId( $values['Id'] );
         $this -> setPluginName( $values['Name'] );
         $this -> setPluginType( $values['Type'] );
         $this -> setPluginPermission( $values['Permission'] );
         $this -> setPluginStatus( $values['Status'] );
         $this -> setPluginInfo( $values['Info'] );
         } 
    
    /**
     * loads the object's attributes.
     */
    public function load_With_SID() {
        $dbl = new DBLayer( "lib" );
         $statement = $dbl -> executeWithoutParams( "SELECT * FROM plugins" );
         $row = $statement -> fetch();
         $this -> set( $row );
         } 
    
    /**
     * get plugin id attribute of the object.
     * 
     * @return integer id
     */
    public function getId() {
        return $this -> Id;
         } 
    
    /**
     * get plugin permission attribute of the object.
     */
    public function getPluginPermission() {
        return $this -> plugin_permission;
         } 
    
    /**
     * get plugin Type attribute of the object.
     */
    public function getPluginType() {
        return $this -> plugin_version;
         } 
    
    /**
     * get plugin status attribute of the object.
     */
    public function getPluginStatus() {
        return $this -> plugin_status;
         } 
    
    /**
     * get plugin name attribute of the object.
     */
    public function getPluginName() {
        return $this -> plugin_name;
         } 
    
    /**
     * get plugin info array attribute of the object.
     */
    public function getPluginInfo() {
        return $this -> plugin_info;
         } 
    
    /**
     * set plugin id attribute of the object.
     * 
     * @param  $s integer id
     */
    public function setId( $s ) {
        $this -> Id = $s;
         } 
    
    /**
     * set plugin permission attribute of the object.
     * 
     * @param  $t type of the query, set permission
     */
    public function setPluginPermission( $t ) {
        $this -> plugin_permission = $t;
         } 
    
    /**
     * set plugin version attribute of the object.
     * 
     * @param  $q string to set plugin version
     */
    public function setPluginType( $q ) {
        $this -> plugin_version = $q;
         } 
    
    /**
     * set plugin status attribute of the object.
     * 
     * @param  $d status code type int
     */
    public function setPluginStatus( $d ) {
        $this -> plugin_status = $d;
         } 
    
    /**
     * get plugin name attribute of the object.
     */
    public function getPluginName() {
        return $this -> plugin_name;
         } 
    
    /**
     * set plugin name attribute of the object.
     * 
     * @param  $p_n string to set plugin name.
     */
    public function setPluginName( $p_n ) {
        $this -> plugin_name = $p_n;
         } 
    
    /**
     * set plugin info attribute array of the object.
     * 
     * @param  $p_n array
     */
    public function setPluginInfo( $p_n ) {
        $this -> plugin_info = $p_n;
         } 
    
    }
