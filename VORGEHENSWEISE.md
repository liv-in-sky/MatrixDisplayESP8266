# MatrixDisplayESP8266 - welche Schritte sind notwendig

Anleitung zum Zusammenbau sind im Orginal-Projekt zu finden. Zusätlich kann noch diese Seite helfen: https://www.schellenberger.biz/matrix-display-fuer-homematic-im-nachbau/


WICHTIG
- In Arduino Voreinstellungen unbedingt http://arduino.esp8266.com/versions/2.4.1/package_esp8266com_index.json als "Zusätzliche Boardverwalter-URL einsetzen !!!!!!

**aus dem Orginal:**
```diff
+ Hardware zusammenbauen
+ Arduino Software installieren
+ download Software für Sketch (von diesem Projekt !)
+ libriary Folder entpacken und an die richtige Stelle kopieren
+ MAX_DEVICES in Datei "MatrixDisplayESP8266.ino" anpassen!!!
+ Flashen 
+ konfigurieren wie im Orginal beschrieben - beim Einschalten des ESP muss einer der Tasten  gedrückt werden (mit GND überbrücken, falls keine Taster vorhanden)
```

**Zusätzlich:**
```diff
+ 2 Datenpunkte im IOBroker anlegen (für Daten und Setting) 
+ Datenpunkte befüllen Beispiel für Setting: 17;1;15;2;16 - sonst ERROR Message auf Display zu sehen
+ individuelle IOBroker-Scripts erstellen
```
Das war es auch schon.
