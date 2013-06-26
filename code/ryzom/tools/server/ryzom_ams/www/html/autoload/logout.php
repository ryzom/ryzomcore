<?php

session_unset();
session_destroy(); 
$pageElements['no_visible_elements'] = 'TRUE';
helpers :: loadtemplate( 'logout', $pageElements);
exit();
