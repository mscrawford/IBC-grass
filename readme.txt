28.4.2014
Update 2.0
-reduce number of hierarchie levels
-restructure file input 
 (one file for simulation definition and one file for PFT definition)


12.6.2012
Wechsel zu Eclipse KK

15:14 30.11.2012
-new Project RootHerb
-no seed rain
-grid size is 173cells
-no grid-save activated
-no root herbivory activated
-file input: pft-files, simfile and pftinit-file(lina)
-file output: annually Grid-, pft- and survival information (3 files)

11:53 08.03.2011
-dokumentation nach 10, 30 jahren und am Ende
-zusätzliche Mortalität durch BG möglich BGThres=1 - ohne
-Output: mean Shannon ist mean der letzten 25a

11:44 09.12.2010
Belowground Grazing -Version GMBG22.exe
verkürzte Dokumentation

10:34 25.11.2010
Belowground Grazing -Version GMBG21.exe
eingearbeitet: Linas Veränderungen in LinaStd und neue Features durchs UpScaling

----------------------------------------
Die Version vom 12.8. entspricht der vorigen mit dem Unterschied, dass die Option der Mahd (Felix May Feb2010) zugefügt wurde. Mahd wird berechnet, wenn die entsprechenden Parameter gesetzt werden.

Die Version vom 19.7. simuliert Vegetationsentwicklung mit realistischen PFTs (aus Datei: "InitPFTdat.txt")
Die Simulationen sind in 'SimFile.txt' definiert.

-----------------------------------------------------------------------------
Die Versionen vom 16.4. simulieren beide unten genannten Einwanderungen (Qestion1, Question2) für Funktionalität von Konkurrenzversionen 2 und 3. Dazu im Eingabefile Version=1 oder 2 setzen -'0' ist die Standardmethode für Typenundifferenzierte Konkurrenz.

Die Version vom 11.4. simuliert die Einwanderung nicht-klonaler Typen in ein klonales System.
Die Version vom 8.4. simuliert die Einwanderung klonaler typen in ein nicht-klonales System.


Herkunft der Version:
kopiert am 30.03.2010 aus der letzten belowground-version; 
alle Elemente der klonalen Version von Ines müssten noch enthalten sein. Diese gilt es zu aktivieren und zu verändern/korrigieren.
Nischendifferenzierung wurde bei Ines nicht angenommen. Ich würde Version2 von Felix übernehmen und mit Ines' variante vergleichen, aber dafür müsste ich wahrscheinlich klonal/nicht klonal als unterschiedliche Typen bei gleichem PFT sichtbar machen.
