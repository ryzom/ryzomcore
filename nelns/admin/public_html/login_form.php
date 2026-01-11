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

	function LoginForm()
	{
		global $command, $admlogin, $admpassword;

		htmlProlog($_SERVER['PHP_SELF'], "Administration", false);
		
		?>

		<form method="post" action="<?php echo basename($_SERVER['PHP_SELF']); ?>" name=loginform>
		<table frame=void rules=none WIDTH="300">
		<tr>
		<td align="left"> login: </td>
		<td align="left"> <input type="text" name="admlogin" maxlength=16 size=16> </td>
		<tr>
		<td align="left"> password: </td>
		<td align="left"> <input type="password" name="admpassword" maxlength=16 size=16> </td>
		<tr>
		<td align="left">&nbsp; </td>
		<td align="left">&nbsp; </td>
		<tr>
		<td align="left"> &nbsp; </td>
		<td align="left"> <input type=submit value="login" name="cmdlogin"> </td>
		<td align="left"> <input type=hidden name="command" value="login"> </td>
		</table>
		</form>
		<script type="text/javascript">
		<!--
		if (document.loginform)
		{
			document.loginform.login.focus();
		}
		// -->
		</script>
		
		<?php
		
		$admlogin = '';
		$admpassword = '';

		htmlEpilog();
	}  

?>
