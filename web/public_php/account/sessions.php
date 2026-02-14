<?php

// Account Management Tool - Ring Sessions Page

$pageTitle = 'Sessions';
$uid = $_SESSION['account_uid'];

$sessionsByDomain = array();
$actionResult = '';
$actionError = '';

try {
	$db = getNelDatabase();

	// Get all domains the user has access to
	$stmt = $db->prepare('SELECT d.domain_id, d.domain_name, d.ring_db_name, d.session_manager_address FROM permission p JOIN domain d ON p.DomainId = d.domain_id WHERE p.UId = :uid');
	$stmt->execute(array(':uid' => $uid));
	$userDomains = $stmt->fetchAll();

	foreach ($userDomains as $domain) {
		if (empty($domain['ring_db_name'])) {
			continue;
		}
		try {
			$ringDb = getRingDatabase($domain['ring_db_name']);

			// Get user's characters in this domain
			$stmt = $ringDb->prepare('SELECT char_id, char_name FROM characters WHERE user_id = :uid ORDER BY char_name');
			$stmt->execute(array(':uid' => $uid));
			$characters = $stmt->fetchAll();
			$charIds = array_map(function ($c) { return (int)$c['char_id']; }, $characters);
			$charMap = array();
			foreach ($characters as $c) {
				$charMap[(int)$c['char_id']] = $c['char_name'];
			}

			// Get sessions owned by this user (exclude mainland sessions which represent shards)
			$stmt = $ringDb->prepare('SELECT s.session_id, s.session_type, s.title, s.description, s.state, s.host_shard_id, s.subscription_slots, s.owner, c.char_name as owner_name FROM sessions s LEFT JOIN characters c ON s.owner = c.char_id WHERE s.session_type != \'st_mainland\' AND s.owner IN (SELECT char_id FROM characters WHERE user_id = :uid) ORDER BY s.session_id DESC');
			$stmt->execute(array(':uid' => $uid));
			$ownedSessions = $stmt->fetchAll();

			// Get sessions the user is participating in (through any character, exclude mainland)
			$participatingSessions = array();
			if (!empty($charIds)) {
				$placeholders = implode(',', array_fill(0, count($charIds), '?'));
				$stmt = $ringDb->prepare("SELECT sp.session_id, sp.char_id, sp.status, sp.kicked, s.session_type, s.title, s.state, s.owner, c.char_name as owner_name FROM session_participant sp JOIN sessions s ON sp.session_id = s.session_id LEFT JOIN characters c ON s.owner = c.char_id WHERE s.session_type != 'st_mainland' AND sp.char_id IN ($placeholders) ORDER BY sp.session_id DESC");
				$stmt->execute($charIds);
				$participatingSessions = $stmt->fetchAll();
			}

			$sessionsByDomain[] = array(
				'domain' => $domain,
				'characters' => $characters,
				'owned_sessions' => $ownedSessions,
				'participating_sessions' => $participatingSessions,
				'char_map' => $charMap,
			);
		} catch (PDOException $e) {
			$sessionsByDomain[] = array(
				'domain' => $domain,
				'characters' => array(),
				'owned_sessions' => array(),
				'participating_sessions' => array(),
				'char_map' => array(),
				'error' => 'Could not connect to ring database.',
			);
		}
	}
} catch (PDOException $e) {
	$error = 'Could not load session information.';
}

