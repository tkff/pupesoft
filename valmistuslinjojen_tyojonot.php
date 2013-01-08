<?php

require 'inc/parametrit.inc';

require 'valmistuslinjat.inc';
require 'valmistus.class.php';

$title = array('VT' => 'Valmista tarkastukseen', 'TK' => 'Keskeyt� ty�');

if (isset($tee) and $tee == 'verify') {
	// Haetaan aina valmistus
	$valmistus = Valmistus::find($tunnus);

	// N�ytet��n edit formi (valmista_tarkastukseen tai keskeytys)
	// Formilla kysyt��n valmistettu m��r� ja kommentit
	if ($tila == Valmistus::VALMIS_TARKASTUKSEEN or $tila == Valmistus::KESKEYTETTY) {
		$title = $title[$tila];
		include 'views/valmistus/edit.php';
	} else {
		$tee = 'update';
	}
}

if (isset($tee) and $tee == 'update') {

	// Haetaan aina valmistus
	$valmistus = Valmistus::find($tunnus);

	// Loopataan p�ivitett�v�t valmisteet l�pi ja tarkistetaan sy�tetyt m��r�t
	// Splitataan tarvittaessa
	try {

		// Jos on tultu lomakkeen kautta ($tee=='verify')
		if (isset($valmisteet)) {
			$tuotteet = $valmistus->tuotteet();

			if (empty($tuotteet)) {
				throw new Exception("Valmistuksella ei ole yht��n valmistetta");
			}

			// Loopataan valmistukset l�pi
			foreach ($tuotteet as $valmiste) {

				// Sy�tetyt arvot
				$maara = $valmisteet[$valmiste['tuoteno']]['maara'];
				$tunnit = $valmisteet[$valmiste['tuoteno']]['tunnit'];

				// M��r�� pienennet��n (splitataan valmistus)
				if ($maara < $valmiste['varattu']) {
					throw new Exception("Virhe valmistuksen keskeytyksess� (ei jaettu)");
				}
				// M��r� on sama (valmistus on valmistettu kokonaan)
				else if ($maara == $valmiste['varattu']) {
					echo "m��r� sama! p�ivitet��n vaan tila ja lis�t��n kommentit";

				}
				// Virhe
				else {
					throw new Exception("Valmistettava m��r� ei voi olla suurempi kuin tilattu m��r�");
				}
			}
		}

		// P�ivitet��n lopuksi tila
		$valmistus->setTila($tila);
	} catch (Exception $e) {
		$errors = "VIRHE: {$e->getMessage()}";
	}

	// Palataan ty�jono n�kym��n
	$tee = '';

}

if (isset($tee) and $tee == 'paivita_tila') {

	// // Otsikko formille
	// $title = "Valmista tarkastukseen";

	// // Haetaan valmistus
	// $valmistus = Valmistus::find($tunnus);

	// $action = '';
	// include 'views/valmistus/edit.php';
	// exit();

	// // Jos tila muutetaan valmis tarkastukseen, keskeytetty tai osavalmistus tulee k�yt�t�j�n
	// // sy�tt�� lis�tietoja
	// if ($tila == 'VT' or $tila == 'TK') {
	// 	echo "<div class='info'>
	// 		<font class='head'>" . t($title[$tila]) . "</font>
	// 		<form method='post'>
	// 			<input type='hidden' name='tee' value='paivita_tila'>
	// 			<input type='hidden' name='tila' value='{$tila}'>
	// 			<input type='hidden' name='paivita' value='true'>
	// 			<input type='hidden' name='tunnus' value='{$valmistus->tunnus()}'>
	// 			<table>
	// 			<tr>
	// 				<th>Valmistus</th>
	// 				<th>M��r�</th>
	// 				<th>Ylity�tunnit</th>
	// 				<th>Kommentti</th>
	// 			</tr>";

	// 			foreach($valmistus->tuotteet() as $tuote) {
	// 				echo "<tr>";
	// 				echo "<td>" . $tuote['nimitys'] . " " . $tuote['varattu'] . " " . $tuote['yksikko'] . "</td>";
	// 				echo "<td><input size=6 type='text' name='jaettavat_valmisteet[{$tuote[tunnus]}]' value='{$tuote['varattu']}'></td>";
	// 				echo "<td><input size=6 type='text' name='ylityotunnit[{$tuote[tunnus]}]' value=''></td>";
	// 				echo "<td><input size=20 type='text' name='kommentit[{$tuote[tunnus]}]' value=''></td>";
	// 				echo "</tr>";
	// 			}

	// 	echo 	"<tr>
	// 				<td colspan=4 align='right'>
	// 					<a href='valmistuslinjojen_tyojonot.php'>Takaisin</a>
	// 					<input type='submit' value='Valmis'>
	// 				</td>
	// 			</tr>
	// 			</table>
	// 		</form>
	// 	</div>";
	// }

	// P�ivitet��n valmistus, Valmis Tarkastukseen tai Ty� Keskeytetty
	if ($paivita == true and ($tila == 'VT' or $tila == 'TK')) {
		echo "p�ivitet��n valmistus<br>";

		// Loopataan valmistuksen valmisteet l�pi
		foreach($valmistus->tuotteet() as $valmiste) {

			// Tarvitseeko valmistus splitata, eli j�ik� jotain valmistamatta?
			if ($jaettavat_valmisteet[$valmiste['tunnus']] >= 0 AND $jaettavat_valmisteet[$valmiste['tunnus']] < $valmiste['varattu']) {
				echo "splitataan valmistus: jaa_valmistus({$valmiste['tunnus']}, $jaettavat_valmisteet)";

				try {
					$uusi_valmistus = jaa_valmistus($valmistus->tunnus(), $jaettavat_valmisteet);
				} catch (Exception $e) {
					echo "Virhe, ". $e->getMessage();
				}
			}
			// Ei splitata splitata jos sy�tetty m��r� on sama kuin valmisteen
			elseif ($jaettavat_valmisteet[$valmiste['tunnus']] == $valmiste['varattu']) {
				echo "$valmiste[nimitys] $valmiste[varattu] $valmiste[yksikko] " . $jaettavat_valmisteet[$valmiste[tunnus]] . "<br>";

			}
			// Ei voi valmistaa enemm�n kuin on tilattu
			else {
				echo "et voi valmistaa enemm�n kuin on tilattu";
			}

			// Merkataan alkuper�inen valmiiksi tai keskeytetyksi
			$valmistus->setTila($tila);

			// Ylity�tunnit ja kommentit kalenteri tauluun
			$ylityo = $ylityotunnit[$valmiste['tunnus']];
			$kommentti = $kommentit[$valmiste['tunnus']];

			$query = "UPDATE kalenteri SET kentta01='{$ylityo}', kentta02='{$kommentti}' WHERE yhtio='{$kukarow['yhtio']}' AND otunnus={$valmistus->tunnus()}";
			pupe_query($query);
		}

		echo "valmistus p�ivitetty<br>";
		if (isset($uusi_valmistus)) echo "valmistus splitattu, uuden id: $uusi_valmistus<br>";
	}
}

