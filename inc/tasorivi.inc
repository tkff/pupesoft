<?php

	$ulos   = "";
	$jatko  = 1; // oletetaan normaali k�sittely
	$tyyppi = 1; // oletetaan rivin n�kyvyys

	// yhti�t� ei n�ytet�
	if ($i == 0) {
		$tyyppi = 0;
	}

	if (mysql_field_name($result, $i) == "tyyppi") {

		$sel = array();
		$sel[$trow[$i]] = "SELECTED";

		$ulos  = "<td><select name='$nimi'>";
		$ulos .= "<option value = 'S' $sel[S]>".t("Sis�inen")."</option>";
		$ulos .= "<option value = 'U' $sel[U]>".t("Ulkoinen")."</option>";
		$ulos .= "<option value = 'A' $sel[A]>".t("ALV")."</option>";
		$ulos .= "<option value = 'B' $sel[B]>".t("Tulosseuranta")."</option>";
		$ulos .= "</select></td>";
		$jatko = 0;
	}

	if (mysql_field_name($result, $i) == "kumulatiivinen") {

		$sel = array();
		$sel[$trow[$i]] = "SELECTED";

		$ulos  = "<td><select name='$nimi'>";
		$ulos .= "<option value = ''>".t("Ei")."</option>";
		$ulos .= "<option value = 'X' $sel[X]>".t("Kyll�")." ".t("Tulosseurannassa t�m� taso lasketaan tilikauden alusta")."</option>";
		$ulos .= "</select></td>";
		$jatko = 0;
	}
