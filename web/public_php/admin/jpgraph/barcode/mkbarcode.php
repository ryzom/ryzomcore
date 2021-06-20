<?php
require_once('jpgraph/jpgraph_barcode.php');

/*=======================================================================
 // File:        MKBARCODE.PHP
 // Description: Comman line tool to generate linear barcodes
 // Created:     2009-06-20
 // Ver:         $Id: mkbarcode.php 1455 2009-07-03 18:52:25Z ljp $
 //
 // Copyright (c) Asial Corporation. All rights reserved.
 //=======================================================================
 */

//----------------------------------------------------------------------
// CLASS ParseArgs
// Parse command line arguments and make sanity checks
//----------------------------------------------------------------------
class ParseArgs {
    var $argc,$argv;

    function ParseArgs() {
        // Get command line argument
        $this->argv = ($_SERVER['argv']);
        $this->argc = ($_SERVER['argc']);
    }

    function PrintUsage() {
    	$n = $this->argv[0];
        echo "$n -b <symbology> [-r -h -c -o <output format> -m <width> -s <scale> -y <height> -f <filename> ] datastring \n".
            "Create the specified barcode\n".
            "-b           What symbology to use, one of the following strings (case insensitive)\n".
            "             UPCA \n".
            "             UPCE \n".
            "             EAN128 \n".
            "             EAN13 \n".
            "             EAN8 \n".
            "             CODE11 \n".
            "             CODE39 \n".
            "             CODE128 \n".
            "             CODE25 \n".
            "             CODEI25 \n".
            "             CODABAR \n".
            "             BOOKLAND \n".
            "-c           Add checkdigit for symbologies where this is optional\n".
            "-o           Output format. 0=Image, 1=PS, 2=EPS\n".
            "-m           Module width\n".
            "-s           Scale factor\n".
            "-h           Show this help\n".
			"-f           Filename to write to\n".
        	"-r           Rotate barcode 90 degrees\n".
        	"-y height    Set height in pixels\n".
            "-x           Hide the human readable text\n".
        	"--silent     Silent. Don't give any error mesages\n";
        exit(1);
    }

    function Get() {
        $barcode='code39';
        $hide=false;
        $checkdigit=false;
        $modulewidth=2;
        $scale=1;
        $output=0;
        $filename='';
        $data = '';
        $rotate = false;
        $silent=false;
        $height = 70;
        if( ($n=$this->GetNum()) > 0 ) {
            $i=1;
            while( $i <= $n ) {
                switch( $this->argv[$i] ) {
                    case '-h':
                        $this->PrintUsage();
                        exit(0);
                        break;
                    case '-b':
                        $barcode = $this->argv[++$i];
                        break;
                    case '-o':
                        $output = (int)$this->argv[++$i];
                        break;
                    case '-y':
                        $height = (int)$this->argv[++$i];
                        break;
					case '-x':
                        $hide=true;
                        break;
                    case '-r':
                        $rotate=true;
                        break;
                    case '-c':
                        $checkdigit=true;
                        break;
                    case '--silent':
                        $silent=true;
                        break;
                    case '-s':
                        $scale = (float)$this->argv[++$i];
                        break;
                    case '-m':
                        $modulewidth = (float)$this->argv[++$i];
                        break;
                    case '-f':
                        $filename = $this->argv[++$i];
                        break;
                    default:
                    	if( $data == '' ) {
                        	$data = $this->argv[$i];
                    	}
                    	else {
  							$this->PrintUsage();
  							die("Illegal specified parameters");
                    	}
                        break;
                }
                ++$i;
            }

        }

        if( $output < 0 || $output > 2 ) {
        	fwrite(STDERR,"Unkown output format ($output)\n");
        	exit(1);
        }

        if( $output === 0  ) {
        	$modulewidth = floor($modulewidth);
        }

        // Sanity check
        if( $modulewidth > 15 ) {
        	fwrite(STDERR,"Too large modulewidth\n");
        	exit(1);
        }

        // Sanity check
        if( $height > 1000 ) {
        	fwrite(STDERR,"Too large height\n");
        	exit(1);
        }

		// Sanity check
        if( $scale > 15 ) {
        	fwrite(STDERR,"Too large scale factor\n");
        	exit(1);
        }

        if( strlen($filename) > 256 ) {
        	fwrite(STDERR,"Too long filename\n");
        	exit(1);
        }

        if( trim($data) == '' ) {
			fwrite(STDERR,"No input data specified\n");
			exit(1);
        }

        $barcodes = array(
            'UPCA' => ENCODING_UPCA,
            'UPCE' => ENCODING_UPCE,
            'EAN128' => ENCODING_EAN128,
            'EAN13' => ENCODING_EAN13,
            'EAN8' => ENCODING_EAN8,
            'CODE11' => ENCODING_CODE11,
            'CODE39' => ENCODING_CODE39,
            'CODE128' => ENCODING_CODE128,
            'CODE25' => ENCODING_CODE25,
            'CODEI25' => ENCODING_CODEI25,
            'CODABAR' => ENCODING_CODABAR,
            'BOOKLAND' => ENCODING_BOOKLAND,
        );
        $barcode = strtoupper($barcode);
        if( key_exists($barcode,$barcodes) ) {
        	$barcode = $barcodes[$barcode];
        }
        else {
        	fwrite(STDERR,'Specified barcode symbology ('.$barcode.") is not supported\n");
        	exit(1);
        }

		$ret = array(
				'barcode'     => $barcode,
		        'hide' 	      => $hide,
		        'modulewidth' => $modulewidth,
		        'scale'       => $scale,
		        'output'      => $output,
		        'data'        => $data,
		        'silent'      => $silent,
		        'rotate'      => $rotate,
		        'height'      => $height,
				'checkdigit'  => $checkdigit,
		        'filename'    => $filename
			);

		return $ret;
    }

