<?php

/*
 * The one place that decides what a game account login may look like.
 *
 * The client pastes the login straight into the login request query string
 * without encoding it (login.cpp: "?cmd=login&login=" + login + ...), so a
 * name outside this set cannot be sent back reliably by an existing client:
 * an account carrying one is an account nobody can log into. The account
 * tool registration has always enforced this rule; every other path that
 * creates an account has to enforce the same one, or it hands out accounts
 * that are dead on arrival.
 *
 * Deliberately NOT applied when checking an existing login. Accounts that
 * predate the rule -- a dot or a dash in the name, say -- work fine today
 * and must keep working; refusing them at login would lock out the very
 * players this is meant to protect.
 */

if (!function_exists('nel_is_valid_account_name'))
{
	function nel_account_name_min_length()
	{
		return 3;
	}

	/** nel.user.Login is varchar(64). */
	function nel_account_name_max_length()
	{
		return 64;
	}

	/** True when the length is within bounds (charset checked separately). */
	function nel_account_name_length_ok($login)
	{
		if (!is_string($login))
			return false;
		$len = strlen($login);
		return $len >= nel_account_name_min_length() && $len <= nel_account_name_max_length();
	}

	/** True when $login may be handed out as a new account name. */
	function nel_is_valid_account_name($login)
	{
		if (!nel_account_name_length_ok($login))
			return false;
		return (bool)preg_match('/^[a-zA-Z0-9_]+$/', $login);
	}

	/** One line describing the rule, for messages shown to a person. */
	function nel_account_name_rule_text()
	{
		return 'Username must be between ' . nel_account_name_min_length() . ' and '
			. nel_account_name_max_length()
			. ' characters, and may only contain letters, numbers and underscores.';
	}
}

/* end of file */
