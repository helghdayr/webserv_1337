#!/usr/bin/env python3
import os

print("Content-Type: text/html")
print()

print("""
<!DOCTYPE html>
<html>
<head>
    <title>Cookie Demo</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .info { background: #e7f1ff; padding: 20px; border-radius: 5px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="info">
        <h2>Cookie Demo</h2>
        <p>This CGI no longer sets a session cookie. Sessions are managed by the server.</p>
    </div>
</body>
</html>
""") 