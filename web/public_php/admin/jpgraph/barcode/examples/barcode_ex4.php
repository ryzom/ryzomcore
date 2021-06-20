<?php
// ==============================================
// Output Image using Code Interleaved 2 of 5
// ==============================================
require_once ('jpgraph/jpgraph_barcode.php');

$encoder = BarcodeFactory::Create(ENCODING_CODEI25);
$e = BackendFactory::Create(BACKEND_IMAGE,$encoder);
$e->SetModuleWidth(2);
$e->Stroke('1234');

?>
