<?php

require( '../config.php' );

require( '../../ams_lib/libinclude.php' );

if (isset($_POST["function"])){
    require("inc/".$_POST["function"].".php");
    $_POST["function"]();
}

function loadpage ($page){
    require_once('autoload/'.$page.'.php');
}

$page = 'home';
if (isset($_GET["page"])) {
    $page = $_GET["page"];  
}


helpers::loadtemplate('templates/header.phtml');
loadpage($page);
helpers::loadtemplate('templates/footer.phtml');