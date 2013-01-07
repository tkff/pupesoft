<?php

/**
* Valmistus
*/
class Valmistus {

	// Pakolliset kent�t
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

	// Valmistuksen tilat
	const ODOTTAA 				= 'OV';
	const VALMISTUKSESSA 		= 'VA';
	const KESKEYTETTY 			= 'TK';
	const VALMIS_TARKASTUKSEEN 	= 'VT';
	const TARKASTETTU 			= 'TA';

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

	function raaka_aineet() {
		global $kukarow;

		$query = "SELECT * FROM tilausrivi WHERE yhtio='{$kukarow['yhtio']}' AND otunnus=$this->tunnus";
		$result = pupe_query($query);

		$raaka_aineet = array();
		while ($row = mysql_fetch_assoc($result)) {
			$raaka_aineet[] = $row;
		}

		return $raaka_aineet;
	}

	/** etsit��n valmistuksen puutteet
	*/
	function puutteet() {
		global $kukarow;

		$aloitus_pvm = $this->alkupvm();

		// Haetaan raaka-aineet
		$query = "SELECT *
		            FROM tilausrivi
		            WHERE yhtio='{$kukarow['yhtio']}'
		            AND otunnus='{$this->tunnus}'
		            AND tuoteno!='TY�'
		            AND tyyppi='V'";
		$result = pupe_query($query);

		$puutteet = array();

		// Tarkistetaan kaikkien raaka-aineiden saldot
		while ($raaka_aine = mysql_fetch_assoc($result)) {
			$saldo = array();
			list($saldo['saldo'], $saldo['hyllyssa'], $saldo['myytavissa']) = saldo_myytavissa($raaka_aine['tuoteno']);

			#echo "tuoteno: $raaka_aine[tuoteno] saldo: $saldo[saldo] hyllyssa: $saldo[hyllyssa] myytavissa: $saldo[myytavissa]<br>";

			// Varatut kappaleet valmistuksilta jotka ovat jo valmistuslinjalla.
			// Valmistuslinjalla olevat valmistukset varaavat saldoa ja uuden valmistuksen on
			// tarkistettava paljon ne v�hent�v�t raaka-aineiden saldoa.
			$muut_query = "SELECT tilausrivi.otunnus, COALESCE(sum(tilausrivi.varattu), 0) AS varattu
			            FROM kalenteri
			                  JOIN lasku ON (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
			                  JOIN tilausrivi ON (lasku.yhtio=tilausrivi.yhtio AND lasku.tunnus=tilausrivi.otunnus)
			            WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
			                  AND kalenteri.tyyppi='valmistus'
			                  AND tilausrivi.tyyppi='V'
			                  AND tilausrivi.tuoteno='{$raaka_aine['tuoteno']}'
			                  AND kalenteri.pvmalku < '$aloitus_pvm'";
			$muut_valmistukset_result = pupe_query($muut_query);
			$muut_valmistukset = mysql_fetch_assoc($muut_valmistukset_result);
			#echo "muut: $muut_valmistukset[varattu] ($muut_valmistukset[otunnus])<br>";


			// Haetaan raaka-aineen ostotilauksia, jotka vaikuttavat valmistuksen aloitukseen
			$query = "SELECT COALESCE(sum(varattu), 0) AS varattu
			            FROM tilausrivi
			            WHERE yhtio='{$kukarow['yhtio']}'
			            AND tuoteno='{$raaka_aine['tuoteno']}'
			            AND tyyppi='O'
			            AND kerattyaika != '0000-00-00 00:00:00'
			            AND kerattyaika < '$aloitus_pvm'";
			$ostotilaukset_result = pupe_query($query);
			$ostotilaukset = mysql_fetch_assoc($ostotilaukset_result);

			#echo "ostot: $ostotilaukset[varattu]<br>";

			$_saldo = $saldo['myytavissa'] - $muut_valmistukset['varattu'] + $ostotilaukset['varattu'];

			if ($_saldo <= 0) {
				$puutteet[$raaka_aine['tuoteno']] = $_saldo;
			}
		}
		return $puutteet;
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
			// Etit��n edellinen valmistus
			# return new Valmistus($tunnus);
		}
	}

	function seuraava() {
		if ($this->valmistuslinja != '') {
			// Etit��n seuraava valmistus
			# return new Valmistus($tunnus);
		}
	}

	function getTila() {
		return $this->tila;
	}

