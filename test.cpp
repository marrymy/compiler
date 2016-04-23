long $r1 = 10;
long $r2 = 0;
loop($r2 < 3){
	$r1 = 1 + $r1;
	$r2 = $r2 + 1;
	showDec $r1;
}
if($r1==13){
	showHex $r1 ;
}