using namespace std;
String index_page = "\
    <html>\n\
    <body>\n\
        <h1 style=\"background-color:powderblue;font-size:300%;text-align:center;\">Setting for Module GPIO!</h1>\n\
        <input type='number' id='number1' min=\"0\" max=\"255\" value=\"0\" style=\"width:64px;height:32px;text-align:center; font-size:120%; position: relative; margin: 0 5px;\">\n\
        <input type='number' id='number2' min=\"0\" max=\"255\" value=\"0\" style=\"width:64px;height:32px;text-align:center; font-size:120%; position: relative; margin: 0 5px;\">\n\
        <input type='number' id='number3' min=\"0\" max=\"255\" value=\"0\" style=\"width:64px;height:32px;text-align:center; font-size:120%; position: relative; margin: 0 5px;\">\n\
        <input type='number' id='number4' min=\"0\" max=\"255\" value=\"0\" style=\"width:64px;height:32px;text-align:center; font-size:120%; position: relative; margin: 0 5px;\">\n\
        <br><br><button onclick='sendData()'>Save</button>\n\
        <script>\n\
            function sendData() {\n\
              const value1=document.getElementById('number2').value; \n\
              fetch('/senddata', {\n\
                  method: 'POST',\n\
                  headers: {\n\
                      'Accept': 'application/json',\n\
                      'Content-Type': 'application/json'\n\
                  },\n\
                  body: JSON.stringify({ \"id\": value1 })\n\
              })\n\
                 .then(response => response.text())\n\
                 .then(response => alert(response))\n\
            }\n\
            function fetchData() {\n\
                fetch('/data')\n\
                .then(response => response.text())\n\
                .then(data => {\n\
                      document.getElementById('number1').value = data;\n\
                });\n\
            }\n\
            setInterval(fetchData, 1000);\n\
        </script>\n\  
    </body>\n\
    </html>\n\
";
