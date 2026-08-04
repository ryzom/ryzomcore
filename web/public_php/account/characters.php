<?php

// Account Management Tool - Characters Page

$pageTitle = 'Characters';
$uid = $_SESSION['account_uid'];

$charactersByDomain = array();

try {
	$db = getNelDatabase();

	// Get all domains the user has access to
	$stmt = $db->prepare('SELECT d.domain_id, d.domain_name, d.ring_db_name FROM permission p JOIN domain d ON p.DomainId = d.domain_id WHERE p.UId = :uid');
	$stmt->execute(array(':uid' => $uid));
	$userDomains = $stmt->fetchAll();

	// For each domain, query the ring database for the user's characters
	foreach ($userDomains as $domain) {
		if (empty($domain['ring_db_name']) || !isSafeDatabaseName($domain['ring_db_name'])) {
			continue;
		}
		try {
			$ringDb = getRingDatabase($domain['ring_db_name']);

			// Get ring user info
			$stmt = $ringDb->prepare('SELECT user_id, user_name, current_status, current_activity, current_session FROM ring_users WHERE user_id = :uid');
			$stmt->execute(array(':uid' => $uid));
			$ringUser = $stmt->fetch();

			// Get characters for this user
			$stmt = $ringDb->prepare('SELECT char_id, char_name, race, civilisation, cult, current_session, ring_access, creation_date FROM characters WHERE user_id = :uid ORDER BY char_name');
			$stmt->execute(array(':uid' => $uid));
			$characters = $stmt->fetchAll();

			$charactersByDomain[] = array(
				'domain' => $domain,
				'ring_user' => $ringUser,
				'characters' => $characters,
			);
		} catch (PDOException $e) {
			$charactersByDomain[] = array(
				'domain' => $domain,
				'ring_user' => null,
				'characters' => array(),
				'error' => 'Could not connect to ring database.',
			);
		}
	}
} catch (PDOException $e) {
	$error = 'Could not load character information.';
}

ob_start();
?>
<div class="container">
	<?php if ($error): ?>
		<div class="alert alert-error"><?php echo h($error); ?></div>
	<?php endif; ?>

	<?php if (empty($charactersByDomain)): ?>
		<div class="card">
			<h2>Characters</h2>
			<div class="empty-state">
				<p>You don't have access to any domains yet.</p>
				<p>Characters will appear here once you have domain access.</p>
			</div>
		</div>
	<?php else: ?>
		<?php foreach ($charactersByDomain as $entry): ?>
			<div class="card">
				<h2><?php echo h($entry['domain']['domain_name']); ?> &mdash; Characters</h2>
				<?php if (isset($entry['error'])): ?>
					<div class="alert alert-error"><?php echo h($entry['error']); ?></div>
				<?php else: ?>
					<?php if ($entry['ring_user']): ?>
						<div style="margin-bottom: 1rem; display:flex; gap: 1rem; flex-wrap: wrap;">
							<div>
								<span class="stat-label">Ring User</span>
								<span style="color:#ecf0f1; margin-left:0.25rem;"><?php echo h($entry['ring_user']['user_name']); ?></span>
							</div>
							<div>
								<span class="stat-label">Status</span>
								<?php
								$st = $entry['ring_user']['current_status'];
								if ($st === 'cs_online') echo '<span class="badge badge-green">Online</span>';
								elseif ($st === 'cs_logged') echo '<span class="badge badge-yellow">Logged In</span>';
								else echo '<span class="badge badge-gray">Offline</span>';
								?>
							</div>
							<?php if ($entry['ring_user']['current_activity'] && $entry['ring_user']['current_activity'] !== 'ca_none'): ?>
							<div>
								<span class="stat-label">Activity</span>
								<?php
								$act = $entry['ring_user']['current_activity'];
								if ($act === 'ca_play') echo '<span class="badge badge-green">Playing</span>';
								elseif ($act === 'ca_edit') echo '<span class="badge badge-blue">Editing</span>';
								elseif ($act === 'ca_anim') echo '<span class="badge badge-yellow">Animating</span>';
								else echo '<span class="badge badge-gray">' . h($act) . '</span>';
								?>
							</div>
							<?php endif; ?>
						</div>
					<?php endif; ?>

					<?php if (empty($entry['characters'])): ?>
						<div class="empty-state">
							<p>No characters found in this domain.</p>
						</div>
					<?php else: ?>
						<table>
							<thead>
								<tr>
									<th>Name</th>
									<th>Race</th>
									<th>Civilisation</th>
									<th>Cult</th>
									<th>Ring Access</th>
									<th>Created</th>
								</tr>
							</thead>
							<tbody>
								<?php foreach ($entry['characters'] as $char): ?>
								<tr>
									<td style="color:#ecf0f1; font-weight:600;"><?php echo h($char['char_name']); ?></td>
									<td><?php echo $char['race'] ? h($char['race']) : '&mdash;'; ?></td>
									<td><?php echo $char['civilisation'] ? h($char['civilisation']) : '&mdash;'; ?></td>
									<td><?php echo $char['cult'] ? h($char['cult']) : '&mdash;'; ?></td>
									<td>
										<?php if ($char['ring_access']): ?>
											<span class="badge badge-blue"><?php echo h($char['ring_access']); ?></span>
										<?php else: ?>
											<span class="badge badge-gray">None</span>
										<?php endif; ?>
									</td>
									<td style="color:#8899a6; font-size:0.85rem;"><?php echo $char['creation_date'] ? h($char['creation_date']) : '&mdash;'; ?></td>
								</tr>
								<?php endforeach; ?>
							</tbody>
						</table>
					<?php endif; ?>
				<?php endif; ?>
			</div>
		<?php endforeach; ?>
	<?php endif; ?>
</div>
<?php
$content = ob_get_clean();

/* end of file */
