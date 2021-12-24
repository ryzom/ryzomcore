<?php
// ==============================================
// Output Image using Code 39 using only default values
// ==============================================
require_once ('jpgraph/jpgraph_barcode.php');

$encoder = BarcodeFactory::Create(ENCODING_CODE39);
$e = BackendFactory::Create(BACKEND_IMAGE,$encoder);
$e->Stroke('ABC123');

?>
