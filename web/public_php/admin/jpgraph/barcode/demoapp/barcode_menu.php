<H2>JpGraph Barcode 1.0</h2>
<hr>
</font>
<form name="barcodespec" action="barcode_image.php" target=barcode
method post>
<table cellspacing=4 cellpadding=0>
<tr>
<td colspan=2>
Data:<br>
<input type=text name=data size=25 maxlength=30>
</td>
<tr><td>
Encoding:<br>
<select name=code>
<option selected value=-1> Choose encoding </option>
<option   value=4> UPC A </option>
<option   value=5> UPC E </option>
<option   value=3> EAN 8 </option>
<option   value=2> EAN 13 </option>
<option   value=1> EAN 128 </option>
<option   value=11> Industrial 2 of 5 </option>
<option   value=12> Interleaved 2 of 5 </option>
<option   value=14> CODE 11 </option>
<option   value=6> CODE 39 </option>
<option   value=20> CODE 39 Extended </option>
<option   value=8> CODE 128 </option>
<option   value=13> CODABAR </option>
<option   value=10> BOOKLAND (ISBN) </option>
</select>
</td>
<td>
Module width:<br>
<select name=modwidth>
<option value=1> One </option>
<option value=2> Two </option>
<option value=3> Three </option>
<option value=4> Four </option>
<option value=5> Five </option>
</select>
</td>
</tr>

<tr>
<td>
Add checksum:<br>
<input type=checkbox value=1 name=checksum>
</td>

<td>
Hide text:<br>
<input type=checkbox value=1 name=notext>
</td>
</tr>
<tr>
<td>
Show frame:<br>
<input type=checkbox value=1 name=showframe>
</td>
<td>
Vertical layout:<br>
<input type=checkbox value=1 name=vertical>
</td>
</tr>

<tr>
<td>
Height:<br>
<input type=text name=height value="70" size=3 maxlength=3>
</td>
<td>
Scale:<br>
<input type=text name=scale value="1.0" size=4 maxlength=4>
</td>
</tr>



<tr>
<td>
Write to file:<br>
<input type=text name=file size=15 maxlength=80>
</td>
<td>
Format:<br>
<select name=backend>
<option selected value="IMAGEPNG">Image (PNG)</option>
<option value="IMAGEJPG">Image (JPEG)</option>
<option value="PS">Postscript</option>
<option value="EPS">EPS</option>
</select>
</td>

<tr>
<td>
PS module width:
</td>
<td>
<input type=text name=pswidth size=4 maxlength=4><br>
</td>
</tr>

<tr>
<td colspan=2>
<small><i>(If specified will override Module width above)</i></small><br>
</td>
</tr>


<tr>
<td>
Debug info:<br>
<input type=checkbox value=1 name=info>
</td>
<td align=right valign=bottom>
<br>
<input type=submit name=submit value="&nbsp; Create &nbsp;" style="font-weight:bold;">
</td>
</tr></table>
</form>

<p>
<hr>



