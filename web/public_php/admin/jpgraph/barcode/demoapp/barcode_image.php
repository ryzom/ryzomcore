<?php
require_once "jpgraph/jpgraph.php";
require_once "jpgraph/jpgraph_canvas.php";
require_once "jpgraph/jpgraph_barcode.php";

$params = array(
    array('code',1),array('data',''),array('modwidth',1),array('info',false),
    array('notext',false),array('checksum',false),array('showframe',false),
    array('vertical',false) , array('backend','IMAGE'), array('file',''),
    array('scale',1), array('height',70), array('pswidth','') );

$n=count($params);
for($i=0; $i < $n; ++$i ) {
    $v  = $params[$i][0];
    if( empty($_GET[$params[$i][0]]) ) {
	$$v = $params[$i][1];
    }
    else
	$$v = $_GET[$params[$i][0]];
}

if( $modwidth < 1 || $modwidth > 5 ) {
    echo "<h4>Module width must be between 1 and 5 pixels</h4>";
}
elseif( $data==="" ) {
    echo "<h3>Please enter data to be encoded, select symbology and press 'Ok'.</h3>";
    echo "<i>Note: Data must be valid for the choosen encoding.</i>";
}
elseif( $code==-1 ) {
    echo "<h4>No code symbology selected.</h4>";
}
elseif( $height < 10 || $height > 500 ) {
    echo "<h4> Height must be in range [10, 500]</h4>";
}
elseif( $scale < 0.1 || $scale > 15 ) {
    echo "<h4> Scale must be in range [0.1, 15]</h4>";
}
else {
    if( $code==20 ) {
	$encoder = BarcodeFactory::Create(6);
	$encoder->UseExtended();
    }
    else {
	$encoder = BarcodeFactory::Create($code);
    }
    $b =  $backend=='EPS' ? 'PS' : $backend;
    $b = substr($backend,0,5) == 'IMAGE' ? 'IMAGE' : $b;
    $e = BackendFactory::Create($b,$encoder);
    if( substr($backend,0,5) == 'IMAGE' ) {
	if( substr($backend,5,1) == 'J' ) 
	    $e->SetImgFormat('JPEG');
    }
    if( $e ) {
	if( $backend == 'EPS' )
	    $e->SetEPS();
	if( $pswidth!='' )
	    $modwidth = $pswidth;
	$e->SetModuleWidth($modwidth);
	$e->AddChecksum($checksum);
	$e->NoText($notext);
	$e->SetScale($scale);
	$e->SetVertical($vertical);
	$e->ShowFrame($showframe);
	$e->SetHeight($height);
	$r = $e->Stroke($data,$file,$info,$info);
	if( $r )
	    echo nl2br(htmlspecialchars($r));
	if( $file != '' )
	    echo "<p>Wrote file $file.";
    }
    else
	echo "<h3>Can't create choosen backend: $backend.</h3>";
}

?>