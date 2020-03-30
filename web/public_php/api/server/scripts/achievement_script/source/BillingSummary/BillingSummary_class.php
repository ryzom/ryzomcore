<?php
	class BillingSummary extends SourceDriver {


		function drive($cdata) {
			/*
			global $DBc_char,$_DISPATCHER;

			$res = $DBc_char->sendSQL("SELECT SUM(amount) as anz, currency FROM coupons_billing WHERE iduser='".$cdata['aid']."' AND status='captured' GROUP by 'currency'","ARRAY");

			$billed = 0;

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				if($res[$i]['currency'] == "USD") {
					$res[$i]['anz'] = $res[$i]['anz']*0.7950;
				}

				if($res[$i]['currency'] == "GBP") {
					$res[$i]['anz'] = $res[$i]['anz']*1.2623;
				}

				$billed += $res[$i]['anz'];
			}

			$_DISPATCHER->dispatchValue("user_billed_sum",$billed);*/
		}
	}
?>