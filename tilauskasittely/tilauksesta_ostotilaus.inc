<?php

	// jos halutaan generoida ostotilaus t�m�n tilauksen riveist�
	// tarvitaan myyntitilauksen tunnus muuttujassa $otunnus

	if (!function_exists("tilauksesta_ostotilaus")) {
		function tilauksesta_ostotilaus($otunnus, $tyyppi) {
			global $yhtiorow, $kukarow, $toim;

			$tilauksesta_ostotilaus = "";
			$lisa_varattusiirto = "";

			if ($tyyppi == "T") {
				$vars = " and tilausrivi.var  = 'T' ";
			}
			elseif ($tyyppi == "U") {
				$vars = " and tilausrivi.var  = 'U' ";
			}
			elseif ($tyyppi == "KAIKKI") {
				$vars = " and tilausrivi.var != 'P' ";
			}
			else {
				echo t("VIRHE: Funktiota kutsuttiin v��r�ll� parametrilla")."!<br><br>";
				return false;
			}

			$query = "	SELECT *
						FROM lasku
						WHERE yhtio	= '$kukarow[yhtio]'
						and tunnus	= '$otunnus'";
			$result = pupe_query($query);
			$myytilrow = mysql_fetch_assoc($result);

			//otetaan ensin vain lis�varusteettomat tuotteet tai lis�varusteperheiden is�t
			$query = "	SELECT tilausrivin_lisatiedot.*, tilausrivi.*, tilausrivi.tunnus rivitunnus, tuote.ei_saldoa, tuote.sarjanumeroseuranta
						FROM tilausrivi
						LEFT JOIN tilausrivin_lisatiedot ON (tilausrivin_lisatiedot.yhtio = tilausrivi.yhtio and tilausrivin_lisatiedot.tilausrivitunnus = tilausrivi.tunnus)
						JOIN tuote ON (tuote.yhtio = tilausrivi.yhtio and tuote.tuoteno = tilausrivi.tuoteno)
						WHERE tilausrivi.yhtio = '$kukarow[yhtio]'
						and tilausrivi.otunnus = '$otunnus'
						and tilausrivi.tyyppi != 'D'
						$vars
						and (tilausrivi.perheid2 = tilausrivi.tunnus or tilausrivi.perheid2 = 0)
						ORDER BY tilausrivi.perheid2, tilausrivi.tunnus";
			$result = pupe_query($query);

			if (mysql_num_rows($result) > 0) {

				while ($rivi = mysql_fetch_assoc($result)) {

					if ($rivi['ei_saldoa'] != '' and $rivi['perheid'] == $rivi['rivitunnus']) {
						// saldottomat tuotteet laitetaan j�lkk�riin jos ne on tuoteperheen isi� ja skipataan loopissa
						$query = "	UPDATE tilausrivi
									SET var = 'J'
									WHERE yhtio = '$kukarow[yhtio]'
									AND tunnus = '$rivi[rivitunnus]'";
						$ei_saldoa_res = pupe_query($query);
						continue;
					}
					elseif ($rivi['ei_saldoa'] != '') {
						// saldottomat tuotteet, jotka ei ole perheiden isi�, skipataan kokonaan
						continue;
					}

					//	Jos tehd��n vaan t�m� tilaus niin haetaan t�m�n oletustoimittaja, by default
					if ($tyyppi == "KAIKKI" and $rivi["toimittajan_tunnus"] == 0) {
						$query = "	SELECT liitostunnus, if (jarjestys = 0, 9999, jarjestys) sorttaus
									FROM tuotteen_toimittajat
									WHERE yhtio = '$kukarow[yhtio]'
									and tuoteno = '$rivi[tuoteno]'
									ORDER BY sorttaus
									LIMIT 1";
						$abures = pupe_query($query);

						if (mysql_num_rows($abures) == 1) {
							$aburow = mysql_fetch_assoc($abures);

							$query = "	UPDATE tilausrivin_lisatiedot
										SET
										toimittajan_tunnus	= '$aburow[liitostunnus]',
										muutospvm			= now(),
										muuttaja			= '$kukarow[kuka]'
										WHERE yhtio		 	 = '$kukarow[yhtio]'
										and tilausrivitunnus = '$rivi[rivitunnus]'";
							$updres = pupe_query($query);

							$rivi["toimittajan_tunnus"] = $aburow["liitostunnus"];
						}
						else {
							$tilauksesta_ostotilaus .= "<font class='error'>".t("VIRHE: Toimittajaa ei l�ydy riville").": '$rivi[tunnus]'<br>";
						}
					}

					// tehd��n aluksi v�h�n oikeellisuustarkastuksia
					$query = "SELECT * from tuotteen_toimittajat where yhtio='$kukarow[yhtio]' and tuoteno='$rivi[tuoteno]' and liitostunnus='$rivi[toimittajan_tunnus]'";
					$erres = pupe_query($query);
					$ttrow = mysql_fetch_assoc($erres);

					// tehd��n aluksi v�h�n oikeellisuustarkastuksia
					$query = "SELECT * from toimi where yhtio='$kukarow[yhtio]' and tunnus='$ttrow[liitostunnus]'";
					$erres = pupe_query($query);

					if (mysql_num_rows($erres) != 1) {
						$tilauksesta_ostotilaus .= "<font class='error'>".t("VIRHE: Toimittajan tietoja ei l�ytynyt tuotteelta").": $rivi[tuoteno]!<br>";
					}
					else {
						// toimittaja l�ytyi, tehd��n t�st� ostotilaus
						$tiltoi = mysql_fetch_assoc($erres);

						// Katsotaan onko toimittajalla avoimia ostotilauksia
						// Pit�� olla my�s oikeat osoitetiedot
						if ($tyyppi == "U" or ($tyyppi == 'KAIKKI' and $yhtiorow["tee_osto_myyntitilaukselta"] == 'Q')) {
							$toim_nimi 			= $myytilrow["toim_nimi"];
							$toim_nimitark 		= $myytilrow["toim_nimitark"];
							$toim_osoite 		= $myytilrow["toim_osoite"];
							$toim_postino 		= $myytilrow["toim_postino"];
							$toim_postitp 		= $myytilrow["toim_postitp"];
							$toim_maa 			= $myytilrow["toim_maa"];
						}
						else {
							$toim_nimi 			= $yhtiorow["nimi"];
							$toim_nimitark 		= $yhtiorow["nimitark"];
							$toim_osoite 		= $yhtiorow["osoite"];
							$toim_postino 		= $yhtiorow["postino"];
							$toim_postitp 		= $yhtiorow["postitp"];
							$toim_maa 			= $yhtiorow["maa"];
						}

						$valkoodi = $tiltoi["oletus_valkoodi"];

						if ($valkoodi == '') {
							$valkoodi = $yhtiorow["valkoodi"];
						}

						$vquery = "	SELECT nimi, kurssi, tunnus
									FROM valuu
									WHERE yhtio = '$kukarow[yhtio]'
									and nimi = '$valkoodi'";
						$vresult = pupe_query($vquery);
						$vrow = mysql_fetch_array($vresult);

						$commentsit = trim(t("Myyntitilaus").": $myytilrow[tunnus] \n".t("Myyntitilauksen toimitustapa").": $myytilrow[toimitustapa] \n".$tiltoi["comments"]);

						$query = "	SELECT *
									from lasku
									where yhtio 		= '$kukarow[yhtio]'
									and liitostunnus 	= '$tiltoi[tunnus]'
									and tila 			= 'O'
									and alatila 		= ''
									and toim_nimi		= '$toim_nimi'
									and toim_nimitark	= '$toim_nimitark'
									and toim_osoite		= '$toim_osoite'
									and toim_postino	= '$toim_postino'
									and toim_postitp	= '$toim_postitp'
									and toim_maa		= '$toim_maa'
									and left(luontiaika,10) = left(now(),10)
									and laatija 		= '$kukarow[kuka]'";
						$jtsre = pupe_query($query);

						// ei l�ydy, tehd��n uus otsikko
						if (mysql_num_rows($jtsre) == 0) {

							$query = "	INSERT INTO lasku SET
										alatila 			= '',
										comments 			= '$commentsit',
										huolitsija 			= '$tiltoi[huolitsija]',
										jakelu 				= '$jakelu',
										kerayspvm 			= '$myytilrow[kerayspvm]',
										kuljetus 			= '$tiltoi[kuljetus]',
										laatija 			= '$kukarow[kuka]',
										liitostunnus		= '$tiltoi[tunnus]',
										luontiaika			= now(),
										maa 				= '$tiltoi[maa]',
										maksuteksti 		= '$tiltoi[maksuteksti]',
										myyja 				= '$myytilrow[myyja]',
										nimi 				= '$tiltoi[nimi]',
										nimitark 			= '$tiltoi[nimitark]',
										osoite 				= '$tiltoi[osoite]',
										ovttunnus 			= '$tiltoi[ovttunnus]',
										postino 			= '$tiltoi[postino]',
										postitp 			= '$tiltoi[postitp]',
										sisviesti1			= '$myytilrow[sisviesti1]',
										tila 				= 'O',
										tilaustyyppi		= '',
										tilausyhteyshenkilo	= '$kukarow[nimi]',
										toimaika 			= '$myytilrow[toimaika]',
										toimitusehto 		= '$tiltoi[toimitusehto]',
										toimitustapa 		= '$myytilrow[toimitustapa]',
										toim_maa 			= '$toim_maa',
										toim_nimi 			= '$toim_nimi',
										toim_nimitark 		= '$toim_nimitark',
										toim_osoite 		= '$toim_osoite',
										toim_ovttunnus		= '$tiltoi[ovttunnus]',
										toim_postino 		= '$toim_postino',
										toim_postitp 		= '$toim_postitp',
										valkoodi 			= '$valkoodi',
										vanhatunnus			= '',
										varasto 			= '',
										verkkotunnus		= '$myytilrow[liitostunnus]',
									 	vienti_kurssi 		= '$vrow[kurssi]',
										viesti 				= '$myytilrow[viesti]',
										viikorkopros 		= '$yhtiorow[viivastyskorko]',
										yhtio 				= '$kukarow[yhtio]',
										ytunnus				= '$tiltoi[ytunnus]'";
							$updre = pupe_query($query);
							$tunnus = (string) mysql_insert_id();

							// hintojen ja alennuksien hakuun tarvitaan laskurow, tehd��n t�ss� array kuntoon alehinta_osto() funktiota varten
							$jtsro = array(
								'liitostunnus' 	=> $tiltoi['tunnus'],
								'valkoodi' 		=> $valkoodi,
								'ytunnus' 		=> $tiltoi['ytunnus'],
								'vienti_kurssi' => $vrow['kurssi']
							);
						}
						else {
							// tilaus l�yty, otetaan tunnus
							$jtsro = mysql_fetch_assoc($jtsre);
							$tunnus = $jtsro["tunnus"];
						}

						//Haetaan my�s ns. lis�varusteperheet.
						$query = "	SELECT tilausrivin_lisatiedot.*, tilausrivi.*, tilausrivi.tunnus rivitunnus, jt+varattu kpl
									FROM tilausrivi
									LEFT JOIN tilausrivin_lisatiedot ON (tilausrivin_lisatiedot.yhtio=tilausrivi.yhtio and tilausrivin_lisatiedot.tilausrivitunnus=tilausrivi.tunnus)
									where tilausrivi.yhtio = '$kukarow[yhtio]'
									and tilausrivi.otunnus = '$otunnus'
									and tilausrivi.tyyppi != 'D'
									and (tilausrivi.tunnus='$rivi[tunnus]' or tilausrivi.perheid2='$rivi[tunnus]')
									order by tilausrivi.perheid2, tilausrivi.tunnus";
						$tilrivires = pupe_query($query);

						$pid = 0;

						while ($tilrivirow = mysql_fetch_assoc($tilrivires)) {
							// haetaan oletuspaikan tiedot niin laitetaan se riville
							$query = "SELECT * from tuotepaikat where yhtio='$kukarow[yhtio]' and tuoteno='$tilrivirow[tuoteno]' and oletus!=''";
							$jtsre = pupe_query($query);
							$jtstu = mysql_fetch_assoc($jtsre);

							// varattu m��r�� saa p�ivitt�� vaan jos kpl = 0 ja uusiotunnus = 0, tila = O, alatila =''
							$query =  "	SELECT tilausrivi.*
										FROM tilausrivi
										JOIN lasku ON (lasku.yhtio = tilausrivi.yhtio and lasku.tunnus = tilausrivi.otunnus and lasku.tila = 'O' and lasku.alatila = '')
										LEFT JOIN tilausrivin_lisatiedot ON (tilausrivin_lisatiedot.yhtio = tilausrivi.yhtio and tilausrivin_lisatiedot.tilausrivitunnus = tilausrivi.tunnus)
										WHERE tilausrivi.yhtio 	= '{$kukarow["yhtio"]}'
										AND tilausrivi.otunnus 	= '$tunnus'
										AND tilausrivi.tunnus 	= '{$tilrivirow["tilausrivilinkki"]}'
										AND tilausrivi.kpl 		= 0
										AND tilausrivi.uusiotunnus = ''";
							$upsres = pupe_query($query);

							if (mysql_num_rows($upsres) == 1) {
								$loytyi = mysql_fetch_assoc($upsres);

								$query =  "	UPDATE tilausrivi
											SET varattu = '{$tilrivirow["kpl"]}',
											tilkpl 		= '{$tilrivirow["kpl"]}'
											WHERE yhtio = '{$kukarow["yhtio"]}'
											AND tunnus	= '{$loytyi["tunnus"]}'";
								$upsaresult = pupe_query($query);

								$lisatty_tun = $loytyi["tunnus"];
							}
							else {

								// hintojen ja alennuksien hakuun tarvitaan tuoterow, tehd��n t�ss� array kuntoon alehinta_osto() funktiota varten
								$tuote_row = array(
									'tuoteno' 	=> $tilrivirow['tuoteno']
								);

								list($hinta,$netto,$ale,) = alehinta_osto($jtsro, $tuote_row, $tilrivirow['kpl'], '', '', '');
								// lis�t��n ostotilausrivi
								$query = "	INSERT into tilausrivi set
											hyllyalue 		= '$jtstu[hyllyalue]',
											hyllynro 		= '$jtstu[hyllynro]',
											hyllyvali 		= '$jtstu[hyllyvali]',
											hyllytaso 		= '$jtstu[hyllytaso]',
											tilaajanrivinro = '',
											laatija 		= '$kukarow[kuka]',
											laadittu 		= now(),
											yhtio 			= '$kukarow[yhtio]',
											tuoteno 		= '$tilrivirow[tuoteno]',
											varattu 		= '$tilrivirow[kpl]',
											yksikko 		= '$tilrivirow[yksikko]',
											kpl 			= 0,
											kpl2			= 0,
											tilkpl 			= '$tilrivirow[kpl]',
											jt				= 0,
											hinta			= '$hinta',
											netto			= '$netto',
											ale1			= '$ale[ale1]',
											ale2			= '$ale[ale2]',
											ale3			= '$ale[ale3]',
											alv 			= '',
											kerayspvm 		= '$tilrivirow[kerayspvm]',
											otunnus 		= '$tunnus',
											tyyppi 			= 'O',
											toimaika 		= '$tilrivirow[toimaika]',
											kommentti 		= '$tilrivirow[kommentti]',
											var 			= '',
											try				= '$tilrivirow[try]',
											osasto			= '$tilrivirow[osasto]',
											perheid			= '',
											perheid2		= '$pid',
											nimitys 		= '$tilrivirow[nimitys]',
											jaksotettu		= ''";
								$updre = pupe_query($query);
								$lisatty_tun = mysql_insert_id();

								// Lis�t��n ostorivin lis�tiedot
								$query = "	INSERT INTO tilausrivin_lisatiedot
											SET tilausrivitunnus 	= '$lisatty_tun',
											luontiaika				= now(),
											laatija 				= '$kukarow[kuka]',
											yhtio		 	 		= '$kukarow[yhtio]'";
								$updres = pupe_query($query);
							}

							// Onko suoratoimitus
							if ($tyyppi == "U" or ($tyyppi == 'KAIKKI' and $yhtiorow["tee_osto_myyntitilaukselta"] == 'Q')) {
								$suoraan_laskutukseen = "o";
							}
							else {
								$suoraan_laskutukseen = "";
							}

							// P�ivitet��n myyntirivin lis�tiedot
							$query = "	UPDATE tilausrivin_lisatiedot
										SET tilausrivilinkki 	= '$lisatty_tun',
										toimittajan_tunnus		= '$ttrow[liitostunnus]',
										suoraan_laskutukseen	= '$suoraan_laskutukseen',
										muutospvm				= now(),
										muuttaja				= '$kukarow[kuka]'
										WHERE yhtio		 	 = '$kukarow[yhtio]'
										and tilausrivitunnus = '$tilrivirow[rivitunnus]'";
							$updres = pupe_query($query);

							$tilauksesta_ostotilaus .= "<font class='message'>Tuote: $tilrivirow[tuoteno] $tilrivirow[kpl] kpl lis�tty ostotilaukselle: $tunnus</font><br>";

							if ($tilrivirow["tunnus"] == $tilrivirow["perheid2"] and mysql_num_rows($tilrivires) > 1) {
								$pid = $lisatty_tun;

								$query = "	UPDATE tilausrivi
											set perheid2 = $pid
											where yhtio = '$kukarow[yhtio]' and tunnus=$pid";
								$perheres = pupe_query($query);
							}
						}
					}

					// tehd��n rivist� tavallinen JT jos meill� oli joku tilattava rivi
					if ($tyyppi == "KAIKKI" or $rivi["var"] == "T" or $rivi["var"] == "U") {
						$query = "	UPDATE tilausrivi
									set var = 'J'
									where yhtio = '$kukarow[yhtio]'
									and tunnus = '$rivi[rivitunnus]'";
						$updre = pupe_query($query);

						// Vapautetaan JT-rivin er�t jos niit� on
						if ($rivi["sarjanumeroseuranta"] == "E" or $rivi["sarjanumeroseuranta"] == "F" or $rivi["sarjanumeroseuranta"] == "G") {
							vapauta_sarjanumerot($toim, $rivi["otunnus"], " and tilausrivi.tunnus = '$rivi[rivitunnus]' ", FALSE);
						}

						if ($yhtiorow["varaako_jt_saldoa"] == "") {
							// Siirret��n kappaleet, jos ne on j��ny v��r�lle puolelle
							$query = "	UPDATE tilausrivi
										set jt = varattu, varattu = 0
										where yhtio = '$kukarow[yhtio]'
										and tunnus = '$rivi[rivitunnus]'
										and jt = 0
										and varattu != 0";
							$updre = pupe_query($query);
						}
					}
				}

				// Katsotaan oliko kolmikantakauppaa
				$triangulation = triangulation($otunnus) != "" ? "K" : "";

				$query = "	UPDATE lasku SET
							kolmikantakauppa = '{$triangulation}'
							WHERE yhtio = '{$kukarow['yhtio']}'
							AND tunnus = '{$otunnus}'";
				$upd_res = pupe_query($query);
			}

			return $tilauksesta_ostotilaus;
		}
	}
