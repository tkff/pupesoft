<?php
require 'inc/parametrit.inc';
require 'valmistuslinjat.inc';
// require "inc/connect.inc";
// require "inc/functions.inc";

echo "<link rel='stylesheet' type='text/css' href='fullcalendar.css' />
	<script type='text/javascript' src='fullcalendar.js'></script>
	<script type='text/javascript' src='valmistuslinjat.js'></script>";

echo "<style type='text/css'>
a {
	text-decoration: none;
}

.controls {
	width: 100%;
	height: 23px
}

.vasemmalle {
	position:relative;
	top: 6px;
	padding-right: 5px;
	float: left;
}
.oikealle {
	position:relative;
	position:relative;
	top: 6px;
	float: right;
}

.form {
	width: 100%;
	text-align: center;
}

.fc-event-inner {
	text-align: center;
}


.fc-event-title {
	float:left;
	text-align: center;
	width: 100%;
}

.fc-resourceName {
	text-align: center;
	font-size: 10pt;
}

.fc-event-prev {
	float: left;
}
.fc-event-next {
	float: right;
}

#bubble {
	display:none;
	position: fixed;
	color: black;
	background-color: #FFF;
	z-index: 99;

	padding: 10px;
	border: solid 1px;
	border-color: #AAA;
	border-radius: 10px;

	text-align: center;
}

#info {
	font-weight: bold;
	margin: 2px;
}

#content {
	padding: 3px;
}
</style>";


echo "<font class='head'>".t("Työjono työsuunnittelu")."</font><hr>";

echo "<input type='hidden' id='yhtiorow' value='{$kukarow['yhtio']}'>";
echo "<div id='calendar'></div>";

echo "<br>";
echo "<a href='tuotannonsuunnittelu.php?laske_kestot_uudelleen=true'>Laske valmistuslinjojen kestot uudelleen</a>";
echo "<br>";

if (!isset($tee)) $tee = '';

if (isset($laske_kestot_uudelleen) and $laske_kestot_uudelleen == true) {
	rebuild_valmistuslinjat();
}

/** Valmistusten siirtäminen valmistuslinjalla */
if (isset($method) and $method == 'move') {

	// Haetaan valitun valmistuksen tiedot
	$query = "SELECT *, kalenteri.tunnus as tunnus
				FROM kalenteri
				JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
				WHERE kalenteri.yhtio='{$kukarow['yhtio']}' AND kalenteri.tunnus='{$tunnus}'";
	$result = pupe_query($query);
	$valittu_valmistus = mysql_fetch_assoc($result);

	// Siiretään aiemmaksi
	if ($direction == 'left') {
		$edellinen = etsi_edellinen_valmistus($valittu_valmistus);
		if ($edellinen) {
			// HUOM, vasemmanpuoleinen aina ensin (eli aiempi valmistus)
			vaihda_valmistusten_paikkaa($edellinen, $valittu_valmistus);
		}
	}
	// Siirretään myöhäisemmäksi
	elseif ($direction == 'right') {
		$seuraava = etsi_seuraava_valmistus($valittu_valmistus);
		if ($seuraava) {
			vaihda_valmistusten_paikkaa($valittu_valmistus, $seuraava);
		}

	}
}

