<?php
	$semaphore = sem_get(1);
	// wait to unlock
	if (sem_acquire($semaphore)) {
    		$servername = "localhost";
		$username = "mvibot";
		$password = "Mvibot@v1";
		$dbname = "mvibot_database";
    		//
		$conn = new mysqli($servername, $username, $password, $dbname);
		if ($conn->connect_error) {
  			die("Connection failed: " . $conn->connect_error);
		}
		// get json string
		$json = file_get_contents('php://input');
		$data_json = json_decode($json);
		//
		$emparray = array();
		$data="";
		if(!($data_json->name_seri === NULL)){
			// check gpio_moudle_v2
			$sql = "SELECT `name_seri` FROM my_module_gpio_v2";
			$result = $conn->query($sql);
			$is_have= 0;
			if ($result->num_rows > 0) {
				// output data of each row
				while($row = $result->fetch_assoc()) {
					if($row["name_seri"] == $data_json->name_seri){
						$is_have=1;
					}
				}
			}
			if($is_have == 0 ){
				// insert module
				$sql = "INSERT INTO my_module_gpio_v2 (name_seri)  VALUES('$data_json->name_seri')";
				$result = $conn->query($sql);
			}else{
				//update status
				$sql = "UPDATE `my_module_gpio_v2` set status='1' where name_seri='$data_json->name_seri'";
				$result = $conn->query($sql);
				// update input
				$sql = "UPDATE `my_module_gpio_v2` set input_status_string='$data_json->name_seri";
				for ($i = 0; $i <= (int)$data_json->num_in-1; $i++) {
						$n=$i+1;
						$k="in$n";
						$m=$data_json->$k;
						$sql=$sql."|in$n:$m";
				}
				$sql=$sql."' where name_seri='$data_json->name_seri'";
				$result = $conn->query($sql);
				// update input
				$sql = "UPDATE `my_module_gpio_v2` set output_status_string='$data_json->name_seri";
				for ($i = 0; $i <= (int)$data_json->num_out-1; $i++) {
						$n=$i+1;
						$k="out$n";
						$m=$data_json->$k;
						$sql=$sql."|out$n:$m";
				}
				$sql=$sql."' where name_seri='$data_json->name_seri'";
				$result = $conn->query($sql);
				// update battery
				$battery_string=$data_json->battery." date:".date('H:i:s');
				$sql="UPDATE `my_module_gpio_v2` set battery='$battery_string";
				$sql=$sql."' where name_seri='$data_json->name_seri'";
				$result = $conn->query($sql);
				// update output_user_set_string_fesp_json
				$output_user_set_string_fesp_json=$data_json->output_user_set_string_fesp;
				if(!($output_user_set_string_fesp_json === NULL)){
					if(!(empty($output_user_set_string_fesp_json))){
						$sql = "UPDATE `my_module_gpio_v2` set output_user_set_string_fesp='".$output_user_set_string_fesp_json."'";
						$sql = $sql."  where name_seri='$data_json->name_seri'";
						$result = $conn->query($sql);
					}
				}
			}
		}
		// read input table
 		$sql = "SELECT *  FROM input_user_status";
                $result = $conn->query($sql);
		if ($result->num_rows > 0) {
 			 // output data of each row
  			while($row = $result->fetch_assoc()) {
				$data=$data."($row[name_seri]|";
				for ($x = 1; $x <= 30; $x++) {
 					if($row["in$x"] === NULL){
						break;
					}
					else{
						$data=$data."~in$x=";  
						$data=$data.$row["in$x"]."~";
					}
				}
				$data=$data.")";
  			}
			$emparray["input_table"]=$data;
		} else {
  			echo "0 results";
		}
		// read out put table
		$data="";
		$sql = "SELECT *  FROM output_user_status";
		$result = $conn->query($sql);
		if ($result->num_rows > 0) {
			// output data of each row
			while($row = $result->fetch_assoc()) {
					$data=$data."($row[name_seri]|";
					for ($x = 1; $x <= 30; $x++) {
							if($row["out$x"] === NULL){
									break;
							}
							else{
									$data=$data."~out$x=";  
									$data=$data.$row["out$x"]."~";
							}
					}
					$data=$data.")";
			}
			$emparray["output_table"]=$data;
		} else {
				echo "0 results";
		}
		// read output set string
		$data="";
		$sql = "SELECT `output_user_set_string`  FROM `my_module_gpio_v2` where name_seri='$data_json->name_seri'";
		$result = $conn->query($sql);
		if ($result->num_rows > 0) {
				// output data of each row
				while($row = $result->fetch_assoc()) {
					$data=$row["output_user_set_string"];
					if(!($data === NULL)){
						if(!(empty($data))){
							$emparray["output_user_set_string"]=$data;
						}
					}
				}

		} else {
				echo "0 results";
		}
		$sql ="UPDATE `my_module_gpio_v2` set output_user_set_string='' where name_seri='$data_json->name_seri'";
		$result = $conn->query($sql);
		// read mission data
		$data="";
		$sql = "SELECT `mission_normal`  FROM `my_module_gpio_v2` where name_seri='$data_json->name_seri'";
		$result = $conn->query($sql);
		if ($result->num_rows > 0) {
			// output data of each row
			while($row = $result->fetch_assoc()) {
				$data=$row["mission_normal"];
				if(!($data === NULL)){
					if(!(empty($data))){
						$emparray["mission_normal"]=$data;
					}
				}
			}
		} else {
			echo "0 results";
		}
		$sql ="UPDATE `my_module_gpio_v2` set mission_normal='' where name_seri='$data_json->name_seri'";
		$result = $conn->query($sql);
		// read mission data backup
		if(!($data_json->frist_time_get_mission === NULL)){
			$data="";
                	$sql = "SELECT `mission_normal_backup`  FROM `my_module_gpio_v2` where name_seri='$data_json->name_seri'";
                	$result = $conn->query($sql);
                	if ($result->num_rows > 0) {
                        	// output data of each row
                        	while($row = $result->fetch_assoc()) {
                                	$data=$row["mission_normal_backup"];
                                	if(!($data === NULL)){
                                        	if(!(empty($data))){
                                                	$emparray["mission_normal_backup"]=$data;
                                        	}
                                	}
                        	}
                	} else {
                       		echo "0 results";
                	}
		}
		//
		$emparray["date"]=date('l d/m/y');
		$emparray["time_h"]=date('H');
		$emparray["time_m"]=date('i');
		$emparray["time_s"]=date('s');
		//
		echo json_encode($emparray);
		sem_release($semaphore);
	} else {
		echo "Request is  Lock so exit";
	}

?>
