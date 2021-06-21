<?php
// =======================================================
// Example of how to format US Postal shipping information
// =======================================================
require_once ('jpgraph/jpgraph_barcode.php');

// The Full barcode standard is described in
// http://www.usps.com/cpim/ftp/pubs/pub91/91c4.html#508hdr1
//
// The data start with AI=420 which means
// "Ship to/Deliver To Postal Code (within single authority)
//
class USPS_Confirmation {
    function USPS_Confirmation() {
    }

    // Private utility function
    function _USPS_chkd($aData) {
	$n = strlen($aData);

	// Add all even numbers starting from position 1 from the end
	$et = 0 ;
	for( $i=1; $i <= $n; $i+=2 ) {
	    $d = intval(substr($aData,-$i,1));
	    $et += $d;
	}

	// Add all odd numbers starting from position 2 from the end
	$ot = 0 ;
	for( $i=2; $i <= $n; $i+=2 ) {
	    $d = intval(substr($aData,-$i,1));
	    $ot += $d;
	}
	$tot = 3*$et + $ot;
	$chkdigit = (10 - ($tot % 10))%10;;
	return $chkdigit;
    }

    // Get type 1 of confirmation code (with ZIP)
    function GetPICwithZIP($aZIP,$aServiceType,$aDUNS,$aSeqNbr) {
	// Convert to USPS format with AI=420 and extension starting with AI=91
	$data = '420'. $aZIP . '91' . $aServiceType . $aDUNS . $aSeqNbr;
	// Only calculate the checkdigit from the AI=91 and forward
	// and do not include the ~1 (FUNC1) in the calculation
	$cd = $this->_USPS_chkd(substr($data,8));
	$data = '420'. $aZIP . '~191' . $aServiceType . $aDUNS . $aSeqNbr;
	return $data . $cd;
    }

    // Get type 2 of confirmation code (without ZIP)
    function GetPIC($aServiceType,$aDUNS,$aSeqNbr) {
	// Convert to USPS format with AI=91
	$data = '91' . $aServiceType . $aDUNS . $aSeqNbr;
	$cd = $this->_USPS_chkd($data);
	return $data . $cd;
    }

}

$usps = new USPS_Confirmation();
$zip     = '92663';
$service = '21';
$DUNS    = '805213907';
$seqnr   = '04508735';
$data = $usps->GetPICwithZIP($zip,$service,$DUNS,$seqnr);
//$data = $usps->GetPIC('01','123456789','00000001');

$encoder = BarcodeFactory::Create(ENCODING_EAN128);
$e = BackendFactory::Create(BACKEND_IMAGE,$encoder);
$e->SetModuleWidth(2);
$e->SetFont(FF_ARIAL,FS_NORMAL,14);
$e->Stroke($data);

?>