/** Valmistuksen tilan päivittäminen */
if (isset($method) and $method == 'update') {

	echo "päivitetään valmistuksen $tunnus tilaa!<br>";

	// Haetaan valitun valmistuksen tiedot
	$query = "SELECT *, kalenteri.tunnus as tunnus
				FROM kalenteri
				JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
				WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
				AND kalenteri.tunnus='{$tunnus}'";
	$result = pupe_query($query);
	$valittu_valmistus = mysql_fetch_assoc($result);

	switch ($tila) {
		// Siirretään valmistus takaisin parkkiin
		case 'OV':
			echo "Odottaa valmistusta";

			// Päivitetään valmistuksen_tila ja poistetaan valmistus kalenterista
			$query = "UPDATE lasku SET valmistuksen_tila='OV' WHERE tunnus='{$valittu_valmistus['otunnus']}'";
			pupe_query($query);

			// Poistetaan kalenterista
			$query = "DELETE FROM kalenteri WHERE tunnus='{$valittu_valmistus['tunnus']}'";
			pupe_query($query);

			break;
		// Valmistus valmistukseen
		case 'VA':
			echo "valmistukseen";

			// Tarkistetaan että linjalla ei ole muuta työtä valmistuksessa.
			$query = "SELECT kalenteri.kuka, otunnus, valmistuksen_tila
						FROM kalenteri
						JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
						WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
						AND kalenteri.henkilo='{$valittu_valmistus['henkilo']}'
						AND valmistuksen_tila='VA'";
			$result = pupe_query($query);

			// Jos linjalla on joku muu työ valmistuksessa
			if (mysql_num_rows($result) > 0) {
				echo "<font class='error'>Linjalla on keskeneräinen valmistus</font>";
			}
			else {
				// Aloitetaan työ puolentunnin tarkkuudella
				// TODO: Alkuajan tulisi alkaa seuraavana vapaana työaikana
				// (esim. klo 15:00 -> seuraava aamu klo 07:00)
				$pvmalku = round_time(strtotime('now'));
				$kesto = valmistuksen_kesto($valittu_valmistus['otunnus']);
				$pvmloppu = laske_loppuaika($pvmalku, $kesto*60);

				$pvmalku = date('Y-m-d H:i:s', $pvmalku);
				$pvmloppu = date('Y-m-d H:i:s', $pvmloppu);

				// Päivitetään valmistuksen uudet ajat
				$query = "UPDATE kalenteri
							SET pvmalku='{$pvmalku}', pvmloppu='{$pvmloppu}'
							WHERE tunnus='{$valittu_valmistus['tunnus']}'";
				pupe_query($query);

				// Päivitetään valmistuksen tila
				$query = "UPDATE lasku
							SET valmistuksen_tila='{$tila}'
							WHERE tunnus='{$valittu_valmistus['otunnus']}'";
				pupe_query($query);

				$valittu_valmistus['pvmalku'] = $pvmalku;
				$valittu_valmistus['pvmloppu'] = $pvmloppu;

				// Uudelleenlasketaan kestot
				rebuild_valmistuslinjat();
			}

			break;
		case 'TK':
			echo "keskeytetty";
			$query = "UPDATE lasku SET valmistuksen_tila='TK' WHERE tunnus='{$valittu_valmistus['otunnus']}'";
			pupe_query($query);

			break;
		case 'VT':
			echo "valmis tarkastukseen";
			$query = "UPDATE lasku SET valmistuksen_tila='VT' WHERE tunnus='{$valittu_valmistus['otunnus']}'";
			pupe_query($query);

			break;
		default:
			echo "Tuntematon tila";
			break;
	}
}

