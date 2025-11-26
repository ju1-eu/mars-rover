# Was ist Mapping?

Mapping ist in der Robotik der Prozess, bei dem ein Roboter eine Karte seiner Umgebung erstellt, während er sich durch diese hindurchbewegt.

Für einen Menschen klingt das trivial (wir sehen einen Raum und "wissen", wo die Wände sind). Für einen Roboter ist Mapping jedoch eine hochkomplexe mathematische Herausforderung.

### 1. Die Analogie: Der Blinde im dunklen Raum
Stell dir vor, du betrittst mit verbundenen Augen einen unbekannten Raum.
1.  Du tastest dich vorwärts und berührst eine Wand.
2.  Du machst in deinem Kopf einen Strich auf einer imaginären Karte: "Hier ist ein Hindernis."
3.  Du machst zwei Schritte nach links und tastest erneut.
4.  Du verbindest die Punkte.

Genau das tut dein Roboter. Nur tastet er nicht mit Händen, sondern mit **Licht (Lidar)**, und er zeichnet nicht im Kopf, sondern in einem **Raster (Grid)**.

### 2. Die technische Realität: The Occupancy Grid (Besetztheitsraster)
Ein Computer versteht keine "Wände" oder "Türen". Er versteht nur Wahrscheinlichkeiten in einem Raster.
In ROS 2 ist die Standard-Karte ein sogenanntes **Occupancy Grid**.

Stell dir den Boden deines Raumes als riesiges Blatt kariertes Papier vor. Jedes Kästchen (Pixel) ist z. B. $5 \times 5$ cm groß. Der Roboter weist jedem Kästchen einen von drei Zuständen zu:

* **Weiß (Free):** Der Laser ist hier durchgegangen, ohne auf etwas zu treffen. Der Bereich ist befahrbar.
* **Schwarz (Occupied):** Der Laser ist hier abgeprallt. Hier steht eine Wand oder ein Stuhl.
* **Grau (Unknown):** Hier war der Roboter noch nie und der Laser hat diesen Bereich noch nicht erreicht.



### 3. Das "Henne-Ei-Problem": SLAM
Warum ist Mapping so schwer? Weil der Roboter zwei Dinge gleichzeitig tun muss, die voneinander abhängen. Das nennt man **SLAM** (*Simultaneous Localization and Mapping* – Gleichzeitige Lokalisierung und Kartierung).

* Um eine korrekte Karte zu zeichnen, muss der Roboter wissen, wo er genau steht (**Lokalisierung**).
* Um zu wissen, wo er genau steht, braucht er eine Karte, an der er sich orientieren kann (**Mapping**).

**Das Problem:**
Wenn dein Roboter denkt, er ist 1 Meter geradeaus gefahren (laut Encoder), aber in Wirklichkeit sind die Räder etwas durchgedreht und er ist nur 90 cm gefahren, zeichnet er die nächste Wand an der falschen Stelle in die Karte ein. Die Karte verzerrt sich.

### 4. Die Lösung: Wahrscheinlichkeit & Loop Closure
Wie löst der Profi das? Mit Statistik.

1.  **Bayes-Filter:** Der Roboter trägt eine Wand nicht sofort als "Schwarz" ein. Er sagt: "Der Laser hat hier etwas gesehen. Die Wahrscheinlichkeit, dass hier ein Hindernis ist, steigt auf 60%." Wenn er es noch 5-mal sieht, steigt sie auf 99%. Das filtert Messfehler ("Rauschen") heraus.
2.  **Loop Closure (Schleifenschluss):** Das ist die Magie von SLAM.
    * Der Roboter fährt im Kreis durch die Wohnung und kommt wieder am Startpunkt an.
    * Aufgrund von kleinen Fehlern (Drift) denkt er aber, er stünde 20 cm neben dem Startpunkt. Die Karte passt nicht zusammen (Doppelwände).
    * Der Algorithmus erkennt: "Moment mal, das Muster der Laserdaten hier sieht exakt so aus wie am Anfang!"
    * **Der "Ruck":** Der Roboter korrigiert seine Position rückwirkend und zieht die gesamte verzerrte Karte wieder gerade, bis Start und Ende zusammenpassen.

### 5. Zusammenfassung für dein Projekt
Mit deinem **Lidar (RPLIDAR)** und dem **Raspberry Pi 5** wirst du genau das machen.

1.  Du startest den **ROS 2 SLAM-Node** (z. B. `slam_toolbox` oder `Cartographer`).
2.  Du steuerst den Roboter per Joystick einmal durch alle Zimmer.
3.  Am Bildschirm (in *RViz*) siehst du live, wie aus grauen Flächen langsam weiße Räume und schwarze Wände werden.
4.  Am Ende speicherst du diese Karte (`map_saver`).

Ab dann braucht der Roboter den SLAM nicht mehr zwingend. Er lädt die gespeicherte Karte und nutzt sie nur noch zur Navigation (um Wege von A nach B zu finden).

