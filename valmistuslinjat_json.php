<?php

require "inc/connect.inc";
require "inc/functions.inc";
require "valmistuslinjat.inc";

// Haetaan yhtiö
$yhtio = hae_yhtion_parametrit($_GET['yhtio']);
$kukarow['yhtio'] = $yhtio['yhtio'];

/**
* Siirretään valmistusta
*/
if ($_POST['method'] == 'move') {

	// Haetaan siirrettävän tiedot
	$query = "SELECT *
				FROM kalenteri
				WHERE tunnus=$_POST[tunnus]";
	$result = pupe_query($query);
	$selected = mysql_fetch_assoc($result);

	// Kumpaan suuntaan siirretän
	if ($_POST['direction'] == 'left') {
		$direction = "pvmloppu <= '$selected[pvmalku]' ORDER BY pvmloppu desc";
	} else {
		$direction = "pvmalku >= '$selected[pvmloppu]' ORDER BY pvmalku asc";
	}

	// Etsitän siirrettävän viereinen valmistus
	$query = "SELECT *
				FROM kalenteri
				JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
				WHERE kalenteri.kuka='$selected[kuka]'
				AND $direction
				LIMIT 1";
	echo $query."\n";
	$result = pupe_query($query);
	$target = mysql_fetch_assoc($result);

	// Päivitetän paikkoja vain jos viereinen löytyy ja se ei ole valmistuksessa
	if (!empty($target) or $target['valmistuksen_tila'] != 'VA') {

		// Oikealle siirrettäessä siirretänkin oikeanpuoleista valmistusta vasemmalle
		// Sama logiikka toimii
		if ($_POST['direction'] == 'right') {
			$_temp = $selected;
			$selected = $target;
			$target = $_temp;
		}

		// Siirretän valitun alku oikeaan paikkaan
		$selected[pvmalku] = $target[pvmalku];

		// Lasketaan valitun uusi loppuaika
		$kesto = valmistuksen_kesto($selected[otunnus]);
		echo "Selected kesto: $kesto\n";
		$selected[pvmloppu] = laske_loppuaika(strtotime($selected[pvmalku]), $kesto * 60);

		// Siirretän kohde alkamaan valitun lopusta
		$target[pvmalku] = date('Y-m-d, H:i:s', $selected[pvmloppu]);
		$kesto = valmistuksen_kesto($target[otunnus]);
		echo "Target kesto: $kesto\n";
		$target[pvmloppu] = laske_loppuaika(strtotime($target[pvmalku]), $kesto * 60);

		echo "\n\n";
		echo "Selected: $selected[pvmalku] - ". date('Y-m-d H:i:s', $selected[pvmloppu])."\n";
		echo "Target $target[pvmalku] - ". date('Y-m-d H:i:s', $target[pvmloppu])."\n";

		// Päivitetän valittu
		$pvmalku = $selected[pvmalku];
		$pvmloppu = date('Y-m-d H:i:s', $selected[pvmloppu]);
		$query = "UPDATE kalenteri SET pvmalku='$pvmalku', pvmloppu='$pvmloppu' WHERE yhtio='{$kukarow['yhtio']}' and tunnus=$selected[tunnus]";
		pupe_query($query);
		echo $query."\n";

		// Päivitetän kohde
		$pvmalku = $target[pvmalku];
		$pvmloppu = date('Y-m-d H:i:s', $target[pvmloppu]);
		$query = "UPDATE kalenteri SET pvmalku='$pvmalku', pvmloppu='$pvmloppu' WHERE yhtio='{$kukarow['yhtio']}' and tunnus=$target[tunnus]";
		pupe_query($query);
		echo $query."\n";
	}
	echo "ei voi siirtä!";
}

/**
* Päivitetän kalenterin tila
*/
if ($_POST['method'] == 'update') {

	// TILAT
	$tilat = array(1 => 'VA', 2 => 'VT', 3 => 'TK');

	// Haetaan kalenterista valmistus
	$query = "SELECT * FROM kalenteri WHERE tunnus='{$_POST['tunnus']}'";
	$result = pupe_query($query);

	if ($valmistus = mysql_fetch_assoc($result)) {
		// Päivitetän valmistuksen tila
		$query = "UPDATE lasku SET valmistuksen_tila='OV' WHERE tunnus='{$valmistus['otunnus']}'";
		pupe_query($query);

		// Poistetaan kalenterista
		$query = "DELETE FROM kalenteri WHERE tunnus='{$_POST['tunnus']}'";
		pupe_query($query);
	}
}

$valmistuslinjat = array();

// Haetaan valmistuslinjat
$query = "SELECT selite as id, selitetark as name
			FROM avainsana
			WHERE yhtio='{$kukarow['yhtio']}'
			AND laji='VALMISTUSLINJA'
			ORDER BY selite";
$result = pupe_query($query);

while($linja = mysql_fetch_assoc($result)) {
	$valmistuslinjat[] = $linja;
}

