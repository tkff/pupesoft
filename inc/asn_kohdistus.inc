<?php

	if (!function_exists("asn_kohdistus")) {
		function asn_kohdistus() {
			global $yhtiorow, $kukarow;

			$php_cli = FALSE;

			if (php_sapi_name() == 'cli') {
				$php_cli = TRUE;
			}

			if (!$php_cli) {
				$d = "<br><br>";
			}
			else {
				$d = "\n\n";
			}

			// M��ritell��n k�ytett�v�t muuttujat
			unset($edellinen_toimittaja);
			unset($edellinen_asn_numero);
			unset($edellinen_paketinnumero);
			unset($edellinen_paketintunniste);

			$paketin_tunnukset 	= array();
			$paketin_rivit 		= array();
			$paketin_tuotteet 	= array();
			$virheet 			= 0;
			$virhe 				= "";

			// Haetaan kaikki k�sittelem�tt�m�t ASN-rivit
			$query = "	SELECT *
						FROM asn_sanomat
						WHERE yhtio = '{$kukarow['yhtio']}'
						AND status = ''
						AND tilausrivi = ''
						AND laji = 'asn'
						ORDER BY toimittajanumero, asn_numero, paketinnumero";
			$result = pupe_query($query);

			$kaytetyt_tilausrivit = array();

			// Loopataan tuoterivit l�pi
			while ($row = mysql_fetch_assoc($result)) {

				unset($loysin_lapsen);
				$loysin_lapsen = FALSE;

				if (count($kaytetyt_tilausrivit) == 0) {
					$tilausrivi_muuttuja = "";
				}
				else {
					$tilausrivi_muuttuja = "and tilausrivi.tunnus not in ('".implode("','", $kaytetyt_tilausrivit)."')";
				}

				// Toimittaja vaihtuu tai ASN-sanoma vaihtuu tai paketti vaihtuu ja ei ole virheit�, teid��n keikka ja suuntalava sek� kohdistetaan rivit
				if ((isset($edellinen_toimittaja) and $edellinen_toimittaja != $row["toimittajanumero"]) or (isset($edellinen_asn_numero) and $edellinen_asn_numero != $row["asn_numero"]) or (isset($edellinen_paketinnumero) and $edellinen_paketinnumero != $row["paketinnumero"])) {

					if (count($paketin_rivit) > 0) {

						unset($talteen_paketin_tiedot);
						$talteen_paketin_tiedot = array();
						$talteen_paketin_tiedot = $paketin_rivit;

						foreach ($paketin_rivit as $tilausrivitunnus) {

							// tehd��n tuoteperhe-satula-tapaus-checkki
							$lapsiperhesql = "	SELECT
												tuoteperhe.isatuoteno,
												tuoteperhe.tuoteno,
												tuote.tuoteno, tuote.try,tuote.osasto,
												tuote.nimitys,tuote.yksikko,tuote.myyntihinta,
												tilausrivi.yhtio, tilausrivi.tyyppi,tilausrivi.toimaika, tilausrivi.kerayspvm,
												tilausrivi.otunnus,tilausrivi.perheid,tilausrivi.tilkpl, tilausrivi.varattu,
												tilausrivi.tunnus,tuotepaikat.hyllyalue, tuotepaikat.hyllynro, tuotepaikat.hyllytaso, tuotepaikat.hyllyvali,
												tilausrivi.tilaajanrivinro
												FROM tilausrivi
												JOIN tuoteperhe ON (tuoteperhe.yhtio = tilausrivi.yhtio AND tilausrivi.tuoteno = tuoteperhe.isatuoteno AND tuoteperhe.tyyppi in ('P','')  AND tuoteperhe.ohita_kerays != '')
												JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuoteperhe.tuoteno)
												JOIN tuotepaikat ON (tuotepaikat.yhtio = tilausrivi.yhtio AND tuotepaikat.tuoteno = tuote.tuoteno AND tuotepaikat.oletus != '')
												WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
												AND tilausrivi.tunnus = '{$tilausrivitunnus}'
												ORDER BY tuoteperhe.tuoteno, tuoteperhe.tunnus";
							echo "<pre>",str_replace("\t", "", $lapsiperhesql),"</pre>";
							$lapsiresult = pupe_query($lapsiperhesql);

							while ($lapsitieto = mysql_fetch_assoc($lapsiresult)) {
								// hae tuotteen ostohinta
								$laskuselect	= "	SELECT * 
													FROM lasku 
													WHERE yhtio = '{$kukarow['yhtio']}' 
													AND tunnus = '{$lapsitieto['otunnus']}'";
								$laskures		= pupe_query($laskuselect);
								$laskurow		= mysql_fetch_assoc($laskures);

								list($hinta,,$ale,) = alehinta_osto($laskurow, $lapsitieto, $lapsitieto["tilkpl"]);

								$lisainsert = "	INSERT INTO tilausrivi SET
												yhtio			= '{$lapsitieto['yhtio']}',
												tyyppi			= '{$lapsitieto['tyyppi']}',
												toimaika		= '{$lapsitieto['toimaika']}',
												kerayspvm		= '{$lapsitieto['kerayspvm']}',
												otunnus			= '{$lapsitieto['otunnus']}',
												tuoteno			= '{$lapsitieto['tuoteno']}',
												try				= '{$lapsitieto['try']}',
												osasto			= '{$lapsitieto['osasto']}',
												nimitys			= '{$lapsitieto['nimitys']}',
												yksikko			= '{$lapsitieto['yksikko']}',
												tilkpl			= '{$lapsitieto['tilkpl']}',
												varattu			= '{$lapsitieto['varattu']}',
												ale1			= '{$ale['ale1']}',
												ale2			= '{$ale['ale2']}',
												ale3			= '{$ale['ale3']}',
												hinta			= '{$hinta}',
												laatija			= 'lapset',
												kommentti		= 'ASN-sanoma: TL:{$lapsitieto['tunnus']} tuotteelle: {$lapsitieto['isatuoteno']} lis�t��n lapsituote: {$lapsitieto['tuoteno']}',
												laadittu		=  now(),
												hyllyalue		= '{$lapsitieto['hyllyalue']}',
												hyllynro		= '{$lapsitieto['hyllynro']}',
												hyllytaso		= '{$lapsitieto['hyllytaso']}',
												hyllyvali		= '{$lapsitieto['hyllyvali']}',
												perheid			= '{$lapsitieto['tunnus']}',
												tilaajanrivinro = '{$lapsitieto['tilaajanrivinro']}'";
								echo "<pre>",str_replace("\t", "", $lisainsert),"</pre>";
								$inskres = pupe_query($lisainsert);
								$id = mysql_insert_id();
								$talteen_paketin_tiedot[] = $id;

								// p�ivitet��n is�
								$updateisa = "UPDATE tilausrivi SET perheid = tunnus WHERE yhtio = '{$kukarow['yhtio']}' AND tunnus = '{$tilausrivitunnus}'";
								$updateres = pupe_query($updateisa);

								$query = "	SELECT *
											FROM asn_sanomat
											WHERE yhtio = '{$kukarow['yhtio']}'
											AND laji = 'asn'
											AND tilausrivi LIKE '%{$tilausrivitunnus}%'";
								$info_res = pupe_query($query);
								$info_row = mysql_fetch_assoc($info_res);

								// Tehd��n uusi rivi, jossa on j�ljelle j��neet kappaleet
								$fields = "yhtio";
								$values = "'{$kukarow['yhtio']}'";

								// Ei monisteta tunnusta
								for ($ii = 1; $ii < mysql_num_fields($info_res) - 1; $ii++) {

									$fieldname = mysql_field_name($info_res,$ii);

									$fields .= ", ".$fieldname;

									switch ($fieldname) {
										case 'tilausrivi':
											$values .= ", '{$id}'";
											break;
										case 'tuoteno':
										case 'toim_tuoteno':
										case 'toim_tuoteno2':
											$values .= ", '{$lapsitieto['tuoteno']}'";
											break;
										case 'hinta':
											$values .= ", '{$hinta}'";
											break;
										default:
											$values .= ", '".$info_row[$fieldname]."'";
									}
								}

								$kysely  = "INSERT INTO asn_sanomat ({$fields}) VALUES ({$values})";
								echo "<pre>$kysely</pre>";
								$uusires = pupe_query($kysely);
							}
						}
						// unsetataan alkuper�inen paketin_rivit ja korvataan se talteen otetulla.
						unset($paketin_rivit);
						$paketin_rivit = array();
						$paketin_rivit = $talteen_paketin_tiedot;
					}

					if ($virheet == 0) {
						asn_kohdista_suuntalava($edellinen_toimittaja, $edellinen_asn_numero, $paketin_rivit, $paketin_tuotteet, $paketin_tunnukset, $edellinen_paketintunniste,'');
					}
					else {
						$virheet = 0;
						echo "VIRHE 1: Paketti hylatty. Toimittaja {$edellinen_toimittaja} ASN-sanoma {$edellinen_asn_numero} {$virhe} Paketti {$edellinen_paketinnumero} {$d}";
					}
					$paketin_rivit = array();
					$paketin_tunnukset = array();
					$paketin_tuotteet = array();
					$talteen_paketin_tiedot = array();
				}

				// Katsotaan, l�ytyyk� t�m� rivi ostotilaukselta
				// Siirtym�vaiheen j�lkeen poista lasku.comments tilausnumero ehto !!!!
				// poikkeusk�sittely� muutamalle toimittajalle.

				$toinen_tuoteno = "";

				if ($row["toimittajanumero"] == "123067") {
					$orgtuote = $row["toim_tuoteno"];
					$lyhennetty_tuoteno = substr($row["toim_tuoteno"], 0, -3);
					$jatkettu_tuoteno = $lyhennetty_tuoteno."090";

					if ($row["toim_tuoteno2"] != "") {
						$toinen_tuoteno = ",'{$row["toim_tuoteno2"]}'";
					}

					$poikkeus_tuoteno =" in ('{$orgtuote}','{$lyhennetty_tuoteno}','{$jatkettu_tuoteno}' {$toinen_tuoteno})";
				}
				elseif ($row["toimittajanumero"] == "123453") {
					$suba = substr($row["toim_tuoteno"],0,3);
					$subb = substr($row["toim_tuoteno"],3);
					$tuote = $suba."-".$subb;
					$yhteen = $row["toim_tuoteno"];

					if ($row["toim_tuoteno2"] != "") {
						$toinen_tuoteno = ",'{$row["toim_tuoteno2"]}'";
					}

					$poikkeus_tuoteno = " in ('{$tuote}','{$yhteen}' {$toinen_tuoteno}) ";
				}
				elseif ($row["toimittajanumero"] == "123178") {
					$orgtuote = $row["toim_tuoteno"];
					$lyhennetty = substr($row["toim_tuoteno"],3);

					if ($row["toim_tuoteno2"] != "") {
						$lyhennetty_toinen = substr($row["toim_tuoteno2"],3);
						$toinen_tuoteno = ",'{$row["toim_tuoteno2"]}','{$lyhennetty_toinen}'";
					}

					$poikkeus_tuoteno = " in ('{$orgtuote}','{$lyhennetty}' {$toinen_tuoteno}) ";
				}
				elseif ($row["toimittajanumero"] == "123084") {
					$orgtuote = $row["toim_tuoteno"];
					$lyhennetty = ltrim($row["toim_tuoteno"],'0');

					if ($row["toim_tuoteno2"] != "") {
						$lyhennetty_toinen = ltrim($row["toim_tuoteno2"],'0');
						$toinen_tuoteno = ",'{$row["toim_tuoteno2"]}','{$lyhennetty_toinen}'";
					}

					$poikkeus_tuoteno = " in ('{$orgtuote}','{$lyhennetty}' {$toinen_tuoteno}) ";
				}
				else {
					if ($row["toim_tuoteno2"] != "") {
						$toinen_tuoteno = ",'{$row["toim_tuoteno2"]}'";
					}

					$poikkeus_tuoteno = " in ('{$row['toim_tuoteno']}' {$toinen_tuoteno}) ";
				}

				// tiukka 1
				$query = "	SELECT tilausrivi.*
							FROM tilausrivi
							JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio
											AND lasku.tunnus = tilausrivi.otunnus)
							JOIN toimi ON (toimi.yhtio = tilausrivi.yhtio
											AND toimi.tunnus = lasku.liitostunnus
											AND toimi.toimittajanro = '{$row['toimittajanumero']}'
											AND toimi.tyyppi != 'P')
							JOIN tuotteen_toimittajat ON (tuotteen_toimittajat.yhtio = tilausrivi.yhtio
															AND tuotteen_toimittajat.liitostunnus = toimi.tunnus
															AND tuotteen_toimittajat.tuoteno = tilausrivi.tuoteno
															AND tuotteen_toimittajat.toim_tuoteno {$poikkeus_tuoteno})
							JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuotteen_toimittajat.tuoteno AND tuote.status != 'P')
							WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
							AND tilausrivi.tyyppi = 'O'
							AND tilausrivi.uusiotunnus = 0
							AND tilausrivi.otunnus LIKE '%{$row['tilausnumero']}'
							AND tilausrivi.tilaajanrivinro = '{$row['tilausrivinpositio']}'
							{$tilausrivi_muuttuja}
							ORDER BY laadittu asc";
				$kokeillaan = pupe_query($query);

				if (mysql_num_rows($kokeillaan) == 0) {
					// tiukka 2
					$query = "	SELECT tilausrivi.*
								FROM tilausrivi
								JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio
												AND lasku.tunnus = tilausrivi.otunnus)
								JOIN toimi ON (toimi.yhtio = tilausrivi.yhtio
												AND toimi.tunnus = lasku.liitostunnus
												AND toimi.toimittajanro = '{$row['toimittajanumero']}'
												AND toimi.tyyppi != 'P')
								JOIN tuotteen_toimittajat ON (tuotteen_toimittajat.yhtio = tilausrivi.yhtio
																AND tuotteen_toimittajat.liitostunnus = toimi.tunnus
																AND tuotteen_toimittajat.tuoteno = tilausrivi.tuoteno
																AND tuotteen_toimittajat.toim_tuoteno {$poikkeus_tuoteno})
								JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuotteen_toimittajat.tuoteno AND tuote.status != 'P')
								WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
								AND tilausrivi.tyyppi = 'O'
								AND tilausrivi.uusiotunnus = 0
								AND lasku.comments LIKE '%{$row['tilausnumero']}%'
								AND tilausrivi.tilaajanrivinro = '{$row['tilausrivinpositio']}'
								{$tilausrivi_muuttuja}
								ORDER BY laadittu ASC";
					$kokeillaan = pupe_query($query);
				}

				if (mysql_num_rows($kokeillaan) == 0) {
					// v�ljempi 1
					$query = "	SELECT tilausrivi.*
								FROM tilausrivi
								JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio
												AND lasku.tunnus = tilausrivi.otunnus)
								JOIN toimi ON (toimi.yhtio = tilausrivi.yhtio
												AND toimi.tunnus = lasku.liitostunnus
												AND toimi.toimittajanro = '{$row['toimittajanumero']}'
												AND toimi.tyyppi != 'P')
								JOIN tuotteen_toimittajat ON (tuotteen_toimittajat.yhtio = tilausrivi.yhtio
																AND tuotteen_toimittajat.liitostunnus = toimi.tunnus
																AND tuotteen_toimittajat.tuoteno = tilausrivi.tuoteno
																AND tuotteen_toimittajat.toim_tuoteno {$poikkeus_tuoteno})
								JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuotteen_toimittajat.tuoteno AND tuote.status != 'P')
								WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
								AND tilausrivi.tyyppi = 'O'
								AND tilausrivi.uusiotunnus = 0
								AND tilausrivi.otunnus LIKE '%{$row['tilausnumero']}'
								{$tilausrivi_muuttuja}
								ORDER BY laadittu ASC";
					$kokeillaan = pupe_query($query);
				}

				if (mysql_num_rows($kokeillaan) == 0) {
					// v�ljempi 2
					$query = "	SELECT tilausrivi.*
								FROM tilausrivi
								JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio
												AND lasku.tunnus = tilausrivi.otunnus)
								JOIN toimi ON (toimi.yhtio = tilausrivi.yhtio
												AND toimi.tunnus = lasku.liitostunnus
												AND toimi.toimittajanro = '{$row['toimittajanumero']}'
												AND toimi.tyyppi != 'P')
								JOIN tuotteen_toimittajat ON (tuotteen_toimittajat.yhtio = tilausrivi.yhtio
																AND tuotteen_toimittajat.liitostunnus = toimi.tunnus
																AND tuotteen_toimittajat.tuoteno = tilausrivi.tuoteno
																AND tuotteen_toimittajat.toim_tuoteno {$poikkeus_tuoteno})
								JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuotteen_toimittajat.tuoteno AND tuote.status != 'P')
								WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
								AND tilausrivi.tyyppi = 'O'
								AND tilausrivi.uusiotunnus = 0
								AND lasku.comments LIKE '%{$row['tilausnumero']}%'
								{$tilausrivi_muuttuja}
								ORDER BY laadittu ASC";
					$kokeillaan = pupe_query($query);

				}

				if (mysql_num_rows($kokeillaan) == 0) {
					// l�ys�
					$query = "	SELECT tilausrivi.*
								FROM tilausrivi
								JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio
												AND lasku.tunnus = tilausrivi.otunnus)
								JOIN toimi ON (toimi.yhtio = tilausrivi.yhtio
												AND toimi.tunnus = lasku.liitostunnus
												AND toimi.toimittajanro = '{$row['toimittajanumero']}'
												AND toimi.tyyppi != 'P')
								JOIN tuotteen_toimittajat ON (tuotteen_toimittajat.yhtio = tilausrivi.yhtio
																AND tuotteen_toimittajat.liitostunnus = toimi.tunnus
																AND tuotteen_toimittajat.tuoteno = tilausrivi.tuoteno
																AND tuotteen_toimittajat.toim_tuoteno {$poikkeus_tuoteno})
								JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuotteen_toimittajat.tuoteno AND tuote.status != 'P')
								WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
								AND tilausrivi.tyyppi = 'O'
								AND tilausrivi.uusiotunnus = 0
								{$tilausrivi_muuttuja}
								ORDER BY laadittu ASC";
				}

				$checkres = pupe_query($query);
				$tilausrivirow = mysql_fetch_assoc($checkres);

				// Tarkistetaan kappaleet ja splitataan ostotilausrivi, jos kappaleita on asn-sanomalla V�HEMM�N kuin tilausrivill�, vanhan tilausrivin tunnus j�� j�ljell�
				if (mysql_num_rows($checkres) == 0) {
					// muutos. Jos tulee enemm�n kuin ollaan tilattu, niin t�m� ei ole virhe, vaan lis�t��n se tilaukselle uudeksi riviksi.
					$virheet++;
					echo "VIRHE: Sopivaa ostotilausrivia ei loydy. Toimittaja {$row['toimittajanumero']} ASN-sanoma {$row['asn_numero']} Paketti {$row['paketinnumero']} Tuote {$row['toim_tuoteno']} {$d}";
				}
				elseif ($row["kappalemaara"] == $tilausrivirow["varattu"]) {
					// p�ivitet��n tilausrivin tunnus talteen asn_sanomat-tauluun, koska t�m� kohdistuu kyseiseen riviin
					$query = "	UPDATE asn_sanomat SET
								tilausrivi = '{$tilausrivirow['tunnus']}',
								tuoteno = '{$tilausrivirow['tuoteno']}'
								WHERE yhtio = '{$kukarow['yhtio']}'
								AND tunnus = '{$row['tunnus']}'";
					$upd_res = pupe_query($query);

					$kaytetyt_tilausrivit[] = $tilausrivirow['tunnus'];
					$paketin_rivit[] = $tilausrivirow["tunnus"];
				}
				elseif ($row["kappalemaara"] > $tilausrivirow["varattu"] and $tilausrivirow["varattu"] > 0) {
					// tuli enemm�n kuin tilattu
					$laskuri = 0;
					$kohdistamatta = $row["kappalemaara"];
					$lisa_array = array();
					$split_array = array();

					// Lasketaan ensin kaikki rivit yhteen ja sen j�lkeen katsotaan tarvitaanko lis�rivi�
					$olen_apu_result = pupe_query($query);

					while ($roll = mysql_fetch_assoc($olen_apu_result)) {

						if ($kohdistamatta <= 0) break;

						$laskuri += $roll["varattu"];
						$lisa_array[] = $roll["tunnus"];
						$split_array[] = $roll["tunnus"];
						$kohdistamatta -= $roll["varattu"];
					}

					$ylimaaraista = $row["kappalemaara"] - $laskuri; // liikaa miinus tilauksella

					// haetaan viel� varmuudeksi l�ysimm�ll� queryll� lis�rivej�.
					if ($ylimaaraista > 0) {

						// $tilausrivi_muuttuja sis�lt�� jo k�ytetyt rivit. Otetaan lisa_array:sta extraa viel�.
						$loysan_tilausrivi_muuttuja = "and tilausrivi.tunnus not in ('".implode("','", $kaytetyt_tilausrivit)."','".implode("','",$lisa_array)."')";

						$query = "	SELECT tilausrivi.*
									FROM tilausrivi
									JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio
													AND lasku.tunnus = tilausrivi.otunnus)
									JOIN toimi ON (toimi.yhtio = tilausrivi.yhtio
													AND toimi.tunnus = lasku.liitostunnus
													AND toimi.toimittajanro = '{$row['toimittajanumero']}'
													AND toimi.tyyppi != 'P')
									JOIN tuotteen_toimittajat ON (tuotteen_toimittajat.yhtio = tilausrivi.yhtio
																	AND tuotteen_toimittajat.liitostunnus = toimi.tunnus
																	AND tuotteen_toimittajat.tuoteno = tilausrivi.tuoteno
																	AND tuotteen_toimittajat.toim_tuoteno {$poikkeus_tuoteno})
									JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuotteen_toimittajat.tuoteno AND tuote.status != 'P')
									WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
									AND tilausrivi.tyyppi = 'O'
									AND tilausrivi.uusiotunnus = 0
									{$loysan_tilausrivi_muuttuja}
									ORDER BY laadittu ASC";
						$loysa_result = pupe_query($query);

						while ($loysatpois = mysql_fetch_assoc($loysa_result)) {

							if ($kohdistamatta <= 0) break;

							$laskuri		+= $loysatpois["varattu"];
							$lisa_array[]	 = $loysatpois["tunnus"];
							$split_array[]	 = $loysatpois["tunnus"];
							$kohdistamatta 	-= $loysatpois["varattu"];
						}

						$ylimaaraista = $row["kappalemaara"] - $laskuri; // liikaa miinus tilauksella

					}

					if ($ylimaaraista > 0) {
						// Tehd��n uusi rivi, jossa on ylim��r�iset kappaleet
						$lisaaquery = "	INSERT INTO tilausrivi SET
										yhtio					= '{$tilausrivirow['yhtio']}',
										tyyppi					= '{$tilausrivirow['tyyppi']}',
										toimaika				= '{$tilausrivirow['toimaika']}',
										kerayspvm				= '{$tilausrivirow['kerayspvm']}',
										otunnus					= '{$tilausrivirow['otunnus']}',
										tuoteno					= '{$tilausrivirow['tuoteno']}',
										try						= '{$tilausrivirow['try']}',
										osasto					= '{$tilausrivirow['osasto']}',
										nimitys					= '{$tilausrivirow['nimitys']}',
										yksikko					= '{$tilausrivirow['yksikko']}',
										tilkpl					= '{$ylimaaraista}',
										varattu					= '{$ylimaaraista}',
										hinta					= '{$tilausrivirow['hinta']}',
										erikoisale				= '{$tilausrivirow['erikoisale']}',
										erikoisale_saapuminen	= '{$tilausrivirow['erikoisale_saapuminen']}',
										ale1					= '{$tilausrivirow['ale1']}',
										ale2					= '{$tilausrivirow['ale2']}',
										ale3					= '{$tilausrivirow['ale3']}',
										laatija					= 'extraa',
										kommentti				= 'ASN-sanomalla 1: {$row['asn_numero']} tuli {$row['kappalemaara']} ja tilaukselta l�ytyi {$laskuri} Tehtiin tilausrivi {$ylimaaraista} m��r�lle',
										laadittu				=  now(),
										hyllyalue				= '{$tilausrivirow['hyllyalue']}',
										hyllynro				= '{$tilausrivirow['hyllynro']}',
										hyllytaso				= '{$tilausrivirow['hyllytaso']}',
										hyllyvali				= '{$tilausrivirow['hyllyvali']}',
										tilaajanrivinro 		= '{$tilausrivirow['tilaajanrivinro']}'";
						$inskres = pupe_query($lisaaquery);
						$extraa = mysql_insert_id();

						$kaytetyt_tilausrivit	= array_unique(array_merge($kaytetyt_tilausrivit, $lisa_array));
						$paketin_rivit			= array_unique(array_merge($paketin_rivit, $lisa_array));
						$siistitty 				= implode(",", $lisa_array);
						$lisa_array[]			= $extraa;

						// p�ivitet��n tilausrivin tunnus talteen asn_sanomat-tauluun, koska t�m� kohdistuu kyseiseen riviin
						$updatequery = "UPDATE asn_sanomat SET
										tilausrivi = '{$siistitty},{$extraa}',
										tuoteno = '{$tilausrivirow['tuoteno']}'
										WHERE yhtio = '{$kukarow['yhtio']}'
										AND tunnus = '{$row['tunnus']}'";
						$upd_res = pupe_query($updatequery);
						$kaytetyt_tilausrivit[] = $extraa;
						$paketin_rivit[]		= $extraa;

					}
					elseif ($ylimaaraista == 0) {
						// Jos menee tasan tilatut ja tulleet.
						$kaytetyt_tilausrivit	= array_unique(array_merge($kaytetyt_tilausrivit, $lisa_array));
						$paketin_rivit			= array_unique(array_merge($paketin_rivit, $lisa_array));
						$siistitty 				= implode(",", $lisa_array);

						// p�ivitet��n tilausrivin tunnus talteen asn_sanomat-tauluun, koska t�m� kohdistuu kyseiseen riviin
						$updatequery = "UPDATE asn_sanomat SET
										tilausrivi = '{$siistitty}',
										tuoteno = '{$tilausrivirow['tuoteno']}'
										WHERE yhtio = '{$kukarow['yhtio']}'
										AND tunnus = '{$row['tunnus']}'";
						$upd_res = pupe_query($updatequery);

					}
					elseif ($ylimaaraista < 0) {
						$positiivinen_arvo = abs($ylimaaraista);
						$vika_rivi = array_pop($split_array);
						// vika_rivi on se tilausrivi joka splitataan ett� m��r� t�sm��

						$split_query	= "SELECT * FROM tilausrivi WHERE yhtio = '{$kukarow['yhtio']}' AND tunnus = '{$vika_rivi}'";
						$split_result	= pupe_query($split_query);
						$split_row		= mysql_fetch_assoc($split_result);
						$erotus			= $split_row["tilkpl"] - $positiivinen_arvo;

						// P�ivitet��n alkuper�iselle riville "ylim��r�inen" ja uudelle splitille "erotus"
						$query = "	UPDATE tilausrivi SET
									varattu = '{$erotus}',
									tilkpl	= '{$erotus}',
									kommentti = CONCAT(kommentti,' Ennen splittausta {$split_row['tilkpl']} : kohdistettiin {$erotus}')
									WHERE yhtio = '{$kukarow['yhtio']}'
									AND tunnus = '{$split_row['tunnus']}'";
						$upres = pupe_query($query);

						$kaytetyt_tilausrivit	= array_unique(array_merge($kaytetyt_tilausrivit, $lisa_array));
						$paketin_rivit			= array_unique(array_merge($paketin_rivit, $lisa_array));
						$siistitty 				= implode(",", $lisa_array);

						// p�ivitet��n tilausrivin tunnus talteen asn_sanomat-tauluun, koska t�m� kohdistuu kyseiseen riviin
						$updatequery = "UPDATE asn_sanomat SET
										tilausrivi = '{$siistitty}',
										tuoteno = '{$tilausrivirow['tuoteno']}'
										WHERE yhtio = '{$kukarow['yhtio']}'
										AND tunnus = '{$row['tunnus']}'";
						$upd_res = pupe_query($updatequery);

						// Tehd��n uusi rivi, jossa on j�ljelle j��neet kappaleet
						$query = "	INSERT INTO tilausrivi SET
									yhtio					= '{$split_row['yhtio']}',
									tyyppi					= '{$split_row['tyyppi']}',
									toimaika				= '{$split_row['toimaika']}',
									kerayspvm				= '{$split_row['kerayspvm']}',
									otunnus					= '{$split_row['otunnus']}',
									tuoteno					= '{$split_row['tuoteno']}',
									try						= '{$split_row['try']}',
									osasto					= '{$split_row['osasto']}',
									nimitys					= '{$split_row['nimitys']}',
									yksikko					= '{$split_row['yksikko']}',
									tilkpl					= '{$positiivinen_arvo}',
									varattu					= '{$positiivinen_arvo}',
									hinta					= '{$split_row['hinta']}',
									erikoisale				= '{$split_row['erikoisale']}',
									erikoisale_saapuminen	= '{$split_row['erikoisale_saapuminen']}',
									ale1					= '{$split_row['ale1']}',
									ale2					= '{$split_row['ale2']}',
									ale3					= '{$split_row['ale3']}',
									laatija					= 'split',
									kommentti				= 'ASN-sanomalla 2: {$row['asn_numero']}. Tehtiin puuttuva tilausrivi m��r�lle {$positiivinen_arvo} tuotteelle {$split_row["tuoteno"]}',
									laadittu				=  now(),
									hyllyalue				= '{$split_row['hyllyalue']}',
									hyllynro				= '{$split_row['hyllynro']}',
									hyllytaso				= '{$split_row['hyllytaso']}',
									hyllyvali				= '{$split_row['hyllyvali']}',
									tilaajanrivinro 		= '{$split_row['tilaajanrivinro']}'";
						$inskres = pupe_query($query);
					}
				}
				elseif ($row["kappalemaara"] < $tilausrivirow["varattu"]) {
					// Splitataan tilausrivi, tallennetaan vanha rivitunnus tilausrivi.tilaajanrivinro kentt��n
					$kappaleerotus = $tilausrivirow["varattu"] - $row["kappalemaara"];

					// P�ivitet��n alkuper�iselle riville saapunut kappalem��r�
					$query = "	UPDATE tilausrivi SET
								varattu = '{$row['kappalemaara']}',
								tilkpl	= '{$row['kappalemaara']}',
								kommentti = CONCAT(kommentti,' ennen splittausta {$tilausrivirow['tilkpl']} : {$tilausrivirow['varattu']}, kohdistettiin {$row['kappalemaara']}')
								WHERE yhtio = '{$kukarow['yhtio']}'
								AND tunnus = '{$tilausrivirow['tunnus']}'";
					$upres = pupe_query($query);

					// p�ivitet��n tilausrivin tunnus talteen asn_sanomat-tauluun, koska t�m� kohdistuu kyseiseen riviin
					$query = "	UPDATE asn_sanomat SET
								tilausrivi = '{$tilausrivirow['tunnus']}',
								tuoteno = '{$tilausrivirow['tuoteno']}'
								WHERE yhtio = '{$kukarow['yhtio']}'
								AND tunnus = '{$row['tunnus']}'";
					$upd_res = pupe_query($query);
					$paketin_rivit[] = $tilausrivirow['tunnus'];

					$kaytetyt_tilausrivit[] = $tilausrivirow['tunnus'];

					// Tehd��n uusi rivi, jossa on j�ljelle j��neet kappaleet
					$query = "	INSERT INTO tilausrivi SET
								yhtio					= '{$tilausrivirow['yhtio']}',
								tyyppi					= '{$tilausrivirow['tyyppi']}',
								toimaika				= '{$tilausrivirow['toimaika']}',
								kerayspvm				= '{$tilausrivirow['kerayspvm']}',
								otunnus					= '{$tilausrivirow['otunnus']}',
								tuoteno					= '{$tilausrivirow['tuoteno']}',
								try						= '{$tilausrivirow['try']}',
								osasto					= '{$tilausrivirow['osasto']}',
								nimitys					= '{$tilausrivirow['nimitys']}',
								yksikko					= '{$tilausrivirow['yksikko']}',
								tilkpl					= '{$kappaleerotus}',
								varattu					= '{$kappaleerotus}',
								hinta					= '{$tilausrivirow['hinta']}',
								erikoisale				= '{$tilausrivirow['erikoisale']}',
								erikoisale_saapuminen	= '{$tilausrivirow['erikoisale_saapuminen']}',
								ale1					= '{$tilausrivirow['ale1']}',
								ale2					= '{$tilausrivirow['ale2']}',
								ale3					= '{$tilausrivirow['ale3']}',
								laatija					= 'split',
								kommentti				= 'ASN-sanomalla 3: {$row['asn_numero']} tuli {$row['kappalemaara']} ja tilaukselta l�ytyi {$tilausrivirow['tilkpl']} Tehtiin tilausrivi {$kappaleerotus} m��r�lle',
								laadittu				=  now(),
								hyllyalue				= '{$tilausrivirow['hyllyalue']}',
								hyllynro				= '{$tilausrivirow['hyllynro']}',
								hyllytaso				= '{$tilausrivirow['hyllytaso']}',
								hyllyvali				= '{$tilausrivirow['hyllyvali']}',
								tilaajanrivinro 		= '{$tilausrivirow['tilaajanrivinro']}'";
					$inskres = pupe_query($query);
				}

				// Otetaan talteen t�m�n kierroksen tiedot
				$edellinen_toimittaja = $row["toimittajanumero"];
				$edellinen_asn_numero = $row["asn_numero"];
				$edellinen_paketinnumero = $row["paketinnumero"];
				$edellinen_paketintunniste = $row["paketintunniste"];
				$paketin_tunnukset[] = $row["tunnus"];
				$paketin_tuotteet[] = $tilausrivirow["tuoteno"];
			}

			// Tehd��n ennen suuntalavakohdistusta lapsicheckit kun tullaan pois while-loopista.
			if (count($paketin_rivit) > 0) {

				unset($talteen_paketin_tiedot);
				$talteen_paketin_tiedot = array();
				$talteen_paketin_tiedot = $paketin_rivit;

				foreach ($paketin_rivit as $tilausrivitunnus) {

					// tehd��n tuoteperhe-satula-tapaus-checkki
					$lapsiperhesql = "	SELECT
										tuoteperhe.isatuoteno,
										tuoteperhe.tuoteno,
										tuote.tuoteno, tuote.try,tuote.osasto,
										tuote.nimitys,tuote.yksikko,tuote.myyntihinta,
										tilausrivi.yhtio, tilausrivi.tyyppi,tilausrivi.toimaika, tilausrivi.kerayspvm,
										tilausrivi.otunnus,tilausrivi.perheid,tilausrivi.tilkpl, tilausrivi.varattu,
										tilausrivi.tunnus,tuotepaikat.hyllyalue, tuotepaikat.hyllynro, tuotepaikat.hyllytaso, tuotepaikat.hyllyvali,
										tilausrivi.tilaajanrivinro
										FROM tilausrivi
										JOIN tuoteperhe ON (tuoteperhe.yhtio = tilausrivi.yhtio AND tilausrivi.tuoteno = tuoteperhe.isatuoteno AND tuoteperhe.tyyppi IN ('P','') AND tuoteperhe.ohita_kerays != '')
										JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio AND tuote.tuoteno = tuoteperhe.tuoteno)
										JOIN tuotepaikat ON (tuotepaikat.yhtio = tilausrivi.yhtio AND tuotepaikat.tuoteno = tuote.tuoteno AND tuotepaikat.oletus != '')
										WHERE tilausrivi.yhtio = '{$kukarow['yhtio']}'
										AND tilausrivi.tunnus = '{$tilausrivitunnus}'
										ORDER BY tuoteperhe.tuoteno, tuoteperhe.tunnus";
					echo "<pre>",str_replace("\t", "", $lapsiperhesql),"</pre>";
					$lapsiresult = pupe_query($lapsiperhesql);

					while ($lapsitieto = mysql_fetch_assoc($lapsiresult)) {

						// hae tuotteen ostohinta
						$laskuselect= "SELECT * FROM lasku WHERE yhtio = '{$kukarow['yhtio']}' AND tunnus = '{$lapsitieto['otunnus']}'";
						$laskures	= pupe_query($laskuselect);
						$laskurow	= mysql_fetch_assoc($laskures);

						list($hinta,,$ale,) = alehinta_osto($laskurow, $lapsitieto, $lapsitieto["tilkpl"]);

						$lisainsert = "	INSERT INTO tilausrivi SET
										yhtio			= '{$lapsitieto['yhtio']}',
										tyyppi			= '{$lapsitieto['tyyppi']}',
										toimaika		= '{$lapsitieto['toimaika']}',
										kerayspvm		= '{$lapsitieto['kerayspvm']}',
										otunnus			= '{$lapsitieto['otunnus']}',
										tuoteno			= '{$lapsitieto['tuoteno']}',
										try				= '{$lapsitieto['try']}',
										osasto			= '{$lapsitieto['osasto']}',
										nimitys			= '{$lapsitieto['nimitys']}',
										yksikko			= '{$lapsitieto['yksikko']}',
										tilkpl			= '{$lapsitieto['tilkpl']}',
										varattu			= '{$lapsitieto['varattu']}',
										ale1			= '{$ale['ale1']}',
										ale2			= '{$ale['ale2']}',
										ale3			= '{$ale['ale3']}',
										hinta			= '{$hinta}',
										laatija			= 'lapset',
										kommentti		= 'ASN-sanoma: TL:{$lapsitieto['tunnus']} tuotteelle: {$lapsitieto['isatuoteno']} lis�t��n lapsituote: {$lapsitieto['tuoteno']}',
										laadittu		=  now(),
										hyllyalue		= '{$lapsitieto['hyllyalue']}',
										hyllynro		= '{$lapsitieto['hyllynro']}',
										hyllytaso		= '{$lapsitieto['hyllytaso']}',
										hyllyvali		= '{$lapsitieto['hyllyvali']}',
										perheid			= '{$lapsitieto['tunnus']}',
										tilaajanrivinro = '{$lapsitieto['tilaajanrivinro']}'";
						echo "<pre>",str_replace("\t", "", $lisainsert),"</pre>";
						$inskres = pupe_query($lisainsert);
						$id = mysql_insert_id();
						$talteen_paketin_tiedot[] = $id;

						// p�ivitet��n is�
						$updateisa = "UPDATE tilausrivi SET perheid = tunnus WHERE yhtio = '{$kukarow['yhtio']}' AND tunnus = '{$tilausrivitunnus}'";
						$updateres = pupe_query($updateisa);

						$query = "	SELECT *
									FROM asn_sanomat
									WHERE yhtio = '{$kukarow['yhtio']}'
									AND laji = 'asn'
									AND tilausrivi LIKE '%{$tilausrivitunnus}%'";
						$info_res = pupe_query($query);
						$info_row = mysql_fetch_assoc($info_res);

						// Tehd��n uusi rivi, jossa on j�ljelle j��neet kappaleet
						$fields = "yhtio";
						$values = "'{$kukarow['yhtio']}'";

						// Ei monisteta tunnusta
						for ($ii = 1; $ii < mysql_num_fields($info_res) - 1; $ii++) {

							$fieldname = mysql_field_name($info_res,$ii);

							$fields .= ", ".$fieldname;

							switch ($fieldname) {
								case 'tilausrivi':
									$values .= ", '{$id}'";
									break;
								case 'tuoteno':
								case 'toim_tuoteno':
								case 'toim_tuoteno2':
									$values .= ", '{$lapsitieto['tuoteno']}'";
									break;
								case 'hinta':
									$values .= ", '{$hinta}'";
									break;
								default:
									$values .= ", '".$info_row[$fieldname]."'";
							}
						}

						$kysely  = "INSERT INTO asn_sanomat ({$fields}) VALUES ({$values})";
						echo "<pre>$kysely</pre>";
						$uusires = pupe_query($kysely);
					}
				}

				// unsetataan alkuper�inen paketin_rivit ja korvataan se talteen otetulla.
				unset($paketin_rivit);
				$paketin_rivit =  array();
				$paketin_rivit = $talteen_paketin_tiedot;
				$talteen_paketin_tiedot = array();

			}

			if ($virheet == 0 and count($paketin_rivit) > 0) {
				asn_kohdista_suuntalava($edellinen_toimittaja, $edellinen_asn_numero, $paketin_rivit, $paketin_tuotteet, $paketin_tunnukset, $edellinen_paketintunniste,'');
			}
			elseif ($virheet != 0) {
				echo "VIRHE 2: Paketti hylatty. Toimittaja $edellinen_toimittaja ASN-sanoma $edellinen_asn_numero $virhe Paketti $edellinen_paketinnumero $d";
			}
		}
	}

	if (!function_exists("asn_kohdista_suuntalava")) {
		function asn_kohdista_suuntalava($edellinen_toimittaja, $edellinen_asn_numero, $paketin_rivit, $paketin_tuotteet, $paketin_tunnukset, $edellinen_paketintunniste,$SSCC='') {
			global $yhtiorow, $kukarow;

			// Haetaan toimittajan tiedot
			$toimhaku = "	SELECT *
							FROM toimi
							WHERE yhtio = '{$kukarow['yhtio']}'
							AND toimittajanro = '{$edellinen_toimittaja}'
							AND tyyppi != 'P'";
			$checkres = pupe_query($toimhaku);

			if (mysql_num_rows($checkres) != 1) {
				echo "VIRHE: Toimittajanumero: {$edellinen_toimittaja} ei l�ydy jarjestelmasta!\n";
			}
			else {
				// Jos kaikki paketin rivit olivat kunnossa, tehd��n suuntalava ja linkataan ostotilausrivi saapumiseen sek� suuntalavaan
				$toimittajarow = mysql_fetch_array($checkres);

				// Tehd��n keikka, jos sit� ei viel� ole. ASN-numero l�ytyy comments kent�st�.
				$query = "	SELECT tunnus
							FROM lasku
							WHERE yhtio = '{$kukarow['yhtio']}'
							AND tila = 'K'
							AND vanhatunnus = 0
							AND liitostunnus = '{$toimittajarow['tunnus']}'
							AND comments = '{$edellinen_asn_numero}'";
				$checkres = pupe_query($query);

				// Tehd��n keikka
				if (mysql_num_rows($checkres) == 0) {

					$query = "	SELECT kurssi
								FROM valuu
								WHERE yhtio = '{$kukarow['yhtio']}'
								AND nimi = '{$toimittajarow['oletus_valkoodi']}'";
					$checkres = pupe_query($query);
					$row = mysql_fetch_array($checkres);

					$kurssi = $row["kurssi"];

					$query  = "LOCK TABLE lasku WRITE";
					$result = pupe_query($query);

					$query = "	SELECT MAX(laskunro)
								FROM lasku
								WHERE yhtio = '{$kukarow['yhtio']}'
								AND tila = 'K'";
					$checkres = pupe_query($query);
					$row = mysql_fetch_array($checkres);

					$id = $row[0] + 1;

					$maa_lahetys = $toimittajarow['maa_lahetys'] != '' ? $toimittajarow['maa_lahetys'] : $toimittajarow['maa'];

					// meill� on $toimittajarow haettuna ylh��ll�
					$query = "	INSERT INTO lasku SET
								yhtio        			= '{$kukarow['yhtio']}',
								laskunro     			= '{$id}',
								ytunnus	     			= '{$toimittajarow['ytunnus']}',
								nimi         			= '{$toimittajarow['nimi']}',
								valkoodi     			= '{$toimittajarow['oletus_valkoodi']}',
								vienti       			= '{$toimittajarow['oletus_vienti']}',
								vienti_kurssi			= '{$kurssi}',
								toimitusehto 			= '{$toimittajarow['toimitusehto']}',
								osoite       			= '{$toimittajarow['osoite']}',
								postitp      			= '{$toimittajarow['postitp']}',
								maa			 			= '{$toimittajarow['maa']}',
								maa_lahetys 			= '{$maa_lahetys}',
								maa_maara				= '{$yhtiorow['maa']}',
								comments				= '{$edellinen_asn_numero}',
								kauppatapahtuman_luonne	= '{$toimittajarow['kauppatapahtuman_luonne']}',
								kuljetusmuoto			= '{$toimittajarow['kuljetusmuoto']}',
								rahti					= '{$toimittajarow['oletus_kulupros']}',
								swift					= '{$toimittajarow['swift']}',
								liitostunnus 			= '{$toimittajarow['tunnus']}',
								tila         			= 'K',
								luontiaika	 			= now(),
								laatija		 			= '{$kukarow['kuka']}'";
					$insertres = pupe_query($query);
					$keikan_tunnus = mysql_insert_id();

					$query  = "UNLOCK TABLE";
					$result = pupe_query($query);
				}
				else {
					$keikkarow = mysql_fetch_array($checkres);
					$keikan_tunnus = $keikkarow["tunnus"];
				}

				$query = "	SELECT vh.keraysvyohyke, keraysvyohyke.terminaalialue, COUNT(*)
							FROM tilausrivi AS tp
							JOIN varaston_hyllypaikat AS vh ON (vh.yhtio = tp.yhtio AND vh.hyllyalue = tp.hyllyalue AND vh.hyllynro = tp.hyllynro AND vh.hyllyvali = tp.hyllyvali AND vh.hyllytaso = tp.hyllytaso)
							JOIN keraysvyohyke ON (keraysvyohyke.yhtio = tp.yhtio AND keraysvyohyke.tunnus = vh.keraysvyohyke)
							WHERE tp.yhtio = '{$kukarow['yhtio']}'
							AND tp.tunnus IN ('".implode("','", $paketin_rivit)."')
							GROUP BY 1,2
							ORDER BY 3 DESC
							LIMIT 1";
				$checkres = pupe_query($query);
				$row = mysql_fetch_array($checkres);

				// Tehd��n uusi suuntalava
				$tee 			= "lisaa";
				$otunnus 		= $keikan_tunnus;
				$sscc 			= $edellinen_paketintunniste;
				$tyyppi 		= 10;
				$keraysvyohyke 	= $row['keraysvyohyke'];
				$kaytettavyys 	= "Y";
				$terminaalialue = $row['terminaalialue'];
				$korkeus 		= 0;
				$paino 			= 0;
				$alkuhyllyalue 	= "";
				$alkuhyllynro 	= "";
				$alkuhyllyvali 	= "";
				$alkuhyllytaso 	= "";
				$loppuhyllyalue = "";
				$loppuhyllynro 	= "";
				$loppuhyllyvali = "";
				$loppuhyllytaso = "";
				$automaattinen_paivitys = "";
				$suuntalavat_ei_kayttoliittymaa = "KYLLA";

				require ("tilauskasittely/suuntalavat.inc");

				// Saadaan $uusi_suuntalavan_tunnus ^ incist�.
				// P�ivitet��n paketin ostotilausrivit kohdistetuksi keikkan ja oikealle suuntalavalle
				$query = "	UPDATE tilausrivi SET
							uusiotunnus = '{$keikan_tunnus}',
							suuntalava = '{$uusi_suuntalavan_tunnus}'
							WHERE yhtio = '{$kukarow['yhtio']}'
							AND tunnus IN (".implode(",", $paketin_rivit).")";
				echo "<pre>$query</pre>";
				$updateres = pupe_query($query);

				$query = "	UPDATE asn_sanomat SET
							status = 'X'
							WHERE yhtio = '{$kukarow['yhtio']}'
							AND tunnus IN (".implode(",", $paketin_tunnukset).")";
				echo "<pre>$query</pre>";
				$updateres = pupe_query($query);

				// Laitetaan suuntalava siirtovalmiiksi.
				$suuntalavan_tunnus = $uusi_suuntalavan_tunnus;
				$tee = "siirtovalmis";
				$suuntalavat_ei_kayttoliittymaa = "KYLLA";

				require ("tilauskasittely/suuntalavat.inc");
			}
		}
	}
