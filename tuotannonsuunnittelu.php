<?php

require 'inc/parametrit.inc';

require 'valmistuslinjat.inc';
require 'valmistus.class.php';

echo "<link rel='stylesheet' type='text/css' href='fullcalendar.css' />
	<link rel='stylesheet' type='text/css' href='valmistuslinjat.css' />
	<script type='text/javascript' src='fullcalendar.js'></script>
	<script type='text/javascript' src='valmistuslinjat.js'></script>";


// Jos $teet� ei ole
if (!isset($tee)) $tee = '';

// Debug
if (isset($laske_kestot_uudelleen)) {
	rebuild_valmistuslinjat();
}

/** Valmistusten siirt�minen valmistuslinjalla */
if (isset($method) and $method == 'move') {

	// Haetaan valitun valmistuksen tiedot
	$query = "SELECT *, lasku.tunnus
				FROM kalenteri
				JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
				WHERE kalenteri.yhtio='{$kukarow['yhtio']}' AND kalenteri.otunnus='{$tunnus}'";
	$result = pupe_query($query);
	#echo $query;
	$valittu_valmistus = mysql_fetch_assoc($result);

	#$valittu_valmistus = Valmistus::find($tunnus);

	// Siiret��n aiemmaksi
	if ($direction == 'left') {
		$edellinen = etsi_edellinen_valmistus($valittu_valmistus);
		if ($edellinen) {
			// HUOM, vasemmanpuoleinen aina ensin (eli aiempi valmistus)
			vaihda_valmistusten_paikkaa($edellinen, $valittu_valmistus, $valittu_valmistus['henkilo']);
		}
	}
	// Siirret��n my�h�isemm�ksi
	elseif ($direction == 'right') {
		$seuraava = etsi_seuraava_valmistus($valittu_valmistus);
		if ($seuraava) {
			vaihda_valmistusten_paikkaa($valittu_valmistus, $seuraava, $valittu_valmistus['henkilo']);
		}
	}
}

/** Poistetaan kalenterista kalenterimerkint� */
if (isset($tee) and $tee == 'poista' and is_numeric($tunnus)) {
	$query = "DELETE FROM kalenteri WHERE yhtio='{$kukarow['yhtio']}' AND tunnus={$tunnus}";
	if (pupe_query($query)) {
		echo "Poistettiin kalenterimerkint�!";
		$tee = '';
	}
}

/**
 * Valmistuksen tilan p�ivitt�minen
 */
if ($tee == 'paivita' and isset($method) and $method == 'update') {
	$valmistus = Valmistus::find($tunnus);

	// Keskeyt� ty� (TK) ja Valmis tarkastukseen (VT) kysyy lis�formilla tiedot valmistuksesta
	if (!isset($varmistus) and ($tila == 'TK' or $tila == 'VT')) {
		$otsikko = ($tila=='TK') ? 'Keskeyt� ty�' : 'Valmista tarkastukseen';

		echo "<font class='head'>" . t($otsikko) . "</font>";

		echo "<form method='POST'>";
		echo "<input type='hidden' name='tunnus' value='$tunnus'>";
		echo "<input type='hidden' name='tila' value='$tila'>";
		echo "<input type='hidden' name='tee' value='paivita'>";
		echo "<input type='hidden' name='varmistus' value='ok'>";

		echo "<table>";
		echo "<tr><th>Valmistus</th><td>{$valmistus->tunnus()}</td></tr>";

		// Haetaan valmisteet
		foreach($valmistus->tuotteet() as $valmiste) {
			echo "<tr>
				<th>Tuoteno</th>
				<td>{$valmiste['tuoteno']}
				</tr>";
			echo "<tr>
				<th>Valmistettava m��r�</th>
				<td><input type='text' name='valmisteet[{$valmiste['tunnus']}][maara]' value='{$valmiste['varattu']}'></td>
				</tr>";
			echo "<tr>
				<th>Ylity�tunnit</th>
				<td><input type='text' name='valmisteet[{$valmiste['tunnus']}][tunnit]'></td>
				</tr>";
		}

		echo "<tr><th>Kommentit</th><td><input type='text' name='kommentti'></td></tr>";
		echo "</table>";

		echo "<br>";
		echo "<a href='tuotannonsuunnittelu.php'>Takaisin</a> ";
		echo "<input type='submit' value='Valmis'>";
		echo "</form>";
	}
	else {
		// Muut tilat p�ivitet��n suoraan
		$varmistus = 'ok';
	}

	// Jos kaikki ok, p�ivitet��n valmistus
	if ($varmistus == 'ok') {

		// Splitatanko valmistus flag
		$splitataan = false;

		// Tarkistetaan sy�tetyt m��r�t ja verrataan valmisteen tilauksen m��riin
		foreach ($valmistus->tuotteet() as $valmiste) {

			// Tarkastetaan tarvitseeko valmistusta splitata
			if ($valmiste['varattu'] > $valmisteet[$valmiste['tunnus']]['maara'] and ($tila == 'TK' or $tila == 'VT')) {
				#echo $valmiste['varattu']. " > " . $valmisteet[$valmiste['tunnus']['maara']] . "<br>";
				$jaettavat_valmisteet[$valmiste['tunnus']] = $valmisteet[$valmiste['tunnus']]['maara'];
				$splitataan = true;
			}
		}

		// Valmistetta on valmistettu v�hemmin kuin sit� on tilattu.
		if ($splitataan) {
			// Yritet��n jakaa valmistus
			try {
				$kopion_id = jaa_valmistus($valmistus->tunnus(), $jaettavat_valmisteet);
			} catch (Exception $e) {
				$errors = "<font class='error'>Virhe valmistuksen jakamisessa, " . $e->getMessage() . "</font>";
			}
		}

		// Yritet��n vaihtaa valmistuksen tilaa
		try {
			$valmistus->setTila($tila);
			$tee = '';
		} catch (Exception $e) {
			$errors .= "<font class='error'>Valmistuksen tilan muuttaminen ep�onnistui. <br>{$e->getMessage()}</font>";
			$tee = '';
		}
	}

	rebuild_valmistuslinjat();
}

