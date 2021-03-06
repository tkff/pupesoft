<?php

function hinnastorivi($tuote, $laskurow) {
    
	$myyntihinta = tuotteen_myyntihinta($laskurow, $tuote, 1, "ei");

    // muutetaan yksik�t ISO-standardin mukaisiksi
	$yksikko="";
	if ($tuote['yksikko']=='KPL' or $tuote['yksikko']=='PCE')
		$yksikko = "PCE";
	if ($tuote['yksikko']=='SRJ')
		$yksikko = "SET";
	if ($tuote['yksikko']=='PAR')
		$yksikko = "PAR";
	
    // tehd��n tietuetta
	$out  .= sprintf("%-35.35s", $tuote['tuoteno']);					// tuotenumero
	$out  .= sprintf("%-10.10s", sprintf("%02s", $tuote['try']));		// tuotteen tuoteryhm� kahteen merkkiin asti zeropaddattuna
	$out  .= sprintf("%-10.10s", sprintf("%02s", $tuote['aleryhma']));	// tuotteen alennusryhm� kahteen merkkiin asti zeropaddattuna
	$out  .= sprintf("%-50.50s", $tuote['nimitys']);		   			// tuotteen nimitys
	$out  .= sprintf("%-1.1s",   "1"); 				   					// verollisuus 1=verollinen 0=veroton
	$out  .= sprintf("%11.11s",  $myyntihinta);	   						// tuotteen luettelohinta
	$out  .= sprintf("%-1.1s",   "");					   				// hinnan yksikk�
	$out  .= sprintf("%-35.35s", $tuote['eankoodi']);	   				// tuotteen eankoodi
	$out  .= sprintf("%-1.1s",   "");					   				// elinkaaren vaihe 1-9
	$out  .= sprintf("%-11.11s", "0");					   				// minimitoimituser�
	$out  .= sprintf("%-3.3s",   $yksikko);			   					// minimitoimituksen yksikk�
	$out  .= sprintf("%-3.3s",   "");					   				// hinnoittelujakaja
	$out  .= sprintf("%-16.16s", "");					   				// tarjouskampanjan voimassaolo CCYYMMDDCCYYMMDD
	$out  .= sprintf("%-11.11s", "");					   				// tarjouskampanjahinta
	$out  .= sprintf("%-16.16s", "");					   				// 1. alennuksen voimassaolo CCYYMMDDCCYYMMDD
	$out  .= sprintf("%-19.19s", "");					   				// m��r�n minimi
	$out  .= sprintf("%-19.19s", "");					   				// m��r�n maksimi
	$out  .= sprintf("%-11.11s", "");					   				// alennushinta
	$out  .= sprintf("%-16.16s", "");					   				// 2. alennuksen voimassaolo CCYYMMDDCCYYMMDD
	$out  .= sprintf("%-19.19s", "");					   				// m��r�n minimi
	$out  .= sprintf("%-19.19s", "");					   				// m��r�n maksimi
	$out  .= sprintf("%-11.11s", "");					   				// alennushinta
	$out  .= sprintf("%-16.16s", "");					   				// 3. alennuksen voimassaolo CCYYMMDDCCYYMMDD
	$out  .= sprintf("%-19.19s", "");					   				// m��r�n minimi
	$out  .= sprintf("%-19.19s", "");					   				// m��r�n maksimi
	$out  .= sprintf("%-11.11s", "");					   				// alennushinta
	$out .= "\n";
	
    return $out;
}
?>