#!/usr/bin/env python3
import os
import json

print("Content-Type: text/html")
print()

print("""
<!DOCTYPE html>
<html>
<head>
    <title>Session Manager</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .info { background: #e3f2fd; padding: 20px; border-radius: 5px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="info">
        <h2>Session Info</h2>
        <p>Sessions and cookies are managed by the server. This CGI does not set cookies.</p>
    </div>
    <p><a href="/www/cookie_demo.html">← Back to Cookie Demo</a></p>
</body>
</html>
""") 