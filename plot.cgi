#!/usr/bin/python3.6
import cgi
import cgitb; cgitb.enable()  # for troubleshooting
import os
print("Content-type: text/html\r\n\r")
arguments = cgi.FieldStorage()
dirname = arguments["directory"].value

pid = os.fork()
if (pid == 0):
	os.execl("./my-histogram", "my-histogram", dirname)
else:
	os.waitpid(pid, 0)
# "histogram.png" should now exist

# pretty prints
print("""
<!DOCTYPE HTML>
<!--- Centers an image called "histogram.png" for our CS410 Webserver --->
<HTML>
   <HEAD>
	<style>
		img {
			display: block;
			margin-left: auto;
			margin-right: auto;
		}
	</style>
   	<TITLE>Histogram</TITLE>
   </HEAD>
   <BODY>
      	<h1 style="color: red; font-size: 16pt; text-align: center;">CS410 Webserver</h1>
	<br>
	<img src="histogram.png" alt="File frequency">
   </BODY>
</HTML>
""")
