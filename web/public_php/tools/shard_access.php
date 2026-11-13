<?php

/*
 * The one place on the web side that decides what the permission rows of an
 * account give it on the shards of a domain. The shard unifier holds the same
 * rule for the game side (shard_unifier_service/shard_access.h).
 *
 * nel.permission has one row per (UId, DomainId, ShardId). ShardId is -1 for a
 * row that counts on all shards of the domain (and 0, which an earlier version
 * of the account tool wrote for the same thing), or the id of one shard.
 * AccessPrivilege is a SET of OPEN, DEV and RESTRICTED: the status of a domain
 * and the access level of a shard each ask for the member of the same name
 * (ds_open for OPEN and so on; ds_close for none, nobody enters).
 *
 * The rule: the access of an account on a shard is what its row for all shards
 * holds together with what its row for that shard holds. Either row can give
 * the access; a row for one shard gives nothing on any other shard.
 *
 * The web side does not know which shards a domain has: nothing keeps the
 * shard table of the nel database in step with the shards that run. So the
 * login goes by the rows alone (nel_account_domain_access), and the shard
 * unifier, which knows the shards, checks each shard it sends the account to.
 *
 * The functions here work on rows already read (arrays with ShardId and
 * AccessPrivilege), so they can be tested without a database.
 */

if (!function_exists('nel_account_shard_access'))
{
	/** The members of the SET column permission.AccessPrivilege, in its order. */
	function nel_access_privilege_members()
	{
		return array('OPEN', 'DEV', 'RESTRICTED');
	}

	/**
	 * The access privilege a domain status or a shard access level asks for:
	 * OPEN for ds_open, DEV for ds_dev, RESTRICTED for ds_restricted. '' for
	 * ds_close and for anything else: no privilege gives access then.
	 */
	function nel_access_privilege_of_status($status)
	{
		$status = (string)$status;
		if (strncmp($status, 'ds_', 3) !== 0)
			return '';
		$accessPriv = strtoupper(substr($status, 3));
		return in_array($accessPriv, nel_access_privilege_members(), true) ? $accessPriv : '';
	}

	/**
	 * The members a stored AccessPrivilege value holds, each once, in the
	 * order of the SET. An empty value holds none.
	 */
	function nel_access_privilege_set($value)
	{
		$stored = explode(',', (string)$value);
		return array_values(array_intersect(nel_access_privilege_members(), $stored));
	}

	/** True for the ShardId of a row that counts on all shards of its domain. */
	function nel_permission_is_all_shards($shardId)
	{
		$shardId = (int)$shardId;
		return $shardId === -1 || $shardId === 0;
	}

	/** What the rows for all shards hold, as a list of members in the order of the SET. */
	function nel_account_all_shards_access($rows)
	{
		$held = array();
		foreach ($rows as $row)
		{
			if (nel_permission_is_all_shards($row['ShardId']))
				$held = array_merge($held, nel_access_privilege_set($row['AccessPrivilege']));
		}
		return array_values(array_intersect(nel_access_privilege_members(), $held));
	}

	/**
	 * The access of an account on one shard: what its rows for all shards
	 * hold together with what its row for that shard holds.
	 *
	 * @param array $rows     the permission rows of the account for the domain
	 * @param int   $shardId  the id of a shard of that domain
	 * @return array  members of the SET, in its order
	 */
	function nel_account_shard_access($rows, $shardId)
	{
		$shardId = (int)$shardId;
		$held = nel_account_all_shards_access($rows);
		// the id of a shard is positive: -1 and 0 are no shard, they name all of them
		if ($shardId > 0)
		{
			foreach ($rows as $row)
			{
				if ((int)$row['ShardId'] === $shardId)
					$held = array_merge($held, nel_access_privilege_set($row['AccessPrivilege']));
			}
		}
		return array_values(array_intersect(nel_access_privilege_members(), $held));
	}

	/**
	 * Where the rows give the account the access privilege $accessPriv: on
	 * all shards of the domain, or on the shards its rows for single shards
	 * name.
	 *
	 * @param array  $rows        the permission rows of the account for the domain
	 * @param string $accessPriv  OPEN, DEV or RESTRICTED
	 * @return array  'all' is true when a row for all shards holds it;
	 *                'shards' are the ids of the shards whose own row holds
	 *                it, in the order of the rows
	 */
	function nel_account_access_shards($rows, $accessPriv)
	{
		$where = array('all' => false, 'shards' => array());
		if (!in_array($accessPriv, nel_access_privilege_members(), true))
			return $where;
		foreach ($rows as $row)
		{
			if (!in_array($accessPriv, nel_access_privilege_set($row['AccessPrivilege']), true))
				continue;
			if (nel_permission_is_all_shards($row['ShardId']))
				$where['all'] = true;
			// the id of a shard is positive; a row with another id names no shard and gives nothing
			else if ((int)$row['ShardId'] > 0)
				$where['shards'][] = (int)$row['ShardId'];
		}
		return $where;
	}

	/**
	 * True when the account may log in to a domain whose status asks for
	 * $accessPriv: its row for all shards, or one of its rows for single
	 * shards, holds it. Whether such a shard is up, and what its own level
	 * asks for, is for the shard unifier to say. With '' (a closed domain)
	 * nobody may log in.
	 *
	 * @param array  $rows        the permission rows of the account for the domain
	 * @param string $accessPriv  what the domain status asks for, see nel_access_privilege_of_status
	 */
	function nel_account_domain_access($rows, $accessPriv)
	{
		$where = nel_account_access_shards($rows, $accessPriv);
		return $where['all'] || !empty($where['shards']);
	}

	/**
	 * The row an access privilege is added to when the login gives one by
	 * itself ($AcceptUnknownUser): always a row for all shards, never a row
	 * for one shard.
	 *
	 * @param array  $rows        the permission rows of the account for the domain
	 * @param string $accessPriv  the member to add
	 * @return array  'row' is the row for all shards to update (the -1 row
	 *                before a 0 row), or null when one has to be inserted
	 *                with ShardId -1; 'value' is the AccessPrivilege to store
	 */
	function nel_account_all_shards_grant($rows, $accessPriv)
	{
		$target = null;
		foreach ($rows as $row)
		{
			if (!nel_permission_is_all_shards($row['ShardId']))
				continue;
			if ($target === null || (int)$row['ShardId'] < (int)$target['ShardId'])
				$target = $row;
		}
		$held = ($target === null) ? array() : nel_access_privilege_set($target['AccessPrivilege']);
		$held[] = $accessPriv;
		$value = implode(',', array_values(array_intersect(nel_access_privilege_members(), $held)));
		return array('row' => $target, 'value' => $value);
	}

	/** The rows as one line for a log or debug message: "all shards: OPEN; shard 302: OPEN,RESTRICTED". */
	function nel_account_access_text($rows)
	{
		$parts = array();
		foreach ($rows as $row)
		{
			$shardId = (int)$row['ShardId'];
			if (nel_permission_is_all_shards($shardId))
				$name = 'all shards';
			else if ($shardId > 0)
				$name = 'shard ' . $shardId;
			else
				$name = 'unknown shard id ' . $shardId; // names no shard and gives nothing
			$held = implode(',', nel_access_privilege_set($row['AccessPrivilege']));
			$parts[] = $name . ': ' . ($held === '' ? 'nothing' : $held);
		}
		return empty($parts) ? 'no row' : implode('; ', $parts);
	}
}

/* end of file */