	/** P�ivitt�� valmistuksen tilan
	*/
	function setTila($tila) {
		global $kukarow;

		/** Sallitut tilat ja niiden mahdolliset vaihtoehdot*/
		$states = array(
			Valmistus::ODOTTAA 				=> array(Valmistus::VALMISTUKSESSA, Valmistus::VALMIS_TARKASTUKSEEN, Valmistus::KESKEYTETTY, Valmistus::ODOTTAA),
			Valmistus::VALMISTUKSESSA 		=> array(Valmistus::KESKEYTETTY, Valmistus::VALMIS_TARKASTUKSEEN),
			Valmistus::KESKEYTETTY 			=> array(Valmistus::VALMISTUKSESSA),
			Valmistus::VALMIS_TARKASTUKSEEN => array(Valmistus::TARKASTETTU, Valmistus::ODOTTAA)
			);

		// Voidaanko uuteen tilaan vaihtaa,
		// eli l�ytyyk� nykyisen tilan vaihtoehdoista haluttu tila
		if (in_array($tila, $states[$this->tila])) {

			switch ($tila) {
				// Odottaa valmistusta
				case Valmistus::ODOTTAA:
					// Poistetaan valmistus kalenterista
					$query = "DELETE FROM kalenteri WHERE yhtio='{$kukarow['yhtio']}' AND otunnus={$this->tunnus}";
					if (! pupe_query($query)) {
						throw new Exception("Kalenteri merkint�� ei poistettu");
					}

					break;

				// Valmistukseen
				case Valmistus::VALMISTUKSESSA:
					// Valmistuslinjalla voi olla vain yksi valmistus VALMISTUKSESSA tilassa kerrallaan
					$query = "SELECT kalenteri.kuka, otunnus, valmistuksen_tila
								FROM kalenteri
								JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
								WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
								AND kalenteri.henkilo='{$this->valmistuslinja}'
								AND valmistuksen_tila = 'VA'";
					$result = pupe_query($query);

					// Jos keskener�inen valmistus l�ytyy
					if (mysql_num_rows($result) > 0) {
						throw new Exception("Valmistuslinjalla on keskener�inen valmistus");
					}

					// Py�ristet��n aloitusaika
					$pvmalku = round_time(strtotime('now'));
					$kesto = valmistuksen_kesto(array('tunnus' => $this->tunnus));
					echo "kesto: $kesto<br>";
					$pvmloppu = laske_loppuaika($pvmalku, $kesto*60, $this->valmistuslinja);

					// P�iv�m��r�t oikeaan muotoon
					$pvmalku = date('Y-m-d H:i:s', $pvmalku);
					$pvmloppu = date('Y-m-d H:i:s', $pvmloppu);

					// P�ivitet��n valmistuksen uudet ajat
					$query = "UPDATE kalenteri
								SET pvmalku='{$pvmalku}', pvmloppu='{$pvmloppu}'
								WHERE yhtio='{$kukarow['yhtio']}'
								AND otunnus='{$this->tunnus}'";
					if (! pupe_query($query)) {
						throw new Exception("Valmistuksen aikoja ei p�ivitetty");
					}

					// P�ivitet��n lasku.alatila='C' # Ker�tty!

					echo "valmis";
					break;

				// Valmistus keskeytetty
				case Valmistus::KESKEYTETTY:
					#throw new Exception("Tyhj�");
					break;

				// Valmis tarkastukseen
				case Valmistus::VALMIS_TARKASTUKSEEN:
					//throw new Exception("Tyhj�");
					echo "valmistus valmis tarkastukseen";

					break;

				// Muut
				default:
					throw new Exception("Tuntematon tila");
					break;
			}

			// Jos kaikki on ok, p�ivitet��n valmistuksen tila
			$query = "UPDATE lasku
						SET valmistuksen_tila='$tila'
						WHERE yhtio='{$kukarow['yhtio']}'
						AND tunnus=$this->tunnus";
			$result = pupe_query($query);
		}
		else {
			throw new Exception("Ei voida muuttaa tilasta '$this->tila' tilaan '$tila'");
		}
	}

	/** Hakee kaikki valmistukset */
	static function all() {
		global $kukarow;

		// Hakee kaikki keskener�iset valmistukset (lasku/kalenteri)
		// Valmistukset jotka ovat tilassa kesken tai tulostusjonossa
		$query = "SELECT
						lasku.yhtio,
						lasku.tunnus,
						lasku.valmistuksen_tila as tila,
						kalenteri.henkilo as valmistuslinja
					FROM lasku
					LEFT JOIN kalenteri ON (lasku.yhtio=kalenteri.yhtio AND lasku.tunnus=kalenteri.otunnus)
					WHERE lasku.yhtio='{$kukarow['yhtio']}'
					AND lasku.tila='V'
					AND lasku.alatila in ('', 'J')";
		$result = pupe_query($query);

		$valmistukset = array();
		while($valmistus = mysql_fetch_object($result, 'valmistus')) {
			$valmistukset[] = $valmistus;
		}

		return $valmistukset;
	}

	/** Hakee yksitt�isen valmistuksen */
	static function find($tunnus) {
		global $kukarow;

		$query = "SELECT
						lasku.yhtio,
						lasku.nimi,
						lasku.ytunnus,
						lasku.tunnus,
						lasku.valmistuksen_tila as tila,
						kalenteri.henkilo as valmistuslinja,
						kalenteri.kentta01 as ylityotunnit,
						kalenteri.kentta02 as kommentti
					FROM lasku
					LEFT JOIN kalenteri on (lasku.yhtio=kalenteri.yhtio AND lasku.tunnus=kalenteri.otunnus)
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

	/** hakee valmistukset tilan mukaan */
	static function find_by_tila($tila) {
		global $kukarow;

		$query = "SELECT
						lasku.valmistuksen_tila as tila,
						lasku.yhtio,
						lasku.tunnus,
						kalenteri.kentta01 as ylityotunnit,
						kalenteri.kentta02 as kommentti
					FROM lasku
					LEFT JOIN kalenteri ON (lasku.yhtio=kalenteri.yhtio AND lasku.tunnus=kalenteri.otunnus)
					WHERE lasku.yhtio='{$kukarow['yhtio']}'
					AND lasku.tila='V'
					AND lasku.valmistuksen_tila='VT'";

		$result = pupe_query($query);

		$valmistukset = array();
		while ($valmistus = mysql_fetch_object($result, 'valmistus')) {
			$valmistukset[] = $valmistus;
		}

		return $valmistukset;
	}
}

