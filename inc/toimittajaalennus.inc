<?php

$otsikko = 'Toimittaja alennukset';
$otsikko_nappi = 'toimittaja alennus';

if ($from == "") {
	// HUOM HUOM: eth�n muuta n�it� ilman, ett� korjaat my�s yllapito.php:ss� iframen lukitse_avaimeen muuttujat
	$kentat = "tunnus, concat_ws('<br>', toimittaja, (SELECT nimi from toimi where toimittajaalennus.yhtio=toimi.yhtio and toimittajaalennus.toimittaja=toimi.tunnus)) Toimittaja, concat_ws('<br>',ytunnus,(SELECT group_concat(distinct nimi separator '<br>') from toimi where toimittajaalennus.yhtio=toimi.yhtio and toimittajaalennus.ytunnus=toimi.ytunnus and toimi.ytunnus != '')) 'Ytunnus', tuoteno, ryhma 'Alennusryhm�', alennus, alennuslaji, minkpl 'Minkpl', concat_ws('<br>',alkupvm, loppupvm) 'Alku/Loppu'";
}
else {
	// HUOM HUOM: eth�n muuta n�it� ilman, ett� korjaat my�s yllapito.php:ss� iframen lukitse_avaimeen muuttujat
	$kentat = "tunnus, concat_ws('<br>', toimittaja, (SELECT nimi from toimi where toimittajaalennus.yhtio=toimi.yhtio and toimittajaalennus.toimittaja=toimi.tunnus)) Toimittaja, concat_ws('<br>',ytunnus,(SELECT group_concat(distinct nimi separator '<br>') from toimi where toimittajaalennus.yhtio=toimi.yhtio and toimittajaalennus.ytunnus=toimi.ytunnus and toimi.ytunnus != '')) 'Ytunnus', ryhma 'Alennusryhm�', alennus, alennuslaji, minkpl 'Minkpl', concat_ws('<br>',alkupvm, loppupvm) 'Alku/Loppu'";
}
// HUOM HUOM: eth�n muuta n�it� ilman, ett� korjaat my�s yllapito.php:ss� iframen lukitse_avaimeen muuttujat
$hakukentat = "tunnus, toimittaja, ytunnus, tuoteno, ryhma, alennus, alennuslaji, minkpl, alkupvm/loppupvm";

$jarjestys = 'ytunnus, tuoteno, ryhma, minkpl, alkupvm, loppupvm';

if ($nayta_eraantyneet != "") {
	$rajauslisa = " and toimittajaalennus.loppupvm > '0000-00-00' and toimittajaalennus.loppupvm < now() ";
}

?>