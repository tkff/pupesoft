<?php

require "inc/connect.inc";
require "inc/functions.inc";
require "valmistuslinjat.inc";

// Haetaan yhtiö
$yhtio = hae_yhtion_parametrit($_GET['yhtio']);
$kukarow['yhtio'] = $yhtio['yhtio'];

// Haetaan valmistuslinjat
$query = "SELECT selite as id, selitetark as name
			FROM avainsana
			WHERE yhtio='{$kukarow['yhtio']}'
			AND laji='VALMISTUSLINJA'
			ORDER BY selite";
$result = pupe_query($query);

$valmistuslinjat = array();
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
* GET /valmistuslinjat/valmistukset
* Haetaan kaikki valmistuslinjoille laitetut valmistukset
*/
if (isset($_GET['valmistukset']) and $_GET['valmistukset'] == 'true') {

	// Kaikki valmistuslinjan tapahtumat
	$all_events = array();

	// Haetaan yhtiökohtaiset merkinnät
	$query = "SELECT kalenteri.pvmalku,
					kalenteri.pvmloppu ,
					kalenteri.kuka,
					kalenteri.henkilo,
					kalenteri.tyyppi
				FROM kalenteri
				WHERE yhtio='{$kukarow['yhtio']}'
				AND tyyppi='PY'";
	$result = pupe_query($query);

	$muut_merkinnat = array();
	// Lomat ja muut yhtiökohtaiset merkinnät
	while ($pyha = mysql_fetch_assoc($result)) {
		$muut_merkinnat[] = $pyha;
	}

	// Loopataan valmistuslinjat yksi kerrallaan
	foreach($valmistuslinjat as $linja) {

		foreach($muut_merkinnat as $merkinta) {
			$json = array();
			$json['title'] 	= utf8_encode("Pyhä");
			$json['start'] 	= $merkinta['pvmalku'];
			$json['end'] 		= $merkinta['pvmloppu'];
			$json['allDay'] 	= false;
			$json['resource'] = $linja['id'];
			$json['color'] 	= '#666';
			$all_events[] 	= $json;
		}

		// Valmistuslinjalla olevat valmistukset
		$valmistukset = hae_valmistuslinjan_valmistukset($linja);
		foreach($valmistukset as $valmistus) {
			#echo "valmistus: $valmistus[otunnus] $valmistus[pvmalku] $valmistus[pvmloppu]<br>";

			$json = array();

			$json['start'] = $valmistus['pvmalku'];
			$json['end'] = $valmistus['pvmloppu'];
			$json['kesto'] = valmistuksen_kesto($valmistus);

			$title = '';

			// Valmistuksella olevat tuotteet
			$tuotteet = hae_valmistuksen_tuotteet($valmistus);
			foreach($tuotteet as $tuote) {
				#echo "tuote: $tuote[nimitys] $tuote[tuoteno] $tuote[varattu] $tuote[yksikko]<br>";
				$title .= "$tuote[nimitys] $tuote[varattu] $tuote[yksikko]\n";
			}

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

			$json['title'] = utf8_encode($title);
			$json['allDay'] = false;
			$json['tunnus'] = $valmistus['otunnus'];
			$json['resource'] = $linja['id'];
			$json['tila'] = $valmistus['valmistuksen_tila'];
			$json['tyyppi'] = $valmistus['tyyppi'];

			$all_events[] = $json;
		}
	}

	// Vastaus
	header('Content-type: application/json');
	echo json_encode($all_events);
}