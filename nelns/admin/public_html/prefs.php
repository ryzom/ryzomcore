<?php
// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

	$publicAccess = true;
	include('authenticate.php');


	if (isset($chCookieState))
	{
		sqlquery("UPDATE user SET useCookie='$chCookieState' WHERE uid='$uid'");
		$useCookie = $chCookieState;
	}

	htmlProlog($_SERVER['PHP_SELF'], "Preferences");
	
	echo "<br><b>Preferences edit</b><br>\n";

	echo "<table><tr><td>\n";

	echo "<br><table><tr><td>Use login cookie</td><form method=post action='".$_SERVER['PHP_SELF']."'><td>\n";
	echo "<select name='chCookieState' onChange='submit()'>\n";
	echo "<option value='yes'".($useCookie == 'yes' ? " selected" : "").">Yes\n";
	echo "<option value='no'".($useCookie != 'yes' ? " selected" : "").">No\n";
	echo "</td></form></tr></table><br>\n";

	echo "</td></tr><tr><td><hr></td></tr><tr><td>\n";

	echo "<table><tr><td colspan=2>Change password</td></tr>\n";
	echo "<form method=post action='".$_SERVER['PHP_SELF']."'><input type=hidden name='command' value='chPassword'><input type=hidden name='admlogin' value='$admlogin'><input type=hidden name='admpassword' value='$admpassword'>\n";
	echo "<tr><td>Enter previous password</td><td><input type=password name='chOldPass' size=16 maxlength=16></td></tr>\n";
	echo "<tr><td>Enter new password</td><td><input type=password name='chNewPass' size=16 maxlength=16></td></tr>\n";
	echo "<tr><td>Reenter new password</td><td><input type=password name='chConfirmNewPass' size=16 maxlength=16></td></tr>\n";
	echo "<tr><td colspan=2 align=center><input type=submit type=password name='chPassword' value='Change password'></td></tr>\n";
	echo "</form></table><br>\n";

	echo "</td></tr></table>\n";
	
	htmlEpilog();
?>
