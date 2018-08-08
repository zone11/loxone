<?php
	require_once("./vendor/Netatmo-API-PHP/src/Netatmo/autoload.php");
	use Netatmo\Clients\NAWSApiClient;
	use Netatmo\Exceptions\NAClientException;

	date_default_timezone_set("Europe/Zurich");

	$config = array();
	$config['client_id'] = 'enter your netatmo client id';
	$config['client_secret'] = 'enter your netatmo client secret';
	$config['username'] = 'enter your netatmo username';
	$config['password'] = 'enter your netatmo password';
	$config['scope'] = 'read_station';

	$client = new NAWSApiClient($config);
	$tokens = $client->getAccessToken();
	
	// Grab data
	$data = $client->getData(NULL, TRUE);
	
	// Echo module 0 data (the outdoor sensor)
	printf("Aussen.Temp.Value=%.01lf\n",$data['devices'][0]['modules'][0]['dashboard_data']['Temperature']);
	printf("Aussen.Temp.Trend=%s\n",$data['devices'][0]['modules'][0]['dashboard_data']['temp_trend']);
	printf("Aussen.Hum.Value=%.01lf\n",$data['devices'][0]['modules'][0]['dashboard_data']['Humidity']);
?>