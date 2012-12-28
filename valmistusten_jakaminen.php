<?php

require 'inc/parametrit.inc';
require 'valmistus.class.php';

$tee = (isset($tee)) ? $tee : '';

// Jaetaan valmistus ja sen valmisteet ja niiden raaka-aineet
if ($tee == "jaa_valmistus") {
	$errors = array();

	// Haetaan laskun tiedot (valmistus)
	$original_query = "SELECT *
						FROM lasku
						WHERE yhtio='{$kukarow['yhtio']}'
						AND tunnus=$valmistus
						LIMIT 1";
	$original_result = pupe_query($original_query);
	$original = mysql_fetch_array($original_result, 1);

	if (mysql_num_rows($original_result) == 0) {
		exit("Virhe laskun kopioinnissa, laskua ei löytynyt");
	}

	// laskun kopio, ei luoda ennenkuin tiedetään että tilausrivejä tarvitsee kopioida
	$copy = '';

	// Haetaan kaikki valmistuksen valmisteet (tyypit 'W' tai 'M')
	$valmisteet_query = "SELECT *
							FROM tilausrivi
							WHERE yhtio='{$kukarow['yhtio']}'
							AND otunnus=$valmistus
							AND tyyppi in ('W', 'M')";
	$valmisteet_result = pupe_query($valmisteet_query);

	// Loopataan valmisteet
	while($valmiste = mysql_fetch_assoc($valmisteet_result)) {

		// Alkuperäiselle jätettävä määrä ja kopiolle tuleva määrä
		$original_kpl = $jaettavat_valmisteet[$valmiste['tunnus']]; // esim. valmistettiin 3
		$copy_kpl = $valmiste['varattu'] - $original_kpl;			// 9 = 12 - 3, uudelle valmisteelle siirretään 9 kpl
		$raaka_aineen_suhde = $original_kpl / $valmiste['varattu'];

		// Jos syötetty nolla ei tehdä kopioita
		if (($original_kpl == 0 or $original_kpl == $valmiste['varattu']) and mysql_num_rows($valmisteet_result) == 1) {
			$errors[] = "Valmistusta ei jaettu";
			continue;
		}

		// Jos splitatavan määrä on oikein eli välillä (0 - tilattu_määrä)
		if (is_numeric($original_kpl) and $original_kpl >= 0 and $original_kpl <= $valmiste['varattu']) {

			// Jos määrä on sallitun rajoissa, tehdään laskusta kopio
			if (empty($copy)) {

				// Päivitetään kopiolle tulevat tiedot
				$copy = $original;
				$copy['laatija'] = $kukarow['kuka'];
				$copy['luontiaika'] = 'now()';
				$copy['muutospvm'] = '';
				$copy['tunnus'] = '';

				// Luodaan otsikon kopio
				$copy_query = "INSERT INTO
								lasku (".implode(", ", array_keys($copy)).")
								VALUES('".implode("', '", array_values($copy)). "')";

				if (!pupe_query($copy_query)) {
					exit("Virhe laskun kopioinnissa, uutta laskua ei luotu");
				}

				// Kopioidun laskun tunnus talteen
				$copy['tunnus'] = mysql_insert_id();
			}

			// Kopioidaan valmiste
			$poikkeukset = array('tilausrivi.varattu' => $copy_kpl, 'tilausrivi.otunnus' => $copy['tunnus']);
			$kopio_id = kopioi_tilausrivi($valmiste['tunnus'], $poikkeukset);

			// Päivitetään alkuperäinen valmiste
			$query = "UPDATE tilausrivi
						SET varattu=$original_kpl
						WHERE yhtio='{$kukarow['yhtio']}'
						AND tunnus={$valmiste['tunnus']}";
			pupe_query($query);

			// Loopataan raaka-aineet
			$raaka_aine_query = "SELECT *
									FROM tilausrivi
									WHERE yhtio='{$kukarow['yhtio']}'
									AND otunnus={$valmiste['otunnus']}
									AND perheid={$valmiste['perheid']}
									AND tyyppi='V'";
			$raaka_aine_result = pupe_query($raaka_aine_query);

			// Loopataan valmisteen raaka-aineet
			while($raaka_aine = mysql_fetch_assoc($raaka_aine_result)) {

				// Raaka-aineen kappaleet
				$r_original_kpl = $raaka_aine['varattu'] * $raaka_aineen_suhde; # 3 = 12 * 0.25
				$r_copy_kpl = $raaka_aine['varattu'] - $r_original_kpl;	# 9 = 12 - 3

				// Kopioidaan Raaka-aine
				$poikkeukset = array('tilausrivi.varattu' => $r_copy_kpl, 'tilausrivi.otunnus' => $copy['tunnus']);
				$kopion_id = kopioi_tilausrivi($raaka_aine['tunnus'], $poikkeukset);

				// Päivitetään alkuperäinen raaka_aine
				$query = "UPDATE tilausrivi SET varattu=". $r_original_kpl ." WHERE yhtio='{$kukarow['yhtio']}' AND tunnus={$raaka_aine['tunnus']}";
				pupe_query($query);
			}

		}
		else {
			$errors[] = "Syötetty määrä ei kelpaa";
		} // End of splittailut

	} // End of valmisteet
}

// VIEW //
echo "<font class='head'>" . t("Valmistusten jakaminen") . "</font>";
echo "<hr>";

// Virheet
echo "<font class='error'>";
foreach ($errors as $error) {
	echo t($error);
}
echo "</font>";

// Haetaan kaikki valmistukset
$valmistukset = Valmistus::all();

// Valmistukset
foreach($valmistukset as $valmistus) {
	echo "<table>";
	echo "<tr>
			<th>Valmistus</th>
			<th>Nimitys</th>
			<th>Varattu</th>
			<th>Valmistettu</th>
		</tr>";
	echo "<tr><td>" . $valmistus->tunnus() . "</td></tr>";

	echo "<form method='POST'>";
	echo "<input type='hidden' name='tee' value='jaa_valmistus'>";
	echo "<input type='hidden' name='valmistus' value='" . $valmistus->tunnus() . "'>";

	// Valmistuksen valmisteet
	foreach ($valmistus->tuotteet() as $valmiste) {
		echo "<tr>";
		echo "<td>" . $valmistus->tunnus() . "</td>";
		echo "<td>" . $valmiste['nimitys'] . "</td>";
		echo "<td>" . $valmiste['varattu'] . $valmiste['yksikko'] . "</td>";
		echo "<td><input type='text' size='8' name='jaettavat_valmisteet[{$valmiste['tunnus']}]' value='" . $valmiste['varattu'] . "'></td>";
		echo "</tr>";
	}
	echo "<tr><td><input type='submit' value='Jaa'></td></tr>";
	echo "</form>";
	echo "</table>";
}