/** Lisätään valmistus valmistusjonoon
*/
if ($tee == 'lisaa_tyojonoon') {

	echo "Lisätään valmistus $valmistus valmistusjonoon '$valmistuslinja'<br>";

	// Haetaan valmistuksen tiedot
	$query = "SELECT varattu
				FROM tilausrivi
				WHERE yhtio='{$kukarow['yhtio']}'
				AND otunnus=$valmistus
				AND tyyppi='W'";
	$result = pupe_query($query);
	$valmistettava = mysql_fetch_assoc($result);

	// Tarkistetaan edellisen valmistuksen loppupvm
	$query = "SELECT *
				FROM kalenteri
				WHERE yhtio='{$kukarow['yhtio']}'
				AND tyyppi='valmistus'
				AND henkilo='{$valmistuslinja}'
				ORDER BY pvmloppu desc LIMIT 1";
	echo $query."<br>";

	$result = pupe_query($query);
	$edellinen_valmistus = mysql_fetch_assoc($result);

	// Linjalla ei ole valmistuksia, lisätään uusi valmistus tästä hetkestä lähtien
	if ($edellinen_valmistus['pvmloppu'] != '') {
		echo "viimeisin valmistus päättyy: $edellinen_valmistus[pvmloppu]<br>";
		$pvmalku = strtotime($edellinen_valmistus['pvmloppu']);

		// TODO: hajoo perjantaisin
		if (date('H', $pvmalku) == 15) {
			$pvmalku = mktime(7, 0, 0, date('m', $pvmalku), date('d', $pvmalku)+1, date('Y', $pvmalku));
		}
	}
	else {
		// Etitään seuraava sopiva paikka, esim. seuraava aamu jos yritetää lisätä työajalla
		$pvmalku = round_time(strtotime('now'));

		echo "etitään sopiva aloitusaika<br>";

		$day_of_week = date('w', $pvmalku);
		$day = date('d', $pvmalku);
		echo "day: $day<br>";
		$hour = date('H', $pvmalku);


		// lauantai -> maanantai
		if ($day_of_week == 6) {
			echo "lauantai<br>";
			$day += 2;
			$hour = 7;
		}
		// sunnuntai -> maanantai
		elseif ($day_of_week == 0) {
			echo "sunnuntai<br>";
			$day += 1;
			$hour = 7;
		}

		// Jos ollaan työpäivän ohi
		elseif($hour > 15 and $hour <= 23) {
			// Perjantai-iltapäivä -> maanantai
			if ($day_of_week == 5) {
				$day += 3;
				$hour = 7;
			}
			$day += 1;
			$hour = 7;
		}
		// Jos ollaan aamuyössä
		elseif($hour >= 0 and $hour < 7) {
			// Tunnit aamuun
			$hour = 7;
		}

		echo "day: $day<br>";
		// Luodaan uusi alkupvm
		$pvmalku = mktime($hour, date('i', $pvmalku), 0, date('m', $pvmalku), $day, date('Y', $pvmalku));
	}

	$debug[] = "valmistus alkaa: ".date('Y-m-d H:i:s', $pvmalku)."<br>";

	// Lasketaan valmistukseen kuluva aika
	$valmistukseen_kuluva_aika = valmistuksen_kesto($valmistus);
	$debug[] = "Lisätty valmistus kestää n. ".$valmistukseen_kuluva_aika. " tuntia <br>";

	$loppuaika = laske_loppuaika($pvmalku, $valmistukseen_kuluva_aika*60);

	$pvmalku = date('Y-m-d H:i:s', $pvmalku);
	$pvmloppu = date('Y-m-d H:i:s', $loppuaika);
	$debug[] = "valmistus loppuu: ".$pvmloppu."<br>";

	// Lisätään valmistus kalenteriin
	$query = "INSERT INTO kalenteri SET
				yhtio		= '$kukarow[yhtio]',
				kuka 		= '$kukarow[kuka]',
				henkilo		= '$_POST[valmistuslinja]',
				pvmalku 	= '$pvmalku',
				pvmloppu 	= '$pvmloppu',
				tyyppi		= 'valmistus',
				otunnus		= '$_POST[valmistus]'";
	$debug[] = $query;

	$result = pupe_query($query);

	// DEBUGGIA
	$alkupvm = new DateTime();

	$kesto_paivissa = round(valmistuksen_kesto($valmistus) / 8);

	$loppupvm = new DateTime();
	$loppupvm->add(new DateInterval("P".$kesto_paivissa."D"));

	$debug[] = "<br>alkupvm: ".$alkupvm->format('d.m.Y H:i');
	$debug[] = "<br>loppupvm: ".$loppupvm->format('d.m.Y H:i');
}

