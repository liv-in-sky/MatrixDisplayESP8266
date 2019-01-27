# MatrixDisplayESP8266 fur IOBroker optimiert

Ich benutze IOBroker - es kann aber jeder WebServer, der  "httpget" empfangen kann, genutzt werden.

**folgende Änderungen zum Orginal:**
```diff
+ Einführung von Modi
+ Schalter optional (durch IOBroker ausführbar)
+ Standard(Config)-Werte (durch IOBroker ausführbar)
+ Sonderzeichen und Umlaute werden unterstützt 
+ Fehleranzeige bei falschen Setting-Daten
```


Folgende Modi sind vorhanden:

- 1 - wie Orginal - Text (Daten) kommt von rechts und geht nach oben - mit Uhrzeit
- 2 - nur Uhrzeit - feststehend
- 3 - Daten laufen von rechts nach links durch - ohne Uhrzeit
- 4 - Daten kommen von rechts - gehen wieder nach rechts - mit Uhrzeit
- 5 - Daten laufen von rechts nach links durch mit Uhrzeit
- 6 - Daten feststehend (zu lange Nachrichten werden abgeschnitten) (nur ein Wert)
- 7 - Daten feststehend und blinkend (zu lange Nachrichten werden abgeschnitten) - abwechselnd bei mehreren Daten
- 8 - Daten kommen von oben - gehen wieder nach unten - mit Uhrzeit
- 9 - Display aus

Datumsanzeige: Ob es eine Datumsanzeige gibt, wird durch die Anzahl der verbauten Matrix-Module besimmt. Bei 2 * 4 Modulen (MAX_DEVICES=8) wird das Datum übersprungen. Erst ab 3 * 4 Modulen (MAX_DEVICES=12) wird auch das Datum neben der Uhrzeit angezeigt. Der MAX_DEVICES - Wert muss VOR der Programmierung des Wemos in das Arduino-Sketch-File (MatrixDisplayESP8266.ino) eingegeben werden.

![Sampleum](/MatrixDisplayESP8266/images/test-ani-gif9.gif)

WICHTIG: Umlaute funktionieren - die ZIP-Datei im libraries-Ordner muss in den Arduino-Sketch-libraries-Ordner entpackt werden damit alles richtig kompiliert werden kann (u.U. müssen noch weitere Bibliotheken geladen werden))

![Sampleum](/MatrixDisplayESP8266/images/uml-ani-gif10.gif)

Die beiden Schalter für Helligkeit und Modiänderung wurden als Abfragepunkte eingeführt und sind durch IOBroker steuerbar - IOBroker dominiert das Setting und überschreibt die Schalter nach der definierten Zeit! Die Schalter sind optional.Die Modi sind von 1 bis 9. Intensity (Helligkeit) geht von 1 bis 10.

Auch die Standardwerte, welche beim ersten Einrichten angegeben werden müssen, können übergeben werden (von  z.B. IOBroker). ScrollPause, ScrollSpeed und der Wert, wie oft man die Daten abholen will (Refresh Time). ScrollSpeed:  0 bis 300; ScrollPause: 0 bis 30(sek); RefreshTime (erzeugt Taffic im Netz!): 1 bis 600 = 1 min bis 10 min).
Die Daten werden wie im Orginal im folgenden Format übegeben:  xxx;yyy;zzz
Die Standard(Config)-Werte werden wie die zu anzeigenden Daten übergeben. Z.B 5;1;60;5;20 . In der Reihenfolge bedeutend: Mode, Intensity(Helligkeit);RefreshTime;ScrollPause;ScrollSpeed

Im IOBRoker muss der Simple-Api-Adapter installiert sein - in diesem Beispiel mit dem Standardport 8087.

Die Einrichtung ist wie im Orginal - es kommt eine weitere Url-Abfragen dazu.

Url für Daten werden wie Orginal angelegt
- http://192.168.xxx.xxx/getPlainValue/control-own.0.ESPMatrix.Matrix (IOBroker-Baum: control-own.0.ESPMatrix.**Matrix**)
anschliessend müssen noch ein weiterer Datenpunkt in IOBroker angelegt werden. Diese muss notwendigerweise im selben Vewrzeichnis sein und wie folgt benannt werden:
- control-own.0.ESPMatrix.**MatrixSetting**

Im Config-Mode des Wemos D1 wird nur eine URL angegeben (DatenURL). Bei richtiger Bennenung des zweiten Datenpunktes, wird er automatisch gefunden ! 

```diff
-                !!!      ALLE DATENPUNKTE IM IOBROKER MÜSSEN ALS TEXTFELDER DEFININIERT SEIN     !!!
```
Eine weitere Hilfe für den Bau ist auf folgender Seite zu finden:

https://www.schellenberger.biz/matrix-display-fuer-homematic-im-nachbau/

und im IOBroker Forum: 

https://forum.iobroker.net/viewtopic.php?f=35&t=20891&p=219001#p219001

