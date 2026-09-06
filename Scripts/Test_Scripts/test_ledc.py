import urllib.request
html = urllib.request.urlopen("https://raw.githubusercontent.com/crankyoldgit/IRremoteESP8266/master/src/IRsend.cpp").read().decode("utf-8")
print([line for line in html.split('\n') if 'ledc' in line or 'digitalWrite' in line][:20])
