<?PHP
  $company=$_GET['company'];				//get the company symbol from the URL
  $change=$_GET['change'];					//get the change percentage from the URL
  $visitor_email = "mishra14@gmail.com";	//recipient email address
  $message = "Hi, \nThis is an automated alert regarding the stock of $company. The prices are down by $change % since yesterday. It is advisable to buy the stock.\n\nThanks\nStock Pebble App";		//email body
  $sub = "Pebble Alert - buy stock";		//subject for the email
  $frm = "alerts@stockpebble.herebo.com";	//the from email address
  $hdr = "From: ".$frm;						// add the from email address to the headers
  $sentOK = 0;								//set the flag to 0
  if (mail($visitor_email, $sub, $message, $hdr)) 			//call the mail function to generate the email
  { 
      $sentOK = 1; 										//if the email is sent then set the flag to 1
  }
  $result = array('AlertStatus', $sentOK);				//set the result to indicate the status of the sent email
  print json_encode($result);							//return the result as a json object
 
?>		