<?php
	// Katsotaan tyyppi $tilausrivityyppi kent�st� ja annetaan se selv�kielisen� takaisin

	switch ($tilausrivityyppi) {
		case "0":
			$tilausrivityyppi = "Sopimus";
			break;
		case "B":
			$tilausrivityyppi = "Ostoskori";
			break;
		case "D":
			$tilausrivityyppi = "Mit�t�ity";
			break;
		case "E":
			$tilausrivityyppi = "Enakkotilaus";
			break;
		case "F":
			$tilausrivityyppi = "Web-tilaus/tarjous";
			break;
		case "G":
			$tilausrivityyppi = "Varastosiirto";
			break;
		case "L":
			$tilausrivityyppi = "Myynti";
			break;
		case "M":
			$tilausrivityyppi = "Valmistus/Valmiste (Rekursiivinen)";
			break;
		case "N":
			$tilausrivityyppi = "D15 konversio";
			break;
		case "O":
			$tilausrivityyppi = "Osto";
			break;
		case "T":
			$tilausrivityyppi = "Tarjous";
			break;
		case "V":
			$tilausrivityyppi = "Valmistus/Raaka-aine";
			break;
		case "W":
			$tilausrivityyppi = "Valmistus/Valmiste";
			break;
		case "Z":
			$tilausrivityyppi = "Terminaalitoimitus";
			break;
	}

?>