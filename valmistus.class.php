<?php

/**
* Valmistus
*/
class Valmistus {

	// Pakolliset kentät
	private $yhtio;
	private $tunnus;

	// Valmistus voi olla valmistuslinjalla
	private $valmistuslinja;

	// Valmistuksen tila
	private $tila;

	// Valmistuksen tuotteet
	private $tuotteet = array();

	// Valmistuksen kesto, tuotteiden kestot summattuna
	private $kesto;


	function __construct() {}

	function valmistuslinja() {
		return $this->valmistuslinja;
	}

	/** Hakee valmistuksella olevat tuotteet, eli tilausrivit joiden tyyppi='W'
	*/
	function tuotteet() {

		if (empty($this->tuotteet)) {
			$query = "SELECT *
						FROM tilausrivi
						WHERE yhtio='$this->yhtio'
						AND otunnus=$this->tunnus
						AND tyyppi IN ('W', 'M')";
			$result = pupe_query($query);

			while($tuote = mysql_fetch_assoc($result)) {
				$this->tuotteet[] = $tuote;
			}

		}

		// Palautetaan tuotteet array
		return $this->tuotteet;
	}

	/** etsitään valmistuksen puutteet
	*/
	function puutteet() {
		return "Puutteet";
	}

	/** Valimstuksen alkupvm */
	function alkupvm() {
		$query = "SELECT pvmalku FROM kalenteri WHERE yhtio='$this->yhtio' AND otunnus=$this->tunnus";
		$result = pupe_query($query);
		$valmistus = mysql_fetch_assoc($result);

		return $valmistus['pvmalku'];
	}

	/** Valmistuksen loppupvm */
	function loppupvm() {
		$query = "SELECT pvmloppu FROM kalenteri WHERE yhtio='$this->yhtio' AND otunnus=$this->tunnus";
		$result = pupe_query($query);
		$valmistus = mysql_fetch_assoc($result);

		return $valmistus['pvmloppu'];
	}

	/** Hakee valmistuksen keston */
	function kesto() {
		if (empty($this->kesto)) {
			$query = "SELECT sum(varattu) as kesto
						FROM tilausrivi
						WHERE yhtio='$this->yhtio'
						AND otunnus=$this->tunnus
						AND yksikko='H'";
			$result = pupe_query($query);

			$valmistus = mysql_fetch_assoc($result);
			$this->kesto = $valmistus['kesto'];
		}

		return $this->kesto;
	}

	function tunnus() {
		return $this->tunnus;
	}

	###
	function edellinen() {
		if ($this->valmistuslinja != '') {
			// Etitään edellinen valmistus
			# return new Valmistus($tunnus);
		}
	}

	function seuraava() {
		if ($this->valmistuslinja != '') {
			// Etitään seuraava valmistus
			# return new Valmistus($tunnus);
		}
	}

	/** Päivittää valmistuksen tilan
	*/
	function tila($tila) {
		global $kukarow;

		/** Sallitut tilat ja niiden mahdolliset vaihtoehdot
		* OV => Odottaa valmistusta,
		* VA => Valmistuksessa,
		* TK => Työ keskeytetty,
		* VT => Valmis tarkastukseen,
		* TA => Tarkistettu
		*/
		$states = array(
			'OV' => array('VA'),
			'VA' => array('TK', 'VT'),
			'TK' => array('VA'),
			'VT' => array('TA', 'OV')
			);

		// Voidaanko uuteen tilaan vaihtaa,
		// eli löytyykö haluttu tila nykyisen tilan vaihtoehdoista.
		if (in_array($tila, $states[$this->tila]))  {

			// TODO: tarkistetaan mikä tila on nyt jne...
			$query = "UPDATE lasku SET valmistuksen_tila='$tila' WHERE yhtio='{$kukarow['yhtio']}' AND tunnus=$this->tunnus";
			echo $query;
			#$result = pupe_query($query);
		}
		else {
			exit("Laiton tilan muutos");
		}
	}

	/** Hakee kaikki valmistukset */
	static function all() {
		global $kukarow;
		$query = "SELECT
						lasku.yhtio,
						lasku.tunnus,
						lasku.valmistuksen_tila as tila,
						kalenteri.henkilo as valmistuslinja
					FROM lasku
					LEFT JOIN kalenteri ON (lasku.yhtio=kalenteri.yhtio AND lasku.tunnus=kalenteri.otunnus)
					WHERE lasku.yhtio='{$kukarow['yhtio']}'
					AND lasku.tila='V'
					AND lasku.alatila=''";
		$result = pupe_query($query);

		$valmistukset = array();
		while($valmistus = mysql_fetch_object($result, 'valmistus')) {
			$valmistukset[] = $valmistus;
		}

		return $valmistukset;
	}

	/** Hakee yksittäisen valmistuksen */
	static function find($tunnus) {
		global $kukarow;

		$query = "SELECT
						lasku.yhtio,
						lasku.tunnus,
						lasku.valmistuksen_tila as tila,
						kalenteri.henkilo as valmistuslinja
					FROM lasku
					LEFT JOIn kalenteri on (lasku.yhtio=kalenteri.yhtio AND lasku.tunnus=kalenteri.otunnus)
					WHERE lasku.yhtio='{$kukarow['yhtio']}'
					AND lasku.tunnus=$tunnus LIMIT 1";
		$result = pupe_query($query);

		if ($valmistus = mysql_fetch_object($result, 'valmistus')) {
			return $valmistus;
		}
		else {
			return false;
		}


	}
}