if ($tee == '') {

	/* TY�JONO TY�NTEKIJ� */
	echo "<font class='head'>".t("Valmistuslinjojen ty�jonot")."</font>";
	echo "<hr>";

	// Haetaan valmistuslinjat
	$linjat = hae_valmistuslinjat();

	if (empty($linjat)) {
		echo "Ei valmistuslinjoja";
	}

	foreach($linjat as $linja) {

		echo "<table>";
		echo "<tr><th colspan=4>" . t("Valmistuslinja") . ": " . $linja['selitetark']."</th></tr>";
		echo "<th>Tila</th><th>Nimitys</th><th>Aika</th><th></th>";

		// Haetaan linjan 4 uusinta kalenterimerkinn�t
		$tyojono_query = "SELECT kalenteri.kuka, kalenteri.henkilo, nimitys, varattu, yksikko, pvmalku, pvmloppu, kalenteri.tunnus, lasku.valmistuksen_tila, kalenteri.otunnus
						FROM kalenteri
						JOIN tilausrivi on (tilausrivi.yhtio=kalenteri.yhtio and tilausrivi.otunnus=kalenteri.otunnus)
						JOIN lasku on (lasku.yhtio=kalenteri.yhtio and lasku.tunnus=kalenteri.otunnus)
						WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
						AND henkilo='{$linja['selite']}'
						AND tilausrivi.tyyppi='W'
						ORDER BY pvmalku
						LIMIT 4";
		$tyojono_result = pupe_query($tyojono_query);

		// Jos ty�jono on tyhj�
		if (mysql_num_rows($tyojono_result) == 0) {
			echo "<tr>";
			echo "<td colspan=4>";
			echo "Ei valmistuksia jonossa.";
			echo "</td>";
			echo "</tr>";
		}
		else {
			// Ty�jonon ty�t
			while($tyojono = mysql_fetch_assoc($tyojono_result)) {
				echo "<tr>";
				echo "<td>" . $tyojono['valmistuksen_tila'] . "</td>";
				echo "<td>" . $tyojono['nimitys'] . " " . $tyojono['varattu'] . " " . $tyojono['yksikko'] . "</td>";

				echo "<td>({$tyojono['pvmalku']} - {$tyojono['pvmloppu']})</td>";

				echo "<td>";
				echo "<form method='post'>";
				echo "<input type='hidden' name='tee' value='verify'>";
				echo "<input type='hidden' name='tunnus' value={$tyojono['otunnus']}>";

				echo "<select name='tila' onchange='submit()'>";
				echo "<option value=''>Valitse</option>";

				// Aloittamaton valmistus voidaan aloittaa tai jakaa
				if ($tyojono['valmistuksen_tila'] == Valmistus::ODOTTAA) {
					echo "<option value='VA'>Aloita valmistus</option>";

				}
				// Aloitettu valmistus voidaan merkata valmistetuksi tai keskeytt��
				else if ($tyojono['valmistuksen_tila'] == Valmistus::VALMISTUKSESSA) {
					echo "<option value='TK'>Keskeyt� valmistus</option>";
					echo "<option value='VT'>Valmis tarkistukseen</option>";
				}
				else if ($tyojono['valmistuksen_tila'] == Valmistus::KESKEYTETTY) {
					echo "<option value='VA'>Aloita valmistus</option>";
				}

				echo "<option value='OV'>(Siir� parkkiin)</option>"; # TODO: T�t� ei tarvita t��ll�.
				echo "</select>";

				echo "</form>";
				echo "</td>";

				echo "</tr>";
			}
			echo "</table>";
			echo "<br>";
		}
	}

	// Virheet
	if (isset($errors)) echo "<font class='error'>$errors</font>";

}