/** Lis�t��n valmistus valmistusjonoon
*/
if ($tee == 'lisaa_tyojonoon') {
	// Lis�t��n valmistus valmistuslinjalle
	lisaa_valmistus($valmistus, $valmistuslinja);
	$tee = '';
}

if ($tee == 'lisaa_kalenteriin') {

	// Loppuaika timestampiksi
	if (!empty($pvmloppu)) $pvmloppu = strtotime($pvmloppu);

	// Alkuaika on pakko sy�tt��
	if (empty($pvmalku)) {
		$errors .= "<font class='error'>".t("Alkuaika ei voi olla tyhj�")."</font>";
	}
	else {
		$pvmalku = strtotime($pvmalku);

		// Jos pvmloppu on tyhj�, tehd��n p�iv�n eventti aloitusajan mukaan
		if (empty($pvmloppu)) {
			$pvmloppu = mktime(23, 59, 59, date('m', $pvmalku), date('d', $pvmalku), date('Y', $pvmalku));
		}

		// Jos alkuaika on pienempi kuin loppuaika, lis�t��n tapahtuma kalenteriin
		if ($pvmalku < $pvmloppu) {
			// p�iv�m��r�t ok
			$pvmalku = date('Y-m-d H:i:s', $pvmalku);
			$pvmloppu = date('Y-m-d H:i:s', $pvmloppu);

			// Lis�t��n tietokantaan
			$query = "INSERT INTO kalenteri SET yhtio='{$kukarow['yhtio']}', tyyppi='$tyyppi', pvmalku='$pvmalku', pvmloppu='$pvmloppu', henkilo='$valmistuslinja'";
			pupe_query($query);
		}
		else {
			$errors .= "<font class='error'>".t("Loppuajan on oltava suurempi kuin alkuajan")."</font>";
		}

		#echo "alkuaika: ".date('Y-m-d H:i:s', $pvmalku).", loppuaika: ".date('Y-m-d H:i:s', $pvmloppu);

		$tee = '';
	}
}

