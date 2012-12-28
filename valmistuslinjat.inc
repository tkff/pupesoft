<?php
/**
* Tiedosto sis‰lt‰‰ funktioita tuotannonsuunnitteluun valmistuslinjojen k‰sittelyyn.
*/

/** P‰ivit‰‰ valmistuksen tilaa (lasku.valmistuksen_tila)
*/
function paivita_valmistus($tunnus, $tila) {
      global $kukarow;

      echo "p‰ivitet‰‰n valmistus $tunnus tila $tila";

      if (empty($tunnus) or empty($tila)) {
            exit("Virhe valmistuksen p‰ivityksess‰");
      }
      $query = "UPDATE lasku SET valmistuksen_tila='{$tila}' WHERE yhtio='{$kukarow['yhtio']}' AND tunnus='{$tunnus}'";
      $result = pupe_query($query) or exit("Virhe tilan p‰ivityksess‰" . mysql_error());

      return $result;
}

function hae_valmistuslinjan_valmistukset($linja) {
      global $kukarow;

      // Haetaan linjalla olevat valmistukset
      $query = "SELECT *
                  FROM kalenteri
                  JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
                  WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
                  AND henkilo='{$linja['id']}'";
      $result = pupe_query($query);

      $valmistuslinjan_tuotteet = array();
      while ($valmiste = mysql_fetch_assoc($result)) {
            $valmistuslinjan_tuotteet[] = $valmiste;
      }

      return $valmistuslinjan_tuotteet;
}

function hae_valmistuksen_tuotteet($valmistus) {
      global $kukarow;

      $query = "SELECT *
                  FROM tilausrivi
                  WHERE yhtio='{$kukarow['yhtio']}'
                  AND otunnus='{$valmistus['otunnus']}'
                  AND tyyppi='W'";
      $result = pupe_query($query);

      $tuotteet = array();
      while($tuote = mysql_fetch_assoc($result)) {
            $tuotteet[] = $tuote;
      }

      return $tuotteet;
}

