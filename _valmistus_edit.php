<div class='head'><?php echo $title ?></div>

<form method='POST'>
<input type='hidden' name='tee' value='update'>
<input type='hidden' name='tunnus' value='<?php echo $valmistus->tunnus() ?>'>
<input type='hidden' name='tila' value='<?php echo $tila ?>'>

<table>
	<tr>
		<th>Valmistus</th>
		<td colspan='3'><?php echo $valmistus->tunnus() ?></td>
	</tr>
	<tr>
		<th>Kommentti</th>
		<td colspan='3'><input type='text' size='40'></td>
	</tr>

	<tr>
		<th>Valmiste</th>
		<th>M��r�</th>
		<th>Valmistettava M��r�</th>
		<th>Ylity�tunnit</th>
	</tr>

	<?php foreach($valmistus->tuotteet() as $valmiste) { ?>
		<tr>
			<td><?php echo $valmiste['nimitys'] ?></td>
			<td><?php echo $valmiste['varattu'] ?></td>
			<td><input type='text' name='valmisteet[<?php echo $valmiste['tuoteno'] ?>][maara]' value='<?php echo $valmiste['varattu'] ?>'></td>
			<td><input type='text' name='valmisteet[<?php echo $valmiste['tuoteno'] ?>][tunnit]'></td>
		</tr>
	<?php } ?>

</table>

<a href='valmistuslinjojen_tyojonot.php'>Takaisin</a>
<input type='submit' value='Valmis'>

</form>

