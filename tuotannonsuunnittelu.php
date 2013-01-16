<?php

require 'inc/parametrit.inc';

require 'valmistuslinjat.inc';
require 'valmistus.class.php';

echo "<link rel='stylesheet' type='text/css' href='fullcalendar.css' />
	<link rel='stylesheet' type='text/css' href='valmistuslinjat.css' />
	<script type='text/javascript' src='fullcalendar.js'></script>
	<script type='text/javascript' src='valmistuslinjat.js'></script>";


// Jos $teetä ei ole
if (!isset($tee)) $tee = '';

// Debug
if (isset($laske_kestot_uudelleen)) {
	rebuild_valmistuslinjat();
}

/** Valmistusten siirtäminen valmistuslinjalla */
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

	// Siiretään aiemmaksi
	if ($direction == 'left') {
		$edellinen = etsi_edellinen_valmistus($valittu_valmistus);
		if ($edellinen) {
			// HUOM, vasemmanpuoleinen aina ensin (eli aiempi valmistus)
			vaihda_valmistusten_paikkaa($edellinen, $valittu_valmistus, $valittu_valmistus['henkilo']);
		}
	}
	// Siirretään myöhäisemmäksi
	elseif ($direction == 'right') {
		$seuraava = etsi_seuraava_valmistus($valittu_valmistus);
		if ($seuraava) {
			vaihda_valmistusten_paikkaa($valittu_valmistus, $seuraava, $valittu_valmistus['henkilo']);
		}
	}
}

/** Valmistuksen tilan päivittäminen */
if ($tee == 'paivita' and isset($method) and $method == 'update') {
	$valmistus = Valmistus::find($tunnus);

	// Keskeytä työ (TK) ja Valmis tarkastukseen (VT) kysyy lisäformilla tiedot valmistuksesta
	if (!isset($varmistus) and ($tila == 'TK' or $tila == 'VT')) {
		$otsikko = ($tila=='TK') ? 'Keskeytä työ' : 'Valmista tarkastukseen';

		echo "<font class='head'>" . t($otsikko) . "</font>";

		echo "<form method='post'>";
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
				<th>Valmistettava määrä</th>
				<td><input type='text' name='valmisteet[{$valmiste['tunnus']}][maara]' value='{$valmiste['varattu']}'></td>
				</tr>";
			echo "<tr>
				<th>Ylityötunnit</th>
				<td><input type='text' name='valmisteet[{$valmiste['tunnus']}][tunnit]'></td>
				</tr>";
		}

		echo "<tr><th>Kommentit</th><td><input type='text' name='kommentti'></td></tr>";
		echo "</table>";

		echo "<a href='tuotannonsuunnittelu.php'>Takaisin</a>";
		echo "<input type='submit' value='Valmis'>";
		echo "</form>";
	}
	else {
		// Muut tilat päivitetään suoraan
		$varmistus = 'ok';
	}

	// Jos kaikki ok, päivitetään valmistus
	if ($varmistus == 'ok') {
		#echo "valmistus: $tunnus<br>";
		#echo "kommentti: $kommentti<br>";

		// Päivitetään valmisteet
		foreach ($valmistus->tuotteet() as $valmiste) {
			#echo "valmiste: {$valmiste['tunnus']} {$valmiste['varattu']} uudet: ";
			#echo "maarat: " . $valmisteet[$valmiste['tunnus']]['maara'];
			#echo "ylityot: " . $valmisteet[$valmiste['tunnus']]['tunnit'];
		}

		// Yritetään vaihtaa valmistuksen tilaa
		try {
			$valmistus->setTila($tila);
			$tee = '';
		} catch (Exception $e) {
			echo "<font class='error'>Valmistuksen tilan muuttaminen epäonnistui. <br>{$e->getMessage()}</font>";
		}
	}

	rebuild_valmistuslinjat();

}

/** Lisätään valmistus valmistusjonoon
*/
if ($tee == 'lisaa_tyojonoon') {
	// Lisätään valmistus valmistuslinjalle
	lisaa_valmistus($valmistus, $valmistuslinja);
	$tee = '';
}

