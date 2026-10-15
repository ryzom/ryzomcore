Hello <?php echo $user['User']['name']; ?>,
Thank you for registering to WebTT!

Your password is: <?php echo $user['User']['passwd']; echo "\n"; ?>
<?php
$url = $this->Html->url(array('controller' => 'users', 'action' => 'confirm', $user['User']['confirm_hash']));
$completeUrl = 'http://' . $serverName . $url;
?>
Go to this link to confirm you account: <?php echo $this->Html->link(__('Confirm', true), $completeUrl); ?>
