<?php

// Account Management Tool - Admin Page
// Requires :DEV:, :SGM:, or :GM: privilege

$pageTitle = 'Admin';
$uid = $_SESSION['account_uid'];

$users = array();
$editUser = null;
$editPerms = array();
$domains = array();
$shards = array();
$searchQuery = isset($_GET['q']) ? trim($_GET['q']) : '';
$editUid = isset($_GET['uid']) ? (int)$_GET['uid'] : 0;

try {
	$db = getNelDatabase();

	// Fetch domains and shards for reference
	$stmt = $db->query('SELECT domain_id, domain_name, status FROM domain ORDER BY domain_name');
	$domains = $stmt->fetchAll();

	$stmt = $db->query('SELECT ShardId, domain_id, Name FROM shard ORDER BY Name');
	$shards = $stmt->fetchAll();

	// Handle POST actions
	if ($_SERVER['REQUEST_METHOD'] === 'POST' && csrfValidate()) {
		if (isset($_POST['save_privileges'])) {
			$targetUid = (int)$_POST['target_uid'];
			$newPrivilege = trim($_POST['privilege']);
			// Ensure privilege format is correct: colon-delimited, e.g. ":DEV:GM:"
			if ($newPrivilege !== '' && $newPrivilege[0] !== ':') {
				$newPrivilege = ':' . $newPrivilege;
			}
			if ($newPrivilege !== '' && substr($newPrivilege, -1) !== ':') {
				$newPrivilege .= ':';
			}
			$stmt = $db->prepare('UPDATE user SET Privilege = :priv WHERE UId = :uid');
			$stmt->execute(array(':priv' => $newPrivilege, ':uid' => $targetUid));
			$success = 'Privileges updated.';
			$editUid = $targetUid;
		}
		if (isset($_POST['add_permission'])) {
			$targetUid = (int)$_POST['target_uid'];
			$domainId = (int)$_POST['domain_id'];
			$shardId = (int)$_POST['shard_id'];
			$accessPriv = trim($_POST['access_privilege']);
			if ($accessPriv === '') {
				$accessPriv = 'OPEN';
			}
			// Check if permission already exists
			$stmt = $db->prepare('SELECT COUNT(*) as cnt FROM permission WHERE UId = :uid AND DomainId = :did AND ShardId = :sid');
			$stmt->execute(array(':uid' => $targetUid, ':did' => $domainId, ':sid' => $shardId));
			$row = $stmt->fetch();
			if ($row['cnt'] > 0) {
				$stmt = $db->prepare('UPDATE permission SET AccessPrivilege = :priv WHERE UId = :uid AND DomainId = :did AND ShardId = :sid');
				$stmt->execute(array(':priv' => $accessPriv, ':uid' => $targetUid, ':did' => $domainId, ':sid' => $shardId));
			} else {
				$stmt = $db->prepare('INSERT INTO permission (UId, DomainId, ShardId, AccessPrivilege) VALUES (:uid, :did, :sid, :priv)');
				$stmt->execute(array(':uid' => $targetUid, ':did' => $domainId, ':sid' => $shardId, ':priv' => $accessPriv));
			}
			$success = 'Permission added.';
			$editUid = $targetUid;
		}
		if (isset($_POST['remove_permission'])) {
			$targetUid = (int)$_POST['target_uid'];
			$domainId = (int)$_POST['domain_id'];
			$shardId = (int)$_POST['shard_id'];
			$stmt = $db->prepare('DELETE FROM permission WHERE UId = :uid AND DomainId = :did AND ShardId = :sid');
			$stmt->execute(array(':uid' => $targetUid, ':did' => $domainId, ':sid' => $shardId));
			$success = 'Permission removed.';
			$editUid = $targetUid;
		}
	}

	// If editing a user, load their details
	if ($editUid > 0) {
		$stmt = $db->prepare('SELECT UId, Login, Email, Privilege, GroupName, State, ExtendedPrivilege FROM user WHERE UId = :uid');
		$stmt->execute(array(':uid' => $editUid));
		$editUser = $stmt->fetch();

		if ($editUser) {
			$stmt = $db->prepare('SELECT perm.DomainId, perm.ShardId, perm.AccessPrivilege, d.domain_name FROM permission perm LEFT JOIN domain d ON perm.DomainId = d.domain_id WHERE perm.UId = :uid');
			$stmt->execute(array(':uid' => $editUid));
			$editPerms = $stmt->fetchAll();
		}
	}

	// Search or list users
	if ($searchQuery !== '') {
		$stmt = $db->prepare('SELECT UId, Login, Email, Privilege, State FROM user WHERE Login LIKE :q OR Email LIKE :q2 ORDER BY Login LIMIT 50');
		$stmt->execute(array(':q' => '%' . $searchQuery . '%', ':q2' => '%' . $searchQuery . '%'));
		$users = $stmt->fetchAll();
	} elseif (!$editUser) {
		$stmt = $db->query('SELECT UId, Login, Email, Privilege, State FROM user ORDER BY UId LIMIT 50');
		$users = $stmt->fetchAll();
	}
} catch (PDOException $e) {
	$error = 'Database error: ' . $e->getMessage();
}