if ($tee == '') {

	// Valmistuslinjojen info popup
	echo "<div id='bubble'>";
	echo "<div id='header'></div>";
	echo "<div id='content'></div>";
	echo "<form action='tuotannonsuunnittelu.php?method=update' method='post' id='toiminto' name='bubble'>
			<input type='hidden' name='tunnus' id='valmistuksen_tunnus'>
			<input type='hidden' name='tee' value='paivita'>
			<select name='tila' onchange='submit()'>
			<option value=''>Valitse</option>
			<option value='OV'>Siir� parkkiin</option>
			<option value='VA'>Aloita valmistus</option>
			<option value='TK'>Keskeyt� valmistus</option>
			<option value='VT'>Valmis tarkistukseen</option>
			</select>
		</form>";
	echo "<br><a href='#' id='close_bubble'>sulje</a>";
	echo "</div>";

	// html
	echo "<font class='head'>".t("Ty�jono ty�suunnittelu")."</font><hr>";

	echo "<input type='hidden' id='yhtiorow' value='{$kukarow['yhtio']}'>";
	echo "<div id='calendar'></div>";

	echo "<br>";
	echo "<a href='tuotannonsuunnittelu.php?laske_kestot_uudelleen=true'>Laske valmistuslinjojen kestot uudelleen</a>";
	echo "<br>";

	// Virheilmoitukset
	if (!empty($errors)) {
		echo $errors;
	}

	echo "<br>";

	/* PARKKI */
	echo "<br>";
	echo "<font class='head'>".t("Parkki")."</font>";
	echo "<hr>";

	echo "<table border=1>";
	echo "<tr>";
	echo "<th>Valmistus</th>";
	echo "<th>Tila</th>";
	echo "<th>Nimitys</th>";
	echo "<th>M��r�</th>";
	echo "<th>Kesto (h)</th>";
	echo "<th></th>";
	echo "<th>Puutteet</th>";
	echo "</tr>";

	// Hetaan valmistuslinjat avainsanoista
	$linjat = hae_valmistuslinjat();
	// Haetaan valmistukset
	$valmistukset = Valmistus::all();

	//Listataan parkissa olevat valmistukset
	foreach($valmistukset as $valmistus) {
		echo "<tr>";

		echo "<td>" . $valmistus->tunnus() . "</td>";
		echo "<td>" . $valmistus->getTila() . "</td>";
		echo "<td>";
		// Valmistuksella olevat tuotteet
		$kpl = '';
		foreach($valmistus->tuotteet() as $tuote) {
			echo $tuote['tuoteno'] . " "
				. $tuote['nimitys'] . "<br>";
			$kpl .= $tuote['varattu'] . " " . $tuote['yksikko'] . "<br>";
		}

		echo "</td>";

		echo "<td>$kpl</td>";

		echo "<td>" . $valmistus->kesto() . "</td>";

		echo "<td>";
		// Valmistuslinjan valintalaatikko
		if ($valmistus->valmistuslinja() == NULL) {
			echo "<form method='post' name='lisaa_tyojonoon'>";
			echo "<input type='hidden' name='tee' value='lisaa_tyojonoon'>";
			echo "<input type='hidden' name='valmistus' value='{$valmistus->tunnus()}'>";
			echo "<select name='valmistuslinja'>";
			echo "<option value=''>Valitse linja</option>";

			foreach($linjat as $linja) {
				echo "<option value='$linja[selite]'>$linja[selitetark]</option>";
			}

			echo "</select>";
			echo "<input type='submit' value='".t("Aloita valmistus")."'>";
			echo "</form>";
		}
		else {
			echo $valmistus->alkupvm() . " - " . $valmistus->loppupvm();
		}
		echo "</td>";

		echo "<td>";
		// foreach($valmistus->puutteet() as $tuoteno => $maara) {
		// 	echo "tuoteno: $tuoteno saldo: $maara<br>";
		// }
		echo "</td>";

		echo "</tr>";
	}

	echo "</table>";

	/* Muut kalenterimerkinn�t*/
	echo "<br>";
	echo "<font class='head'>" . t("Muut") . "</font>";
	echo "<hr>";

	echo "<form method='POST'>";
	echo "<input type='hidden' name='tee' value='lisaa_kalenteriin'>";
	echo "<table>";
	echo "<tr>";
	echo "<th>Valmistuslinja:</th>";
	echo "<td>";
	echo "<select name='valmistuslinja'>";
	echo "<option value=''>Yhti�kohtainen</option>";

	foreach($linjat as $linja) {
		echo "<option value='$linja[selite]'>$linja[selitetark]</option>";
	}

	echo "</select>";
	echo "</td>";
	echo "</tr>";
	echo "<tr>";
	echo "<th>Tyyppi:</th>";
	echo "<td>";
	echo "<select name='tyyppi'>";
	echo "<option value='PE'>Pekkasp�iv�</option>";
	echo "<option value='SA'>Sairasloma</option>";
	echo "<option value='MT'>Muu ty�</option>";
	echo "<option value='LO'>Loma</option>";
	echo "<option value='PY'>Pyh� (yhti�kohtainen)</option>";
	echo "<option value='PO'>Vapaa/Poissa</option>";
	echo "</select>";
	echo "</td>";
	echo "</tr>";
	echo "<tr>";
	echo "<th>Alkuaika:</th>";
	echo "<td><input type='text' name='pvmalku' value='" . date('d.m.Y'). " 07:00'></td><td class='back'>pp.kk.vvvv hh:mm</td>";
	echo "</tr></tr>";
	echo "<th>Loppuaika:</th>";
	echo "<td><input type='text' name='pvmloppu' value='" . date('d.m.Y'). " 15:00'></td><td class='back'>pp.kk.vvvv hh:mm</td>";
	echo "</tr>";
	echo "<table>";
	echo "<br>";
	echo "<input type='submit' value='Lis�� kalenteriin'>";
	echo "</form>";
}

// FOOTER
require ("inc/footer.inc");
