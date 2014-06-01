<?php

/**
 * File with function plugins to get
 * plugins from the Database using pagination object	
 * @author shubham meena mentored by Mathew Lagoe
 */

function plugins()
{
	 if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
	 	     
	/**require("../../ams_lib/plugins/plugin.handler.php");
	
	$plugin=new plugin();
	$plugin->init();
	print_r(plugin::$plugins);**/

	$pagination = new Pagination("SELECT * FROM plugins","lib",5,"Plugincache");
        $pageResult['plug']= Gui_Elements::make_table($pagination->getElements() , Array						("getId","getPluginName","getPluginVersion","getPluginPermission","getIsActive"), Array("id","plugin_name","plugin_version","plugin_permission","plugin_isactive"));
        $pageResult['links'] = $pagination->getLinks(5);
        $pageResult['lastPage'] = $pagination->getLast();
        $pageResult['currentPage'] = $pagination->getCurrent();
        
        global $INGAME_WEBPATH;
        $pageResult['ingame_webpath'] = $INGAME_WEBPATH;
        
        //check if shard is online
        try{
            $dbs = new DBLayer("shard");
            $pageResult['shard'] = "online";
        }catch(PDOException $e){ 
            $pageResult['shard'] = "offline";
        }
        return( $pageResult);
    }else{
        //ERROR: No access!
        $_SESSION['error_code'] = "403";
        header("Location: index.php?page=error");
        exit;
    }
	
}
 
 
