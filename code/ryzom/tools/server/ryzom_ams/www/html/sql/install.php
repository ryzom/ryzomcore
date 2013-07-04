<?php
    require( '../../config.php' );
    require( '../../../ams_lib/libinclude.php' );
    ini_set( "display_errors", true );
    error_reporting( E_ALL );
    
    global $cfg;
    

    try{
        //SETUP THE WWW DB
        $dbw = new DBLayer($cfg['db']['web']);
        $sql = "
            CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['web']['name'] ."`;
            USE `". $cfg['db']['web']['name'] . "`;
            DROP TABLE IF EXISTS ams_user;
            
            CREATE TABLE IF NOT EXISTS `ams_user` (
            `UId` int(10) NOT NULL AUTO_INCREMENT,
            `Login` varchar(64) NOT NULL DEFAULT '',
            `Password` varchar(13) DEFAULT NULL,
            `Email` varchar(255) NOT NULL DEFAULT '',
            `Permission` int(3) NOT NULL DEFAULT 1,
            `FirstName` varchar(255) NOT NULL DEFAULT '',
            `LastName` varchar(255) NOT NULL DEFAULT '',
            `Gender` tinyint(1) unsigned NOT NULL DEFAULT '0',
            `Country` char(2) NOT NULL DEFAULT '',
            PRIMARY KEY (`UId`)
            ) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all users information for ryzom_ams';
            
            );           
        ";
        $dbw->executeWithoutParams($sql);
        
        //SETUP THE AMS_LIB DB
        $dbl = new DBLayer($cfg['db']['lib']);
        $sql = "
            CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`;
            USE `" . $cfg['db']['lib']['name'] ."`;
            DROP TABLE IF EXISTS ams_querycache;
            
            CREATE TABLE ams_querycache (
            `SID` INT NOT NULL AUTO_INCREMENT PRIMARY KEY ,
            `type` VARCHAR( 64 ) NOT NULL ,
            `query` VARCHAR( 512 ) NOT NULL 
            );
            

        ";
        $dbl->executeWithoutParams($sql);
        print "The Lib & Web database were correctly installed! <br />";
        
        //Now create an admin account!
        $hashpass = crypt("admin", Users::generateSALT());
        $params = array(
          'name' => "admin",
          'pass' => $hashpass,
          'mail' => "admin@admin.com",
        );
        Users::createUser($params);
        try{
            $params['permission'] = 2;
            $dbw = new DBLayer($cfg['db']['web']);
            $dbw->execute("INSERT INTO ams_user (Login, Password, Email, Permission) VALUES (:name, :pass, :mail, :permission)",$params);
            print "The admin account is created, you can login with id: admin, pass: admin!";
        }catch (PDOException $e){
            print "There was an error while creating the admin account! ";
        }
        
        
    
        

        
    }catch (PDOException $e) {
        //go to error page or something, because can't access website db
        print "There was an error while installing";
        print_r($e);
    }
    
        