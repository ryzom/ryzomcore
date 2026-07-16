<?php
// ==============================================
// Output Encapsulated Postscript of interleaved 2 of 5
// ==============================================
require_once ('jpgraph/jpgraph_barcode.php');

echo "Start ...<br>";
$encoder = BarcodeFactory::Create(ENCODING_CODEI25);
$e = BackendFactory::Create(BACKEND_PS,$encoder);
$e->SetModuleWidth(2);
$e->SetHeight(70);
$e->SetEPS();
$ps = $e->Stroke('3125134772');
echo nl2br(htmlspecialchars($ps));


?>