/** Päivitetään valmistuksen tila
*/
if ($tee == 'paivita_tila') {
	$valmistus = $_POST['tunnus'];
	$tila = $_POST['tila'];

	// Haetaan valmistuksen tiedot
	$query = "SELECT kalenteri.kuka, kalenteri.tunnus, kalenteri.henkilo kalenteri.otunnus, tilausrivi.nimitys, tilausrivi.varattu, tilausrivi.yksikko
				FROM kalenteri
				JOIN lasku ON (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
				JOIN tilausrivi ON (lasku.yhtio=tilausrivi.yhtio AND lasku.tunnus = tilausrivi.otunnus AND tilausrivi.tyyppi='W')
				WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
				AND kalenteri.tunnus='{$valmistus}';";
	$result = pupe_query($query);
	$valmistus = mysql_fetch_assoc($result);

	// Valmista tarkastukseen
	if ($tila == 'VT') {

	}
	// Keskeytä työ
	elseif ($tila == 'TK' or $tila == 'VT') {

		echo "<div class='info'>
			<h3>Keskeytä työ</h3>
			<form method='post'>
				<table>
				<tr>
					<th>Valmistus</th>
					<td>{$valmistus['tunnus']} {$valmistus['varattu']} {$valmistus['yksikko']}</td>
				</tr>
				<tr>
					<th>Käytetyt tunnit</th>
					<td><input type='text' value='{$valmistus['varattu']}'></td>
				</tr>
				<tr>
					<th>Ylityötunnit</th>
					<td><input type='text'></td>
				</tr>
				<tr>
					<th>Kommentit</th>
					<td><input type='text'></td>
				</tr>
				<tr>
					<td></td>
					<td align='right'><input type='submit' value='Valmis'></td>
				</tr>
				</table>
			</form>
		</div>";
	}
	// Valmistukseen ( Muita valmistuksia voi joutua siirtelemään )
	elseif ($tila == 'VA') {

		// Tarkistetaan ettei linjalla ole jo työtä valmistuksessa
		$query = "SELECT kalenteri.kuka, otunnus, valmistuksen_tila
					FROM kalenteri
					JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
					WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
					AND kalenteri.kuka='{$valmistus['kuka']}'
					AND valmistuksen_tila='VA'";
		$result = pupe_query($query);

		// Jos linjalla on jo valmistus kesken
		if (mysql_num_rows($result) > 0) {
			echo "<font class='error'>Valmistuslinjalla on jo työ valmistuksessa.</font>";
		}
		else {
			// Pitää järjestellä uusiks
			$aloitusaika = strtotime('now');
			#echo "aloitusaika: ".date('Y-m-d H:i:s', $aloitusaika)."<br>";
			$pvmalku = round_time($aloitusaika);
			#echo "pyöristetty aloitusaika: ".date('Y-m-d H:i:s', $pvmalku). "<br>";

			// Aloitetaan valmistus
			#echo "Aloitetaan valmistus: {$valmistus['tunnus']}<br>";

			// Lasketaan valmistuksen kesto
			$kesto = valmistuksen_kesto($valmistus['otunnus']);
			#echo "kesto: $kesto<br>";

			// Lasektaan valmistuksen oikea loppuaika
			$pvmloppu = laske_loppuaika($pvmalku, $kesto*60, $valmistus['henkilo']);
			#echo "valmistuksen arvioitu päättymisaika: " . date('Y-m-d, H:i:s', $pvmloppu) . "<br>";

			// Päivitetään valmistuksen pvmalku ja pvmloppu
			$pvmalku = date('Y-m-d H:i', $pvmalku);
			$pvmloppu = date('Y-m-d H:i', $pvmloppu);
			$query = "UPDATE kalenteri SET pvmalku='{$pvmalku}', pvmloppu='{$pvmloppu}' WHERE tunnus='{$valmistus['tunnus']}'";
			#echo $query."<br>";
			pupe_query($query);

			// Päivitetään valmistuksen tila
			$query = "UPDATE lasku SET valmistuksen_tila='{$tila}' WHERE tunnus='{$valmistus['otunnus']}'";
			#echo $query;
			pupe_query($query);

			uudelleenlaske_kestot($valmistus['kuka'], $valmistus['pvmalku']);
		}
	}
	elseif ($tila=='OV') {
		// Takaisin parkkiin?

		// Päivitetään valmistuksen_tila ja poistetaan valmistus kalenterista
		$query = "UPDATE lasku SET valmistuksen_tila='{$tila}' WHERE tunnus='{$valmistus['otunnus']}'";
		pupe_query($query);
		echo $query;

		// Poistetaan kalenterista
		$query = "DELETE FROM kalenteri WHERE tunnus='{$valmistus['tunnus']}'";
		pupe_query($query);

		uudelleenlaske_kestot($valmistus['kuka']);
	}
	else {
		// Päivitetään valmistuksen_tila
		$query = "UPDATE lasku SET valmistuksen_tila='{$tila}' WHERE tunnus='{$valmistus['otunnus']}'";
		#pupe_query($query);
		echo $query;
	}

	$tee = '';

	echo "<br>päivitetään valmistus: {$valmistus['tunnus']}<br>";
	echo "tilaksi: {$tila}<br>";
	var_dump($_POST);
}

if ($tee == 'lisaa_kalenteriin') {

	// Loppuaika timestampiksi
	if (!empty($pvmloppu)) $pvmloppu = strtotime($pvmloppu);

	// Alkuaika on pakko syöttää
	if (empty($pvmalku)) {
		echo "<font class='error'>".t("Alkuaika ei voi olla tyhjä")."</font>";
	}
	else {
		$pvmalku = strtotime($pvmalku);

		// Jos pvmloppu on tyhjä, tehdään päivän eventti aloitusajan mukaan
		if (empty($pvmloppu)) {
			$pvmloppu = mktime(23, 59, 59, date('m', $pvmalku), date('d', $pvmalku), date('Y', $pvmalku));
		}

		// Jos alkuaika on pienempi kuin loppuaika, lisätään tapahtuma kalenteriin
		if ($pvmalku < $pvmloppu) {
			// päivämäärät ok
			$pvmalku = date('Y-m-d H:i:s', $pvmalku);
			$pvmloppu = date('Y-m-d H:i:s', $pvmloppu);

			$query = "INSERT INTO kalenteri SET yhtio='{$kukarow['yhtio']}', tyyppi='$tyyppi', pvmalku='$pvmalku', pvmloppu='$pvmloppu', henkilo='$valmistuslinja'";
			pupe_query($query);
		}
		else {
			echo "<font class='error'>".t("Loppuajan on oltava suurempi kuin alkuajan")."</font>";
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
			<option value='OV'>Siirä parkkiin</option>
			<option value='VA'>Aloita valmistus</option>
			<option value='TK'>Keskeytä valmistus</option>
			<option value='VT'>Valmis tarkistukseen</option>
			</select>
		</form>";
	echo "<br><a href='#' id='close_bubble'>sulje</a>";
	echo "</div>";

	// html
	echo "<font class='head'>".t("Työjono työsuunnittelu")."</font><hr>";

	echo "<input type='hidden' id='yhtiorow' value='{$kukarow['yhtio']}'>";
	echo "<div id='calendar'></div>";

	echo "<br>";
	echo "<a href='tuotannonsuunnittelu.php?laske_kestot_uudelleen=true'>Laske valmistuslinjojen kestot uudelleen</a>";
	echo "<br>";

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
	echo "<th>Määrä</th>";
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

	/* MUUT */
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
	echo "<option value=''>Yhtiökohtainen</option>";

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
	echo "<option value='PE'>Pekkaspäivä</option>";
	echo "<option value='SA'>Sairasloma</option>";
	echo "<option value='MT'>Muu työ</option>";
	echo "<option value='LO'>Loma</option>";
	echo "<option value='PY'>Pyhä (yhtiökohtainen)</option>";
	echo "<option value='PO'>Vapaa/Poissa</option>";
	echo "</select>";
	echo "</td>";
	echo "</tr>";
	echo "<tr>";
	echo "<th>Alkuaika:</th>";
	echo "<td><input type='text' name='pvmalku' value='" . date('d.m.Y'). " 08:00'></td><td class='back'>pp.kk.vvvv hh:mm</td>";
	echo "</tr></tr>";
	echo "<th>Loppuaika:</th>";
	echo "<td><input type='text' name='pvmloppu' value='" . date('d.m.Y'). " 16:00'></td><td class='back'>pp.kk.vvvv hh:mm</td>";
	echo "</tr>";
	echo "<tr>";
	echo "<td><input type='submit' value='Valmis'></td>";
	echo "</tr>";
	echo "<table>";
	echo "</form>";
}

// FOOTER
require ("inc/footer.inc");
