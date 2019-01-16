# MatrixDisplayESP8266 fur IOBroker optimiert


**folgende Änderungen zum Orginal:**
```diff
+ Einführung von Modi
+ Schalter optional (durch IOBroker ausführbar)
+ die wichtigsten Sonderzeichen und Umlaute werden unterstützt (ok: "äöüÄÖÜ%°&,.:!/|+-()#" ko: "ß€[]{})
```


Folgende Modi sind vorhanden:
- 0 - display aus
- 1 - wie Orginal - Text (Daten) kommen von rechts und gehen nach oben - mit Uhrzeit
- 2 - nur Uhrzeit - feststehend
- 3 - Daten laufen von rechts nach links durch - ohne Uhrzeit
- 4 - Daten kommen von rechts - gehen wieder nach rechts - mit Uhrzeit
- 5 - wie Mode 3 mit Uhrzeit
- 6 - Daten feststehend (zu lange Nachrichten werden abgeschnitten)
- 7 - Daten feststehend und blinkend (zu lange Nachrichten werden abgeschnitten)

Umlaute sollten auch funktionieren - falls damit Probleme auftreten, könnte es sein, dass man eine Libriary austauschen muss. Diese findet man unter dem help-Ordner. die Datei muss in den Arduino-Sketch libraries-Ordner (aber vorher ohne ausprobieren!) werden. Pfad:"...\libraries\MD_MAX72XX\src"

![Sampleum](/images/Sampleum.png)

Die beiden Schalter für Helligkeit und Modiänderung wurden als Abfragepunkte eingeführt und sind durch IOBroker steuerbar - IOBroker dominiert das Setting und überschreibt die Schalter nach der definierten Zeit!

Im IOBRoker muss der Simple-Api-Adapter installiert sein - in diesem Beispiel mit dem Standardport 8087.

Die Einrichtung ist wie im Orginal - es kommen weitere Url-Abfragen dazu.

Url für Daten werden wie Orginal angelegt
- http://192.168.xxx.xxx/getPlainValue/control-own.0.ESPMatrix.Matrix (IOBroker-Baum: control-own.0.ESPMatrix.Matrix)
anschliessend müssen noch weitere Datenpunkte in IOBroker angelegt werden. Diese müssen notwendigerweise im selben Vewrzeichnis sein und wie folgt benannt werden:
- control-own.0.ESPMatrix.MatrixMode
- control-own.0.ESPMatrix.MatrixIntensity

```diff
-                !!!      ALLE DATENPUNKTE MÜSSEN ALS TEXTFELDER DEFININIERT SEIN     !!!
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
Hinweis: Es gibt eine tolle Variante mit MQTT und Doppelzeilen Anzeige: [https://github.com/prysme01/DotMatrixDisplay]
```



#    -----  von jp112sd übernommen    ----------




![Sample1](/images/Sample1.png)


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

![Konfiguration1](/images/Back1.jpg)

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

![Konfiguration1](/images/Konfiguration1.png)


![Konfiguration2](/images/Konfiguration2.png)

**Beispiel**

![Konfiguration3](/images/Konfiguration3.png)



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
