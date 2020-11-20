        strLED1 = "";
        strLED2 = "";
        var LED2_state = 0;

        /* track if the deregister button has been pressed */
        var deregister = "";

        function GetArduinoIO()
        {
            console.log("test2");
            nocache = "&nocache=" + Math.random() * 1000000;
            var request = new XMLHttpRequest();
            request.onreadystatechange = function()
            {
                console.log("test1");
                if (this.readyState == 4) {
                    if (this.status == 200) {
                        console.log("response received: " + this.responseXML);
                        if (this.responseXML != null) {
                            /* valid XML file received*/
                            
                            let registered = this.responseXML.getElementsByTagName('registered')[0].childNodes[0].nodeValue;
                            if (registered === "true") { /* registered */
                                document.getElementById("deregister").removeAttribute("disabled");
                                document.getElementById("reg-status").innerHTML = "Registration Status: registered";

                                let ardID = this.responseXML.getElementsByTagName('ardID')[0].childNodes[0].nodeValue;
                                let raspyID = this.responseXML.getElementsByTagName('raspyID')[0].childNodes[0].nodeValue;
                                let raspyIP = this.responseXML.getElementsByTagName('raspyIP')[0].childNodes[0].nodeValue;
                                console.log("registered: " + registered + ", ardID: " + ardID + ", raspyID: " + raspyID + "raspyIP: " + raspyIP);
                                
                                document.getElementById("ardid").innerHTML = "ArdID    : " + ardID;
                                document.getElementById("raspyid").innerHTML = "RaspyID  : " + raspyID;
                                document.getElementById("raspyip").innerHTML = "raspyIP  : " + raspyIP;
                            }
                            else {
                                //$("deregister").attr("disabled", "disabled"); // disable button
                                document.getElementById("deregister").setAttribute("disabled", "disabled");
                                document.getElementById("reg-status").innerHTML = "Registration Status: not registered";
                                document.getElementById("ardid").innerHTML = "ardID    : N/A";
                                document.getElementById("raspyid").innerHTML = "raspyID  : N/A";
                                document.getElementById("raspyip").innerHTML = "raspyIP  : N/A";
                                   
                            }

                            /* uptime */
                            let receivedUptime = this.responseXML.getElementsByTagName('uptime')[0].childNodes[0].nodeValue;
                            document.getElementById("uptime").innerHTML = "uptime  : " + msToTime(receivedUptime);

                        }
                    }
                }
            }
            /* send HTTP GET request with LEDs to switch on/off if any */
            let requestURL = "ajax_inputs" + deregister + nocache;
            request.open("GET", "ajax_inputs" + requestURL, true);
            request.send(null);
            setTimeout('GetArduinoIO()', 6000);

            deregister = "";

        }
        // service LEDs when checkbox checked/unchecked
        function GetCheck()
        {

        }
        function GetButton1()
        {
            deregister = "&dereg=1";
        }

        function msToTime(duration) {
            var milliseconds = parseInt((duration % 1000) / 100),
            seconds = Math.floor((duration / 1000) % 60),
            minutes = Math.floor((duration / (1000 * 60)) % 60),
            hours = Math.floor((duration / (1000 * 60 * 60)) % 24);

            hours = (hours < 10) ? "0" + hours : hours;
            minutes = (minutes < 10) ? "0" + minutes : minutes;
            seconds = (seconds < 10) ? "0" + seconds : seconds;

            return hours + ":" + minutes + ":" + seconds + "." + milliseconds;
        }