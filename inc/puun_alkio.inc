<?PHP
// puun_alkio.inc

if ($laji == 'asiakas') {
	$rajauslisa = " and laji = 'asiakas' ";
	$laji = 'asiakas';
	$otsikko = 'Asiakas-segmentit';
}
else {
	$rajauslisa = " and laji = 'tuote' ";
	$laji = 'tuote';
	$otsikko = 'Tuote-segmentit';
}

$otsikko_nappi 	= 'segmentti-liitos';

// HUOM HUOM: eth�n muuta n�it� ilman, ett� korjaat my�s yllapito.php:ss� iframen lukitse_avaimeen muuttujat
$kentat = "tunnus, liitos, kieli, laji, puun_tunnus, (SELECT nimi FROM dynaaminen_puu WHERE yhtio = '{$kukarow['yhtio']}' AND tunnus = puun_alkio.puun_tunnus AND laji = '$laji') nimi";

$jarjestys = 'liitos, laji, kieli, puun_tunnus, nimi';

?>