<?php

	$ulos = "";
	$jatko = 1; // oletetaan normaali k�sittely
	$tyyppi = 1; // oletetaan rivin n�kyvyys

	if ($i == 0) { //yhti�t� ei n�ytet�
		$tyyppi = 0; 
	}
	
	if (mysql_field_name($result, $i) == "ytunnus" and $lukitse_avaimeen != "") {
		$ulos = "<input type='hidden' name='$nimi' value='$lukitse_avaimeen'>";
		$tyyppi = 4;
		$jatko = 0;
	}
	
?>
