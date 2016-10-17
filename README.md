# httpclient

Parameters: \<host> \<port> \<method> \<path> \<number of requests> [\<data> [\<headers>]]

Examples:

POST: ./client 45.62.226.182 80 POST /cgi-bin/action.php 2 "item1=6" "Content-Type: application/x-www-form-urlencoded"

GET: ./client 45.62.226.182 80 GET / 4
