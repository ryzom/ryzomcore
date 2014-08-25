<?php

/**
 * API for loading and interacting with plugins
 * contains getters and setters.
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
     private $update_info = array();
    
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
         $this -> setPluginInfo( json_decode( $values['Info'] ) );
         @$this -> setUpdateInfo( json_decode( $values['UpdateInfo'] ) );
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
    
    /**
     * functionalities for plugin updates
     */
    
    /**
     * set update info attribute array of the object.
     * 
     * @param  $p_n array
     */
    public function setUpdateInfo( $p_n ) {
        $this -> update_info = $p_n;
         } 
    
    /**
     * get plugin info array attribute of the object.
     */
    public function getUpdateInfo() {
        return $this -> update_info;
         } 
    
    /**
     * some more plugin function that requires during plugin operations
     */
    
    /**
     * function to remove  a non empty directory
     * 
     * @param  $dir directory address
     * @return boolean 
     */
    public static function rrmdir( $dir ) {
        $result = array_diff( scandir( $dir ), array( '.', '..' ) );
         foreach( $result as $item )
         {
            if ( !@unlink( $dir . '/' . $item ) )
                 Plugincache :: rrmdir( $dir . '/' . $item );
             } 
        return rmdir( $dir );
         } 
    
    /**
     * function to unzip the zipped files
     * 
     * @param  $target_path path to the target zipped file
     * @param  $destination path to the destination
     * @return boolean 
     */
    public static function zipExtraction( $target_path, $destination )
     {
        $zip = new ZipArchive();
         $x = $zip -> open( $target_path );
         if ( $x === true ) {
            if ( $zip -> extractTo( $destination ) )
                 {
                $zip -> close();
                 return true;
                 } 
            else
                 {
                $zip -> close();
                 return false;
                 } 
            } 
        } 
    
    /**
     * Returns plugin information with respect to the id.
     * 
     * @param $id  plugin id.
     * @param $fieldName string plugin field to return
     *  
     * @return info field from the db.
     */
    public static function pluginInfoUsingId( $id, $fieldName )
     {
        $db = new DBLayer( 'lib' );
         $sth = $db -> selectWithParameter( $fieldName, 'plugins', array( 'id' => $id ), 'Id=:id' );
         $row = $sth -> fetch();
         return $row[$fieldName];
         } 
    
    /**
     * Function provides list of active plugins
     * 
     * @return  list of active plugins
     */
    public static function activePlugins()
     {
        $db = new DBLayer( 'lib' );
         $sth = $db -> selectWithParameter( 'Id', 'plugins', array( 'status' => 1 ), 'Status=:status' );
         $row = $sth -> fetchAll();
         return $row;
         } 
    
    /**
     * function to load hooks for the active plugins
     * and return the contents get from them.
     * 
     * -->Get the list of active plugins then call the global
     *    hooks exists in the plugins hook file ($pluginName.php).
     * -->Collect the contents from the hooks and associate within 
     *    array with key referenced plugin name. 
     * -->return the content to use with smarty template loader   
     * 
     * @return $content content get from hooks
     */
    public static function loadHooks()
     {
        $content = array();
         $ac_arr = Plugincache :: activePlugins();
         foreach( $ac_arr as $key => $value )
         {
            $plugin_path = Plugincache :: pluginInfoUsingId( $value['Id'], 'FileName' );
             $template_path = json_decode( Plugincache :: pluginInfoUsingId( $value['Id'], 'Info' ) ) -> TemplatePath;
             $plugin_name = explode( '/', $plugin_path )[4];
            
             // calling hooks in the $pluginName.php
            include $plugin_path . '/' . $plugin_name . '.php';
             $arr = get_defined_functions();
            
             foreach( $arr['user'] as $key => $value )
             {
                if ( stristr( $value, $plugin_name ) == true )
                     {
                    $content['hook_info'][$plugin_name] = call_user_func( $value );
                     } 
                } 
            // path for the template
            $content['hook_info'][$plugin_name]['TemplatePath'] = $template_path;
             } 
        
        return $content;
         } 
    }