/** Päivitetään valmistuksen tila
*/
if ($tee == 'paivita_tila') {
	$valmistus = $_POST['tunnus'];
	$tila = $_POST['tila'];

	// Haetaan valmistuksen tiedot
	$query = "SELECT kalenteri.kuka, kalenteri.tunnus, kalenteri.otunnus, tilausrivi.nimitys, tilausrivi.varattu, tilausrivi.yksikko
				FROM kalenteri
				JOIN lasku ON (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
				JOIN tilausrivi ON (lasku.yhtio=tilausrivi.yhtio AND lasku.tunnus = tilausrivi.otunnus AND tilausrivi.tyyppi='W')
				WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
				AND kalenteri.tunnus='{$valmistus}';";
	$result = pupe_query($query);
	$valmistus = mysql_fetch_assoc($result);

	// Valmista tarkastukseen
	if ($tila == 'VT') {

		echo "<div class='info'>
			<h3>Valmista tarkastukseen</h3>
			<form method='post'>
				<table>
				<tr>
					<th>Valmistus</th>
					<td>{$valmistus['tunnus']} {$valmistus['varattu']} {$valmistus['yksikko']}</td>
				</tr>
				<tr>
					<th>Valmistettava määrä</th>
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
	// Keskeytä työ
	elseif ($tila == 'TK') {

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
			echo "aloitusaika: ".date('Y-m-d H:i:s', $aloitusaika)."<br>";
			$pvmalku = round_time($aloitusaika);
			echo "pyöristetty aloitusaika: ".date('Y-m-d H:i:s', $pvmalku). "<br>";

			// Aloitetaan valmistus
			echo "Aloitetaan valmistus: {$valmistus['tunnus']}<br>";

			// Lasketaan valmistuksen kesto
			$kesto = valmistuksen_kesto($valmistus['otunnus']);
			echo "kesto: $kesto<br>";

			// Lasektaan valmistuksen oikea loppuaika
			$pvmloppu = laske_loppuaika($pvmalku, $kesto*60);
			echo "valmistuksen arvioitu päättymisaika: " . date('Y-m-d, H:i:s', $pvmloppu) . "<br>";

			// Päivitetään valmistuksen pvmalku ja pvmloppu
			$pvmalku = date('Y-m-d H:i', $pvmalku);
			$pvmloppu = date('Y-m-d H:i', $pvmloppu);
			$query = "UPDATE kalenteri SET pvmalku='{$pvmalku}', pvmloppu='{$pvmloppu}' WHERE tunnus='{$valmistus['tunnus']}'";
			echo $query."<br>";
			pupe_query($query);

			// Päivitetään valmistuksen tila
			$query = "UPDATE lasku SET valmistuksen_tila='{$tila}' WHERE tunnus='{$valmistus['otunnus']}'";
			echo $query;
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
		$debug[] = $query;

		uudelleenlaske_kestot($valmistus['kuka']);
	}
	else {
		// Päivitetään valmistuksen_tila
		$query = "UPDATE lasku SET valmistuksen_tila='{$tila}' WHERE tunnus='{$valmistus['otunnus']}'";
		#pupe_query($query);
		echo $query;
	}

	echo "<br>päivitetään valmistus: {$valmistus['tunnus']}<br>";
	echo "tilaksi: {$tila}<br>";
	var_dump($_POST);
}

if ($tee == 'lisaa_kalenteriin') {
	echo "lisätään kalenteriin<br>";

	// TODO: tarkista syötetyt päivämäärät

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
		elseif ($pvmalku < $pvmloppu) {
			// päivämäärät ok
			$pvmalku = date('Y-m-d H:i:s', $pvmalku);
			$pvmloppu = date('Y-m-d H:i:s', $pvmloppu);

			$query = "INSERT INTO kalenteri SET yhtio='{$kukarow['yhtio']}', tyyppi='$tyyppi', pvmalku='$pvmalku', pvmloppu='$pvmloppu', henkilo='$valmistuslinja'";
			pupe_query($query);
			echo $query."<br>";
		}
		else {
			echo "<font class='error'>".t("Loppuajan on oltava suurempi kuin alkuajan")."</font>";
		}

		echo "alkuaika: ".date('Y-m-d H:i:s', $pvmalku).", loppuaika: ".date('Y-m-d H:i:s', $pvmloppu);
	}
}
echo "<br>";

/* PARKKI */
echo "<br>";
echo "<font class='head'>".t("Parkki")."</font>";
echo "<hr>";

echo "aika : ". date('Y-m-d H:i:s', round_time(strtotime('now')));

// Haetaan valmistuslinjat
$query = "SELECT * FROM avainsana WHERE yhtio='{$kukarow['yhtio']}' AND laji='valmistuslinja' ORDER BY selite";
$result = pupe_query($query);

$valmistuslinjat = array();

while($linja = mysql_fetch_assoc($result)) {
	$linjat[] = $linja;
}

// Haetaan valmistukset parkissa olevat valmistukset (OV ja mitkä eivät ole jo kalenterissa)
$query = "SELECT lasku.tunnus, tilausrivi.tuoteno, tilausrivi.nimitys, tilausrivi.varattu, tilausrivi.yksikko, lasku.valmistuksen_tila, kalenteri.pvmalku
			FROM lasku
			JOIN tilausrivi on (tilausrivi.yhtio=lasku.yhtio AND tilausrivi.otunnus=lasku.tunnus)
			LEFT JOIN kalenteri on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
			WHERE lasku.yhtio='{$kukarow['yhtio']}'
			AND tila='V'
			AND tilausrivi.tyyppi='W'
			AND alatila in ('','A','B','J')
			AND pvmalku IS NULL";
$result = pupe_query($query);

echo "<table border=1>";
echo "<tr>";
echo "<th>Tila</th>";
echo "<th>Nimitys</th>";
echo "<th>Määrä</th>";
echo "<th>Kesto (h)</th>";
echo "<th></th>";
echo "</tr>";

while ($valmistus = mysql_fetch_assoc($result)) {

	echo "<tr>";
	echo "<td>{$valmistus['valmistuksen_tila']}</td>";
	echo "<td>{$valmistus['tunnus']} {$valmistus['nimitys']}</td>";
	echo "<td>{$valmistus['varattu']} {$valmistus['yksikko']}</td>";
	echo "<td>" . valmistuksen_kesto($valmistus['tunnus'])."</td>";
	echo "</td>";

	echo "<form method='POST'>";
	echo "<td>";
	echo "<input type='hidden' name='tee' value='lisaa_tyojonoon'>";
	echo "<input type='hidden' name='valmistus' value='$valmistus[tunnus]'>";

	echo "<select name='valmistuslinja'>";
	echo "<option value=''>Valitse linja</option>";

	foreach($linjat as $linja) {
		echo "<option value='$linja[selite]'>$linja[selitetark]</option>";
	}

	echo "</select>";
	echo "<input type='submit' value='".t("Aloita valmistus")."'>";
	echo "</td>";
	echo "</form>";

	###########
	echo "<td>";
	$pvm = '2012-12-14 14:30:00<br>';
	echo "rakaa-aineiden saldot $pvm";
	$puutteet = puuttuvat_raaka_aineet($valmistus['tunnus']);
	if(!empty($puutteet)) {
		foreach($puutteet as $tuoteno => $saldo) {
			echo $tuoteno . " " . $saldo;
			echo "<br>";
		}
	}

	echo "</td>";
	###########
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
echo "<td><input type='text' name='pvmalku'></td>";
echo "</tr></tr>";
echo "<th>Loppuaika:</th>";
echo "<td><input type='text' name='pvmloppu'></td>";
echo "</tr>";
echo "<tr>";
echo "<td><input type='submit' value='Valmis'></td>";
echo "</tr>";
echo "<table>";
echo "</form>";

/* TYÖJONO TYÖNTEKIJÄ */
echo "<br>";
echo "<font class='head'>".t("Tuotannonsuunnittelu")."</font>";
echo "<hr>";

foreach($linjat as $linja) {
	echo "<b>".$linja['selitetark']."</b><br>";

	// Haetaan kaikki linjan kalenterimerkinnät
	$tyojono_query = "SELECT kalenteri.kuka, kalenteri.henkilo, nimitys, varattu, yksikko, pvmalku, pvmloppu, kalenteri.tunnus, lasku.valmistuksen_tila
					FROM kalenteri
					JOIN tilausrivi on (tilausrivi.yhtio=kalenteri.yhtio and tilausrivi.otunnus=kalenteri.otunnus)
					JOIN lasku on (lasku.yhtio=kalenteri.yhtio and lasku.tunnus=kalenteri.otunnus)
					WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
					AND henkilo='{$linja['selite']}'
					AND tilausrivi.tyyppi='W'
					ORDER BY pvmalku";
	$tyojono_result = pupe_query($tyojono_query);

	if (mysql_num_rows($tyojono_result) == 0) {
		echo "Ei valmistuksia jonossa.<br>";
	} else {

		echo "<table border='1'>";

		while($tyojono = mysql_fetch_assoc($tyojono_result)) {
			echo "<tr>";
			echo "<td>";
			echo $tyojono['valmistuksen_tila'] . " " . $tyojono['nimitys'] . " " . $tyojono['varattu'] . " " . $tyojono['yksikko'];
			echo "</td>";

			echo "<form method='post'>";
			echo "<td>";
			echo "<input type='hidden' name='tee' value='paivita_tila'>";
			echo "<input type='hidden' name='tunnus' value={$tyojono['tunnus']}>";
			echo "<select name='tila' onchange='submit()'>";
			echo "<option value=''>Valitse</option>";
			echo "<option value='OV'>Siirä parkkiin</option>";
			echo "<option value='VA'>Aloita valmistus</option>";
			echo "<option value='TK'>Keskeytä valmistus</option>";
			echo "<option value='VT'>Valmis tarkistukseen</option>";
			echo "</select>";
			echo "</td>";
			echo "</form>";

			echo "<td>";
			echo "({$tyojono['pvmalku']} - {$tyojono['pvmloppu']})";
			echo "</td>";
			echo "</tr>";
		}
		echo "</table>";
	}
	echo "<br>";
}

/* DEBUG */
echo "<pre>";
foreach ($debug as $message) {
	echo $message;
}
echo "</pre>";

// Valmistuslinjojen info popup
echo "<div id='bubble'>
	<div id='info'></div>";

echo "<div id='content'></div>
	<form action='tuotannonsuunnittelu.php?method=update' method='post' id='toiminto'>
		<input type='hidden' name='tunnus' id='valmistuksen_tunnus'>
		<select name='tila' onchange='submit()'>
		<option value=''>Valitse</option>
		<option value='OV'>Siirä parkkiin</option>
		<option value='VA'>Aloita valmistus</option>
		<option value='TK'>Keskeytä valmistus</option>
		<option value='VT'>Valmis tarkistukseen</option>
		</select>
	</form>
</div>";

