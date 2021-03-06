<?php

if(!function_exists("luo_myyntitilausotsikko")) {
	function luo_myyntitilausotsikko($toim, $asiakasid, $tilausnumero = '', $myyjanro = '', $viesti = '', $kantaasiakastunnus = '', $ohjelmamoduli = '') {
		global $kukarow, $yhtiorow, $session;

		$query	= "	SELECT *
					FROM asiakas
					WHERE yhtio = '$kukarow[yhtio]'
					and tunnus  = '$asiakasid'";
		$result = pupe_query($query);

		if (mysql_num_rows($result) == 1) {
			$asiakasrow  = mysql_fetch_assoc($result);

			$ytunnus 		   = $asiakasrow["ytunnus"];
			$nimi 			   = $asiakasrow["nimi"];
			$nimitark 		   = $asiakasrow["nimitark"];
			$osoite 		   = $asiakasrow["osoite"];
			$postino 		   = $asiakasrow["postino"];
			$postitp 		   = $asiakasrow["postitp"];
			$maa 			   = $asiakasrow["maa"];
			$tnimi 			   = $asiakasrow["toim_nimi"];
			$tnimitark 		   = $asiakasrow["toim_nimitark"];
			$tosoite 		   = $asiakasrow["toim_osoite"];
			$tpostino 		   = $asiakasrow["toim_postino"];
			$tpostitp 		   = $asiakasrow["toim_postitp"];
			$toim_maa 		   = $asiakasrow["toim_maa"];
			$laskutus_nimi	   = $asiakasrow["laskutus_nimi"];
			$laskutus_nimitark = $asiakasrow["laskutus_nimitark"];
			$laskutus_osoite   = $asiakasrow["laskutus_osoite"];
			$laskutus_postino  = $asiakasrow["laskutus_postino"];
			$laskutus_postitp  = $asiakasrow["laskutus_postitp"];
			$laskutus_maa	   = $asiakasrow["laskutus_maa"];
			$kolm_nimi		   = $asiakasrow["kolm_nimi"];
			$kolm_nimitark	   = $asiakasrow["kolm_nimitark"];
			$kolm_osoite	   = $asiakasrow["kolm_osoite"];
			$kolm_postino	   = $asiakasrow["kolm_postino"];
			$kolm_postitp	   = $asiakasrow["kolm_postitp"];
			$kolm_maa		   = $asiakasrow["kolm_maa"];
			$kolm_ovttunnus	   = $asiakasrow["kolm_maa"];
			$verkkotunnus 	   = $asiakasrow["verkkotunnus"];
			$piiri 			   = $asiakasrow["piiri"];

			$kohde  = "";
			$eilahe = "";

			if ($kukarow['extranet'] != '' and $kukarow['hyvaksyja'] != "") {
				$eilahe = "o";
			}

			if (isset($asiakasrow["spec_ytunnus"]) and $asiakasrow["spec_ytunnus"] != '') {
				$ytunnus 				= $asiakasrow["spec_ytunnus"];
				$asiakasrow["ytunnus"] 	= $asiakasrow["spec_ytunnus"];
			}

			if (isset($asiakasrow["spec_tunnus"]) and $asiakasrow["spec_tunnus"] != '') {
				$asiakasid 				= $asiakasrow["spec_tunnus"];
				$asiakasrow["tunnus"] 	= $asiakasrow["spec_tunnus"];
			}

			$toimvv = date("Y");
			$toimkk = date("m");
			$toimpp = date("d");

			$kervv = date("Y");
			$kerkk = date("m");
			$kerpp = date("d");


			if ($toim == "TARJOUS" or $asiakasrow["vienti"] =='K') {
				$kolm_nimi 		= $asiakasrow["kolm_nimi"];
				$kolm_nimitark 	= $asiakasrow["kolm_nimitark"];
				$kolm_osoite 	= $asiakasrow["kolm_osoite"];
				$kolm_postino 	= $asiakasrow["kolm_postino"];
				$kolm_postitp 	= $asiakasrow["kolm_postitp"];
			}

			$maksuehto = $asiakasrow["maksuehto"];

			if ($myyjanro != "") {
				$apuqu = "	SELECT *
							from kuka use index (yhtio_myyja)
							where yhtio = '$kukarow[yhtio]'
							and myyja = '$myyjanro'
							AND myyja > 0";
				$meapu = pupe_query($apuqu);

				if (mysql_num_rows($meapu)==1) {
					$apuro = mysql_fetch_assoc($meapu);
					$myyja = $apuro['tunnus'];
				}
				elseif (mysql_num_rows($meapu)>1) {
					echo "<font class='error'>".t("Sy�tt�m�si myyj�numero")." $myyjanro ".t("l�ytyi usealla k�ytt�j�ll�")."!</font><br><br>";
					$myyja = 0;
				}
				else {
					echo "<font class='error'>".t("Sy�tt�m�si myyj�numero")." $myyjanro ".t("ei l�ytynyt")."!</font><br><br>";
					$myyja = 0;
				}
			}

			if (!isset($myyja) or $myyja == 0) {
				$myyja = $kukarow["tunnus"];
			}

			$alv 				= $asiakasrow["alv"];
			$ovttunnus 			= $asiakasrow["ovttunnus"];
			$toim_ovttunnus 	= $asiakasrow["toim_ovttunnus"];
			$chn 				= $asiakasrow["chn"];
			$maksuteksti 		= "";
			$tilausvahvistus	= $asiakasrow["tilausvahvistus"];
			$laskutusvkopv 		= $asiakasrow["laskutusvkopv"];
			$vienti 			= $asiakasrow["vienti"];
			$ketjutus 			= $asiakasrow["ketjutus"];
			$valkoodi 			= $asiakasrow["valkoodi"];
			$sisviesti1			= $asiakasrow["sisviesti1"];
			$rahtivapaa 		= $asiakasrow["rahtivapaa"];
			$comments			= "";

			if ($asiakasrow["myynti_kommentti1"] != "") {
				$comments = $asiakasrow["myynti_kommentti1"];
			}

			//annetaan extranet-tilaukselle aina paras prioriteetti, t�m� on hyv� porkkana.
			if ($kukarow["extranet"] != '') {
				$query  = "	SELECT distinct selite
							FROM avainsana
							WHERE yhtio = '$kukarow[yhtio]'
							and laji    = 'asiakasluokka'
							and selite != ''
							ORDER BY 1
							LIMIT 1";
				$prioresult = pupe_query($query);
				$priorow = mysql_fetch_assoc($prioresult);

				$luokka = $priorow["selite"];
			}
			else {
				$luokka	= $asiakasrow["luokka"];
			}

			$erikoisale	= $asiakasrow["erikoisale"];

			// Jos on varasto setattuna, p�ss�t��n eteenp�in
			$varasto = 0;

			if (strpos($kukarow["varasto"], ',') === FALSE and (int) $kukarow["varasto"] > 0) {
				// Jos kukarow varasto on vain YKSI varasto, niin silloin valitaan $kukarow['varasto']
				$varasto = $kukarow['varasto'];
			}
			// valitaan k�ytt�j�n oletus varasto
			elseif ($yhtiorow['oletusvarasto_tilaukselle'] == 'O' and (int) $kukarow["oletus_varasto"] > 0) {
				// jos kukarow['oletus_varasto'] valittu ja myyntitilauksella ei ole varastoa valittuna, valitaan k�ytt�j�n oletus varasto
				$varasto = $kukarow['oletus_varasto'];
			}

			if ($yhtiorow["splittauskielto"] != '') {
				$splittauskielto = "E";
			}
			else {
				$splittauskielto = "";
			}

			$jtkielto 		= $asiakasrow['jtkielto'];
			$toimitustapa 	= $asiakasrow["toimitustapa"];

			// Toimitusehto
			if (trim($asiakasrow["toimitusehto"]) != "") {
				$yhtiorow['oletus_toimitusehto'] = $asiakasrow["toimitusehto"];
			}

			$toimitusehto  = substr($yhtiorow['oletus_toimitusehto'], 0, 3);
			$kasitoimehto  = substr($yhtiorow['oletus_toimitusehto'], 4);

			$toimitusehto2 = substr($yhtiorow['oletus_toimitusehto2'], 0, 3);
			$kasitoimehto2 = substr($yhtiorow['oletus_toimitusehto2'], 4);

			// K�ytet��nk� l�hett�j�n vai vastaanottajan rahtisopparia
			$apuqu = "	SELECT *
						from toimitustapa
						where yhtio	= '$kukarow[yhtio]'
						and selite	= '$toimitustapa'";
			$meapu = pupe_query($apuqu);
			$apuro = mysql_fetch_assoc($meapu);

			$maksaja = $apuro['merahti'];

			// Onko toimitusehdon takana m��ritelty rahdinmaksaja? T�m� yliajaa toimitustavan takana olevan.
			$toimehto_tresult = t_avainsana("TOIMEHTO", "", " and trim(concat_ws(' ', selite, selitetark)) = trim('$yhtiorow[oletus_toimitusehto]') ");

			if (mysql_num_rows($toimehto_tresult) > 0) {
				$toimehto_row = mysql_fetch_assoc($toimehto_tresult);

				if ($toimehto_row["selitetark_3"] == "LAHETTAJAN_SOPIMUS") {
					$maksaja = "K";
				}
				elseif ($toimehto_row["selitetark_3"] == "VASTAANOTTAJAN_SOPIMUS") {
					$maksaja = "";
				}
			}

			$yhteysquery = "SELECT nimi
							FROM yhteyshenkilo
							where yhtio = '$kukarow[yhtio]'
							and liitostunnus = '$asiakasrow[tunnus]'
							and tyyppi = 'A'
							and tilausyhteyshenkilo != ''
							and oletusyhteyshenkilo != ''
							ORDER BY nimi
							LIMIT 1";
			$yhteysresult = pupe_query($yhteysquery);

			if ($yhteysrow = mysql_fetch_assoc($yhteysresult)) {
				$tilausyhteyshenkilo = $yhteysrow['nimi'];
			}

			$yhteyshenkilo_kaupallinen	= "";
			$yhteyshenkilo_tekninen		= "";
		}
		else {
			//yhti�n oletusalvi!
			$xwquery = "SELECT selite from avainsana where yhtio='$kukarow[yhtio]' and laji='alv' and selitetark!=''";
			$xwtres  = pupe_query($xwquery);
			$xwtrow  = mysql_fetch_assoc($xwtres);

			$alv = (float) $xwtrow["selite"];

			$ytunnus = "*";

			// Jos on varasto setattuna, p�ss�t��n eteenp�in
			$varasto = 0;

			if (strpos($kukarow["varasto"], ',') === FALSE and (int) $kukarow["varasto"] > 0) {
				// Jos kukarow varasto on vain YKSI varasto, niin silloin valitaan $kukarow['varasto']
				$varasto = $kukarow['varasto'];
			}
			// valitaan k�ytt�j�n oletus varasto
			elseif ($yhtiorow['oletusvarasto_tilaukselle'] == 'O' and (int) $kukarow["oletus_varasto"] > 0) {
				// jos kukarow['oletus_varasto'] valittu ja myyntitilauksella ei ole varastoa valittuna, valitaan k�ytt�j�n oletus varasto
				$varasto = $kukarow['oletus_varasto'];
			}
		}

		if ($valkoodi == '') {
			$valkoodi = $yhtiorow["valkoodi"]."##";
		}
		else {
			$query = "	SELECT nimi, kurssi
						FROM valuu
						WHERE yhtio = '$kukarow[yhtio]'
						and nimi= '$valkoodi'";
			$vresult = pupe_query($query);

			if (mysql_num_rows($vresult) == 1) {
				$vrow = mysql_fetch_assoc($vresult);

				$valkoodi = $vrow["nimi"]."##".$vrow["kurssi"];
			}
			else {
				$valkoodi = $yhtiorow["valkoodi"]."##";
			}
		}

		$alv_velvollisuus = "";

		// jos meill� on lasku menossa ulkomaille
		if ($maa != "" and $maa != $yhtiorow["maa"]) {
			// tutkitaan ollaanko siell� alv-rekister�ity
			$alhqur = "SELECT * from yhtion_toimipaikat where yhtio='$kukarow[yhtio]' and maa='$maa' and vat_numero != ''";
			$alhire = pupe_query($alhqur);

			// ollaan alv-rekister�ity, aina kotimaa myynti ja alvillista
			if (mysql_num_rows($alhire) == 1) {

				$alhiro  = mysql_fetch_assoc($alhire);

				// haetaan maan oletusalvi
				$query = "SELECT selite from avainsana where yhtio='$kukarow[yhtio]' and laji='ALVULK' and selitetark!='' and selitetark_2='$maa' limit 1";
				$alhire = pupe_query($query);

				// jos ei l�ydy niin menn��n erroriin
				if (mysql_num_rows($alhire) == 0) {
					echo "<font class='error'>Oletus ALV-kantaa ei l�ydy asiakkaan maahan $maa! Ei voida jatkaa!</font><br>";
					exit;
				}
				else {
					$apuro  = mysql_fetch_assoc($alhire);
					// n�m� t�ss� keisiss� aina n�in
					$alv = $apuro["selite"];
					$vienti = "";
					$alv_velvollisuus = $alhiro["vat_numero"];
				}
			}
		}

		if ($yhtiorow['myyntitilaus_osatoimitus'] == 'E' and in_array($toim, array("RIVISYOTTO", "PIKATILAUS"))) {
			$osatoimitus = "o";
		}

		$tilaustyyppi = "N";

		if ($toim == "TARJOUS") {
			$tilaustyyppi = 'T';
		}
		if ($toim == "EXTRANET_REKLAMAATIO") {
			$tilaustyyppi = 'R';
		}
		if ($toim == "MYYNTITILI") {
			$tilaustyyppi = "M";
		}

		$jatka	= "JATKA";
		$tee	= "OTSIK";
		$override_ytunnus_check = "YES";
		$ei_kayttoliittymaa = "kylla";

		require ("otsik.inc");

		if ($session == "") {
			return $id;
		}
		else {
			return $tilausnumero;
		}
	}
}

?>