// Handle session actions (close, invite)
if ($_SERVER['REQUEST_METHOD'] === 'POST' && csrfValidate()) {
	$action = isset($_POST['session_action']) ? $_POST['session_action'] : '';
	$domainRingDb = isset($_POST['ring_db_name']) ? $_POST['ring_db_name'] : '';
	$sessionId = isset($_POST['session_id']) ? (int)$_POST['session_id'] : 0;

	if ($action && $domainRingDb && $sessionId) {
		try {
			$ringDb = getRingDatabase($domainRingDb);

			if ($action === 'close') {
				// Verify the user owns this session
				$stmt = $ringDb->prepare('SELECT s.session_id FROM sessions s JOIN characters c ON s.owner = c.char_id WHERE s.session_id = :sid AND c.user_id = :uid');
				$stmt->execute(array(':sid' => $sessionId, ':uid' => $uid));
				if ($stmt->fetch()) {
					$stmt = $ringDb->prepare("UPDATE sessions SET state = 'ss_closed' WHERE session_id = :sid AND state != 'ss_closed'");
					$stmt->execute(array(':sid' => $sessionId));
					$actionResult = 'Session closed.';
				} else {
					$actionError = 'You can only close your own sessions.';
				}
			} elseif ($action === 'invite') {
				$inviteCharId = isset($_POST['invite_char_id']) ? (int)$_POST['invite_char_id'] : 0;
				if ($inviteCharId && $sessionId) {
					// Verify the user owns this session
					$stmt = $ringDb->prepare('SELECT s.session_id, s.session_type FROM sessions s JOIN characters c ON s.owner = c.char_id WHERE s.session_id = :sid AND c.user_id = :uid');
					$stmt->execute(array(':sid' => $sessionId, ':uid' => $uid));
					$session = $stmt->fetch();
					if ($session) {
						$status = ($session['session_type'] === 'st_edit') ? 'sps_edit_invited' : 'sps_anim_invited';
						// Check if already participating
						$stmt = $ringDb->prepare('SELECT session_id FROM session_participant WHERE session_id = :sid AND char_id = :cid');
						$stmt->execute(array(':sid' => $sessionId, ':cid' => $inviteCharId));
						if ($stmt->fetch()) {
							$actionError = 'This character is already in the session.';
						} else {
							$stmt = $ringDb->prepare('INSERT INTO session_participant (session_id, char_id, status, kicked) VALUES (:sid, :cid, :status, 0)');
							$stmt->execute(array(':sid' => $sessionId, ':cid' => $inviteCharId, ':status' => $status));
							$actionResult = 'Character invited to session.';
						}
					} else {
						$actionError = 'You can only invite to your own sessions.';
					}
				}
			}

			// Reload page to reflect changes
			if ($actionResult) {
				$_SESSION['flash_success'] = $actionResult;
				redirect('sessions');
			}
		} catch (PDOException $e) {
			$actionError = 'Action failed. Please try again.';
		}
	}
}

// Flash messages
if (isset($_SESSION['flash_success'])) {
	$success = $_SESSION['flash_success'];
	unset($_SESSION['flash_success']);
}

/**
 * Format session type for display.
 */
function formatSessionType($type)
{
	$types = array(
		'st_edit' => 'Edit',
		'st_anim' => 'Animation',
		'st_outland' => 'Outland',
		'st_mainland' => 'Mainland',
	);
	return isset($types[$type]) ? $types[$type] : $type;
}

/**
 * Format session state for display.
 */
function formatSessionState($state)
{
	$states = array(
		'ss_planned' => array('Planned', 'badge-blue'),
		'ss_open' => array('Open', 'badge-green'),
		'ss_locked' => array('Locked', 'badge-yellow'),
		'ss_closed' => array('Closed', 'badge-gray'),
	);
	return isset($states[$state]) ? $states[$state] : array($state, 'badge-gray');
}

/**
 * Format participant status for display.
 */
function formatParticipantStatus($status)
{
	$statuses = array(
		'sps_play_subscribed' => array('Subscribed', 'badge-blue'),
		'sps_play_invited' => array('Invited', 'badge-yellow'),
		'sps_edit_invited' => array('Edit Invited', 'badge-yellow'),
		'sps_anim_invited' => array('Anim Invited', 'badge-yellow'),
		'sps_playing' => array('Playing', 'badge-green'),
		'sps_editing' => array('Editing', 'badge-blue'),
		'sps_animating' => array('Animating', 'badge-yellow'),
	);
	return isset($statuses[$status]) ? $statuses[$status] : array($status, 'badge-gray');
}

