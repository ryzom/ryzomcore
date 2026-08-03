<?php

// Account Management Tool - Ring Sessions Page

$pageTitle = 'Sessions';
$uid = $_SESSION['account_uid'];

$sessionsByDomain = array();
$actionResult = '';
$actionError = '';
$error = '';
$success = '';

// Handle session actions (close, invite, remove) -- process before loading data
if ($_SERVER['REQUEST_METHOD'] === 'POST' && csrfValidate()) {
	$action = isset($_POST['session_action']) ? $_POST['session_action'] : '';
	$domainRingDb = isset($_POST['ring_db_name']) ? $_POST['ring_db_name'] : '';
	$sessionId = isset($_POST['session_id']) ? (int)$_POST['session_id'] : 0;
	$rsmAddress = isset($_POST['rsm_address']) ? $_POST['rsm_address'] : '';

	if ($action && $domainRingDb && $sessionId) {
		try {
			$ringDb = getRingDatabase($domainRingDb);

			if ($action === 'close') {
				// Verify the user owns this session and get the owner char_id
				$stmt = $ringDb->prepare('SELECT s.session_id, s.owner FROM sessions s JOIN characters c ON s.owner = c.char_id WHERE s.session_id = :sid AND c.user_id = :uid');
				$stmt->execute(array(':sid' => $sessionId, ':uid' => $uid));
				$sess = $stmt->fetch();
				if ($sess) {
					// Route through the RSM so it can notify the DSS to gracefully stop the scenario
					$rsm = connectToRSM($rsmAddress);
					if ($rsm !== false) {
						$rsm->closeSession((int)$sess['owner'], $sessionId);
						if ($rsm->waitCallback() && $rsm->resultCode == 0) {
							$actionResult = 'Session close requested.';
						} else {
							$actionError = 'Close failed: ' . ($rsm->resultString ?: 'RSM rejected the request (code: ' . $rsm->resultCode . ').');
						}
					} else {
						$actionError = 'Could not connect to the session manager service. Please try again later.';
					}
				} else {
					$actionError = 'You can only close your own sessions.';
				}
			} elseif ($action === 'invite') {
				$inviteCharName = isset($_POST['invite_char_name']) ? trim($_POST['invite_char_name']) : '';
				$inviteCharId = isset($_POST['invite_char_id']) ? (int)$_POST['invite_char_id'] : 0;
				$invCharName = '';

				// Look up character by name if name was provided
				if ($inviteCharName !== '' && !$inviteCharId) {
					$stmt = $ringDb->prepare('SELECT char_id, char_name FROM characters WHERE char_name = :name');
					$stmt->execute(array(':name' => $inviteCharName));
					$found = $stmt->fetch();
					if ($found) {
						$inviteCharId = (int)$found['char_id'];
						$invCharName = $found['char_name'];
					} else {
						$actionError = 'Character "' . $inviteCharName . '" not found.';
					}
				} elseif ($inviteCharId) {
					// Verify the character exists when invited by ID
					$stmt = $ringDb->prepare('SELECT char_id, char_name FROM characters WHERE char_id = :cid');
					$stmt->execute(array(':cid' => $inviteCharId));
					$found = $stmt->fetch();
					if ($found) {
						$invCharName = $found['char_name'];
					} else {
						$actionError = 'Character not found.';
					}
				}

				if ($inviteCharId && $sessionId && !$actionError) {
					// Verify the user owns this session and get the owner char_id
					$stmt = $ringDb->prepare('SELECT s.session_id, s.session_type, s.owner FROM sessions s JOIN characters c ON s.owner = c.char_id WHERE s.session_id = :sid AND c.user_id = :uid');
					$stmt->execute(array(':sid' => $sessionId, ':uid' => $uid));
					$session = $stmt->fetch();
					if ($session) {
						$charRole = new RSMGR_TSessionPartStatus();
						$charRole->fromString(($session['session_type'] === 'st_edit') ? 'sps_edit_invited' : 'sps_anim_invited');

						// Route through the RSM for permission validation
						$rsm = connectToRSM($rsmAddress);
						if ($rsm !== false) {
							$rsm->inviteCharacter((int)$session['owner'], $sessionId, $inviteCharId, $charRole);
							if ($rsm->waitCallback() && $rsm->resultCode == 0) {
								$actionResult = 'Character "' . $invCharName . '" invited to session.';
							} else {
								$actionError = 'Invite failed: ' . ($rsm->resultString ?: 'RSM rejected the request (code: ' . $rsm->resultCode . ').');
							}
						} else {
							$actionError = 'Could not connect to the session manager service. Please try again later.';
						}
					} else {
						$actionError = 'You can only invite to your own sessions.';
					}
				} elseif (!$actionError) {
					$actionError = 'Please enter a character name or ID to invite.';
				}
			} elseif ($action === 'remove') {
				$removeCharId = isset($_POST['remove_char_id']) ? (int)$_POST['remove_char_id'] : 0;
				if ($removeCharId && $sessionId) {
					// Verify the user owns this session and get the owner char_id
					$stmt = $ringDb->prepare('SELECT s.session_id, s.owner FROM sessions s JOIN characters c ON s.owner = c.char_id WHERE s.session_id = :sid AND c.user_id = :uid');
					$stmt->execute(array(':sid' => $sessionId, ':uid' => $uid));
					$sess = $stmt->fetch();
					if ($sess) {
						// Route through the RSM for proper participant removal
						$rsm = connectToRSM($rsmAddress);
						if ($rsm !== false) {
							$rsm->removeInvitedCharacter((int)$sess['owner'], $sessionId, $removeCharId);
							if ($rsm->waitCallback() && $rsm->resultCode == 0) {
								$actionResult = 'Participant removed from session.';
							} else {
								$actionError = 'Remove failed: ' . ($rsm->resultString ?: 'RSM rejected the request (code: ' . $rsm->resultCode . ').');
							}
						} else {
							$actionError = 'Could not connect to the session manager service. Please try again later.';
						}
					} else {
						$actionError = 'You can only manage your own sessions.';
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

// Load session data
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

			// For each owned session, get the current participants
			$sessionParticipants = array();
			foreach ($ownedSessions as $sess) {
				$sid = (int)$sess['session_id'];
				$stmt = $ringDb->prepare('SELECT sp.char_id, sp.status, sp.kicked, c.char_name, c.user_id FROM session_participant sp LEFT JOIN characters c ON sp.char_id = c.char_id WHERE sp.session_id = :sid ORDER BY c.char_name');
				$stmt->execute(array(':sid' => $sid));
				$sessionParticipants[$sid] = $stmt->fetchAll();
			}

			// Get sessions the user is participating in (through any character, exclude mainland)
			$participatingSessions = array();
			if (!empty($charIds)) {
				$placeholders = implode(',', array_fill(0, count($charIds), '?'));
				$stmt = $ringDb->prepare("SELECT sp.session_id, sp.char_id, sp.status, sp.kicked, s.session_type, s.title, s.state, s.owner, c.char_name as owner_name FROM session_participant sp JOIN sessions s ON sp.session_id = s.session_id LEFT JOIN characters c ON s.owner = c.char_id WHERE s.session_type != 'st_mainland' AND sp.char_id IN ($placeholders) ORDER BY sp.session_id DESC");
				$stmt->execute($charIds);
				$participatingSessions = $stmt->fetchAll();
			}

			// Get open public sessions in this domain (not owned by this user, not mainland)
			$stmt = $ringDb->prepare("SELECT s.session_id, s.session_type, s.title, s.description, s.state, s.owner, c.char_name as owner_name, s.subscription_slots, (SELECT COUNT(*) FROM session_participant sp WHERE sp.session_id = s.session_id) as participant_count FROM sessions s LEFT JOIN characters c ON s.owner = c.char_id WHERE s.session_type != 'st_mainland' AND s.state IN ('ss_open', 'ss_planned') AND s.owner NOT IN (SELECT char_id FROM characters WHERE user_id = :uid) ORDER BY s.session_id DESC LIMIT 50");
			$stmt->execute(array(':uid' => $uid));
			$openSessions = $stmt->fetchAll();

			$sessionsByDomain[] = array(
				'domain' => $domain,
				'characters' => $characters,
				'owned_sessions' => $ownedSessions,
				'session_participants' => $sessionParticipants,
				'participating_sessions' => $participatingSessions,
				'open_sessions' => $openSessions,
				'char_map' => $charMap,
			);
		} catch (PDOException $e) {
			$sessionsByDomain[] = array(
				'domain' => $domain,
				'characters' => array(),
				'owned_sessions' => array(),
				'session_participants' => array(),
				'participating_sessions' => array(),
				'open_sessions' => array(),
				'char_map' => array(),
				'error' => 'Could not connect to ring database.',
			);
		}
	}
} catch (PDOException $e) {
	$error = 'Could not load session information.';
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
					<h2><?php echo h($entry['domain']['domain_name']); ?> &mdash; Sessions</h2>
					<div class="alert alert-error"><?php echo h($entry['error']); ?></div>
				</div>
			<?php else: ?>
				<!-- Owned Sessions -->
				<div class="card">
					<h2><?php echo h($entry['domain']['domain_name']); ?> &mdash; Your Sessions</h2>
					<?php if (empty($entry['owned_sessions'])): ?>
						<div class="empty-state"><p>You have no sessions in this domain.</p></div>
					<?php else: ?>
						<?php foreach ($entry['owned_sessions'] as $sess): ?>
							<?php
							$stateInfo = formatSessionState($sess['state']);
							$sid = (int)$sess['session_id'];
							$participants = isset($entry['session_participants'][$sid]) ? $entry['session_participants'][$sid] : array();
							?>
							<div style="border:1px solid #2c3e50; border-radius:0.375rem; padding:1rem; margin-bottom:1rem;">
								<div style="display:flex; align-items:center; gap:0.5rem; margin-bottom:0.75rem; flex-wrap:wrap;">
									<span style="color:#ecf0f1; font-weight:600;">#<?php echo $sid; ?></span>
									<span style="color:#ecf0f1;"><?php echo h($sess['title'] ?: '(untitled)'); ?></span>
									<span class="badge badge-blue"><?php echo formatSessionType($sess['session_type']); ?></span>
									<span class="badge <?php echo $stateInfo[1]; ?>"><?php echo $stateInfo[0]; ?></span>
									<?php if ($sess['state'] !== 'ss_closed'): ?>
										<form method="post" action="index.php?page=sessions" style="display:inline; margin-left:auto;">
											<?php echo csrfField(); ?>
											<input type="hidden" name="session_action" value="close">
											<input type="hidden" name="ring_db_name" value="<?php echo h($entry['domain']['ring_db_name']); ?>">
											<input type="hidden" name="rsm_address" value="<?php echo h($entry['domain']['session_manager_address']); ?>">
											<input type="hidden" name="session_id" value="<?php echo $sid; ?>">
											<button type="submit" class="btn btn-sm btn-danger" onclick="return confirm('Close this session?');">Close</button>
										</form>
									<?php endif; ?>
								</div>

								<?php if ($sess['description']): ?>
									<p style="color:#8899a6; font-size:0.85rem; margin-bottom:0.75rem;"><?php echo h($sess['description']); ?></p>
								<?php endif; ?>

								<!-- Participants -->
								<?php if (!empty($participants)): ?>
									<div style="margin-bottom:0.75rem;">
										<div style="color:#8899a6; font-size:0.8rem; text-transform:uppercase; letter-spacing:0.05em; margin-bottom:0.25rem;">Participants</div>
										<table>
											<thead>
												<tr>
													<th>Character</th>
													<th>Status</th>
													<th></th>
												</tr>
											</thead>
											<tbody>
												<?php foreach ($participants as $part): ?>
													<?php $pStatus = formatParticipantStatus($part['status']); ?>
													<tr>
														<td style="color:#ecf0f1;"><?php echo h($part['char_name'] ?: '#' . $part['char_id']); ?></td>
														<td>
															<span class="badge <?php echo $pStatus[1]; ?>"><?php echo $pStatus[0]; ?></span>
															<?php if ($part['kicked']): ?>
																<span class="badge badge-red">Kicked</span>
															<?php endif; ?>
														</td>
														<td>
															<?php if ($sess['state'] !== 'ss_closed' && (int)$part['user_id'] !== $uid): ?>
																<form method="post" action="index.php?page=sessions" style="display:inline;">
																	<?php echo csrfField(); ?>
																	<input type="hidden" name="session_action" value="remove">
																	<input type="hidden" name="ring_db_name" value="<?php echo h($entry['domain']['ring_db_name']); ?>">
																	<input type="hidden" name="rsm_address" value="<?php echo h($entry['domain']['session_manager_address']); ?>">
																	<input type="hidden" name="session_id" value="<?php echo $sid; ?>">
																	<input type="hidden" name="remove_char_id" value="<?php echo (int)$part['char_id']; ?>">
																	<button type="submit" class="btn btn-sm btn-danger" onclick="return confirm('Remove this participant?');" style="padding:0.1rem 0.4rem; font-size:0.75rem;">Remove</button>
																</form>
															<?php endif; ?>
														</td>
													</tr>
												<?php endforeach; ?>
											</tbody>
										</table>
									</div>
								<?php endif; ?>

								<!-- Invite form -->
								<?php if ($sess['state'] !== 'ss_closed'): ?>
									<form method="post" action="index.php?page=sessions" class="form-inline" style="border-top:1px solid #2c3e50; padding-top:0.75rem;">
										<?php echo csrfField(); ?>
										<input type="hidden" name="session_action" value="invite">
										<input type="hidden" name="ring_db_name" value="<?php echo h($entry['domain']['ring_db_name']); ?>">
										<input type="hidden" name="rsm_address" value="<?php echo h($entry['domain']['session_manager_address']); ?>">
										<input type="hidden" name="session_id" value="<?php echo $sid; ?>">
										<div class="form-group">
											<label>Invite by character name</label>
											<input type="text" name="invite_char_name" placeholder="Character name" style="width:12rem;">
										</div>
										<span style="color:#8899a6; font-size:0.85rem; margin:0 0.25rem;">or</span>
										<div class="form-group">
											<label>by ID</label>
											<input type="text" name="invite_char_id" placeholder="ID" style="width:5rem;">
										</div>
										<button type="submit" class="btn btn-sm btn-secondary">Invite</button>
									</form>
								<?php endif; ?>
							</div>
						<?php endforeach; ?>
					<?php endif; ?>
				</div>

				<!-- Participating Sessions -->
				<?php if (!empty($entry['participating_sessions'])): ?>
				<div class="card">
					<h2><?php echo h($entry['domain']['domain_name']); ?> &mdash; Sessions You're In</h2>
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

				<!-- Open Sessions in this domain (browseable) -->
				<?php if (!empty($entry['open_sessions'])): ?>
				<div class="card">
					<h2><?php echo h($entry['domain']['domain_name']); ?> &mdash; Open Sessions</h2>
					<p style="color:#8899a6; font-size:0.85rem; margin-bottom:0.75rem;">Sessions hosted by other players that are currently open or planned.</p>
					<table>
						<thead>
							<tr>
								<th>ID</th>
								<th>Title</th>
								<th>Type</th>
								<th>State</th>
								<th>Owner</th>
								<th>Players</th>
							</tr>
						</thead>
						<tbody>
							<?php foreach ($entry['open_sessions'] as $osess): ?>
								<?php $osStateInfo = formatSessionState($osess['state']); ?>
								<tr>
									<td>#<?php echo (int)$osess['session_id']; ?></td>
									<td style="color:#ecf0f1;"><?php echo h($osess['title'] ?: '(untitled)'); ?></td>
									<td><span class="badge badge-blue"><?php echo formatSessionType($osess['session_type']); ?></span></td>
									<td><span class="badge <?php echo $osStateInfo[1]; ?>"><?php echo $osStateInfo[0]; ?></span></td>
									<td style="color:#8899a6;"><?php echo h($osess['owner_name'] ?: '#' . $osess['owner']); ?></td>
									<td><?php echo (int)$osess['participant_count']; ?><?php if ($osess['subscription_slots'] > 0): ?>/<?php echo (int)$osess['subscription_slots']; ?><?php endif; ?></td>
								</tr>
								<?php if ($osess['description']): ?>
								<tr>
									<td></td>
									<td colspan="5" style="color:#8899a6; font-size:0.85rem; padding-top:0;"><?php echo h($osess['description']); ?></td>
								</tr>
								<?php endif; ?>
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
