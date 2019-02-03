# MatrixDisplayESP8266 - welche Schritte sind notwendig

Anleitung zum Zusammenbau sind im Orginal-Projekt zu finden. Zusätlich kann noch diese Seite helfen: https://www.schellenberger.biz/matrix-display-fuer-homematic-im-nachbau/

**aus dem Orginal:**
```diff
+ Hardware zusammenbauen
+ Arduino Software installieren
+ download Software für Sketch (von diesem Projekt !)
+ libriary Folder entpacken und an die richtige Stelle kopieren
+ MAX_DEVICES in Datei "MatrixDisplayESP8266.ino" anpassen!!!
+ Flashen 
+ konfigurieren wie im Orginal beschrieben - beim Einschalten des WESP muss einer der Tasten auf gedrückt werden (mit GND überbrücken)
```

**Zusätzlich:**
```diff
+ 2 Datenpunkte im IOBroker anlegen (für Daten und Setting) 
+ Datenpunkte befüllen Beispiel für Setting: 17;1;15;2;16 - sonst ERROR Message auf Display zu sehen
+ individuelle IOBroker-Scripts erstellen
```
Das war es auch schon.