```diff
+Für Interessierte: mit Tasker kann man das Display via Handy und (durch) IOBroker steuern. 
Im IOBRoker muss der Simple-Api-Adapter installiert sein - in diesem Beispiel mit dem Standardport 8087.

Anschliessend einen Task mit http Get hinzufügen:
Server:Port:  "192.168.xxx.xxx:8078"
Path: "set/control-own.0.ESPMatrix.Matrix?value=yyy"
Mime Type: "text/*"
und schon kann man gewünschte Daten (z.B. %BATT anstatt yyy) wie Handy-Akku an das Display senden.
```
```diff
Hinweis: Es gibt eine Variante mit MQTT : 
```
https://github.com/prysme01/DotMatrixDisplay


#    -----  von jp112sd übernommen    ----------




![Sample1](/MatrixDisplayESP8266/images/Sample1.png)


## benötigte Hardware
* 1x Wemos D1 mini (z.B.: http://www.ebay.de/itm/172357074201)
* 2x Dot Matrix Modul 4 8x8 1088AS Matrix mit MAX7219-Treiber (z.B.: http://www.ebay.de/itm/232176384928)
* 2x Taster (beliebig... irgendwas, das beim Draufdrücken schließt :smiley:)

## Verdrahtung
DotMatrix | Wemos D1
------------ | -------------
VCC       | +5V
GND       | GND
DIN       | D7
CLK       | D5
CS        | D8
Taste1 | D1
Taste2 | D2

Die Taster an D1 und D2 sind gegen GND zu schalten. *(Pullup-Widerstand wird nicht benötigt)*

![Konfiguration1](/MatrixDisplayESP8266/images/Back1.jpg)

## Programmierung 

Wie der Wemos D1 (Mini) angeschlossen und in die Arduino IDE integriert wird, ist hier gut beschrieben:

http://www.instructables.com/id/Programming-the-WeMos-Using-Arduino-SoftwareIDE/

Dort wird zwar das große Board gezeigt, aber die Integration in Arduino ist identisch mit dem Mini.

Sollten Kompilierfehler auftreten, bitte [Issue #9](https://github.com/jp112sdl/MatrixDisplayESP8266/issues/9) beachten!

## Taster - Funktion
* Drücken der Taste 1 ändert die Helligkeit der DotMatrix-Anzeige
* Drücken der Taste 2 wechselt zwischen dem Automatik-Modus (nacheinander Einblenden der Werte) und der Auswahl einer fixen Anzeige eines bestimmten Wertes.
* ein Gedrückthalten der Taste 1 oder der Taste 2 bei Einschalten/Stromzufuhr startet das Modul in den Konfigurationsmodus

## Konfiguration
Wird einer der beiden Taster bei Einschalten/Stromzufuhr gedrückt gehalten, startet das Modul im AP-Modus.
Es erscheint bei der WLAN-Suche vom Notebook/Handy ein neues WLAN namens DotMatrix-xx:xx:xx:xx:xx:xx.
Nach dem Verbinden mit diesem WLAN wird automatisch ein Popup des Konfigurationsportals geöffnet.

**WLAN konfigurieren** anklicken

![Konfiguration1](/MatrixDisplayESP8266/images/Konfiguration1.png)


![Konfiguration2](/MatrixDisplayESP8266/images/Konfiguration2.png)

**Beispiel**

![Konfiguration3](/MatrixDisplayESP8266/images/Konfiguration3.png)



## Bereitstellung der Daten
Der Wemos ruft zyklisch die Daten von der URL ab.
Die anzuzeigenden Werte sind als plain text, ohne jegliche Formatierung, nur mit einem **Semikolon getrennt** zu liefern und der **gesamte String muss in Anführungszeichen eingeschlossen sein.**
Die Rückgabe des HTTP-Requests sieht bspw. so aus: "Text1;Text2;Text3;Textn"

Die Daten können auch aus einer HomeMatic Systemvariable vom Typ "Zeichenkette" abgerufen werden.<br>
Dafür muss bei URL eingegeben werden:<br>
`http://1.2.3.4:8181/a.exe?ret=dom.GetObject(%22SV_Matrix%22).State()`<br>
wobei `1.2.3.4` durch die IP der CCU2 und `SV_Matrix` durch den Namen der Systemvariablen ersetzt werden müssen (die %22 müssen bestehen bleiben!).


*Sonderzeichen: Aufgrund der UTF-Zeichencodierung muss das Grad-Zeichen (°) als Dollar ($) übergeben werden.
Beispiel: "Luft: 12.5$C" wird dargestellt als "Luft: 12.5°C"*

### Eine weitere, detailliertere Anleitung (inkl. 3D Gehäuse-Vorlage) findet ihr auf [Michaels Blog]( https://www.schellenberger.biz/matrix-display-fuer-homematic-im-nachbau/).

## Anpassung der Anzahl der verwendeten DotMatrix-Module
Möchte man mehr/weniger Matrix-Module anschließen, so kann die Anzahl im Code leicht geändert werden.
In der Datei MatrixDisplayESP8266.ino:
```C
#define MAX_DEVICES 8
```

## Gehäuse / Rahmen
...gibts z.B. hier:
- [Link](https://www.thingiverse.com/thing:2862875)
