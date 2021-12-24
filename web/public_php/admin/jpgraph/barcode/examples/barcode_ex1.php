<?php
// ==============================================
// Output Image using Code 128
// ==============================================
require_once ('jpgraph/jpgraph_barcode.php');

$encoder = BarcodeFactory::Create(ENCODING_CODE128);
$e = BackendFactory::Create(BACKEND_PS,$encoder);
$e->SetModuleWidth(2);
$e->SetHeight(20);
echo nl2br($e->Stroke('3125134772'));


?>
