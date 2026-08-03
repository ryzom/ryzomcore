<?php

// Account Management Tool - Home / Dashboard Page

$pageTitle = 'Home';
$uid = $_SESSION['account_uid'];

$domains = array();
$shards = array();
$permissions = array();
$userInfo = null;

try {
	$db = getNelDatabase();

	// Fetch user info
	$stmt = $db->prepare('SELECT UId, Login, Email, Privilege, GroupName, State FROM user WHERE UId = :uid');
	$stmt->execute(array(':uid' => $uid));
	$userInfo = $stmt->fetch();

	// Fetch all domains
	$stmt = $db->query('SELECT domain_id, domain_name, status, ring_db_name, web_host FROM domain ORDER BY domain_name');
	$domains = $stmt->fetchAll();

	// Fetch all shards
	$stmt = $db->query('SELECT ShardId, domain_id, Name, Online, State, NbPlayers, MOTD FROM shard ORDER BY Name');
	$shards = $stmt->fetchAll();

	// Fetch user permissions
	$stmt = $db->prepare('SELECT perm.DomainId, perm.ShardId, perm.AccessPrivilege, d.domain_name FROM permission perm LEFT JOIN domain d ON perm.DomainId = d.domain_id WHERE perm.UId = :uid');
	$stmt->execute(array(':uid' => $uid));
	$permissions = $stmt->fetchAll();
} catch (PDOException $e) {
	$error = 'Could not load account information.';
}

// Index shards by domain
$shardsByDomain = array();
foreach ($shards as $shard) {
	$did = $shard['domain_id'];
	if (!isset($shardsByDomain[$did])) {
		$shardsByDomain[$did] = array();
	}
	$shardsByDomain[$did][] = $shard;
}

// Index permissions by domain
$permsByDomain = array();
foreach ($permissions as $perm) {
	$permsByDomain[$perm['DomainId']] = $perm;
}

ob_start();
?>
<div class="container">
	<?php if ($error): ?>
		<div class="alert alert-error"><?php echo h($error); ?></div>
	<?php endif; ?>

	<?php if ($userInfo): ?>
	<div class="card">
		<h2>Account Overview</h2>
		<div class="grid grid-3">
			<div>
				<div class="stat-label">Username</div>
				<div class="stat-value" style="font-size:1.1rem;"><?php echo h($userInfo['Login']); ?></div>
			</div>
			<div>
				<div class="stat-label">Email</div>
				<div style="color:#ecf0f1;"><?php echo h($userInfo['Email']); ?></div>
			</div>
			<div>
				<div class="stat-label">Status</div>
				<div>
					<?php if ($userInfo['State'] === 'Online'): ?>
						<span class="badge badge-green">Online</span>
					<?php else: ?>
						<span class="badge badge-gray">Offline</span>
					<?php endif; ?>
					<?php if ($userInfo['Privilege']):
						$privCodes = parsePrivileges($userInfo['Privilege']);
						foreach ($privCodes as $pc): ?>
							<span class="badge badge-blue" title="<?php echo h(privilegeLabel($pc)); ?>"><?php echo h($pc); ?></span>
						<?php endforeach;
					endif; ?>
				</div>
			</div>
		</div>
	</div>
	<?php endif; ?>

	<div class="card">
		<h2>Your Permissions</h2>
		<?php if (empty($permissions)): ?>
			<div class="empty-state"><p>You have no domain or shard permissions.</p></div>
		<?php else: ?>
			<table>
				<thead>
					<tr>
						<th>Domain</th>
						<th>Shard</th>
						<th>Access Privilege</th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($permissions as $perm): ?>
					<tr>
						<td style="color:#ecf0f1;"><?php echo h($perm['domain_name'] ?: '#' . $perm['DomainId']); ?></td>
						<td>
							<?php if ($perm['ShardId'] > 0): ?>
								<?php
								$shardName = null;
								foreach ($shards as $s) {
									if ((int)$s['ShardId'] === (int)$perm['ShardId']) { $shardName = $s['Name']; break; }
								}
								?>
								<span style="color:#ecf0f1;"><?php echo h($shardName ?: '#' . $perm['ShardId']); ?></span>
							<?php else: ?>
								<span class="badge badge-gray">All Shards</span>
							<?php endif; ?>
						</td>
						<td><span class="badge badge-green"><?php echo h($perm['AccessPrivilege']); ?></span></td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		<?php endif; ?>
	</div>

	<div class="card">
		<h2>Domains &amp; Shards</h2>
		<?php if (empty($domains)): ?>
			<div class="empty-state"><p>No domains configured.</p></div>
		<?php else: ?>
			<?php foreach ($domains as $domain): ?>
				<?php
				$did = $domain['domain_id'];
				$hasPerm = isset($permsByDomain[$did]);
				$domainShards = isset($shardsByDomain[$did]) ? $shardsByDomain[$did] : array();
				$statusBadge = 'badge-gray';
				$statusLabel = h($domain['status']);
				if ($domain['status'] === 'ds_open') { $statusBadge = 'badge-green'; $statusLabel = 'Open'; }
				elseif ($domain['status'] === 'ds_dev') { $statusBadge = 'badge-yellow'; $statusLabel = 'Development'; }
				elseif ($domain['status'] === 'ds_restricted') { $statusBadge = 'badge-blue'; $statusLabel = 'Restricted'; }
				elseif ($domain['status'] === 'ds_close') { $statusBadge = 'badge-red'; $statusLabel = 'Closed'; }
				?>
				<div style="margin-bottom: 1rem;">
					<div style="display:flex; align-items:center; gap:0.5rem; margin-bottom:0.5rem;">
						<strong style="color:#ecf0f1;"><?php echo h($domain['domain_name']); ?></strong>
						<span class="badge <?php echo $statusBadge; ?>"><?php echo $statusLabel; ?></span>
						<?php if ($hasPerm): ?>
							<span class="badge badge-green">Access: <?php echo h($permsByDomain[$did]['AccessPrivilege']); ?></span>
						<?php else: ?>
							<span class="badge badge-red">No Access</span>
						<?php endif; ?>
					</div>
					<?php if (!empty($domainShards)): ?>
					<table>
						<thead>
							<tr>
								<th>Shard</th>
								<th>Status</th>
								<th>Players</th>
								<th>Message</th>
							</tr>
						</thead>
						<tbody>
							<?php foreach ($domainShards as $shard): ?>
							<tr>
								<td><?php echo h($shard['Name']); ?></td>
								<td>
									<?php if ($shard['Online']): ?>
										<span class="badge badge-green">Online</span>
									<?php else: ?>
										<span class="badge badge-red">Offline</span>
									<?php endif; ?>
								</td>
								<td><?php echo (int)$shard['NbPlayers']; ?></td>
								<td style="color:#8899a6; font-size:0.85rem;"><?php echo $shard['MOTD'] ? h($shard['MOTD']) : '&mdash;'; ?></td>
							</tr>
							<?php endforeach; ?>
						</tbody>
					</table>
					<?php else: ?>
						<p style="color:#8899a6; font-size:0.85rem;">No shards in this domain.</p>
					<?php endif; ?>
				</div>
			<?php endforeach; ?>
		<?php endif; ?>
	</div>
</div>
<?php
$content = ob_get_clean();

/* end of file */
