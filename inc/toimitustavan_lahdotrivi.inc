<?php

	$ulos = "";
	$jatko = 1; // oletetaan normaali k�sittely
	$tyyppi = 1; // oletetaan rivin n�kyvyys

	if (!isset($apu)) {
		$apu = array();
	}

	// yhti�t� ei n�ytet�
	if (mysql_field_name($result, $i) == "yhtio") {
		$tyyppi = 0;
	}

	if (mysql_field_name($result, $i) == "liitostunnus") {
		$ulos = "<td><input type='hidden' name='$nimi' value='$lukitse_avaimeen'>$lukitse_avaimeen</td>";
		$jatko = 0;
	}

	if (mysql_field_name($result, $i) == "kerailyn_aloitusaika" or mysql_field_name($result, $i) == "viimeinen_tilausaika" or mysql_field_name($result, $i) == "lahdon_kellonaika") {
		$trow[$i] = substr($trow[$i],0,5);
	}

	if (mysql_field_name($result, $i) == "lahdon_viikonpvm") {

		if($trow[$i] == 0) $sel0 = "SELECTED";
		if($trow[$i] == 1) $sel1 = "SELECTED";
		if($trow[$i] == 2) $sel2 = "SELECTED";
		if($trow[$i] == 3) $sel3 = "SELECTED";
		if($trow[$i] == 4) $sel4 = "SELECTED";
		if($trow[$i] == 5) $sel5 = "SELECTED";
		if($trow[$i] == 6) $sel7 = "SELECTED";

		$ulos = "<td><select name='$nimi'>";
		$ulos .= "<option value='' >".t("Ei valittua p�iv��")."</option>";
		$ulos .= "<option value='1' $sel1>".t("Maanantai")."</option>";
		$ulos .= "<option value='2' $sel2>".t("Tiistai")."</option>";
		$ulos .= "<option value='3' $sel3>".t("Keskiviikko")."</option>";
		$ulos .= "<option value='4' $sel4>".t("Torstai")."</option>";
		$ulos .= "<option value='5' $sel5>".t("Perjantai")."</option>";
		$ulos .= "<option value='6' $sel6>".t("Lauantai")."</option>";
		$ulos .= "<option value='0' $sel7>".t("Sunnuntai")."</option>";

		$ulos .= "</select></td>";

		$jatko = 0;
	}

	if (mysql_field_name($result, $i) == "aktiivi") {

		if ($trow[$i] == "E") {
			$sel2 = "SELECTED";
		}
		else {
			$sel1 = "SELECTED";
		}

		$ulos = "<td><select name='$nimi'>";
		$ulos .= "<option value='' $sel1>".t("K�yt�ss�")."</option>";
		$ulos .= "<option value='E' $sel2>".t("Ei k�yt�ss�")."</option>";
		$ulos .= "</select></td>";

		$jatko = 0;
	}

	if (mysql_field_name($result, $i) == "terminaalialue") {

		$sresult = t_avainsana('TERMINAALIALUE', '', " and selite != '' ");

		$ulos = "<td><select name='$nimi'>";
		$ulos .= "<option value=''>".t("Ei terminaalialuetta")."</option>";

		while ($row = mysql_fetch_array($sresult)) {
			$sel = "";
			if ($row["selite"] == $trow[$i]) $sel = 'selected';
			$ulos .= "<option value='$row[selite]' $sel>$row[selite]</option>";
		}

		$ulos .= "</select></td>";

		$jatko = 0;
	}

?>