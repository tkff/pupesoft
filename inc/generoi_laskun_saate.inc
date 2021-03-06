<?php

if (!function_exists("generoi_laskun_saate")) {
	function generoi_laskun_saate ($lasrow, $saatekirje = '', $kieli = '',$toim = '') {
		global $kukarow, $yhtiorow;

		$content_body 		= "";
		$content_subject 	= "";
		$email_to 			= "";
		$saatekirje 		= mysql_real_escape_string($saatekirje);
		$kieli 				= mysql_real_escape_string($kieli);

		$query_lasku_email = "	SELECT lasku_email, kieli
								FROM asiakas
								WHERE yhtio = '$kukarow[yhtio]'
								AND tunnus = '$lasrow[liitostunnus]'
								AND lasku_email != ''";
		$result_lasku_email = mysql_query($query_lasku_email) or pupe_error($query_lasku_email);

		if ($row_lasku_email = mysql_fetch_array($result_lasku_email)) {
			$email_to = "asiakasemail".$row_lasku_email["lasku_email"];
		}
		else {
			return array("", "", "");
		}

		if ($kieli == "") {
			$kieli = $row_lasku_email["kieli"];
		}

		if ($kieli == "") {
			$kieli = $yhtiorow["kieli"];
		}

		if ($saatekirje != '') {
			$lisa = " AND tunnus = '$saatekirje' ";
		}
		else {
			$lisa = " ORDER BY jarjestys LIMIT 1";
		}

		if ($toim == 'EXTRANET_REKLAMAATIO') {
			$query = "	SELECT *
						FROM avainsana
						WHERE yhtio = '$kukarow[yhtio]'
						AND laji = 'PALAUTUS_SAATE'
						AND kieli = '$kieli' $lisa";
			$result = mysql_query($query) or pupe_error($query);
		}
		else {
			$query = "	SELECT *
						FROM avainsana
						WHERE yhtio = '$kukarow[yhtio]'
						AND laji = 'LASKUTUS_SAATE'
						AND kieli = '$kieli' $lisa";
			$result = mysql_query($query) or pupe_error($query);

			if (mysql_num_rows($result) == 0) {
				$query = "	SELECT *
							FROM avainsana
							WHERE yhtio = '$kukarow[yhtio]'
							AND laji = 'LASKUTUS_SAATE' $lisa";
				$result = mysql_query($query) or pupe_error($query);
			}
		}

		if ($row = mysql_fetch_array($result)) {

			$content_subject	= $row["selitetark"];
			$content_body 		= $row["selitetark_2"];

			foreach ($lasrow as $key => $val) {

				if (preg_match_all("/\[($key)\]/", $content_subject, $match)) {

					foreach ($match[0] as $osuma) {

						if (stristr($val, "-")) {
							list($year, $month, $day) = explode("-", $val);

							if (checkdate($month, $day, $year)) {
								$val = tv1dateconv($val);
							}
						}
						$content_subject = str_replace($osuma, "$val", $content_subject);
					}
				}

				if (preg_match_all("/\[($key)\]/", $content_body, $match)) {

					foreach ($match[0] as $osuma) {

						if (stristr($val, "-")) {
							list($year, $month, $day) = explode("-", $val);

							if (checkdate($month, $day, $year)) {
								$val = tv1dateconv($val);
							}
						}
						$content_body = str_replace($osuma, "$val", $content_body);
					}
				}
			}
		}

		return array($email_to, $content_subject, $content_body);
	}
}

?>