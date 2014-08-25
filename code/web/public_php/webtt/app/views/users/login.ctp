<div class="grid_6">
<p style="font-size: larger">Please login or <?php echo $this->Html->link(__('Register', true), array('admin' => false, 'controller' => 'users', 'action' => 'register')); ?>.</p>
<?php
echo $this->Session->flash('auth');
echo $this->Form->create('User', array('action' => 'login'));
echo $this->Form->inputs(array(
'legend' => __('Login', true),
'username',
'password'
));
echo $this->Form->end('Login');
?>
</div>