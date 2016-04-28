long $r1 = 10;
showHex $r1 ;
long $r2 = 0;

if($r2 < 1)
{
	show hello ;
}
loop($r2 < 2)
{
	$r1 = $r1 + 1;
	showDec $r1;
	$r2 = $r2 + 1;
}