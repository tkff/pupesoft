<?php

require 'inc/parametrit.inc';
require 'valmistuslinjat.inc';
require 'valmistus.class.php';

$tee = (isset($tee)) ? $tee : '';

// Jaetaan valmistus ja sen valmisteet ja niiden raaka-aineet
if ($tee == "jaa_valmistus") {
	// Jaetaan valmistus
	$errors = jaa_valmistus($valmistus, $jaettavat_valmisteet);
}

// VIEW //
echo "<font class='head'>" . t("Valmistusten jakaminen") . "</font>";
echo "<hr>";

// Haetaan kaikki valmistukset
$valmistukset = Valmistus::all();

// Loopataan valmistukset läpi
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

	// Loopataan valmistuksen valmisteet
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

// Virheilmoitukset
if (!empty($errors))	{
	echo "<font class='error'>";
	echo t($errors);
	echo "</font>";
}