/**
* GET /valmistuslinjat/resurssit
* Haetaan valmistuslinjat
*/
if (isset($_GET['resurssit']) and $_GET['resurssit'] == 'true') {

	// Rakennetaan valmistuslinjat JSON viestiksi
	header('Content-type: application/json');
	echo json_encode($valmistuslinjat);
}

/**
* GET /valmistuslinjat/events
* Haetaan kaikki valmistuslinjoille laitetut valmistukset
*/
if (isset($_GET['valmistukset']) and $_GET['valmistukset'] == 'true') {

	$events = array();

	// Haetaan yhtiökohtaiset merkinnät
	$query = "SELECT kalenteri.pvmalku as start,
					kalenteri.pvmloppu as end,
					kalenteri.kuka,
					kalenteri.henkilo as resource,
					kalenteri.tyyppi
				FROM kalenteri
				WHERE yhtio='{$kukarow['yhtio']}'
				AND tyyppi='PY'";
	$result = pupe_query($query);

	foreach($valmistuslinjat as $linja) {

		// Haetaan valmistuslinjan merkkinnät, poislukien yhtiökohtaiset merkinnät.
		// $query = "SELECT
		// 				kalenteri.pvmalku as start,
		// 				kalenteri.pvmloppu as end,
		// 				lasku.valmistuksen_tila as tila,
		// 				tilausrivi.nimitys as title,
		// 				tilausrivi.varattu,
		// 				tilausrivi.yksikko,
		// 				kalenteri.kuka,
		// 				kalenteri.henkilo as resource,
		// 				kalenteri.tunnus as tunnus,
		// 				kalenteri.otunnus,
		// 				kalenteri.tyyppi
		// 			FROM kalenteri
		// 				LEFT JOIN tilausrivi on (tilausrivi.yhtio=kalenteri.yhtio and tilausrivi.otunnus=kalenteri.otunnus)
		// 				LEFT JOIN lasku on (kalenteri.yhtio=lasku.yhtio and kalenteri.otunnus=lasku.tunnus)
		// 			WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
		// 				AND kalenteri.henkilo='{$linja['id']}'
		// 				AND kalenteri.tyyppi in ('valmistus', 'SA')
		// 				AND tilausrivi.tyyppi='W'
		// 			ORDER BY resource, pvmalku";
		#echo $query."<br>";

		$query ="SELECT
				  kalenteri.pvmalku AS start,
				  kalenteri.pvmloppu AS end,
				  lasku.valmistuksen_tila AS tila,
				  	tilausrivi.nimitys AS title,
					tilausrivi.varattu,
					tilausrivi.yksikko,
					kalenteri.kuka,
					if(kalenteri.henkilo=0, {$linja['id']}, kalenteri.henkilo) AS resource,
					kalenteri.tunnus AS tunnus,
					kalenteri.otunnus,
					kalenteri.tyyppi
				FROM kalenteri
				LEFT JOIN lasku ON (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
				LEFT JOIN tilausrivi ON (lasku.yhtio=tilausrivi.yhtio AND lasku.tunnus=tilausrivi.otunnus AND tilausrivi.tyyppi='W')
				WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
				  AND (kalenteri.henkilo='{$linja['id']}' or kalenteri.tyyppi='PY')";

		$result = pupe_query($query);

		// JSON-rakenne
		/*
		{
			'start': '2012-12-12 12:12',	# kalenteri.pvmalku
			'end': '2012-12-12 12:12',		# kalenteri.pvmloppu
			'tila': 'OV',					# lasku.valmistuksen_tila
			'title': 'Otsikko',				# tilausrivi.nimitys
			'varattu': '12',				# tilausrivi.varattu
			'yksikko': 'KPL'				# tilausrivi.yksikko
			'kuka': ?,						# ?
			'resource': '1',				# kalenteri.henkilo
			'tunnus': '12345',				# kalenteri.tunnus
			'allDay': false,				# ei kokopäivän eventtejä
			'color': '#F00',				# väri
			'kesto': '20'					# valmistuksen_kesto()
		}
		*/
		$muu_merkinta = array(
			'PY' => 'Pyhä',
			'SA' => 'Sairasloma'
			);

		$colors = array(
			'VA' => '#006',
			'PY' => '#555',
			'SA' => '#333'
			);

		// Valmistuslinjan tapahtumat
		while($event = mysql_fetch_assoc($result)) {
			$event['puutteet'] = puuttuvat_raaka_aineet($event['otunnus'], $event['start']);

			$event['title'] = ($event['title']) ? utf8_encode($event['title']) : utf8_encode($muu_merkinta[$event['tyyppi']]);

			$event['color'] = $colors[$event['tyyppi']];

			if ($event['tyyppi'] == 'valmistus') {
				$event['color'] = $colors[$event['tila']];
			}
			$event['allDay'] = false;
			$event['kesto'] = valmistuksen_kesto($event['otunnus']);

			$events[] = $event;
		}
	}

	header('Content-type: application/json');
	echo json_encode($events);
}