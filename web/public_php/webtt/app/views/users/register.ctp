<div class="grid_6">
<?php
echo $this->Session->flash('email');

echo $this->Session->flash('auth');
echo $this->Form->create('User', array('action' => 'register'));
echo $this->Form->inputs(array(
'legend' => __('Register', true),
'username',
'name',
'email',
'passwd',
));
echo $this->Form->end('Register');
?>
</div>