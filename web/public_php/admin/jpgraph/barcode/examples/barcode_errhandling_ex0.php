<?php
// ==============================================
// Output Image using Code 39 using only default values
// ==============================================
require_once ('jpgraph/jpgraph_barcode.php');

try {
$encoder = BarcodeFactory::Create(ENCODING_CODE39);
$e = BackendFactory::Create(BACKEND_IMAGE,$encoder);
$e->Stroke('abc123');
} catch( JpGraphException $e ) {
	//echo 'Error: ' . $e->getMessage()."\n";
	JpGraphError::Raise($e->getMessage());
}

?>
