<div class='head'><?php echo t("Keskeyt� ty�") ?></div>

<form method='POST'>
<input type='hidden' name='tee' value='update'>
<input type='hidden' name='tunnus' value='<?php echo $valmistus->tunnus() ?>'>
<input type='hidden' name='tila' value='<?php echo $tila ?>'>

<table>
	<tr>
		<th>Valmistus</th>
		<td><?php echo $valmistus->tunnus() ?></td>
	</tr>
	<tr>
		<th>Ylity�tunnit</th>
		<td><input type='text' name='ylityotunnit'></td>
	</tr>
	<tr>
		<th>K�ytetyt tunnit</th>
		<td><input type='text' name='kaytetyttunnit'></td>
	</tr>
	<tr>
		<th>Kommentti</th>
		<td><input type='text' name='kommentti'></td>
	</tr>

	<tr>
		<th>Valmiste</th>
		<th>M��r�</th>
	</tr>

	<?php foreach($valmistus->tuotteet() as $valmiste) { ?>
		<tr>
			<td><?php echo $valmiste['nimitys'] ?></td>
			<td><?php echo $valmiste['varattu'] ?></td>
		</tr>
	<?php } ?>

</table>

<a href='valmistuslinjojen_tyojonot.php'>Takaisin</a>
<input type='submit' value='Valmis'>

</form>

