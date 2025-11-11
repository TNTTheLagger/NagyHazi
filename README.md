# Walk3D
## A program célja

A feladat egy olyan program létrehozása, amely a terminálban egy 3 dimenziós hatású környezetet jelenít meg szöveges karakterek felhasználásával.  
A program célja egy *raycasting* elven működő megjelenítő motor (rendering engine) megvalósítása, amely a játékos pozíciója valamint nézőiránya alapján kiszámítja, hogy a térben mely falak láthatóak.

A megjelenítés különböző árnyalatú karakterekkel történik, amelyek a távolságtól függően változnak, ezáltal térbeli mélység érzetét keltve.

A program interaktív: a játékos mozoghat a térben előre-hátra, valamint jobbra és balra fordulhat.  
A képernyő ezeknek a mozgásoknak megfelelően frissül, így a felhasználó a mozgás során folyamatosan változó, háromdimenziós hatású nézetet lát.

---

## Program használata

A program futtatásához elegendő a fájlt elindítani a parancssorból.

Futtatáskor megadható egy **CSV formátumú pályafájl**, amely a térkép elrendezését tartalmazza.  
A program a megadott fájlból tölti be a falak és üres területek adatait, így a pálya könnyen módosítható valamint újrahasznosítható.  
Ha a felhasználó nem ad meg pályafájlt, a program egy beépített alapértelmezett térképet használ.

---

### Irányítás

| Billentyű | Funkció |
|------------|----------|
| **W** | Előre mozgás |
| **S** | Hátra mozgás |
| **A** | Balra fordulás |
| **D** | Jobbra fordulás |
| **M** | Térkép (minimap) megjelenítése/elrejtése |
| **N** | Hibakeresés (debug) mód ki- és bekapcsolása |
| **Q** | Kilépés a programból |

---

## A program működése

A felhasználó mozgása során a program valós időben újraszámolja és kirajzolja a környezetet, ezzel létrehozva a 3D-s illúziót, valamint biztosítja, hogy a játékos ne tudjon átmenni a falakon.

---

## A futás eredménye

A program a terminálban folyamatosan frissülő 3D-s nézetet jelenít meg.  
A játékos mozgása során a falak távolsága, a perspektíva és az árnyékhatások változnak, ezzel valósághű hatást létrehozva.

Bekapcsolt **térkép** funkcióval a képernyőn megjelenik a pálya felülnézeti képe, ahol a játékos helyzete külön karakterrel (`X`) van jelölve.  

A **hibakeresési mód** bekapcsolása esetén a program kijelzi a számításokhoz kapcsolódó adatokat (pl. képkockák közti idő).

A program futása a **Q** billentyű megnyomásával zárható le.
