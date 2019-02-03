# MatrixDisplayESP8266 fur IOBroker optimiert

Ich benutze IOBroker - es kann aber jeder WebServer, der  "httpget" empfangen kann, genutzt werden.

**folgende �nderungen zum Orginal:**
```diff
+ Einf�hrung von Modi
+ Schalter optional (durch IOBroker ausf�hrbar)
+ Standard(Config)-Werte (durch IOBroker ausf�hrbar)
+ Sonderzeichen und Umlaute werden unterst�tzt 
+ Fehleranzeige bei falschen Setting-Daten
```
Kurzes Video mit Vorf�hrung: https://youtu.be/VW0sVTzDxuE

ein weiteres Video zeigt nur die verschiedenen Modi: https://youtu.be/n5MVQeNUJ1E

Folgende Modi sind vorhanden:

- 1  - alle Daten - kommt von rechts und geht nach oben - mit Uhrzeit
- 2  - nur Uhrzeit - feststehend
- 3  - alle Daten - laufen von rechts nach links durch - ohne Uhrzeit
- 4  - alle Daten - kommen von rechts - gehen wieder nach rechts - mit Uhrzeit
- 5  - alle Daten - laufen von rechts nach links durch - mit Uhrzeit
- 6  - erster Datensatz - feststehend (zu lange Nachrichten werden abgeschnitten) (nur erster Wert!) - ohne Uhrzeit
- 7  - alle Daten - feststehend und blinkend (zu lange Nachrichten werden abgeschnitten) - ohne Uhrzeit
- 8  - alle Daten - kommen von oben - gehen wieder nach unten - mit Uhrzeit
- 9  - Display aus
- 10 - alle Daten - GrowUp - GrowDown - mit Uhrzeit
- 11 - alle Daten - kommen von links oben - GrowDown - ohne Uhrzeit
- 12 - alle Daten - kommen von links unten - geht nach recht unten - mit Uhrzeit
- 13 - alle Daten - Text wird aus einzelnen Pixeln zusammengesetzt - mit Uhrzeit
- 14 - alle Daten - Daten werden vom Zentrum erzeugt und gel�scht - mit Uhrzeit
- 15 - alle Daten - Daten werden von links nach rechts erzeugt und gel�scht - mit Uhrzeit
- 16 - alle Daten - einzelne Pixel zu Text zusammengef�hrt und PACMAN "entsorgt" Text - mit Uhrzeit
- 17 - alle Daten - PACMAN bringt Text und "entsorgt" Text - mit Uhrzeit
- 18 - alle Daten - Disolve - alle Pixel leuchten - dann Text - mit Uhrzeit
- 19 - alle Daten - Cursur - Daten werden von Cursor gef�hrt - mit Uhrzeit
- 20 - alle Daten - Blinds - eine Art Vorhang - �hnlich Mode 18 - mit Uhrzeit
- 21 - alle Daten - Random  - einzelne Pixel werden zu Text zusammengef�hrt - mit Uhrzeit

Datumsanzeige: Ob es eine Datumsanzeige gibt, wird durch die Anzahl der verbauten Matrix-Module besimmt. Bei 2 * 4 Modulen (MAX_DEVICES=8) wird das Datum �bersprungen. Erst ab 3 * 4 Modulen (MAX_DEVICES=12) wird auch das Datum neben der Uhrzeit angezeigt. 
Der MAX_DEVICES - Wert muss VOR der Programmierung des Wemos in das Arduino-Sketch-File (MatrixDisplayESP8266.ino) eingegeben werden.

![Sampleum](/MatrixDisplayESP8266/images/test-ani-gif9.gif)

WICHTIG: Umlaute funktionieren - die ZIP-Datei im libraries-Ordner muss in den Arduino-Sketch-libraries-Ordner entpackt werden damit alles richtig kompiliert werden kann (u.U. m�ssen noch weitere Bibliotheken geladen werden))

![Sampleum](/MatrixDisplayESP8266/images/uml-ani-gif10.gif)

![Sampleum](/MatrixDisplayESP8266/images/sonderzeichen.gif)



Die beiden Schalter f�r Helligkeit und Modi�nderung (im Orginal) wurden als Abfragepunkte eingef�hrt und sind durch IOBroker steuerbar - IOBroker dominiert das Setting und �berschreibt die Schalter nach der definierten Zeit! Die Schalter sind optional. Die Modi sind von 1 bis 21. Intensity (Helligkeit) geht von 1 bis 10. Durch die Dominanz des IOBroker machen die Schalter keinen wirklichen Sinn - werden nur f�r Config-Mode gebraucht (erstmaliges Konfigurieren des Wemos D1 mini)

Auch die Standardwerte, welche beim ersten Einrichten angegeben werden m�ssen, k�nnen bzw. m�ssen �bergeben werden (von  z.B. IOBroker). ScrollPause, ScrollSpeed und der Wert, wie oft man die Daten abholen will (Refresh Time). ScrollSpeed:  1 bis 300; ScrollPause: 1 bis 30(sek); RefreshTime (erzeugt Taffic im Netz!): 1 bis 600 = 1 sek bis 10 min).
Die Daten werden wie im Orginal im folgenden Format �begeben:  xxx;yyy;zzz
Die Standard(Config)-Werte werden wie die zu anzeigenden Daten �bergeben. Z.B 5;1;60;5;20 . In der Reihenfolge bedeutend: Mode, Intensity(Helligkeit);RefreshTime;ScrollPause;ScrollSpeed

Im IOBRoker muss der Simple-Api-Adapter installiert sein - in diesem Beispiel mit dem Standardport 8087.

Die Einrichtung ist wie im Orginal - es kommt eine weitere Url-Abfragen dazu.

Url f�r Daten werden wie Orginal angelegt - z.B.:
- http://192.168.xxx.xxx/getPlainValue/control-own.0.ESPMatrix.Matrix (IOBroker-Baum: control-own.0.ESPMatrix.**Matrix**)
anschliessend m�ssen noch ein weiterer Datenpunkt in IOBroker angelegt werden. Diese muss notwendigerweise im selben Vewrzeichnis sein und wie folgt benannt werden:
- control-own.0.ESPMatrix.**MatrixSetting** (w�hlt man anstatt "Matrix" z.B. Display -> muss der Wert mit DisplaySetting benannt werden - siehe anim. Gif - ertes Bild)

Im Config-Mode des Wemos D1 wird nur eine URL angegeben (DatenURL). Bei richtiger Bennenung des zweiten Datenpunktes, wird er automatisch gefunden ! 

```diff
-                !!!      ALLE DATENPUNKTE IM IOBROKER M�SSEN ALS TEXTFELDER DEFININIERT SEIN     !!!
```
Eine weitere Hilfe f�r den Bau ist auf folgender Seite