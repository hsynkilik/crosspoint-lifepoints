# X4 LifePoints – Entwicklungsquellcode (nicht flashen)

Basis: CrossPoint 2.0.0 (vom Nutzer bereitgestelltes Source-ZIP).

Änderungen: Zusätzlicher Eintrag `LifePoints` im klassischen Home-Menü, neue Activity mit Querformat, zwei Zählern (40/40), gegenüberliegender 180-Grad-Darstellung, physischen Tasteneingaben, kurzem Druck ±1, langem Druck ab 600 ms ±5 genau einmal (Auswertung beim Loslassen), Reset über beide Plus-Tasten, Verlassen über beide Minus-Tasten 3 Sekunden.

WICHTIG: Der Code wurde hier nicht für die ESP32-C3-Hardware kompiliert oder auf einem X4 getestet. Die Cover-Grid-Startseite benötigt eine gesonderte Integration, da sie ein eigenes festes Navigationsmodell verwendet. Der tatsächliche Zusammenhang zwischen Hardware-Tastennummern und der Reihenfolge der vier Tasten muss auf dem Gerät geprüft werden. Kein .bin-Build enthalten. Nicht direkt flashen.

Für einen Build: PlatformIO und die im Projekt referenzierten Toolchains/Abhängigkeiten einrichten, CrossPoint-Projekt öffnen und die X4-Umgebung kompilieren. Vor dem Flashen Build, Tasten, Rotationsrichtung, Chord-Handling und Rückkehr zum Home-Menü prüfen.