ob_start();
?>
<div class="container">
	<?php if ($error): ?>
		<div class="alert alert-error"><?php echo h($error); ?></div>
	<?php endif; ?>
	<?php if ($success): ?>
		<div class="alert alert-success"><?php echo h($success); ?></div>
	<?php endif; ?>

	<?php if ($editUser): ?>
	<!-- Edit User View -->
	<div style="margin-bottom: 1rem;">
		<a href="index.php?page=admin">&larr; Back to user list</a>
	</div>

	<div class="card">
		<h2>User: <?php echo h($editUser['Login']); ?></h2>
		<div class="grid grid-3">
			<div>
				<div class="stat-label">User ID</div>
				<div style="color:#ecf0f1;"><?php echo (int)$editUser['UId']; ?></div>
			</div>
			<div>
				<div class="stat-label">Email</div>
				<div style="color:#ecf0f1;"><?php echo h($editUser['Email']); ?></div>
			</div>
			<div>
				<div class="stat-label">Status</div>
				<div>
					<?php if ($editUser['State'] === 'Online'): ?>
						<span class="badge badge-green">Online</span>
					<?php else: ?>
						<span class="badge badge-gray">Offline</span>
					<?php endif; ?>
				</div>
			</div>
		</div>
	</div>

	<div class="card">
		<h2>Privileges</h2>
		<p style="font-size:0.85rem; color:#8899a6; margin-bottom:0.75rem;">
			Privileges are colon-delimited, e.g. <code style="color:#5dade2;">:DEV:GM:</code>.
			Known codes: DEV, SGM, GM, VG, SG, G, EM, EG, CM, OBSERVER, PR.
		</p>
		<?php
		$currentPrivs = parsePrivileges($editUser['Privilege']);
		if (!empty($currentPrivs)): ?>
			<div style="margin-bottom:0.75rem;">
				<?php foreach ($currentPrivs as $pc): ?>
					<span class="badge badge-blue" title="<?php echo h(privilegeLabel($pc)); ?>"><?php echo h($pc); ?></span>
				<?php endforeach; ?>
			</div>
		<?php endif; ?>
		<form method="post" action="index.php?page=admin&amp;uid=<?php echo (int)$editUser['UId']; ?>">
			<?php echo csrfField(); ?>
			<input type="hidden" name="target_uid" value="<?php echo (int)$editUser['UId']; ?>">
			<div class="form-inline">
				<div class="form-group" style="flex:1;">
					<input type="text" name="privilege" value="<?php echo h($editUser['Privilege']); ?>"
						placeholder=":DEV:GM:">
				</div>
				<button type="submit" name="save_privileges" class="btn btn-primary btn-sm">Save Privileges</button>
			</div>
		</form>
	</div>

	<div class="card">
		<h2>Domain &amp; Shard Permissions</h2>
		<?php if (!empty($editPerms)): ?>
			<table>
				<thead>
					<tr>
						<th>Domain</th>
						<th>Shard</th>
						<th>Access</th>
						<th></th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($editPerms as $perm): ?>
					<tr>
						<td style="color:#ecf0f1;"><?php echo h($perm['domain_name'] ?: '#' . $perm['DomainId']); ?></td>
						<td>
							<?php if ($perm['ShardId'] > 0):
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
						<td>
							<form method="post" action="index.php?page=admin&amp;uid=<?php echo (int)$editUser['UId']; ?>" style="display:inline;">
								<?php echo csrfField(); ?>
								<input type="hidden" name="target_uid" value="<?php echo (int)$editUser['UId']; ?>">
								<input type="hidden" name="domain_id" value="<?php echo (int)$perm['DomainId']; ?>">
								<input type="hidden" name="shard_id" value="<?php echo (int)$perm['ShardId']; ?>">
								<button type="submit" name="remove_permission" class="btn btn-danger btn-sm">Remove</button>
							</form>
						</td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		<?php else: ?>
			<p style="color:#8899a6; font-size:0.85rem; margin-bottom:0.75rem;">No permissions assigned.</p>
		<?php endif; ?>

		<div style="margin-top:1rem; padding-top:1rem; border-top:1px solid #2c3e50;">
			<h3 style="font-size:0.9rem; color:#ecf0f1; margin-bottom:0.5rem;">Add Permission</h3>
			<form method="post" action="index.php?page=admin&amp;uid=<?php echo (int)$editUser['UId']; ?>">
				<?php echo csrfField(); ?>
				<input type="hidden" name="target_uid" value="<?php echo (int)$editUser['UId']; ?>">
				<div class="form-inline">
					<div class="form-group">
						<label>Domain</label>
						<select name="domain_id">
							<?php foreach ($domains as $d): ?>
								<option value="<?php echo (int)$d['domain_id']; ?>"><?php echo h($d['domain_name']); ?></option>
							<?php endforeach; ?>
						</select>
					</div>
					<div class="form-group">
						<label>Shard (0 = all)</label>
						<select name="shard_id">
							<option value="0">All Shards</option>
							<?php foreach ($shards as $s): ?>
								<option value="<?php echo (int)$s['ShardId']; ?>"><?php echo h($s['Name']); ?></option>
							<?php endforeach; ?>
						</select>
					</div>
					<div class="form-group">
						<label>Access</label>
						<select name="access_privilege">
							<option value="OPEN">OPEN</option>
							<option value="DEV">DEV</option>
							<option value="RESTRICTED">RESTRICTED</option>
						</select>
					</div>
					<button type="submit" name="add_permission" class="btn btn-primary btn-sm">Add</button>
				</div>
			</form>
		</div>
	</div>

	<?php else: ?>
	<!-- User List View -->
	<div class="card">
		<h2>User Administration</h2>
		<form method="get" action="index.php">
			<input type="hidden" name="page" value="admin">
			<div class="form-inline">
				<div class="form-group" style="flex:1;">
					<input type="text" name="q" value="<?php echo h($searchQuery); ?>"
						placeholder="Search by username or email...">
				</div>
				<button type="submit" class="btn btn-primary btn-sm">Search</button>
				<?php if ($searchQuery !== ''): ?>
					<a href="index.php?page=admin" class="btn btn-secondary btn-sm">Clear</a>
				<?php endif; ?>
			</div>
		</form>
	</div>

	<div class="card">
		<h2><?php echo $searchQuery !== '' ? 'Search Results' : 'Users'; ?></h2>
		<?php if (empty($users)): ?>
			<div class="empty-state"><p>No users found.</p></div>
		<?php else: ?>
			<table>
				<thead>
					<tr>
						<th>ID</th>
						<th>Username</th>
						<th>Email</th>
						<th>Privileges</th>
						<th>Status</th>
						<th></th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($users as $u): ?>
					<tr>
						<td><?php echo (int)$u['UId']; ?></td>
						<td style="color:#ecf0f1;"><?php echo h($u['Login']); ?></td>
						<td><?php echo h($u['Email']); ?></td>
						<td>
							<?php
							$privCodes = parsePrivileges($u['Privilege']);
							if (!empty($privCodes)):
								foreach ($privCodes as $pc): ?>
									<span class="badge badge-blue" title="<?php echo h(privilegeLabel($pc)); ?>"><?php echo h($pc); ?></span>
								<?php endforeach;
							else: ?>
								<span style="color:#8899a6;">&mdash;</span>
							<?php endif; ?>
						</td>
						<td>
							<?php if ($u['State'] === 'Online'): ?>
								<span class="badge badge-green">Online</span>
							<?php else: ?>
								<span class="badge badge-gray">Offline</span>
							<?php endif; ?>
						</td>
						<td>
							<a href="index.php?page=admin&amp;uid=<?php echo (int)$u['UId']; ?>" class="btn btn-secondary btn-sm">Manage</a>
						</td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		<?php endif; ?>
	</div>
	<?php endif; ?>
</div>
<?php
$content = ob_get_clean();

/* end of file */
