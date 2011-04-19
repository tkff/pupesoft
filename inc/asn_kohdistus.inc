<?php

	if (!function_exists("asn_kohdistus")) {

		function asn_kohdistus() {

			global $yhtiorow, $kukarow;

			// M��ritell��n k�ytett�v�t muuttujat
			unset($edellinen_toimittaja);
			unset($edellinen_asn_numero);
			unset($edellinen_paketinnumero);
			$paketin_tunnukset = array();
			$paketin_rivit = array();
			$paketin_tuotteet = array();
			$virheet = 0;
			$virhe = "";

			// Haetaan kaikki k�sittelem�tt�m�t ASN-rivit
			$query = "	SELECT *
						FROM asn_sanomat
						WHERE yhtio = '$kukarow[yhtio]'
						AND status = ''
						ORDER BY toimittajanro, asn_numero, paketinnumero";
			$result = mysql_query($query) or pupe_error($sqltark);

			// Loopataan tuoterivit l�pi
			while ($row = mysql_fetch_assoc($result)) {

				// Toimittaja vaihtuu tai ASN-sanoma vaihtuu tai paketti vaihtuu ja ei ole virheit�, teid��n keikka ja suuntalava sek� kohdistetaan rivit
				if ((isset($edellinen_toimittaja) and $edellinen_toimittaja != $row["toimittajanro"]) or (isset($edellinen_asn_numero) and $edellinen_asn_numero != $row["asn_numero"]) or (isset($edellinen_paketinnumero) and $edellinen_paketinnumero != $row["paketinnumero"])) {
					if ($virheet == 0) {
						asn_kohdista_suuntalava($edellinen_toimittaja, $edellinen_asn_numero, $paketin_rivit, $paketin_tuotteet, $paketin_tunnukset);
					}
					else {
						$virheet = 0;
						echo "\nVIRHE: $virhe Paketti $edellinen_paketinnumero toimittajalla $edellinen_toimittaja ASN-sanoma $edellinen_asn_numero\n";
					}
					$paketin_rivit = array();
					$paketin_tunnukset = array();
					$paketin_tuotteet = array();
				}
				elseif ($virheet > 0) {
					// Meill� on jo t�m�n paketin sis�ll� virheit�, ei suotta chekkailla muita
					continue;
				}

				// Katsotaan, l�ytyyk� t�m� rivi ostotilaukselta
				$query = "	SELECT tilausrivi.*
							FROM tilausrivi
							JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio
											AND lasku.tunnus = tilausrivi.otunnus)
							JOIN toimi ON (toimi.yhtio = tilausrivi.yhtio
											AND toimi.tunnus = lasku.liitostunnus
											AND toimi.toimittajanro = '$row[toimittajanro]')
							JOIN tuotteen_toimittajat ON (tuotteen_toimittajat.yhtio = tilausrivi.yhtio
															AND tuotteen_toimittajat.tuoteno = tilausrivi.tuoteno
															AND tuotteen_toimittajat.toim_tuoteno = '$row[toimittajan_tuoteno]')
							WHERE tilausrivi.yhtio = '$kukarow[yhtio]'
							AND tilausrivi.tyyppi = 'O'
							AND tilausrivi.uusiotunnus = 0
							AND tilausrivi.otunnus = '$row[tilausnumero]'
							AND (tilausrivi.tunnus = '$row[tilausrivinpositio]' OR tilausrivi.tilaajanrivinro = '$row[tilausrivinpositio]')";
				$checkres = mysql_query($query) or pupe_error($query);
				$tilausrivirow = mysql_fetch_array($checkres);


				// Tarkistetaan kappaleet ja splitataan ostotilausrivi, jos kappaleita on asn-sanomalla V�HEMM�N kuin tilausrivill�, vanhan tilausrivin tunnus j�� j�ljell�
				if (mysql_num_rows($checkres) == 0 or $row["kappalemaara"] > $tilausrivirow["varattu"]) {
					// Tuli enemm�n, t�m� on virhe
					$virheet++;
					if (mysql_num_rows($checkres) == 0) {
						$virhe = "Ostotilausrivi� ei l�ydy!";
					}
					else {
						$virhe = "Toimittaja on toimittanut enemm�n tuotetta ($row[toimittajan_tuoteno]) kun on tilattu. Tilauksella $row[ostotilausnro] tilattu ($tilausrivirow[varattu]), toimitettu ($row[kappalemaara]).";
					}
				}
				elseif ($row["kappalemaara"] < $tilausrivirow["varattu"]) {
					// Splitataan tilausrivi, tallennetaan vanha rivitunnus tilausrivi.tilaajanrivinro kentt��n

					$kappaleerotus = $tilausrivirow["varattu"] - $row["kappalemaara"];

					// P�ivitet��n alkuper�iselle riville saapunut kappalem��r�
					$query = "	UPDATE tilausrivi SET
								varattu = '$row[kappalemaara]'
								WHERE yhtio = '$kukarow[yhtio]'
								AND tunnus = '$tilausrivirow[tunnus]'";
					$upres = mysql_query($query) or pupe_error($query);

					// Tehd��n uusi rivi, jossa on j�ljelle j��neet kappaleet
					$query = "	INSERT INTO tilausrivi SET
								yhtio = '$tilausrivirow[yhtio]',
								tyyppi = '$tilausrivirow[tyyppi]',
								toimaika = '$tilausrivirow[toimaika]',
								kerayspvm = '$tilausrivirow[kerayspvm]',
								otunnus = '$tilausrivirow[otunnus]',
								tuoteno = '$tilausrivirow[tuoteno]',
								try = '$tilausrivirow[try]',
								osasto = '$tilausrivirow[osasto]',
								nimitys = '$tilausrivirow[nimitys]',
								yksikko = '$tilausrivirow[yksikko]',
								varattu = '$kappaleerotus',
								hinta = '$tilausrivirow[hinta]',
								laatija = '$tilausrivirow[laatija]',
								laadittu = '$tilausrivirow[laadittu]',
								hyllyalue = '$tilausrivirow[hyllyalue]',
								hyllynro = '$tilausrivirow[hyllynro]',
								hyllytaso = '$tilausrivirow[hyllytaso]',
								hyllyvali = '$tilausrivirow[hyllyvali]',
								tilaajanrivinro = '$tilausrivirow[tunnus]'";
					$inskres = mysql_query($query) or pupe_error($query);
				}

				// Otetaan talteen t�m�n kierroksen tiedot
				$edellinen_toimittaja = $row["toimittajanro"];
				$edellinen_asn_numero = $row["asn_numero"];
				$edellinen_paketinnumero = $row["paketinnumero"];
				$paketin_tunnukset[] = $row["tunnus"];
				$paketin_rivit[] = $tilausrivirow["tunnus"];
				$paketin_tuotteet[] = $tilausrivirow["tuoteno"];
			}

			if ($virheet == 0 and count($paketin_rivit) > 0) {
				asn_kohdista_suuntalava($edellinen_toimittaja, $edellinen_asn_numero, $paketin_rivit, $paketin_tuotteet, $paketin_tunnukset);
			}
			elseif ($virheet != 0) {
				echo "\nVIRHE: $virhe\nVirheellinen paketti $edellinen_paketinnumero toimittajalla $edellinen_toimittaja ASN-sanoma $edellinen_asn_numero\n";
			}

		}

	}

	if (!function_exists("asn_kohdista_suuntalava")) {

		function asn_kohdista_suuntalava($edellinen_toimittaja, $edellinen_asn_numero, $paketin_rivit, $paketin_tuotteet, $paketin_tunnukset) {

			global $yhtiorow, $kukarow;

			// Haetaan toimittajan tiedot
			$toimhaku = "	SELECT *
							FROM toimi
							WHERE yhtio = '$kukarow[yhtio]'
							AND toimittajanro = '$edellinen_toimittaja'";
			$checkres = mysql_query($toimhaku) or pupe_error($toimhaku);

			if (mysql_num_rows($checkres) != 1) {
				echo "\nVIRHE: Toimittajanumero: $edellinen_toimittaja ei l�ydy j�rjestelm�st�!\n";
			}
			else {
				// Jos kaikki paketin rivit olivat kunnossa, tehd��n suuntalava ja linkataan ostotilausrivi keikkaan sek� suuntalavaan
				$toimittajarow = mysql_fetch_array($checkres);

				// Tehd��n keikka, jos sit� ei viel� ole. ASN-numero l�ytyy comments kent�st�.
				$query = "	SELECT lasku.tunnus
							FROM lasku
							WHERE yhtio = '$kukarow[yhtio]'
							AND tila = 'K'
							AND vanhatunnus = 0
							AND liitostunnus = '$toimittajarow[tunnus]'
							AND comments = '$edellinen_asn_numero'";
				$checkres = mysql_query($query) or pupe_error($query);

				// Tehd��n keikka
				if (mysql_num_rows($checkres) == 0) {
					$query = "	SELECT max(laskunro)
								FROM lasku
								WHERE yhtio = '$kukarow[yhtio]'
								AND tila = 'K'";
					$checkres = mysql_query($query) or pupe_error($query);
					$row = mysql_fetch_array($checkres);
					$id = $row[0] + 1;

					$query = "	SELECT kurssi
								FROM valuu
								WHERE yhtio = '$kukarow[yhtio]'
								AND nimi = '$toimittajarow[oletus_valkoodi]'";
					$checkres = mysql_query($query) or pupe_error($query);
					$row = mysql_fetch_array($checkres);
					$kurssi = $row["kurssi"];

					$maa_lahetys = $toimittajarow['maa_lahetys'] != '' ? $toimittajarow['maa_lahetys'] : $toimittajarow['maa'];

					// meill� on $toimittajarow haettuna ylh��ll�
					$query = "	INSERT into lasku set
								yhtio        			= '$kukarow[yhtio]',
								laskunro     			= '$id',
								ytunnus	     			= '$toimittajarow[ytunnus]',
								nimi         			= '$toimittajarow[nimi]',
								valkoodi     			= '$toimittajarow[oletus_valkoodi]',
								vienti       			= '$toimittajarow[oletus_vienti]',
								vienti_kurssi			= '$kurssi',
								toimitusehto 			= '$toimittajarow[toimitusehto]',
								osoite       			= '$toimittajarow[osoite]',
								postitp      			= '$toimittajarow[postitp]',
								maa			 			= '$toimittajarow[maa]',
								maa_lahetys 			= '$maa_lahetys',
								comments				= '$edellinen_asn_numero',
								kauppatapahtuman_luonne	= '$toimittajarow[kauppatapahtuman_luonne]',
								kuljetusmuoto			= '$toimittajarow[kuljetusmuoto]',
								rahti					= '$toimittajarow[oletus_kulupros]',
								swift					= '$toimittajarow[swift]',
								liitostunnus 			= '$toimittajarow[tunnus]',
								tila         			= 'K',
								luontiaika	 			= now(),
								laatija		 			= '$kukarow[kuka]'";
					$insertres = mysql_query($query) or pupe_error($query);
					$keikan_tunnus = mysql_insert_id();
				}
				else {
					$keikkarow = mysql_fetch_array($checkres);
					$keikan_tunnus = $keikkarow["tunnus"];
				}

				$query = "	SELECT tuote.keraysvyohyke, keraysvyohyke.terminaalialue, count(*)
							FROM tuote
							JOIN keraysvyohyke on (keraysvyohyke.yhtio = tuote.yhtio and keraysvyohyke.tunnus = tuote.keraysvyohyke)
							WHERE tuote.yhtio = '$kukarow[yhtio]'
							AND tuote.tuoteno IN (".implode(",", $paketin_tuotteet).")
							GROUP BY 1
							ORDER BY 3 DESC
							LIMIT 1";
				$checkres = mysql_query($query) or pupe_error($query);
				$row = mysql_fetch_array($checkres);

				// Tehd��n uusi suuntalava
				$tee = "lisaa";
				$otunnus = $keikan_tunnus;
				$sscc = $edellinen_asn_numero;
				$tyyppi = 10;
				$keraysvyohyke = $row['keraysvyohyke'];
				$kaytettavyys = "A";
				$terminaalialue = $row['terminaalialue'];
				$korkeus = 0;
				$paino = 0;
				$alkuhyllyalue = "";
				$alkuhyllynro = "";
				$alkuhyllyvali = "";
				$alkuhyllytaso = "";
				$loppuhyllyalue = "";
				$loppuhyllynro = "";
				$loppuhyllyvali = "";
				$loppuhyllytaso = "";
				$suuntalavat_ei_kayttoliittymaa = "KYLLA";

				require ("tilauskasittely/suuntalavat.inc");
				// Saadaan $uusi_suuntalavan_tunnus ^ incist�.
				// P�ivitet��n paketin ostotilausrivit kohdistetuksi keikkan ja oikealle suuntalavalle
				$query = "	UPDATE tilausrivi SET
							uusiotunnus = '$keikan_tunnus',
							suuntalava = '$uusi_suuntalavan_tunnus'
							WHERE yhtio = '$kukarow[yhtio]'
							AND tilausrivi.tunnus IN (".implode(",", $paketin_rivit).")";
				$updateres = mysql_query($query) or pupe_error($query);

				$query = "	UPDATE asn_sanomat SET
							status = 'X'
							WHERE yhtio = '$kukarow[yhtio]'
							AND tunnus IN (".implode(",", $paketin_tunnukset).")";
				$updateres = mysql_query($query) or pupe_error($query);
			}

		}
	}

?>