/** Hakee valmistuksen
*/
function hae_valmistus($tunnus) {
      global $kukarow;

      // Haetaan valmistus
      $query = "SELECT kalenteri.kuka, kalenteri.tunnus, kalenteri.otunnus, tilausrivi.nimitys, tilausrivi.varattu, tilausrivi.yksikko
                  FROM kalenteri
                  JOIN lasku ON (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
                  JOIN tilausrivi ON (lasku.yhtio=tilausrivi.yhtio AND lasku.tunnus = tilausrivi.otunnus AND tilausrivi.tyyppi='W')
                  WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
                  AND kalenteri.otunnus='{$tunnus}'";
      $result = pupe_query($query);
      echo $query;
      $valmistus = mysql_fetch_assoc($result);

      return $valmistus;
}

function valmistuksen_tuotteen_kesto($tuote) {
      global $kukarow;

      $query = "SELECT sum(varattu) as kesto FROM tilausrivi WHERE yhtio='{$kukarow['yhtio']}' and otunnus={$valmistus['otunnus']} and yksikko='H'";
      $result = pupe_query($query);
      $kesto = mysql_fetch_assoc($result);

      return $kesto['kesto'];
}

function lisaa_valmistus($valmistus, $linja) {
      global $kukarow;

      // Haetaan valmistuksen tiedot
      $query = "SELECT * FROM lasku where tunnus='{$valmistus}'";
      $result = pupe_query($query);
      $valmistus = mysql_fetch_assoc($result);

      // Haetaan valmistuksen tuotteen tiedot
      $query = "SELECT *
                  FROM tilausrivi
                  WHERE yhtio='{$kukarow['yhtio']}'
                  AND otunnus='{$valmistus['tunnus']}'
                  AND tyyppi='W'";
      $result = pupe_query($query);
      $valmistettava = mysql_fetch_assoc($result);

      // Haetaan viimeisen valmistuksen pvmloppu
      // Uusi valmistus lis‰t‰‰n viimeisimm‰n per‰‰n
      $query = "SELECT *
                  FROM kalenteri
                  WHERE yhtio='{$kukarow['yhtio']}'
                  AND tyyppi='valmistus'
                  AND henkilo='{$linja}'
                  ORDER BY pvmloppu desc LIMIT 1";
      $result = pupe_query($query);
      $edellinen_valmistus = mysql_fetch_assoc($result);

      // Jos linjalla ei ole valmistuksia
      if (mysql_num_rows($result) == 0) {
            $start = round_time(strtotime('now'));
            echo "aloitetaan t‰st‰ hetkest‰ " . date('Y-m-d H:i:s', $start) . "<br>";
      }
      // Lis‰t‰‰n viimeisimm‰n valmistuksen per‰‰n
      else {
            $start = strtotime($edellinen_valmistus['pvmloppu']);
            echo "lis‰t‰‰n viimeisimm‰n valmistuksen per‰‰n" . date('Y-m-d H:i:s', $start) . "<br>";
      }

      // Lasketaan loppuaika
      $kesto = valmistuksen_kesto($valmistus);
      echo "valmistus kest‰‰ $kesto<br>";
      $end = laske_loppuaika($start, $kesto * 60);
      echo "valmistuksen loppuaika on " . date('Y-m-d H:i:s', $end);

      $pvmalku = date('Y-m-d H:i:s', $start);
      $pvmloppu = date('Y-m-d H:i:s', $end);

      // Lis‰t‰‰n valmistus kalenteriin
      $query = "INSERT INTO kalenteri SET
                  yhtio       = '$kukarow[yhtio]',
                  kuka        = '$kukarow[kuka]',
                  henkilo     = '$linja',
                  pvmalku     = '$pvmalku',
                  pvmloppu    = '$pvmloppu',
                  tyyppi      = 'valmistus',
                  otunnus     = $valmistus[tunnus]";
      #echo $query;
      $result = pupe_query($query);
}

/** Valmistuksen kesto
*/
function valmistuksen_kesto($valmistus) {
      global $kukarow;

      // Lasketaan valmistuksen kokoonpanotyˆt yheen
      $query = "SELECT sum(varattu) as kesto
                  FROM tilausrivi
                  WHERE yhtio='{$kukarow['yhtio']}'
                  AND otunnus={$valmistus['tunnus']} AND yksikko='H'";
      $result = pupe_query($query);
      $kesto = mysql_fetch_assoc($result);

      return $kesto['kesto'];
}

function hae_valmistuslinjat() {
      global $kukarow;

      // Haetaan valmistuslinjat avainsanoista
      $query = "SELECT * FROM avainsana WHERE yhtio='{$kukarow['yhtio']}' AND laji='valmistuslinja' ORDER BY selite";
      $result = pupe_query($query);

      $valmistuslinjat = array();

      while($linja = mysql_fetch_assoc($result)) {
            $valmistuslinjat[] = $linja;
      }

      return $valmistuslinjat;
}

/**
* Pyˆrist‰‰ ajan halutulla tarkkuudella
* Oletus 30 minuuttia
*
* @param timestamp $time
* @param integer $seconds
*
* @return timestamp
*/
function round_time($time, $seconds = 1800) {
      $rounded_time = round($time / $seconds) * $seconds;
      return $rounded_time;
}


/** Laskee valmistuksen loppumiskohdan alkuajan ja valmistuksen keston mukaan.
*/
function laske_loppuaika($alkuaika, $minuutit = 0) {
      global $kukarow;

      // Pienin kalenterin yksikkˆ, 30 minuuttia
      define('STEP', 30);

      if ($minuutit == 0) $minuutit = 60;

      // Setataan loppuaika aluksi pyˆristettyyn aloitusaikaan
      $loppuaika = round_time($alkuaika);

      echo "kesto: ". $minuutit/60 ." aloitetaan lis‰‰m‰‰n alkuajasta (alkuaika: ".date('Y-m-d H:i:s', $alkuaika)."): ".date('Y-m-d H:i:s', $loppuaika)."<br>";

      // Haetaan lomat ja muut kalenterimerkinn‰t mitk‰ vaikuttaa loppup‰iv‰n laskemiseen
      $query = "SELECT pvmalku, pvmloppu FROM kalenteri WHERE yhtio='{$kukarow['yhtio']}' AND tyyppi IN ('SA', 'PY') ORDER BY pvmalku";
      $result = pupe_query($query);
      $muut = mysql_fetch_assoc($result);

      // Lis‰t‰‰n aikaa
      while($minuutit > 0) {

            // Viikonloput
            if (date('w', $loppuaika) == 0 or date('w', $loppuaika) == 6) {
                  $loppuaika = mktime(date('H', $loppuaika), date('i', $loppuaika) + STEP, date('s', $loppuaika), date('m', $loppuaika), date('d', $loppuaika), date('Y', $loppuaika));
            }
            // Jos ollaan tyˆajalla, voidaan lis‰t‰ aikaa
            elseif (date('H:i', $loppuaika) >= date('H:i', strtotime('07:00')) and date('H:i', $loppuaika) < date('H:i', strtotime('15:00'))) {
                  $loppuaika = mktime(date('H', $loppuaika), date('i', $loppuaika) + STEP, date('s', $loppuaika), date('m', $loppuaika), date('d', $loppuaika), date('Y', $loppuaika));

                  // Jos ei osuta muille lomap‰iville tai poissaoloille niin v‰hennet‰‰n aikaa
                  if (date('Y-m-d H:i:s', $loppuaika) <= $muut['pvmalku'] or date('Y-m-d H:i:s', $loppuaika) >= $muut['pvmloppu']) {
                        $minuutit -= 30;
                  }
            }
            // Ei tyˆaikaa
            else {
                  // Edet‰‰n seuraavaan mahdolliseen tyˆaikaan
                  $loppuaika = mktime(date('H', $loppuaika), date('i', $loppuaika) + STEP, date('s', $loppuaika), date('m', $loppuaika), date('d', $loppuaika), date('Y', $loppuaika));
            }
      }

      echo "loppuaika: ".date('Y-m-d H:i:s', $loppuaika)."<br>";
      return $loppuaika;
}

/** Rakennetaan valmistuslinjat uudelleen
*/
function rebuild_valmistuslinjat($from = 'now') {
      global $kukarow;

      $from = date('Y-m-d H:i:s', strtotime($from));
      echo "rakennetaan valmistuslinja uudelleen alkaen: $from <br>";

      // Hetaan kaikki valmistuslinjat
      $query = "SELECT selite FROM avainsana WHERE yhtio='{$kukarow['yhtio']}' AND laji='VALMISTUSLINJA' ORDER BY selite";
      $valmistuslinjat_result = pupe_query($query);

      // J‰rjestell‰‰n linja kerrallaan
      while($linja = mysql_fetch_assoc($valmistuslinjat_result)) {
            echo "j‰rjestell‰‰n linja $linja[selite]<br>";

            // Haetaan kaikki linjan valmistukset
            $query = "SELECT *, kalenteri.tunnus as tunnus
                        FROM kalenteri
                        JOIN lasku ON (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
                        WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
                        AND henkilo='{$linja['selite']}'
                        ORDER BY pvmalku";
            $result = pupe_query($query);

            $valmistukset = array();

            // J‰rjestell‰‰n tulokset niin ett‰ valmistuksessa oleva on ensimm‰isen‰
            while($valmistus = mysql_fetch_assoc($result)) {
                  if ($valmistus['valmistuksen_tila'] == 'VA') {
                        array_unshift($valmistukset, $valmistus); // Laitetaan valmistuksessa oleva jonon alkuun
                  }
                  else {
                        array_push($valmistukset, $valmistus); // Muuten lis‰t‰‰n vain taulukkoon
                  }
            }

            $edellinen_valmistus = '';

            // Loopataan valmistukset l‰pi ja j‰rjestell‰‰n
            foreach($valmistukset as $valmistus) {
                  echo "valmistus: $valmistus[tunnus]<br>";
                  if ($edellinen_valmistus == '') $edellinen_valmistus = $valmistus;
                  echo "edellinen_valmistus :$edellinen_valmistus[tunnus]<br>";

                  // Siiret‰‰n vain valmistusta odottavia valmistuksia
                  if ($valmistus['valmistuksen_tila'] == 'OV') {

                        echo "tila ov, voidaan siirell‰<br>";
                        // Jos tarkasteltavan valmistuksen pvmalku ei ole edellisen pvmloppu, eli
                        // valmistukset eiv‰t ole 'toisissaan kiinni', siirret‰‰n tarkasteltava
                        // edellisen loppuun.
                        if ($valmistus['pvmalku'] != $edellinen_valmistus['pvmloppu'] and $valmistus['tunnus'] != $edellinen_valmistus['tunnus']) {
                              // Yhdistet‰‰n edellisen loppu t‰m‰n alkuun
                              $valmistus['pvmalku'] = $edellinen_valmistus['pvmloppu'];

                              echo "valmistus pvmalku $valmistus[pvmalku] pvmloppu $valmistus[pvmloppu]<br>";
                              // lasketaan uusi loppuaika
                              $kesto = valmistuksen_kesto($valmistus['otunnus']);
                              $pvmloppu = laske_loppuaika(strtotime($valmistus['pvmalku']), $kesto * 60);
                              $valmistus['pvmloppu'] = date('Y-m-d H:i:s', $pvmloppu);

                              // P‰ivitet‰‰n p‰iv‰m‰‰r‰t kantaan
                              $query = "UPDATE kalenteri
                                          SET pvmalku='{$valmistus['pvmalku']}', pvmloppu='{$valmistus['pvmloppu']}'
                                          WHERE tunnus='{$valmistus['tunnus']}'";
                              pupe_query($query);
                              echo "$query <br>";
                        }

                  }

                  // Otetaan valmistus talteen seuraavaa kierrosta varten
                  $edellinen_valmistus = $valmistus;
            }
      }
}

/**
* Tarkistaa valmistuksen raaka-aineiden saldot
*/
function puuttuvat_raaka_aineet($tunnus, $aloitus_pvm = 'now') {
      global $kukarow;

      // Ker‰t‰‰n riitt‰m‰ttˆm‰t tuotteet talteen
      $vajaat_saldot = array();

      // Valmistuksen p‰iv‰m‰‰r‰, mink‰ mukaan muut saldot tarkistetaan
      $aloitus_pvm = date('Y-m-d H:i:s', strtotime($aloitus_pvm));

      // Haetaan laskun tiedot
      $query = "SELECT * FROM lasku WHERE yhtio='{$kukarow['yhtio']}' and tunnus='$tunnus'";
      $result = pupe_query($query);
      $lasku = mysql_fetch_assoc($result);

      // Haetaan valmistuksen raaka-aineet
      $query = "SELECT *
                        FROM tilausrivi
                        WHERE yhtio='{$kukarow['yhtio']}'
                        AND otunnus='{$tunnus}'
                        AND tuoteno!='TY÷'
                        AND tyyppi='V'";
      $result = pupe_query($query);

      $html = "<table border=1>
                  <tr>
                  <th></th>
                  <th>tuoteno</th>
                  <th>myytavissa</th>
                  <th>varattu</th>
                  <th>muut_valmistukset</th>
                  <th>ostotilaukset</th>
                  </tr>";

      while($raaka_aine = mysql_fetch_assoc($result)) {

            $saldo = array();
            list($saldo['saldo'], $saldo['hyllyssa'], $saldo['myytavissa']) = saldo_myytavissa($raaka_aine['tuoteno']);

            $html .= "<tr>
                        <td>saldo: {$saldo['saldo']}, hyllyssa: {$saldo['hyllyssa']}, myytavissa: {$saldo['myytavissa']}</td>
                        <td>{$raaka_aine['tuoteno']} {$raaka_aine['yksikko']}</td>
                        <td>{$saldo['myytavissa']}</td>
                        <td><b>{$raaka_aine['varattu']}</b></td>";

            // Varatut kappaleet valmistuksilta jotka ovat jo valmistuslinjalla.
            // Valmistuslinjalla olevat valmistukset varaavat saldoa ja uuden valmistuksen on
            // tarkistettava paljon ne v‰hent‰v‰t raaka-aineiden saldoa.
            $query = "SELECT COALESCE(sum(tilausrivi.varattu), 0) AS varattu
                        FROM kalenteri
                              JOIN lasku ON (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
                              JOIN tilausrivi ON (lasku.yhtio=tilausrivi.yhtio AND lasku.tunnus=tilausrivi.otunnus)
                        WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
                              AND kalenteri.tyyppi='valmistus'
                              AND tilausrivi.tyyppi='V'
                              AND tilausrivi.tuoteno='{$raaka_aine['tuoteno']}'
                              AND kalenteri.pvmalku < '$aloitus_pvm'";
            $muut_valmistukset_result = pupe_query($query);
            $muut_valmistukset = mysql_fetch_assoc($muut_valmistukset_result);

            $html .= "<td>{$muut_valmistukset['varattu']}</td>";

            // Haetaan raaka-aineen ostotilauksia, jotka vaikuttavat valmistuksen aloitukseen
            $query = "SELECT COALESCE(sum(varattu), 0) AS varattu
                        FROM tilausrivi
                        WHERE yhtio='{$kukarow['yhtio']}'
                        AND tuoteno='{$raaka_aine['tuoteno']}'
                        AND tyyppi='O'
                        AND kerattyaika != '0000-00-00 00:00:00'
                        AND kerattyaika < '$aloitus_pvm'"
                        ;
            $ostotilaukset_result = pupe_query($query);
            $ostotilaukset = mysql_fetch_assoc($ostotilaukset_result);

            $html .= "<td>{$ostotilaukset['varattu']}</td>";

            /*
            Raaka-aineen riiton laskentakaava on:
            "Saldo nyt"
            - "Varatut kappaleet valmistuksilta, joiden p‰iv‰ on vanhempi kun ko. p‰iv‰"
            - "Varatut kappaleet myyntitilauksilta, joiden p‰iv‰ on vanhempi kun ko. p‰iv‰"
            + "Vahvistetut kappaleet ostotilausriveilta, joiden saapumisp‰iv‰ on vanhempi kun ko. p‰iv‰".
            */
            // saldo_myytavissa sis‰lt‰‰ valmistuksen varaamat raaka-aineet ja myyntitilausten varaamat kappaleet.
            // Huomioidaan vain muiden valmistusten varaamat ja mahdollisten ostotilausten vaikutus saldoihin.
            $raaka_aine_saldo = $saldo['myytavissa']- $muut_valmistukset['varattu'] + $ostotilaukset['varattu'];

            if ($raaka_aine_saldo <= 0) {
                  $vajaat_saldot[$raaka_aine['tuoteno']] = $raaka_aine_saldo;

                  $html .= "<td>$raaka_aine_saldo (Raaka-aine ei riit‰!)</td>";
            } else {
                  $html .= "<td>$raaka_aine_saldo</td>";
            }
            $html .= "</tr>";
      }

      $html .= "</table>";

     ## echo $html;

      return $vajaat_saldot;
}

/** Etsii edellisen valmistuksen
*
* @param integer $valmistus Valmistuksen tunnus
*/
function etsi_edellinen_valmistus($valmistus) {
      global $kukarow;

      // Etsit‰‰n edellinen valmistus
      $query = "SELECT *, lasku.tunnus
                  FROM kalenteri
                  JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
                  WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
                  AND pvmloppu <= '{$valmistus['pvmalku']}'
                  AND henkilo = '{$valmistus['henkilo']}'
                  ORDER BY pvmloppu DESC LIMIT 1";
      $result = pupe_query($query);
      $edellinen = mysql_fetch_assoc($result);

      // Palautetaan lˆydetty valmistus
      return $edellinen;
}

/**
* Etsii seuraavan_valmistuksen
*
* @param integer $valmistus Valmistuksen tunnus
*/
function etsi_seuraava_valmistus($valmistus) {
      global $kukarow;

      // Etsit‰‰n seuraava valmistus
      $query = "SELECT *, lasku.tunnus
                  FROM kalenteri
                  JOIN lasku on (kalenteri.yhtio=lasku.yhtio AND kalenteri.otunnus=lasku.tunnus)
                  WHERE kalenteri.yhtio='{$kukarow['yhtio']}'
                  AND pvmalku >= '{$valmistus['pvmloppu']}'
                  AND henkilo = '{$valmistus['henkilo']}'
                  ORDER BY pvmalku LIMIT 1";
      $result = pupe_query($query);
      $seuraava = mysql_fetch_assoc($result);

      // Palautetaan lˆydetty valmistus
      return $seuraava;
}

/**
* Vaihtaa valmistusten paikkoja
* Olettaa ett‰ vasen on aina aikaisempi valmistus (vasen[pvmalku] < oikea[pvmalku])
*
* @param array $vasen
* @param array $oikea
*/
function vaihda_valmistusten_paikkaa($vasen, $oikea) {
      global $kukarow;

      // VAlmistuksessa olevia ei siirret‰
      if ($vasen['valmistuksen_tila'] == 'VA' or $oikea['valmistuksen_tila'] == 'VA') {
            echo "<font class='error'>Yritit vaihtaa jo valmistuksessa olevaa valmistusta</font><br>";
            return false;
      }

      // Aloitetaan laskemaan p‰iv‰m‰‰ri‰ oikein
      // Oikea siirtyy alkamaan vasemman alusta
      $oikea['pvmalku'] = $vasen['pvmalku'];

      // swapataan muuttujat, jotta loppu on selke‰mp‰‰
      // (Left is Right, Right!)
      $_temp = $vasen;
      $vasen = $oikea;
      $oikea = $_temp;

      // Lasketaan vasemman loppuaika uudelleen
      $kesto = valmistuksen_kesto($vasen);
      echo "vasen kesto: $kesto <br>";
      $uusi_pvmloppu = laske_loppuaika(strtotime($vasen['pvmalku']), $kesto * 60);
      $vasen['pvmloppu'] = date('Y-m-d H:i:s', $uusi_pvmloppu);

      // Oikean alkuaika on vasemman uusi loppuaika
      $oikea['pvmalku'] = $vasen['pvmloppu'];

      // Lasketaan oikean loppuaika uudelleen
      $kesto = valmistuksen_kesto($oikea);
      echo "oikea kesto: $kesto <br>";
      $uusi_pvmloppu = laske_loppuaika(strtotime($oikea['pvmalku']), $kesto * 60);
      $oikea['pvmloppu'] = date('Y-m-d H:i:s', $uusi_pvmloppu);

      // P‰ivitet‰‰n valmistusten uudet p‰iv‰m‰‰r‰t kantaan
      $query = "UPDATE kalenteri
                  SET pvmalku='{$vasen['pvmalku']}', pvmloppu='{$vasen['pvmloppu']}'
                  WHERE yhtio='{$kukarow['yhtio']}' and otunnus={$vasen['tunnus']}";
      echo $query."<br>";
      pupe_query($query);

      $query = "UPDATE kalenteri
                  SET pvmalku='{$oikea['pvmalku']}', pvmloppu='{$oikea['pvmloppu']}'
                  WHERE yhtio='{$kukarow['yhtio']}' and otunnus={$oikea['tunnus']}";
      pupe_query($query);
      echo $query."<br>";

      return true;
}