    function _Dump() {
        var_dump($this->argv);
    }

    function GetNum() {
        return $this->argc-1;
    }
}

//----------------------------------------------------------------------
// CLASS Driver
// Main driver class to create barcodes with the parmeters specified on
// the command line.
//----------------------------------------------------------------------
class Driver {

	private $iParams;
	static public $silent=false;

	static public function ErrHandlerPS(Exception $e) {
		if( !Driver::$silent )
			fwrite(STDERR,$e->getMessage()."\n");
        exit(1);
	}

	static public function ErrHandlerImg(Exception $e) {
		if( !Driver::$silent )
			fwrite(STDERR,$e->getMessage()."\n");
        $errobj = new JpGraphErrObjectImg();
        $errobj->Raise($e->getMessage());
        exit(1);
	}

	function Run($aParams) {

		$this->iParams = $aParams;

		Driver::$silent = $aParams['silent'];

		$encoder = BarcodeFactory::Create($aParams['barcode']);
		$encoder->AddChecksum($aParams['checkdigit']);
		switch( $aParams['output'] ) {
			case 0:
				$e = BackendFactory::Create(BACKEND_IMAGE,$encoder);
				set_exception_handler(array('Driver','ErrHandlerImg'));
				break;
			case 1:
				$e = BackendFactory::Create(BACKEND_PS,$encoder);
				set_exception_handler(array('Driver','ErrHandlerPS'));
				break;
			case 2:
				$e = BackendFactory::Create(BACKEND_PS,$encoder);
				$e->SetEPS();
				set_exception_handler(array('Driver','ErrHandlerPS'));
				break;
		}
		$e->SetHeight($aParams['height']);
		$e->SetVertical($aParams['rotate']);
		$e->SetModuleWidth($aParams['modulewidth']);
		$e->SetScale($aParams['scale']);
		$e->HideText($aParams['hide']);
		if( $aParams['output'] === 0 ) {
			$err = $e->Stroke($aParams['data'], $aParams['filename']);
		}
		else {
			$s = $e->Stroke($aParams['data'], $aParams['filename']);
			if( $aParams['filename'] == '' ) {
				// If no filename specified then return the generated postscript
				echo $s;
			}
		}
	}
}

$pa = new ParseArgs();
$params = $pa->Get();
$driver = new Driver();
$driver->Run($params);

// Successfull termination
exit(0);

?>