ob_start();
?>
<div class="container">
	<?php if ($error): ?>
		<div class="alert alert-error"><?php echo h($error); ?></div>
	<?php endif; ?>
	<?php if ($actionError): ?>
		<div class="alert alert-error"><?php echo h($actionError); ?></div>
	<?php endif; ?>
	<?php if ($success): ?>
		<div class="alert alert-success"><?php echo h($success); ?></div>
	<?php endif; ?>

	<?php if (empty($sessionsByDomain)): ?>
		<div class="card">
			<h2>Ring Sessions</h2>
			<div class="empty-state">
				<p>You don't have access to any domains yet.</p>
			</div>
		</div>
	<?php else: ?>
		<?php foreach ($sessionsByDomain as $entry): ?>
			<?php if (isset($entry['error'])): ?>
				<div class="card">
					<h2><?php echo h($entry['domain']['domain_name']); ?> — Sessions</h2>
					<div class="alert alert-error"><?php echo h($entry['error']); ?></div>
				</div>
			<?php else: ?>
				<!-- Owned Sessions -->
				<div class="card">
					<h2><?php echo h($entry['domain']['domain_name']); ?> — Your Sessions</h2>
					<?php if (empty($entry['owned_sessions'])): ?>
						<div class="empty-state"><p>You have no sessions in this domain.</p></div>
					<?php else: ?>
						<table>
							<thead>
								<tr>
									<th>ID</th>
									<th>Title</th>
									<th>Type</th>
									<th>State</th>
									<th>Actions</th>
								</tr>
							</thead>
							<tbody>
								<?php foreach ($entry['owned_sessions'] as $sess): ?>
									<?php $stateInfo = formatSessionState($sess['state']); ?>
									<tr>
										<td>#<?php echo (int)$sess['session_id']; ?></td>
										<td style="color:#ecf0f1;"><?php echo h($sess['title'] ?: '(untitled)'); ?></td>
										<td><span class="badge badge-blue"><?php echo formatSessionType($sess['session_type']); ?></span></td>
										<td><span class="badge <?php echo $stateInfo[1]; ?>"><?php echo $stateInfo[0]; ?></span></td>
										<td>
											<?php if ($sess['state'] !== 'ss_closed'): ?>
												<form method="post" action="index.php?page=sessions" style="display:inline;">
													<?php echo csrfField(); ?>
													<input type="hidden" name="session_action" value="close">
													<input type="hidden" name="ring_db_name" value="<?php echo h($entry['domain']['ring_db_name']); ?>">
													<input type="hidden" name="session_id" value="<?php echo (int)$sess['session_id']; ?>">
													<button type="submit" class="btn btn-sm btn-danger" onclick="return confirm('Close this session?');">Close</button>
												</form>
											<?php endif; ?>
										</td>
									</tr>
									<?php if ($sess['state'] !== 'ss_closed'): ?>
									<tr>
										<td></td>
										<td colspan="4">
											<form method="post" action="index.php?page=sessions" class="form-inline">
												<?php echo csrfField(); ?>
												<input type="hidden" name="session_action" value="invite">
												<input type="hidden" name="ring_db_name" value="<?php echo h($entry['domain']['ring_db_name']); ?>">
												<input type="hidden" name="session_id" value="<?php echo (int)$sess['session_id']; ?>">
												<div class="form-group">
													<label>Invite Character ID</label>
													<input type="text" name="invite_char_id" placeholder="Character ID" style="width:10rem;">
												</div>
												<button type="submit" class="btn btn-sm btn-secondary">Invite</button>
											</form>
										</td>
									</tr>
									<?php endif; ?>
								<?php endforeach; ?>
							</tbody>
						</table>
					<?php endif; ?>
				</div>

				<!-- Participating Sessions -->
				<?php if (!empty($entry['participating_sessions'])): ?>
				<div class="card">
					<h2><?php echo h($entry['domain']['domain_name']); ?> — Sessions You're In</h2>
					<table>
						<thead>
							<tr>
								<th>ID</th>
								<th>Title</th>
								<th>Type</th>
								<th>State</th>
								<th>Your Status</th>
								<th>Owner</th>
								<th>As Character</th>
							</tr>
						</thead>
						<tbody>
							<?php foreach ($entry['participating_sessions'] as $part): ?>
								<?php
								$stateInfo = formatSessionState($part['state']);
								$statusInfo = formatParticipantStatus($part['status']);
								$charName = isset($entry['char_map'][(int)$part['char_id']]) ? $entry['char_map'][(int)$part['char_id']] : '#' . $part['char_id'];
								?>
								<tr>
									<td>#<?php echo (int)$part['session_id']; ?></td>
									<td style="color:#ecf0f1;"><?php echo h($part['title'] ?: '(untitled)'); ?></td>
									<td><span class="badge badge-blue"><?php echo formatSessionType($part['session_type']); ?></span></td>
									<td><span class="badge <?php echo $stateInfo[1]; ?>"><?php echo $stateInfo[0]; ?></span></td>
									<td>
										<span class="badge <?php echo $statusInfo[1]; ?>"><?php echo $statusInfo[0]; ?></span>
										<?php if ($part['kicked']): ?>
											<span class="badge badge-red">Kicked</span>
										<?php endif; ?>
									</td>
									<td style="color:#8899a6;"><?php echo h($part['owner_name'] ?: '#' . $part['owner']); ?></td>
									<td><?php echo h($charName); ?></td>
								</tr>
							<?php endforeach; ?>
						</tbody>
					</table>
				</div>
				<?php endif; ?>
			<?php endif; ?>
		<?php endforeach; ?>
	<?php endif; ?>
</div>
<?php
$content = ob_get_clean();

/* end of file */
