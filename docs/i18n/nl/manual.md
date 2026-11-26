# {{APP}} Gebruikershandleiding

Deze handleiding is automatisch vertaald en nog niet gecontroleerd door een Nederlandstalige. Waar iets onduidelijk is, geldt de Engelse handleiding als referentie.

{{APP}} is een schrijfstudio voor lange teksten: romans, scenario's, toneelstukken, stripscripts, scripties en non-fictieboeken. Je schrijft in kleine stukken (scènes, paragrafen, notities), rangschikt ze naar believen en *stelt* ze aan het eind *samen* tot één manuscript.

Deze handleiding behandelt versie {{VERSION}}. Druk op elk moment op **F1** om hem te openen; hij opent bij het onderdeel dat past bij wat je op dat moment ziet: het document, het prikbord, het overzicht of Gecombineerde tekst, de binder, het tabblad van de inspector dat het toetsenbord heeft, of het dialoogvenster dat open is (een dialoogvenster zonder eigen onderdeel opent het begin). Het venster heeft links een **inhoudsopgave** (een klik op een kop springt ernaartoe) en boven de tekst een **zoekveld**: druk op **Ctrl+F** om erheen te gaan, typ een woord, en **Return** toont de volgende treffer, **Shift+Return** de vorige (de knoppen **Volgende** en **Vorige** doen hetzelfde); de regel naast het veld zegt welke treffer van hoeveel wordt getoond, en elke treffer is in de tekst gemarkeerd. Wat bedoeld is voor mensen die {{APP}} bouwen (plugins, pakketten, de projectmap in detail, omgevingsvariabelen) staat in de ontwikkelaarsgids, **Help > Probleemoplossing en ontwikkeling > Ontwikkelaarsgids: updates en add-ons**.

---

## 1. Eerste stappen

### Het programma downloaden

{{APP}} wordt gedownload van de pagina **Releases** van de projectpagina, https://github.com/Shelin-boop/Quillbinder_Collection/releases. Elke release bevat het programma als één bestand (*{{APP}}-&lt;version&gt;-x86_64.AppImage*), een programma-update voor wie al een AppImage van {{APP}} heeft (sectie 17) en de add-ons (*.qbpkg*-bestanden, sectie 17). Maak de AppImage uitvoerbaar (klik met de rechtermuisknop > **Eigenschappen** > **Rechten** > *Uitvoerbaar*, of `chmod +x` in een terminal) en dubbelklik erop; **Help > Aan toepassingsmenu toevoegen** zet hem in het menu van je bureaublad. Het bestand *SHA256SUMS.txt* van elke release bevat de controlesommen van de bestanden.

**{{APP}} is {{LABEL}}-software.** Maak back-ups van je projecten (sectie 15) en bewaar een kopie van je manuscripten buiten het programma.

### Een project starten

Wanneer {{APP}} zonder project start, opent het **startvenster**:

- **Nieuw project**: kies een categorie en een sjabloon (bijvoorbeeld Leeg, Roman, Roman met delen of Kort verhaal), typ een naam, kies waar je het opslaat en klik op **Aanmaken**. **Bestandsindeling** bepaalt hoe het project wordt opgeslagen: als {{APP}}-project (de standaard) of als Scrivener 3-project (*.scriv*), zodat je het ook in Scrivener kunt openen (sectie 14, *Werken in Scrivener-indeling*).
- **Recente projecten**: dubbelklik op een project waaraan je eerder werkte.
- **Anders openen…**: zoek een project op je schijf.
- **Scrivener-project importeren…**: maak een {{APP}}-project van een Scrivener-project (zie sectie 14). Wil je aan het Scrivener-project zelf blijven werken, open dan in plaats daarvan het *.scrivx*-bestand met **Anders openen…**.

{{APP}} heropent het laatste project bij het starten (je kunt dit uitzetten onder **Bestand > Opties > Algemeen**).

**De allereerste start** toont één pagina waarop alles al is ingevuld (**OK** behoudt dat): de back-ups (of een project bij het sluiten een back-up krijgt, hoeveel back-ups worden bewaard en waar), bij een Linux-AppImage de vraag of {{APP}} aan het toepassingsmenu moet worden toegevoegd (en of de AppImage naar *~/Applications* moet worden verplaatst), en de mededeling over de controle op updates met de bijbehorende schakelaar. Het welkomstproject opent als je op **OK** drukt; er verschijnt eerst geen leeg programmavenster. Alles kan later worden gewijzigd in **Bestand > Opties**; daar staat ook, onder **Algemeen > Opstarten**, de keuze welke instellingen voorgaan, die van een project zelf of altijd die van het programma (sectie 16, *Instellingen voor één project*).

### Het welkomstproject en de Snelstartgids

De eerste keer dat {{APP}} start, opent het het **welkomstproject**: een kort project dat in één document per onderwerp laat zien wat {{APP}} méér heeft dan gebruikelijk (de pictogrammenbalk, een zoom voor elk deel van het venster, mindmaps, objectbladen, de opmerkingen van een hele map, Vergelijken en overbrengen, Scrivener-projecten, thema's, updates en add-ons, en wat je tekst beschermt). Begin met het document **Welkom** bovenaan de binder; de koppelingen daarin leiden naar elk onderdeel van de rondleiding. Niets wat je in het welkomstproject doet, kan je eigen projecten schaden. (Een project dat op de opdrachtregel wordt opgegeven, of een project waarop je dubbelklikt, opent in plaats daarvan; het welkomstproject komt dan bij de volgende start.)

Het welkomstproject wordt opgeslagen als "Welcome to {{APP}}" in je map *Documenten*. **Help > Welkomstproject…** opent het opnieuw zoals je het achterliet, of maakt een verse kopie om opnieuw te beginnen (**Verse kopie maken**; de oude kopie gaat naar de prullenbak van het bureaublad). Verwijder het wanneer je wilt; de opdracht maakt een nieuw project.

**Help > Snelstartgids** toont op één pagina waar de belangrijke instellingen staan (thema's en donkere modus, kleuren en lettertypen, opmaak, zoom, werkbalken, sneltoetsen, back-ups ...) en vat samen wat {{APP}} kan.

### Opslaan

Je hoeft nooit handmatig op te slaan. {{APP}} slaat automatisch op een paar seconden nadat je stopt met typen, wanneer je naar een ander programma overschakelt en wanneer je het project sluit. Als je doortypt, slaat het op in de eerste korte pauze (een adempauze tussen woorden) zodra 5 seconden zijn verstreken, en uiterlijk na 10 seconden, ook zonder pauze. **Ctrl+S** slaat direct op als je zeker wilt zijn.

Elk bestand wordt eerst onder een tijdelijke naam geschreven en vervangt het oude pas wanneer het helemaal is geschreven en de computer het op de schijf heeft gezet. Als de schijf vol raakt of het station uitvalt tijdens het opslaan, blijft het oude bestand zoals het was en blijven je wijzigingen in het venster staan tot opslaan weer lukt.

**De opslagstatus staat altijd in beeld** (0.70). De voettekstbalk van de eerste editor (de lege ruimte tussen het maptotaal en het aantal woorden, of links bij de andere voettekstbalken) zegt **Opgeslagen**, **Nog niet opgeslagen** (een paar seconden nadat je typte) of, op een rode achtergrond, **Niet opgeslagen: de laatste keer opslaan mislukte om 14:03**; zolang opslaan mislukt, draagt ook de titel van het venster een markering. Een mislukte opslag toont ook een melding die de niet-opgeslagen documenten noemt en **Teksten elders opslaan…** aanbiedt; de melding neemt nooit het toetsenbord over, dus de volgende toets die je typt komt nog steeds in je tekst terecht. Het programma probeert het zelf opnieuw (eerst elke 15 seconden, daarna elke minuut), de melding komt na vijf minuten falen opnieuw, en het rode teken blijft staan tot een opslag is gelukt. Mindmaps worden op dezelfde manier opgeslagen en gecontroleerd: een bord dat niet kan worden opgeslagen, blijft in het venster staan, telt bij het sluiten van het project als niet opgeslagen en komt in de noodkopie terecht.

**Korte meldingen** (0.93.0). Onderaan het venster is geen statusbalk meer. De meldingen die daar vroeger stonden ("Opnieuw opgeslagen: alle wijzigingen staan op de schijf.", het resultaat van een back-up en dergelijke) verschijnen voor enkele seconden op dezelfde plek als de opslagtekst, in de voettekstbalk van de eerste editor (de linker of bovenste als er twee zijn), en daarna komt de opslagtekst terug. Een melding die te lang is voor de plek wordt ingekort; laat de aanwijzer erop rusten om alles te lezen. In een heel smalle editor wordt de tekst vervangen door een klein gekleurd teken (groen opgeslagen, geel nog niet opgeslagen, rood mislukt) met de tekst als knopinfo. In de schrijfmodus, en zolang er geen project open is, wordt niets getoond.

**Bestanden die {{APP}} niet overschrijft** (0.70). Een tekstbestand dat beschadigd is (een RTF-bestand waarvan het begin kapot is of dat is afgekapt) of dat een ander programma in een andere tekenset heeft opgeslagen (bijvoorbeeld een Markdown-bestand dat als Windows-1252 of UTF-16 is opgeslagen na bewerking buiten het programma), wordt getoond voor zover het te lezen is en kan niet worden bewerkt; een melding zegt dat, en het bestand wordt nooit gewijzigd. Kopieer de tekst naar een nieuw document om verder te gaan, of sla het bestand op in UTF-8 in het programma dat het schreef en sluit en heropen daarna het project; of zet het terug naar een van de momentopnamen (sinds 0.81.1): het beschadigde bestand wordt eerst als kopie bewaard in *settings/conflicts* van de projectmap, en de tekst van de momentopname wordt de tekst van het document, die kan worden bewerkt en weer wordt opgeslagen. Documenten samenvoegen en Splitsen weigeren zo'n document met opgave van de reden, en Project vervangen, Aanhalingstekens omzetten en stijlwijzigingen laten het ongemoeid en noemen het. Als een opdracht het toch wijzigt, gaat de gewijzigde tekst niet verloren: hij wordt als RTF-bestand bewaard in *settings/conflicts*, en een melding zegt waar (0.81.1). Een mindmapbestand dat niet kan worden gelezen, wordt op dezelfde manier behandeld. Tekens die niet kunnen worden opgeslagen (stuurtekens uit geplakte gegevens) worden weggelaten zoals eerder; een korte melding in de voettekstbalk zegt dat nu eenmaal per document.

**Een opslag komt in zijn geheel aan** (0.65). Wanneer één opslag meerdere bestanden schrijft (de tekst van een nieuw document en het projectbestand, een tekst en zijn afbeeldingen, het Objecten-blad), worden ze allemaal eerst naast hun plek geschreven, daarna vermeld in een klein journaal (*settings/journal.json*), en pas dan samen op hun plek gezet. Als het programma of de computer halverwege stopt, maakt de eerstvolgende opening die opslag af aan de hand van het journaal, of, als het journaal nog niet volledig was, laat het project zoals het bij de opslag ervoor was; een melding zegt welke van de twee. Een journaal dat niet kan worden gelezen (een schijffout) wordt nooit voor een leeg journaal aangezien: het en de bestanden van zijn opslag worden bewaard in een map in *settings/conflicts*, er wordt niets op zijn plek gezet of verwijderd, en de melding zegt dat (0.81.1). In een synchronisatiemap wordt het journaal van een opslag van de andere computer 15 minuten met rust gelaten (die opslag kan daar nog lopen, of de bestanden kunnen nog binnenkomen): deze computer slaat het project intussen niet op en probeert het zelf opnieuw, en als de andere computer tijdens die opslag is gestopt, maakt deze computer hem na die 15 minuten af (0.81.1). Als één bestand van een opslag niet kan worden geschreven, vervangt geen enkel bestand iets, en blijven je wijzigingen in het venster staan voor de volgende poging. Sinds 0.93.0 laat een station dat een fout meldt wanneer een map wordt gesynchroniseerd nadat de bestanden op hun plek zijn gezet (een echte fout, geen bestandssysteem dat niet kan synchroniseren) de opslag als mislukt gelden: het journaal blijft staan, de melding zegt dat het station een fout meldde, en de volgende opslag of opening maakt de opslag af aan de hand van het journaal. Zolang een journaal dat niet kan worden gelezen niet opzij kon worden gezet (de map *settings/conflicts* kan niet worden beschreven), schrijft geen enkele opslag er een nieuw journaal overheen: de opslag wacht, zijn melding noemt het bestand, en het programma slaat zelf op zodra het bestand opzij kan worden gezet. Een tijdelijk bestand van een eerdere opslag dat in de weg staat, wordt opzij gezet en nooit overschreven. Een mapsynchronisatie die mislukt voordat iets is vervangen, is een nette mislukking: er is niets gewijzigd. Komt de fout nadat de bestanden op hun plek staan, dan noteert het programma wat er op de schijf staat (zodat de nieuwe pogingen geen valse meldingen van "elders gewijzigd" geven en geen afbeeldingen dubbel schrijven), en synchroniseert het de map bij elke opslag opnieuw: "Opgeslagen" volgt altijd op een gecontroleerde mapsynchronisatie. Een bestandssysteem dat al bij het openen van het project een fout geeft op een mapsynchronisatie, wordt onthouden als een systeem dat mappen niet kan synchroniseren, en zijn opslagen worden niet als mislukt gemeld.

Als je uitlogt of de computer afsluit terwijl {{APP}} open is, slaat het alles op voordat het eindigt (er wordt dan geen back-up gemaakt). Als die opslag mislukt (een volle schijf, een station dat weg is), worden de teksten en notities die niet zijn opgeslagen (en sinds 0.93.0 het projectbestand zoals het in het geheugen staat) als RTF-bestanden weggeschreven in een noodmap, *~/.local/share/{{APP}}/Emergency*, en de volgende start zegt waar ze staan en hoeveel bestanden zijn geschreven (de melding zegt "onvolledig" en noemt wat ontbreekt wanneer een bestand niet kon worden geschreven). Als het programma onverwacht stopt, vertelt de volgende start dat (sectie 20, *Crashrapporten*).

**Een project sluiten dat niet kan worden opgeslagen** vraagt wat te doen; **Open houden** is de standaard. **Teksten elders opslaan…** schrijft de nog niet opgeslagen teksten en notities als RTF-bestanden in een map die je kiest (een ander station, een USB-stick), en het project blijft open. Sinds 0.93.0 bevat deze noodkopie ook het projectbestand zoals het in het venster staat (de binder, titels en labels), de mindmapborden en de gewijzigde objecten, en wordt hij ook gemaakt wanneer alleen de binder of de Objecten zijn gewijzigd. De melding zegt hoeveel bestanden zijn geschreven, als "n van m", en noemt de bestanden die ontbreken en waarom; een kopie waaraan bestanden ontbreken, krijgt de kop **onvolledig** (met een waarschuwingsteken) en ziet er nooit uit als een complete kopie. Een map die er al is, wordt nooit opnieuw gebruikt: een tweede kopie in dezelfde seconde krijgt een nummer. Het projectbestand in de kopie heet als het projectbestand met ".copy" erachter (zodat de map niet per ongeluk voor een project wordt aangezien); de README in de kopie zegt hoe je het terugzet: sluit het project, hernoem het projectbestand in de projectmap (het blijft zoals het was), kopieer het bestand van de noodkopie daarheen, hernoem het naar de naam van het projectbestand en open het project opnieuw. Voor een project in Scrivener-indeling zegt de README dat het bestand bij de werkkopie hoort en geeft ze geen weg terug. Als er toch niets onopgeslagen bleek (een nieuwe poging is intussen gelukt), zegt de melding dat alles is opgeslagen en dat geen kopie nodig was. **Sluiten zonder opslaan** sluit het project en verliest die wijzigingen.

### Waar je werk staat

Een project is een map met de naam *JeTitel*{{SUFFIX}}. Daarin is elk document een gewoon tekstbestand, zodat je tekst ook zonder {{APP}} leesbaar blijft. Dubbelklik op het bestand *JeTitel*{{FILE}} in de map om het project te openen. Hernoem of verplaats geen bestanden in de projectmap met de hand.

Om het bestand van een document buiten {{APP}} te vinden, open je *contents-by-title.txt* in de projectmap (0.65): het somt elk document van de binder op met zijn titel, ingesprongen naar zijn plaats, en de bestanden die zijn tekst, notities of media bevatten (bijvoorbeeld *content/<id>/text.md*). {{APP}} schrijft het bij elke opslag die de binder wijzigt en leest het zelf nooit. (Een project in Scrivener-indeling heeft zo'n lijst niet; het *.scrivx*-bestand van Scrivener noemt de documenten.)

Een project mag maar in één venster tegelijk open zijn. Als het al in een ander venster of op een andere computer open is (bijvoorbeeld via een gesynchroniseerde map), waarschuwt {{APP}} je voordat het opent. Als je {{APP}} opnieuw start met een project dat al in een venster van het programma open is (vanuit het bestandsbeheer of een snelkoppeling op het bureaublad), haalt het dat venster naar voren in plaats van het project twee keer te openen; een ander project opent in een eigen venster.

**Een project in een gesynchroniseerde map** (OneDrive, Dropbox, Nextcloud, Syncthing of een ander synchronisatieprogramma) kan op meerdere computers worden gebruikt, steeds op één tegelijk. Sluit het op de ene computer en laat het synchronisatieprogramma klaar worden voordat je het op de andere opent. {{APP}} helpt daarbij op het niveau van bestanden, welk synchronisatieprogramma het ook is:

- Zolang een project open is, noemt een klein vergrendelbestand erin de computer en wordt het om de paar minuten vernieuwd. Het bevat de naam van de computer, je gebruikersnaam erop, de versie en het procesnummer van het programma, wanneer het project werd geopend en wanneer de vergrendeling voor het laatst werd vernieuwd, en een code waarmee computers worden onderscheiden (een hash, niet leesbaar als naam). Iedereen met toegang tot de gedeelde map kan dit zien. Onder **Bestand > Opties > Algemeen > Opslaan** stelt **Naam van deze computer in gedeelde mappen** een eigen naam in ("Laptop"); die wordt gebruikt in plaats van de computernaam in vergrendelbestanden, in de namen van bewaarde versies en in *settings/ui-<computer>.json*, en je gebruikersnaam wordt dan weggelaten. Op een andere computer toont het openen van het project dan de naam van die computer, sinds wanneer die het project open heeft en zijn laatste teken van leven; **Toch openen** of **Annuleren**. Een vergrendeling die meer dan 15 minuten niet is vernieuwd (het project werd daar niet gesloten, of die computer staat uit) wordt als achtergebleven getoond, en **Openen** is de standaard.
- Als het project op een andere computer wordt geopend terwijl het hier open is, zegt een melding dat en biedt ze aan het hier te sluiten.
- Als een bestand van het project elders is gewijzigd (op een andere computer via het synchronisatieprogramma, of door een ander programma) nadat {{APP}} het las, en het hier ook is gewijzigd, wordt de andere versie bewaard voordat de jouwe wordt opgeslagen: als **momentopname** van het document (vergelijk die op het tabblad Momentopnamen van de inspector) en als kopie in de projectmap *settings/conflicts*. Een melding zegt welke documenten. Als de andere versie niet kan worden bewaard (bijvoorbeeld omdat de schijf vol is of *settings/conflicts* niet kan worden beschreven), wordt de jouwe er niet overheen opgeslagen: opslaan mislukt met een melding die het bestand noemt, en je wijzigingen blijven onopgeslagen tot de andere versie kan worden bewaard. Een document dat je hier alleen bekeek, wordt nooit over een nieuwere versie van elders heen opgeslagen.
- Een project openen en sluiten zonder iets te wijzigen wijzigt geen enkel bestand; de indeling van het venster en de plekken waar je in elk document was, worden per computer bewaard (*settings/ui-<computer>.json*). In Scrivener-indeling blijft ook de *.scriv* onaangeroerd, en krijgt hij een vergrendelbestand van {{APP}} (*quillbinder.lock*) zolang hij open is; Scrivener leest dat niet. Vóór elke opslag in de *.scriv* zoekt {{APP}} naar de eigen vergrendeling van Scrivener: als Scrivener het project opende nadat {{APP}} dat deed, wordt er niets meer in de *.scriv* geschreven (een melding zegt dat één keer, daarna een korte melding in de voettekstbalk); je wijzigingen blijven in de werkkopie van {{APP}}. Sluit het project in Scrivener en sla opnieuw op (**Bestand > Opslaan**). Als je het project eerst hier sluit, gaan de wijzigingen de volgende keer dat je het hier opent in de *.scriv*; waar Scrivener intussen iets wijzigde, wordt zijn versie gelezen.
- Als het project een tekst vermeldt voor een document waarvan het bestand nog niet op deze computer is aangekomen (het synchronisatieprogramma is nog niet klaar), noemt een melding het na het openen; dat document wordt leeg getoond, kan niet worden bewerkt en er wordt niets voor in de plaats opgeslagen. Sluit het project, wacht tot het synchronisatieprogramma klaar is en open het opnieuw. Hetzelfde geldt voor een tekst die maar gedeeltelijk kon worden gelezen (0.65).
- Een elders gewijzigd bestand wordt ook opgemerkt als het zijn grootte en zijn tijd behield (sommige synchronisatieprogramma's stellen de tijd zelf in): {{APP}} vergelijkt de inhoud met wat het las of schreef (0.65).
- De inhoud beslist ook in de andere richting (0.92.0): een bestand waarvan **alleen de tijd** veranderde, is geen andere versie. Sommige synchronisatieprogramma's stellen de tijd van een bestand een paar seconden nadat {{APP}} het schreef opnieuw in, wanneer hun upload klaar is; daarvoor wordt niets bewaard, er komt geen melding en het bestand wordt niet opnieuw geschreven. Ook een projectbestand dat al bevat wat een opslag zou schrijven, wordt met rust gelaten, zodat een opslag die niets wijzigt het synchronisatieprogramma niets te vervoeren geeft.
- De melding **Andere versies bewaard** heeft de knop **Niet meer tonen tot {{APP}} wordt gesloten**. De andere versies worden nog steeds bewaard als momentopnamen en als kopieën in *settings/conflicts*; alleen de melding blijft weg tot je het programma afsluit. De volgende start toont hem weer.
- **Een hangslot bij wat een ander apparaat bewerkt (0.92.0).** Zolang een project open is, noteert elk apparaat naast het vergrendelbestand ook welke documenten het in een editor open heeft of heeft gewijzigd sinds het het project opende. Wanneer een tweede apparaat het project opent, toont de binder een klein hangslot aan het rechteruiteinde van de rijen van die documenten, en van de mappen eromheen. Wijs naar het hangslot: de knopinfo noemt het apparaat en wanneer het het project opende. Het is een hint, geen bewaker: er wordt niets geblokkeerd en je kunt het document nog steeds bewerken. De markering reist mee via je synchronisatieprogramma en kan dus enkele minuten oud zijn; ze vereist 0.92.0 of nieuwer op beide apparaten (Scrivener kondigt niets aan). Een lijst van een apparaat dat langer dan 15 minuten geen teken van leven gaf, wordt niet getoond. De notitie is een klein bestand per apparaat (*settings/lock-work-…json*; voor een project in Scrivener-indeling *quillbinder.lock-work-…json* in de *.scriv*); het bevat de naam van het apparaat zoals in het vergrendelbestand, de tijd en de interne nummers van de documenten, geen titels en geen tekst. Het wordt alleen geschreven wanneer de lijst verandert, nooit per toetsaanslag, het wordt verwijderd wanneer het project wordt gesloten en het blijft buiten back-ups.
- **Scrivener-indeling, twee computers** (0.65): documenten die elders (Scrivener op een andere computer) aan de *.scriv* zijn toegevoegd terwijl het project hier open is, blijven behouden, en de binder die ze noemt wordt niet vervangen; een melding vraagt je het project te sluiten en opnieuw te openen, waarna zowel hun documenten als de jouwe te zien zijn. Afbeeldingen, pdf's, audio- en videobestanden en mindmaps die elders in de *.scriv* zijn gewijzigd, blijven zoals ze zijn (vroeger werd de versie van hier eroverheen gekopieerd); als er een aan beide kanten is gewijzigd, wordt de andere versie eerst bewaard in *settings/conflicts*.

Een project in **Scrivener-indeling** is in plaats daarvan een map *JeTitel.scriv*; open het met het *.scrivx*-bestand. {{APP}} slaat je wijzigingen daarin op, zodat het project een Scrivener-project blijft. **Bestand > Opslaan als…** slaat een project onder een andere naam of op een andere plek op, in beide indelingen (sectie 14).

### Privacy: wat je computer verlaat

{{APP}} maakt voor één ding verbinding met internet: om naar updates te zoeken. Het haalt één klein bestand op, stuurt niets over jou en je kunt dit uitzetten in het menu Help. (Sectie 17, *Controleren op updates*, zegt precies wat wanneer wordt opgehaald. Vóór 0.92.0 zocht het programma helemaal niet naar updates.) Drie dingen gaan verder over internet, en alleen wanneer jij ze start:

- **Bestand > Importeren > Webpagina…** haalt de pagina die je noemt en zijn afbeeldingen van die site op. Net als een browser stuurt het de naam van een gangbare browser (Firefox op Linux), de talen van je systeem (voor de taal van de pagina) en, zoals bij elke verbinding, het internetadres van je computer.
- **Een taalmodel downloaden** voor de add-on Vertalen en proeflezen (sectie 10) haalt het op van de releases van de repository van {{APP}} op GitHub. Het stuurt de naam van de add-on ("Quillbinder-Translation-Addon") en, zoals bij elke verbinding, je internetadres.
- **Een spraakmodel downloaden** (de add-on Spraaktranscriptie, sectie 10) haalt het model op van Hugging Face (huggingface.co, een bedrijf in de VS) of van het adres dat je in de opties ervan instelt. Het stuurt "{{APP}}/<version>" als naam van het programma en, zoals bij elke verbinding, je internetadres. Een model wordt alleen geïnstalleerd als zijn controlesom de bekende is; een bestand van een ander adres of een bestand dat je zelf downloadde, wordt pas gebruikt nadat je akkoord gaat.

Wat anderen van je werk kunnen zien:

- **Projecten in gedeelde mappen** dragen een vergrendelbestand zolang ze open zijn (zie hierboven): computernaam, gebruikersnaam en tijden. Stel onder **Bestand > Opties > Algemeen > Opslaan** een eigen naam in om die weg te laten.
- **Auteursgegevens** (naam, adres, telefoon, e-mail in **Bestand > Opties**) blijven in de instellingen van het programma op deze computer; ze komen nooit in de eigen instellingen van een project. Ze bereiken een manuscript alleen waar de samenstelindeling de plaatsvervangers gebruikt (<$author>, <$address> ...).
- **Verwijderde tekst is niet overal weg**: momentopnamen bewaren eerdere versies van een document (**Project > Oude momentopnamen verwijderen…** verwijdert ze), en back-ups bewaren het hele project zoals het was (sectie 15). Back-ups zijn gewone ZIP-bestanden zonder wachtwoord; een back-upmap in een gesynchroniseerde map wordt door het synchronisatieprogramma geüpload.
- **Je bestanden worden door {{APP}} niet versleuteld** (0.65): teksten, notities, de instellingen van het project en back-ups zijn gewone bestanden, leesbaar voor iedereen die de map kan lezen waarin ze staan. Om een manuscript privé te houden, bewaar je het project en zijn back-ups op een versleutelde schijf (het installatieprogramma van Kubuntu biedt aan de hele schijf te versleutelen; voor een USB-stick of externe schijf zijn er de Plasma Vaults van KDE of een met LUKS versleuteld station) en houd je de back-upmap buiten gedeelde mappen die je niet beheert.
- **Crashrapporten** blijven op deze computer tot je er zelf een kopieert of verstuurt (sectie 20). Ze beschrijven de staat van het programma en noemen het geopende project en zijn map. Op Windows wordt naast elk rapport een geheugendump bewaard; die kan tekst bevatten die je aan het schrijven was, en het rapport zegt dat.

### {{APP}} aan het toepassingsmenu toevoegen

De eerste keer dat je het programmabestand ({{APP}}.AppImage) start, biedt de pagina van de eerste start aan {{APP}} aan het toepassingsmenu toe te voegen. Daarna kun je het starten zoals elk ander programma, en een dubbelklik op een project opent het. Geen venster bevestigt de invoer; een mislukking wordt gemeld. Dezelfde pagina (en **Help > Aan toepassingsmenu toevoegen**) biedt aan **de AppImage naar ~/Applications te verplaatsen** (eerst aangevinkt wanneer het bestand er nog niet staat), zodat de menu-invoer naar een plek wijst waar het bestand blijft; als het verplaatsen mislukt (een volle schijf, een bestand met die naam staat er al), gaat niets verloren: het bestand blijft waar het was, de invoer wijst daarheen, en een melding zegt waarom. Na een verplaatsing gebruiken programma-updates, de kopie *.before-update* en **Help > Probleemoplossing en ontwikkeling > Systeeminformatie** meteen de nieuwe plek.

De ene opdracht van het menu Help, **Help > Aan toepassingsmenu toevoegen**, volgt de staat van de invoer: ze voegt de invoer toe zolang er geen is en heet, zodra er een is, *Uit toepassingsmenu verwijderen* en neemt weg wat het toevoegen maakte (het invoerbestand, het pictogram en de bestandstypen) en laat de AppImage waar hij staat. Als je het programmabestand zelf verplaatst, verwijder dan de invoer en voeg hem opnieuw toe.

### Op Windows: alles in één map

De Windows-versie van {{APP}} (experimenteel, gepubliceerd sinds 0.91.1) is een map die je uitpakt waar je maar wilt, bijvoorbeeld *C:\Users\<your name>\Quillbinder*; ze heeft geen installatie en geen beheerdersrechten nodig. Ze bewaart haar instellingen, add-ons, programma-updates, geïnstalleerde lettertypen en crashrapporten in de map *data* binnen die map, en niets in de eigen mappen van Windows of in het register. De hele map kan worden verplaatst of gekopieerd (naar een USB-stick of een andere computer); de map verwijderen verwijdert het programma. Een map waarnaar het programma niet kan schrijven (zoals *Program Files*) wordt geweigerd met een melding die zegt waar je hem in plaats daarvan neerzet. Programma-updates worden geïnstalleerd met **Help > Add-ons en updates > Update of add-on installeren…**; om over te stappen op een nieuw gedownloade versie pak je die uit en kopieer je de map *data* naar de nieuwe map. Je projecten blijven staan waar je ze bewaart.

Er zijn twee manieren om aan die map te komen (0.90.4). Het **installatieprogramma** (*...-setup.exe*) vraagt om een map en pakt {{APP}} daarin uit; het heeft geen beheerdersrechten nodig en wijzigt niets anders op de computer: geen invoer onder Apps, niets in het register, geen invoer in het Startmenu (de hieronder beschreven gids biedt die aan). Een nieuwere versie uitpakken in dezelfde map behoudt *data*, dat wil zeggen je instellingen en add-ons. Het **zipbestand** bevat dezelfde map: pak het helemaal uit. Windows kan voor beide bestanden waarschuwen omdat ze niet zijn ondertekend (SmartScreen: *Meer informatie*, dan *Toch uitvoeren*).

Twee dingen zijn anders dan op Linux, vanwege Windows zelf (0.90.4). De toets **AltGr** typt zijn teken, zoals je verwacht: op een Duits toetsenbord geven AltGr+7, 8, 9, 0 en < de tekens { [ ] } en |, hoewel {{APP}} opdrachten heeft op Ctrl+Alt met deze toetsen; druk voor de opdrachten op de linker Ctrl- en Alt-toets. En bestanden die maar even nodig zijn (een opgeslagen webpagina die voor weergave is uitgepakt, of zijn kopie voor de browser) worden bewaard in de tijdelijke map van Windows en verwijderd wanneer {{APP}} sluit. Spraaktranscriptie gebruikt op Windows de processor, niet de videokaart.

Bij de eerste start zegt een gids waar het programma zijn bestanden bewaart en waarschuwt ze wanneer de map in de map van een synchronisatiedienst ligt (OneDrive, Dropbox, Google Drive ...) of een lang pad heeft. Ze vraagt ook om deze zaken, die **Help > Probleemoplossing en ontwikkeling > Deze computer…** en **Bestand > Opties > Algemeen > Deze computer** opnieuw tonen:

- **Back-ups**: er is geen standaard back-upmap. Tot je er een kiest, wordt geen automatische back-up gemaakt; de voettekstbalk van de eerste editor toont **Geen back-ups** naast de opslagtekst (klik erop om de map te kiezen), en de eerste back-up die zou lopen vraagt om een map. Het best is een ander station.
- **Notities en transcripties van het Kladblok**: in Documenten (de standaard), in de map van het programma of in een map van jezelf. Een wijziging geeft beide hulpmiddelen de nieuwe plek; wat ze eerder schreven, blijft waar het is.
- **Windows**: een invoer in het Startmenu, een snelkoppeling op het bureaublad en de bestandstypen van projecten (*.qbx*-bestanden openen met {{APP}}, en het wordt aangeboden onder **Openen met** voor de *.scrivx*-bestanden van Scrivener). Dit zijn de enige dingen die het programma buiten zijn map zet, elk alleen wanneer je het toevoegt, en elk met **Verwijderen**. Verwijder ze voordat je de map verwijdert of verplaatst.

Lettertypen die met **Opmaak > Lettertype > Lettertypebestanden installeren** zijn geïnstalleerd, blijven in de map van het programma en zijn alleen van {{APP}}; andere programma's zien ze niet.

De add-ons met programmacode hebben eigen Windows-versies (hun bestandsnamen eindigen op *-windows*): **Media afspelen** speelt af wat Windows zelf afspeelt, **Extra video- en audioformaten** voegt FFmpeg toe en **Spraaktranscriptie** schrijft spraak op met de processor (videokaarten worden op Windows nog niet gebruikt). **Vertalen en proeflezen** maakt ook op Windows deel uit van de volledige versie (0.92.0). De add-ons zonder programmacode (thema's, sjablonen, samenstelindelingen) zijn op elk systeem dezelfde bestanden.

### Op Android-tablets

De Android-versie van {{APP}} (experimenteel, gepubliceerd sinds 0.91.3) is een app voor tablets met Android 9 of later (64-bits ARM). Ze heeft dezelfde vensters als op een computer, gemaakt voor vingers: een vinger scrolt en tikt, twee vingers zoomen, een vinger die een halve seconde op iets blijft rusten opent het rechtsklikmenu daarvan (in een tekst doet een tik op geselecteerde tekst dat), en de werkbalken vouwen zich in zoveel rijen als het scherm nodig heeft. Download het APK-bestand op de tablet en open het; Android vraagt of de browser of de app Bestanden apps mag installeren, wat je voor deze installatie toestaat. Een nieuwe versie komt als nieuwe APK en installeert zich over de oude heen, met behoud van je instellingen en add-ons; **Help > Add-ons en updates > Nu op updates controleren** zegt wanneer er een is en opent de download in de browser (nieuwere add-ons worden binnen {{APP}} geïnstalleerd).

Met een extern toetsenbord en muis (sinds 0.92.0): Ctrl + muiswiel zoomt, en Ctrl + klik of Shift + klik kiest meerdere documenten, zoals op een computer. De lijn tussen twee vensterdelen toont een greep; een muis die enkele pixels ernaast wordt ingedrukt, neemt hem mee. De knop **Terug** van Android sluit het menu, de tip of het venster dat vooraan staat; elk venster heeft een knop die het sluit, en een venster dat groter is dan het scherm scrolt. Op een tablet verschijnt de punaise van de vastgezette mindmap op de geselecteerde binderrij (klik erop, of gebruik het rechtsklikmenu van de rij: *De mindmap van dit item vastzetten*, *De vastgezette mindmap losmaken*), en een tik op het hangslot van een rij toont wie eraan werkt. Een vinger op het overzicht van de mindmap werkt als de muis: een tik verplaatst de weergave daarheen, een vinger sleept de rechthoek van het getoonde deel, en de greep in de hoek verandert de grootte ervan. **Bestand > Opslaan als…** vraagt op een tablet eerst om de indeling en opent daarna het bestandsvenster.

Bij de eerste start vraagt {{APP}} om toegang tot de bestanden van de tablet. Daarmee gaan projecten naar *Documents/Quillbinder*, back-ups naar *Documents/Quillbinder/Backups*, exports naar *Documents/Quillbinder/Exports* en notities van het Kladblok naar *Documents/Quillbinder/Scratch Pad*, waar andere apps, een synchronisatie-app en een via een kabel aangesloten computer erbij kunnen; de knop opent de Android-pagina *Toegang tot alle bestanden*, waar je het voor {{APP}} aanzet. Zonder toegang blijft alles in de eigen map van de app, die geen andere app ziet en die Android met de app verwijdert; *Bestand > Bestanden op deze tablet* vraagt opnieuw en zegt waar dingen staan. Wanneer Android de app opzij zet (de Home-knop, een andere app), slaat {{APP}} meteen op.

De spellingwoordenboeken (Engels en Duits), de schrijfhulpmiddelen, de importers van webpagina's en oude Word-bestanden en (sinds 0.92.0) Vertalen en proeflezen maken deel uit van de app; Vertalen en proeflezen wordt bij de eerste start van een versie geïnstalleerd en downloadt pas een taalmodel wanneer je er een kiest. Add-ons voor Android (hun bestandsnamen eindigen op *-android-arm64*) worden geïnstalleerd met **Help > Add-ons en updates > Update of add-on installeren…**; het bestandsvenster opent in de map Downloads, en {{APP}} start opnieuw wanneer de add-on dat nodig heeft. Pakketten voor Linux of Windows en programma-updates worden met opgave van de reden geweigerd: de app zelf wordt alleen bijgewerkt met een nieuwe APK. Nog niet in de Android-versie: Spraaktranscriptie, audio en video in de editor, en crashrapporten. Tekst die binnen {{APP}} wordt gekopieerd, behoudt zijn opmaak wanneer je hem plakt; het klembord van Android bevat alleen platte tekst, dus tekst die naar andere apps wordt gekopieerd, komt als platte tekst aan.

---

## 2. Het hoofdvenster

Het venster heeft drie kolommen:

| Gebied | Wat het doet |
|------|--------------|
| **Binder** (links) | De inhoudsopgave van je hele project: elk document en elke map. |
| **Editor** (midden) | Toont wat je in de binder selecteerde: tekst, een prikbord met indexkaarten of een overzicht. |
| **Inspector** (rechts) | Informatie over het huidige document: samenvatting, notities, bladwijzers, metagegevens, momentopnamen en opmerkingen. |

De eerste keer dat {{APP}} start op een scherm smaller dan 1600 px, opent het venster gemaximaliseerd. De grootte, plaats en staat (gemaximaliseerd of niet) van het venster worden een moment na elke wijziging opgeslagen, zodat een crash ze niet kwijtraakt. Verberg de binder met **Ctrl+Alt+B** en de inspector met **Ctrl+Alt+I** om meer ruimte te krijgen. Dat is het venster zolang het breed is; een venster dat hoger is dan breed, rangschikt dezelfde drie gebieden anders (volgende sectie). De kleuren en lettertypen van alle drie, en van het prikbord, het overzicht en de schrijfmodus, komen uit het thema (**Weergave > Thema's**) en kunnen per gebied worden gewijzigd (sectie 16), ook met een rechtsklik op het gebied (**Uiterlijk**). Elk project bewaart zijn eigen uiterlijk.

De **werkbalk** bovenaan bevat, van links naar rechts: de knop **Weergave** (binder, verzamelingen, inspector, lay-outs), nieuw document (met een menu voor mappen, sjablonen en importeren), nieuwe map, prullenbak, **ongedaan maken** en **opnieuw**, de knop **zoeken** (projectzoekopdracht; de pijl ernaast toont de andere zoekhulpmiddelen: Zoeken in document, Project vervangen, Zoeken op opmaak), het veld **snel zoeken**, de drie weergaveknoppen (tekst, prikbord, overzicht), projectbladwijzers, snelle naslag, opmerkingen en voetnoten, samenstellen, schrijfmodus, de **microfoon** van Spraaktranscriptie (wanneer die add-on is geïnstalleerd) en de inspectorknop. Jij bepaalt wat erop staat (zie "Werkbalken van jezelf" hieronder).

De **opmaakbalk** eronder bevat de stijl, het lettertype, de grootte, de regelafstand, de standaard tekstopmaak, het lettertypevenster, vet, cursief, onderstrepen, doorhalen, markeren, tekstkleur, uitlijning, lijsten, de ruimte voor en na de alinea, de inspringing van de eerste regel en van de hele alinea, opmerkingen, voetnoten, inline-aantekeningen en afbeeldingen. Toon of verberg de twee balken met **Weergave > Werkbalken**.

**Dialoogvensters.** Een venster dat het programma opent (Opties, Samenstellen, Projectdoelen ...) is nooit groter dan het scherm zonder de panelen van het bureaublad: op een klein laptopscherm scrollt de inhoud of wordt kleiner, en blijven de knoppen in beeld. Wanneer het sluit, gaat het toetsenbord terug naar waar het was, meestal de tekst, zodat je meteen verder kunt typen.

**Waar ben ik?** Dubbelklik op de ruimte naast de tekst (het papier naast de kolom of het gebied eromheen): de binder selecteert het document naast de klik, opent de mappen erboven en laat zijn rij twee keer oranje knipperen. De editor behoudt wat hij toont. **Navigeren > In de Binder tonen** (**Ctrl+Shift+R**) doet hetzelfde voor het document bij de cursor.

De **voettekst** van elke editor toont het aantal woorden en tekens (selecteer tekst om alleen de selectie te tellen) en het doel voor het document. Klik op het aantal voor details. Wanneer de editor een map toont (of een document met subdocumenten), toont de voettekst ook het **totaal** van de map: de woorden van de eigen tekst en van alles erin, bijvoorbeeld "Map: 12.345 woorden". Het prikbord en het overzicht tonen hetzelfde totaal in het midden van hun voettekst; bij meerdere getoonde items is het het totaal van alle items. Een klik op een totaal toont de details (woorden, tekens, tekens zonder spaties, alinea's en pagina's), zoals het aantal van een tekst dat doet. De inspector toont het onder **Totaal** in de metagegevens van een map, en de muis op een map in de binder laten rusten toont het ook (met de eigen woorden van de map wanneer die een eigen tekst heeft).

### De menu's

| Menu | Wat het bevat |
|------|---------------|
| **Bestand** | Projecten, opslaan, de projectindeling ({{APP}} of Scrivener), back-ups, importeren en exporteren, vergelijken en overbrengen met een Word- of LibreOffice-bestand, samenstellen, afdrukken, bestandsconversie, opties |
| **Bewerken** | Ongedaan maken, klembord, selecteren, zoeken en vervangen (in de tekst, in het project, op opmaak), documenten in de binder verplaatsen, schrijfhulpmiddelen |
| **Invoegen** | Afbeeldingen, tabellen, koppelingen naar documenten en objecten, weblinks, opmerkingen, voetnoten, mediatijdstempels, automatische nummers, plaatsvervangers, speciale tekens, datum en tijd |
| **Weergave** | De drie weergavemodi, de gesplitste editor (**Editor splitsen**), de plaatsen van binder, editor en inspector en de opgeslagen lay-outs (**Lay-out**), de kleurthema's (**Thema's**), binder, verzamelingen en inspector (en **Snelle toetsen in de Binder**), werkbalken, opties voor tekstbewerking (paginaweergave, liniaal, typemachinescrollen), prikbord- en overzichtsopties, labelkleuren, zoom, schrijfmodus, volledig scherm |
| **Navigeren** | Terug en vooruit, vorig en volgend document, documenten openen in de andere editor, in de kopijhouder of als snelle naslag, de editorvergrendeling, in welke editor de binder laadt, de tabbladen van de inspector (allemaal, onder de namen van hun knopinfo: **Samenvatting en notities**, **Bladwijzers**, **Metagegevens**, **Momentopnamen**, **Opmerkingen en voetnoten**, de mindmap, de objecten en de tabbladen van schrijfhulpmiddelen) |
| **Project** | Nieuwe documenten, projectinstellingen (ook opgeslagen in en geladen uit een bestand), doelen, trefwoorden, statistieken, schrijfgeschiedenis, prullenbak, oude momentopnamen verwijderen |
| **Documenten** | Momentopnamen, hernoemen, dupliceren, splitsen, samenvoegen, omzetten, pictogram, label, status, bladwijzers |
| **Opmaak** | Lettertype (met lettertypen installeren), alinea, stijl, lijsten, markeren, opmaak kopiëren, revisiemodus |
| **Venster** | Kladblok, vensters met snelle naslag |
| **Help** | **Opdracht zoeken**, deze handleiding, de Snelstartgids, het welkomstproject, de lijst van alle plaatsvervangers, de lijst met sneltoetsen, de help voor mindmaps, het submenu **Add-ons en updates** (Add-onbeheer, Update of add-on installeren, de controle op updates), {{APP}} aan het toepassingsmenu toevoegen (en weghalen), het submenu **Probleemoplossing en ontwikkeling** (systeeminformatie, Deze computer, crashrapporten, Pluginmap openen, de ontwikkelaarsgids), Een probleem melden, de licenties van derden, Over |

**Opdracht zoeken** (**Help > Opdracht zoeken…**) opent een venster met een zoekveld en de lijst van alle opdrachten van de menu's, elk met zijn menu en zijn sneltoets. Typ een deel van een naam (meerdere woorden verfijnen het, in willekeurige volgorde); **Return** of een dubbelklik voert de gekozen opdracht uit. Een opdracht die op dat moment uit staat, is grijs en wordt niet uitgevoerd. Ze heeft zelf geen sneltoets.

Een submenu toont korte namen: **Bestand > Importeren > Bestanden**, **Documenten > Splitsen > bij selectie**, **Navigeren > Openen > als snelle naslag**. De sneltoets van een opdracht staat ernaast en kan worden gewijzigd (sectie 19). Elk item van een menu heeft een eigen letter (onderstreept wanneer je **Alt** ingedrukt houdt), dus met **Alt** en de letters bereik je elke opdracht.

Zonder open project zijn de menu's **Invoegen**, **Navigeren**, **Project**, **Documenten** en **Opmaak** grijs, evenals de opdrachten van de andere menu's die een tekst of een binder nodig hebben (Zoeken, Zoom, de editorindeling, Focus op Binder ...). Wat zonder project werkt: projecten maken, openen en importeren, bestanden omzetten, updates en add-ons installeren, de opties, thema's en lay-outs, de schrijfhulpmiddelen die geen project nodig hebben, en Help.

### De vensterdelen naar de vorm van het venster

Het venster rangschikt de binder, de editor en de inspector naar zijn vorm, en verandert vanzelf wanneer de vorm verandert (je sleept het venster hoger dan breed, of je draait een tablet). Een vierkant venster telt als breed.

- **Breed venster** (breder dan hoog, of vierkant): drie kolommen. Kies hun volgorde onder **Weergave > Lay-out > Breed venster**: elk van de zes volgordes van binder, editor en inspector. De breedtes worden bepaald door de lijnen tussen de kolommen.
- **Staand venster** (hoger dan breed): twee rijen onder de werkbalken. Eén rij is de editor, over de volle breedte van het venster; de andere bevat de binder en de inspector naast elkaar. Onder **Weergave > Lay-out > Staand venster** zet **Editor in het midden** de rij van de editor boven de andere, **Editor onderaan** eronder; **Binder links** en **Binder rechts** kiezen de kant van de binder (de inspector neemt de andere). De hoogtes van de twee rijen en de breedtes van de binder en de inspector worden bepaald door de lijnen ertussen. Terwijl het schermtoetsenbord zichtbaar is (Android), behoudt de rij van de binder en de inspector haar hoogte; verplaats de lijn als je het anders wilt.
- Een verborgen binder of inspector laat zijn ruimte aan de ander in de staande rij, over de volle breedte; als beide verborgen zijn, heeft de editor het hele venster. Twee editors (**Weergave > Editor splitsen**) blijven binnen de plaats van de editor.
- **Weergave > Lay-out > Lay-out herstellen** zet de vorm die het venster nu heeft terug op zijn beginwaarden: binder, editor, inspector in een breed venster; de editor in het midden en de binder links in een staand venster.

Beide groepen staan altijd in het menu; elk werkt zodra het venster die vorm heeft, en elke vorm komt terug zoals je hem achterliet (plaatsen en groottes). De keuzes horen bij deze computer of tablet en worden bewaard in de instellingen van het programma, nooit in een project: een project dat je tussen computers deelt, bewaart de indeling van elk apparaat, en er wordt niets over in de bestanden van het project geschreven.

### Lay-outs

Een lay-out onthoudt hoe het venster is ingedeeld: of binder, verzamelingen, inspector, werkbalk en opmaakbalk worden getoond, hoe breed de kolommen zijn, hoe de editor is gesplitst, welke weergave elke editor gebruikt en in welke editor de binder laadt. Hij zegt niets over documenten, dus elke lay-out werkt in elk project.

Kies een lay-out onder **Weergave > Lay-out** of via de knop **Weergave** van de werkbalk. Zeven lay-outs zijn ingebouwd:

| Lay-out | Indeling |
|--------|-------------|
| **Standaard** | Binder, één editor, inspector |
| **Binder en editor** | Binder en tekst, geen inspector |
| **Alleen editor** | Alleen de tekst met de twee balken |
| **Overzicht en editor** | Overzicht links, tekst rechts. De binder laadt groepen in het overzicht; een klik op een rij opent dat document rechts |
| **Prikbord en editor** | Prikbord boven, tekst onder, op dezelfde manier verbonden |
| **Twee editors** | Twee teksten naast elkaar |
| **Plannen** | Binder, prikbord en de inspector met samenvatting en notities |

**Weergave > Lay-out > Huidige lay-out opslaan** bewaart het venster zoals het nu is onder een naam naar keuze; vink "Ook de grootte van het venster onthouden" aan als de lay-out het venster moet herschalen. **Lay-outs beheren** toont alle lay-outs met een klein plaatje en laat je je eigen lay-outs gebruiken, bijwerken (vervangen door het huidige venster), hernoemen, verwijderen, exporteren en importeren. Ingebouwde lay-outs kunnen niet worden gewijzigd; sla een gewijzigd venster in plaats daarvan onder een nieuwe naam op. Elke lay-out kan een eigen sneltoets hebben (sectie 19, groep "Lay-outs").

### De werkbalken en hun plaatsen

Sinds 0.91 heeft het venster acht werkbalken in twee rijen. De eerste rij bevat **Hoofd** (Weergave, nieuwe tekst, nieuwe map, prullenbak, Ongedaan maken, Opnieuw), **Thema's** (de themaknop, eerst uit), **Zoeken** (de zoekknop, het veld voor snel zoeken, de weergavemodi) en **Hulpmiddelen** (bladwijzers, Snelle naslag, opmerkingen, Samenstellen, Onzichtbare tekens tonen, Schrijfmodus, de knoppen van schrijfhulpmiddelen, de inspector). De tweede rij bevat **Stijl en lettertype**, **Teken en uitlijning**, **Alinea en invoegen** en de **pictogrammenbalk**.

- **Een werkbalk verplaatsen.** Elke werkbalk heeft vooraan een greep (twee kolommen stippen; de aanwijzer verandert in een kruis met pijlen). Sleep de greep, en de werkbalk gaat waar je hem heen brengt:
  - **Langs zijn rij.** Zolang de aanwijzer tussen de middens van de twee naburige werkbalken blijft, volgt de werkbalk hem meteen. De vrije ruimte voor een werkbalk is zo breed als jij hem maakt. Naar rechts neemt de werkbalk eerst de vrije ruimte erachter; wanneer hij de volgende werkbalk bereikt, duwt hij die mee, zover als de werkbalken erachter bij elkaar kunnen staan aan het eind van de rij. Naar links stopt hij waar de werkbalk ervoor eindigt, en blijven de werkbalken erachter staan waar ze staan (sleep ze ernaartoe als je ze weer bij elkaar wilt). De eerste werkbalk van een rij blijft aan de rand van het venster.
  - **Naar een andere plek in de volgorde.** Breng de aanwijzer voorbij het midden van een naburige werkbalk, of naar een andere werkbalk van de rij. De werkbalk blijft voorlopig waar hij is, en een lijn in de accentkleur laat zien waar hij komt te staan; laat de muisknop los om hem daar neer te zetten. Elke volgorde is dus mogelijk, ook voor de eerste en de laatste werkbalk van een rij.
  - **Naar een andere rij.** Breng de aanwijzer naar de andere rij: de lijn toont daar de plek. Elke werkbalk kan in elke rij staan, een opmaakwerkbalk naast de hoofdwerkbalk of de zoekwerkbalk tussen de opmaakwerkbalken.
  - **Naar een eigen rij, of naar de onderrand.** Breng de aanwijzer vlak onder de werkbalkrijen (een lijn over het venster verschijnt) om een nieuwe rij te openen, of naar de onderrand van het venster om de werkbalk daar neer te zetten. De onderrand neemt werkbalken naast elkaar en in rijen, zoals de bovenkant.
  - **Esc**, of loslaten waar geen lijn verschijnt (bijvoorbeeld midden in het venster), laat alles zoals het was.

  De werkbalken achter de oude plek sluiten aan, en vrije ruimte die voor de werkbalk was, blijft in zijn rij; op zijn nieuwe plek staat de werkbalk direct achter de werkbalk ervoor, en je kunt hem vandaar verder naar rechts trekken. In een venster dat te smal is voor de vrije ruimte die je liet, krimpt de ruimte voordat een werkbalk knoppen verliest; ze is terug wanneer het venster weer breed is. Een werkbalk die is uitgezet, behoudt zijn plaats. Een werkbalk kan niet aan de zijkanten staan of als eigen venster zweven. {{APP}} onthoudt de rijen, de volgorde en de ruimten.
- **Een werkbalk uitzetten.** **Weergave > Werkbalken** heeft een schakelaar voor elke werkbalk; dezelfde lijst komt met een rechtsklik op een werkbalk of op de vrije ruimte naast de werkbalken. **Werkbalk tonen** en **Opmaakbalk tonen** schakelen een hele rij zoals voorheen (de vier balken van de eerste rij, de drie opmaakwerkbalken), en de lay-outs gebruiken deze twee; de schakelaar van één balk is niet beschikbaar zolang zijn rij is uitgezet.
- **Weergave > Werkbalken > Werkbalkposities herstellen** zet alle werkbalken terug op hun standaardplaatsen (de twee rijen in hun standaardvolgorde, niets aan de onderrand) en sluit de ruimten ertussen.

### Werkbalken van jezelf

**Weergave > Werkbalken > Werkbalken aanpassen…** (of een rechtsklik op een balk) opent een venster met twee lijsten: de beschikbare opdrachten links, de inhoud van de balk rechts. Kies bovenaan de werkbalk.

- Voeg een opdracht toe met de pijlknop of een dubbelklik. Elke opdracht van de menu's is beschikbaar, ook die van schrijfhulpmiddelen (Bewerken > Schrijfhulpmiddelen); het zoekveld boven de lijst vindt ze.
- Verwijder een knop met de andere pijl of een dubbelklik, en verplaats hem met de knoppen omhoog en omlaag of door te slepen.
- **Scheidingsteken** en **Flexibele ruimte** kun je zo vaak gebruiken als je wilt. Een flexibele ruimte duwt wat erop volgt naar rechts; twee ervan centreren wat ertussen staat.
- Sommige items zijn meer dan één opdracht: **Weergave**, **Toevoegen**, **Zoeken**, het **veld Snel zoeken**, **Weergavemodus**, **Projectbladwijzers**, **Opmerkingen en voetnoten**, **Lay-outs** en **Thema's** (de themaknop met zijn menu) voor de vier werkbalken van de eerste rij; de lijsten voor stijl, lettertype, grootte en regelafstand en de vier alineaknoppen voor de drie opmaakwerkbalken. Zo'n besturingselement bestaat één keer: aan een werkbalk toegevoegd, verhuist het daarheen van de werkbalk die het had. Een besturingselement van de eerste rij kan niet naar een opmaakwerkbalk, en andersom ook niet.
- Onder de lijsten kies je kleine, middelgrote of grote pictogrammen en of de werkbalken van de eerste rij tekst naast of onder de pictogrammen tonen.
- **Standaardset** zet de knoppen waarmee het programma kwam terug in de balk.

Als een balk meer bevat dan in het venster past, opent een pijl aan zijn eind de rest. Een werkbalk zonder enige knop wordt uitgezet; weer aangezet onder **Weergave > Werkbalken** heeft hij zijn standaardset. Een werkbalk die je in een versie vóór 0.91 had ingericht, werd over de nieuwe werkbalken verdeeld: elke knop ging naar de werkbalk die hem in de standaardsets bevat, en wat je had weggehaald, blijft weg.

### De pictogrammenbalk

De **pictogrammenbalk** bevat maximaal tien documentpictogrammen die je vaak gebruikt. Een klik op een ervan geeft dat pictogram aan de geselecteerde documenten en mappen (in de binder, of in het prikbord of overzicht waarin je werkt); de knop **↺** erna geeft hun het standaardpictogram terug, en de laatste knop kiest de pictogrammen van de balk. Toon of verberg de balk met **Weergave > Werkbalken > Pictogrammenbalk**.

**Weergave > Werkbalken > Pictogrammen voor de pictogrammenbalk kiezen…** (ook in het rechtsklikmenu van de balk) opent een venster met links alle pictogrammen, inclusief je eigen projectpictogrammen, en rechts de pictogrammen van de balk. Voeg toe met **Toevoegen →** of een dubbelklik, verwijder met **← Verwijderen** of een dubbelklik, en wijzig de volgorde met **Omhoog** en **Omlaag**. **Standaardset** zet de tien pictogrammen terug waarmee het programma kwam (Groen boek, Proeflezen, Aangevinkt, Niet aangevinkt, Vraagteken, Rode vlag, Geel notitieblok, Personagebld, Locatieblad, Waarschuwing).

---

## 3. De binder

Elk project heeft drie speciale mappen:

- **Concept** (in sommige sjablonen *Manuscript*): alles wat je boek wordt. Alleen wat in het Concept staat, wordt samengesteld.
- **Onderzoek**: notities, personagebladen, afbeeldingen, pdf's en al het andere wat je verzamelt.
- **Prullenbak**: verwijderde items wachten hier tot je **Project > Prullenbak legen…** kiest. De vraag daarbij, zoals elke vraag voordat iets gebeurt wat niet ongedaan kan worden gemaakt, noemt de stap op zijn knop en heeft **Annuleren** als standaard, zodat de Enter-toets nooit iets verwijdert (0.70).

### Met items werken

- **Nieuw document**: **Ctrl+N** (of de knop ➕). Het wordt onder het geselecteerde item toegevoegd. Een document met de naam "Untitled" (of zonder titel) neemt de eerste regel van zijn tekst als titel zodra je schrijft: maximaal 60 tekens, afgekapt bij een woord, zonder inline-aantekeningen, inline-voetnoten en afbeeldingen. De titel volgt de eerste regel tot je het document zelf hernoemt; een document waarvan je de tekst verwijdert, heet weer "Untitled". "Untitled"-documenten die al tekst hebben, krijgen hun naam wanneer het project opent. De titel is een gewone titel: hij wordt opgeslagen, samengesteld (wanneer titels worden samengesteld, verschijnt een eerste regel die een kop is twee keer) en in Scrivener-projecten geschreven.
- **Nieuwe map**: **Alt+Shift+N**.
- **Hernoemen**: **F2**, of dubbelklik langzaam op de titel.
- **Verplaatsen**: sleep items met de muis, of gebruik **Ctrl+Alt+pijltoetsen**: Omhoog en Omlaag verplaatsen binnen de map; Links verplaatst uit de map; Rechts verplaatst in het item erboven. Een document dat *tussen* twee items wordt neergezet, komt daar te staan; neergezet *op* een item wordt het een subdocument daarvan. Als je per ongeluk documenten nest, zet dan het neerzetten op tekstdocumenten uit onder **Bestand > Opties > Gedrag** (mappen accepteren altijd neergezette items).
- **Een hele tak openen of sluiten**: houd **Alt** ingedrukt en klik op de pijl van een map of groep.
- **Verwijderen**: druk op **Delete**. Het item gaat naar de Prullenbak; een prikbord of overzicht dat het toonde als een van meerdere gekozen documenten laat het meteen los (0.94.0).
- **Dupliceren**: **Ctrl+D**.
- **Splitsen en samenvoegen**: **Documenten > Splitsen > bij selectie** (**Ctrl+K**) knipt het document bij de cursor in twee; het werkt niet binnen een tabel of het bijschrift van een afbeelding (zet de cursor ervoor of erna). **Documenten > Samenvoegen** (**Ctrl+Shift+M**) voegt de geselecteerde documenten samen in het eerste, met hun samenvattingen, trefwoorden en notities; de andere gaan naar de Prullenbak.
- **Het pictogram wijzigen**: rechtsklik > **Pictogram wijzigen** (of **Documenten > Pictogram wijzigen**), of de pictogrammenbalk (sectie 2). Zie "Pictogrammen" hieronder.

**Rechtsklik.** Op een document heeft het rechtsklikmenu de opdrachten voor de geselecteerde documenten (hernoemen, dupliceren, naar de Prullenbak verplaatsen, momentopnamen, label, status en meer). Op de lege ruimte onder de documenten biedt het alleen aan wat iets nieuws maakt: **Nieuwe tekst**, **Nieuwe map**, **Nieuw uit sjabloon** en het **Uiterlijk** van de binder.

Een document kan subdocumenten hebben, net als een map. Mappen en documenten gedragen zich hetzelfde; mappen zien er alleen anders uit en openen standaard als prikbord.

**De toetsen van de binder (0.94.0).** Met het toetsenbord in de binder opent **Return** het geselecteerde document in de editor en zet het toetsenbord daar (zoals een bestandsbeheer doet), en de pijltoetsen bewegen door de lijst. **Spatie** hoort bij het typen van een naam: typ de eerste letters van een titel, ook spaties ("Hoofdstuk t"), en de selectie springt naar de eerste titel die zo begint. Het item **Weergave > Snelle toetsen in de Binder** staat eerst uit (ook na een update). Wanneer het aanstaat, heeft de binder de snelle toetsen: **Return** maakt een nieuw leeg document onder het geselecteerde en **Spatie** opent de geselecteerde documenten als panelen Snelle naslag (en in de lijst van een verzameling of een zoekopdracht).

Een klik in de binder laadt het document in de editor. Na **Terug**, een documentkoppeling of met een vergrendelde editor kan de editor een ander document tonen dan het in de binder geselecteerde: een **grijze balk** markeert dan het document van de editor in de binder. **Navigeren > In de Binder tonen** (**Ctrl+Shift+R**) selecteert het.

**Smalle binder, diepe mappen.** Elk niveau van mappen springt 12 px in (de helft van wat het vroeger was; wijzig het onder **Bestand > Opties > Uiterlijk > Binder > Inspringing per niveau**). Wanneer titels langer zijn dan de binder breed is, verschijnt onderaan de binder een schuifbalk, zodat je het eind van lange titels kunt lezen zonder de binder te verbreden.

### Pictogrammen

**Documenten > Pictogram wijzigen** (ook in het rechtsklikmenu van de binder, het prikbord en het overzicht) is gebouwd als het pictogrammenmenu van Scrivener 3 voor Windows:

- **Pictogram standaard maken**: het standaardpictogram van een document, map of bestand komt terug.
- **Pictogram uit tekst…**: maakt een pictogram van maximaal drie tekens (bijvoorbeeld *PR*, *H1* of *A*). Kies de kleur van het vak, van de letters en van de rand; het plaatje toont het resultaat. Het pictogram wordt in het project bewaard en meteen aan de geselecteerde documenten gegeven.
- **Pictogrammen beheren…**: de pictogrammen van dit project. **Afbeelding toevoegen…** maakt van afbeeldingen (PNG, SVG, JPEG en andere) pictogrammen, **Uit tekst…** is hetzelfde als hierboven, **Hernoemen…** en **Verwijderen** wijzigen de lijst (documenten met een verwijderd pictogram krijgen hun standaardpictogram terug), **Gebruiken voor geselecteerde documenten** geeft het gekozen pictogram aan de geselecteerde documenten. De pictogrammen worden in het project bewaard (map *icons*), zodat ze meereizen.
- **Recent**: de laatste vijf pictogrammen die je koos.
- **Boek**, **Personages**, **Vlag**, **Locaties**, **Notities**, **Te doen**, **TV**, **Tussenstation**: submenu's met de pictogrammen van elke groep in meerdere kleuren (bijvoorbeeld blauw, groen, rood en geel boek; zeven vlaggen; vijf notitieblokken; Aangevinkt en Niet aangevinkt, Geslaagd, Mislukt en Onduidelijk in Te doen).
- Daarna de losse pictogrammen: Achterwerk, Schoolbord, Kalender, Klapbord, Klok, Tandwiel, Oog, Cijfers en grafieken, Archiefkast, Voorwerk, Informatie, Katheder, Gloeilamp, Vergrootglas, Muziek, Muzieknoten, Overzicht, Potlood, Proeflezen, Vraagteken, Opmerkingen van de recensent, Schaar, Spraakwolkje, Aanhalingstekens, Statistieken, Synchronisatiemap, Tabellen, Reageerbuis, Gedachtewolkje en Waarschuwing.
- **Projectpictogrammen**: je eigen pictogrammen (wanneer het project er heeft), en **Meer pictogrammen (bureaublad)**: de pictogrammen van het bureaubladthema die eerdere versies aanboden.

**Labelkleur in pictogrammen.** De witte delen van veel pictogrammen (de bladzijden van een notitieblok, de doek van een witte vlag, het papier van een blad, het vak van een pictogram uit tekst) nemen de labelkleur van het document aan. Zet dit aan of uit met **Weergave > Labelkleur gebruiken in > Pictogrammen** (of **Bestand > Opties > Uiterlijk > Binder**).

**Pictogrammen en Scrivener.** De pictogrammen van {{APP}} zijn opnieuw getekend, maar elk draagt de naam van het Scrivener-pictogram waarvoor het staat. Een project in Scrivener-indeling (sectie 14) bewaart deze namen, zodat Scrivener er zijn eigen pictogram voor toont; pictogrammen die in Scrivener zijn gemaakt, komen hier terug als het bijpassende pictogram. Je eigen pictogrammen gaan in de map *Icons* van de *.scriv*. Een pictogram waarvoor dit programma geen afbeelding heeft, behoudt zijn naam en wordt hier als het dichtstbijzijnde pictogram getoond.

### Labels, status en "Opnemen bij samenstellen"

Klik met de rechtermuisknop op een item om zijn **label** (een kleur, bijvoorbeeld voor het personage van wie het gezichtspunt is) en zijn **status** (bijvoorbeeld *Eerste versie* of *Klaar*) in te stellen. Wijzig de lijsten onder **Project > Projectinstellingen…**.

Een document of map met een label toont zijn kleur ook in de inspector: de samenvattingskaart en de documentnotities krijgen een lichte tint van de labelkleur (15 % van de verzadiging), zodat je in één oogopslag ziet welk label het item heeft. Items zonder label, of met een grijs label, behouden de normale kaartkleur.

Items waarvan **Opnemen bij samenstellen** is uitgezet, blijven in het Concept staan maar worden weggelaten wanneer je samenstelt.

### Verzamelingen

Verzamelingen zijn extra lijsten van documenten, bijvoorbeeld "Onderzoek nodig" of "Scènes met Anna". Toon ze met de knop **Verzamelingen**. Gebruik de knop ➕ boven de binder om een verzameling te maken van de geselecteerde documenten, of klik met de rechtermuisknop op documenten en kies **Aan verzameling toevoegen**. Een document uit een verzameling halen verwijdert het niet. Opgeslagen zoekopdrachten (sectie 9) zijn ook verzamelingen; ze werken zichzelf bij.

- **Documenten toevoegen** doe je door ze uit de binder (of uit het kopicoon van een editor, een prikbord, een overzicht) naar het tabblad van de verzameling te slepen.
- **In de lijst** van een verzameling sleep je documenten in de volgorde die je wilt (de volgorde van de binder verandert niet), zet je documenten op een plek neer, en druk je op **Delete** om de geselecteerde documenten uit de verzameling te halen. Zoekverzamelingen kunnen niet op deze manier worden gewijzigd.
- **Sleep de tabbladen** om de verzamelingen in een andere volgorde te zetten.
- Documenten in de lijst kunnen worden gesleept zoals die in de binder, en **Spatie** opent de geselecteerde als panelen Snelle naslag.

---

## 4. Schrijven

### Tekstopmaak

| Actie | Sneltoets |
|--------|----------|
| Vet / cursief / onderstrepen | Ctrl+B / Ctrl+I / Ctrl+U |
| Markeren | Ctrl+Shift+H |
| Superscript / subscript | Ctrl+Shift++ / Ctrl+= (Duitse indeling: Ctrl+Shift+D voor subscript, zie "Sneltoetsen en de toetsenbordindeling" hieronder) |
| Links / gecentreerd / rechts uitlijnen | Ctrl+{ / Ctrl+\| / Ctrl+} (Duitse indeling: Alt+Shift+L / Alt+Shift+E / Alt+Shift+R) |
| Lettertypevenster | Ctrl+T |
| Lettergrootte groter / kleiner | Ctrl+> / Ctrl+< |
| Alineavenster | Ctrl+Alt+P |
| Opmaak kopiëren / opmaak plakken | Ctrl+Alt+C / Ctrl+Alt+V |
| Inzoomen / uitzoomen / zoom herstellen | Ctrl++ / Ctrl+- / Ctrl+0 |
| Het gebied onder de muis zoomen | Ctrl+muiswiel |
| Liniaal tonen of verbergen | Ctrl+Alt+R |
| Plakken zonder opmaak | Ctrl+Shift+V |
| Plakken met opmaak (uit andere programma's) | Ctrl+Alt+Shift+V |
| Ongedaan maken / opnieuw | Ctrl+Z / Ctrl+Shift+Z (zie "Ongedaan maken en opnieuw" hieronder) |

Vet, cursief, superscript en dergelijke werken op de selectie. Zonder selectie veranderen ze binnen een woord het hele woord; aan het eind van een woord (of met niets rond de cursor) veranderen ze wat je hierna typt: typ "H", druk op Ctrl+=, typ "2", druk nogmaals op Ctrl+= en typ "O" voor H₂O.

**Stijlen** (Kop 1–6, Titel, Ondertitel, Blokcitaat en je eigen) kies je in de lijst links van de opmaakbalk. Stijlen vertellen Samenstellen wat een alinea is, dus gebruik ze in plaats van tekst met de hand groot en vet te maken. Zie "Stijlen" hieronder.

**Wat de opmaakbalk toont.** De balk toont de tekst zoals je hem ziet, voor de hele selectie. **Vet**, **Cursief**, **Onderstrepen**, **Doorhalen**, superscript en subscript staan aan wanneer alle geselecteerde tekst zo oogt, ook wanneer de stijl het zo maakt (het vet van een kop). Een klik schakelt wat je ziet om: **Vet** op een vette kop maakt hem niet vet, en hij blijft niet vet wanneer het project wordt opgeslagen en opnieuw geopend. Het lettertype, de grootte, de regelafstand, de uitlijningsknoppen en de stijllijst tonen de selectie ook; wanneer de selectie twee lettertypen, twee groottes, twee uitlijningen of alinea's van twee stijlen bevat, blijft dat veld leeg (geen uitlijningsknop staat aan). Een grootte die in het lege veld wordt getypt, geeft de hele selectie die grootte.

**Lijsten**: gebruik de lijstknoppen, of typ "- " of "1. " aan het begin van een alinea.

**Slimme typografie**: rechte aanhalingstekens worden gekrulde aanhalingstekens, en twee koppeltekens worden tijdens het typen een gedachtestreep. Zet dit uit onder **Bestand > Opties > Bewerken**.

**Revisiemodus** (**Opmaak > Revisiemodus**): alles wat je typt wordt gekleurd met een van vijf revisiekleuren, zodat je kunt zien wat er in een bewerkingsronde is gewijzigd. Kies **Uit** om te stoppen.

**Ruimte en inspringingen in de opmaakbalk.** Vier knoppen na de lijstknoppen stellen de alinea's in die de cursor of de selectie bevatten: de **ruimte voor** en de **ruimte na** de alinea (0, 3, 6, 9, 12 of 18 pt), de **inspringing van de eerste regel** en de **inspringing van de hele alinea** (in centimeters of inches, zoals de liniaal ze toont). De waarde van de alinea bij de cursor is aangevinkt; **Anders…** vraagt om een willekeurige waarde. **Bewerken > Ongedaan maken** neemt een wijziging terug. Een opmaakbalk die je zelf hebt ingericht, krijgt de vier knoppen één keer, na de lijstknoppen; **Weergave > Werkbalken > Werkbalken aanpassen…** verplaatst of verwijdert ze.

**Ongedaan maken en opnieuw.** **Bewerken > Ongedaan maken** en **Bewerken > Opnieuw** werken in het tekstveld dat het toetsenbord heeft (de editor, de samenvatting, de notities, een titel) en zeggen wat ze zullen terugnemen: "Typen ongedaan maken", "Plakken ongedaan maken", "Vet ongedaan maken", "Verwijderen opnieuw". Met het toetsenbord elders, bijvoorbeeld in de binder, zijn ze grijs: documenten naar de Prullenbak verplaatsen, de Prullenbak legen en momentopnamen verwijderen kunnen op deze manier niet ongedaan worden gemaakt (een document in de Prullenbak komt terug door het eruit te slepen; zie sectie 11 voor momentopnamen). De pijl naast de knoppen **Ongedaan maken** en **Opnieuw** van de werkbalk toont de stappen, de eerstvolgende bovenaan; een stap kiezen neemt die stap en elke stap ervoor in de lijst terug (of brengt ze terug). Elke opdracht van het menu Opmaak is één stap.

**Tekstkleur en markering in de opmaakbalk.** De twee knoppen met een gekleurde balk (een pen om te markeren, een letter *T* voor de tekstkleur) werken zoals die van Scrivener en Word: een klik past de kleur uit de balk toe op de geselecteerde tekst, of, zonder selectie, op wat je hierna typt. Laat de muis even op een knop rusten en een klein venster met **rood, blauw, geel, groen, wit en zwart** opent eronder, plus **Geen markering** (of **Automatisch** voor de tekstkleur) en **…** voor een andere kleur. Een klik op een kleur past hem toe en maakt hem de kleur van de knop, die hem bewaart voor de volgende klik (ook na een herstart).

### Lettertypen

Alle tekst van een project heeft één uiterlijk: de **standaardopmaak** (zie hieronder). Daaroverheen kun je elk stuk tekst een eigen lettertype geven.

- **Opmaakbalk**: de lettertypelijst en het grootteveld wijzigen de geselecteerde tekst meteen. Zonder selectie wijzigen ze de **hele alinea** waarin de cursor staat (en wat je daar hierna typt). De pijlen van het grootteveld wijzigen de tekst bij elke klik; een grootte die je typt, gaat in met **Enter**. De opmaakbalk, het menu Opmaak en hun sneltoetsen werken op de tekst waarin de cursor het laatst stond: een editor, de notities in de inspector, de tekst van een bladwijzer in de inspector, of een lang tekstveld van een object (daar: vet, cursief, onderstrepen, doorhalen, tekstkleur en markering). Een klik in het grootteveld verandert dat niet.
- Na een wijziging in de opmaakbalk biedt een balk boven de editor hetzelfde aan voor het hele project: in een alinea met een stijl (bijvoorbeeld een kop) geeft **De stijl “Kop 1” wijzigen** de stijl het nieuwe lettertype of de nieuwe grootte, en volgt alle tekst van die stijl in elk document; in een alinea zonder stijl maakt **Voor alle tekst gebruiken** er het lettertype of de grootte van het project van (zie "Standaardopmaak" hieronder). Klik op **×**, of schrijf gewoon elders verder, om de wijziging te laten zoals ze is. Elk document dat verandert, krijgt eerst een momentopname.
- **Opmaak > Lettertype > Lettertypen tonen…** (**Ctrl+T**) opent het lettertypevenster: lettertype, lettersnede (licht, normaal, vet, cursief en wat het lettertype verder biedt), grootte, onderstrepen, doorhalen, hoofdletters (kleinkapitalen of alleen hoofdletters), letterafstand in procenten en tekstkleur. Alleen wat je in het venster wijzigt, wordt toegepast, dus een selectie met vette en gewone woorden behoudt die. **Projectlettertype** brengt de tekst terug naar het lettertype, de grootte en de kleur van het project.
- **Vet, Cursief, Onderstrepen, Doorhalen**: een knop staat aan wanneer *alle* geselecteerde tekst die opmaak heeft, hoe je ook selecteerde (ook van rechts naar links). Een selectie die maar gedeeltelijk vet is, toont Vet uit, en **Vet** maakt dan alles vet; een tweede klik maakt alles gewoon. In het lettertypevenster wordt een lettersnede die je kiest eveneens voor de hele selectie ingesteld.
- **Opmaak > Lettertype > Groter / Kleiner** (**Ctrl+>** / **Ctrl+<**) wijzigen de grootte met één punt; elk deel van de selectie behoudt zijn eigen grootte en wordt groter of kleiner.
- **Opmaak > Lettertype > Tekstkleur…** en **Automatische tekstkleur**.

Het lettertype of de grootte van het project zelf kiezen wordt opgeslagen als "geen eigen lettertype": zulke tekst volgt het project wanneer je de standaardopmaak later wijzigt.

**Lettertypen met meerdere ontwerpen.** Sommige lettertypen bewaren meerdere ontwerpen van één lettertype onder één naam: Sitka van Windows 11 bijvoorbeeld bevat de optische groottes Small, Text, Subheading, Heading, Display en Banner (elk getekend voor een eigen bereik van groottes, de grotere smaller en met kleinere kleine letters), en sommige lettertypen bevatten op dezelfde manier smalle of brede versies. {{APP}} toont elk ontwerp als een eigen lettertype (bijvoorbeeld **Sitka Small**, **Sitka Text**, **Sitka Banner**), met alleen de lettersneden van dat ontwerp, zodat vet en cursief binnen het gekozen ontwerp blijven; vóór 0.40.1 kon vette tekst in een ander ontwerp uitkomen en kleiner en smaller lijken. De lettertypebestanden worden niet gewijzigd. Het lettertype met de oorspronkelijke naam behoudt zijn eigen ontwerp, zodat tekst die er eerder in is gezet, zijn uiterlijk behoudt.

**Lettertypen installeren.** **Opmaak > Lettertype** heeft de opdrachten voor lettertypen die nog niet op je computer staan:

- **Lettertypebestanden installeren…**: kies lettertypebestanden (TTF, OTF, TTC, PFB, WOFF of een ZIP-bestand dat ze bevat). Ze worden gekopieerd naar je eigen lettertypemap (*~/.local/share/fonts*), en de lettertypelijsten van {{APP}} kennen ze meteen; andere programma's zien ze na hun volgende start. Er is geen wachtwoord nodig, en de lettertypen zijn alleen voor jouw gebruiker.
- **Lettertypemap openen**: opent die map in het bestandsbeheer; lettertypen die je er zelf neerzet, worden gevonden na **Lettertypelijst opnieuw laden**.
- **Lettertypebeheer van het bureaublad openen**: opent het lettertypebeheer van KDE (Systeeminstellingen > Tekst en lettertypen > Lettertypebeheer) of het lettertypeprogramma van een ander bureaublad, waar je ook lettertypen voor alle gebruikers kunt installeren en lettertypen kunt verwijderen.
- **Lettertypelijst opnieuw laden**: zoekt naar nieuwe lettertypen en noemt ze.

### Alinea-opmaak

**Opmaak > Alinea > Alinea…** (**Ctrl+Alt+P**) stelt voor de alinea's die de cursor of de selectie bevatten in: uitlijning, linker en rechter inspringing, inspringing van de eerste regel (een negatieve waarde geeft een hangende inspringing), ruimte voor en na, regelafstand (een veelvoud van de regelhoogte, een exacte hoogte of een minimale hoogte) en tabstops (links, gecentreerd, rechts, decimaalteken). Alleen wat je wijzigt, wordt toegepast. **Standaard herstellen** verwijdert de opmaak die je met de hand instelde; de alinea volgt weer zijn stijl en het project.

Snelle manieren: de uitlijningsknoppen, de lijst met regelafstanden in de opmaakbalk, **Opmaak > Alinea > Regelafstand** en **Inspringing vergroten / Inspringing verkleinen**.

Een alineastijl toepassen vervangt de met de hand ingestelde opmaak in de alinea's die hij bereikt: inspringingen, ruimte, uitlijning en tabstops, en de lettertypen, groottes, kleuren, letterafstand en hoofdletters van de tekst erin. Vet, cursief, onderstrepen, doorhalen, markeringen en tekenstijlen blijven. Selecteer tekst over meerdere alinea's om ze allemaal de stijl te geven, telkens de hele alinea; een selectie die precies aan het begin van een alinea eindigt (bijvoorbeeld na **Shift+Omlaag**) laat die alinea met rust.

**Opmaak > Opmaak kopiëren** (**Ctrl+Alt+C**) onthoudt de lettertype- en alinea-opmaak bij de cursor; **Opmaak plakken** (**Ctrl+Alt+V**) past ze toe op de selectie, of op de hele alinea wanneer niets is geselecteerd.

### Onzichtbare tekens

**Weergave > Tekstbewerking > Onzichtbare tekens tonen** (ook de knop ¶ van de werkbalk) toont de tekens die je niet ziet: een punt voor elke spatie, een pijl voor een tab, een klein rondje voor een harde spatie, een gebogen pijl voor een regeleinde (**Shift+Enter**) en ¶ aan het eind van elke alinea. Ze worden alleen op het scherm getekend, in de hoofdtekst van elke editor; ze maken nooit deel uit van de tekst, een afdruk of het samenstellen. De keuze wordt bewaard. Hun kleur is die van de tekst, vervaagd in het papier; een eigen kleur stel je in onder **Bestand > Opties > Uiterlijk > Tekstmarkeringen** (of rechtsklik op de tekst > **Uiterlijk**), en hij hoort bij het uiterlijk van het project zoals het papier en de tekst (de schrijfmodus behoudt de vervaagde tekstkleur).

### De liniaal

**Weergave > Tekstbewerking > Liniaal tonen** (**Ctrl+Alt+R**) toont een liniaal boven de tekst. Hij hoort bij de alinea waarin de cursor staat.

- De markering bovenaan is de **inspringing van de eerste regel**, de markeringen onderaan zijn de **linker inspringing** en de **rechter inspringing**. Sleep ze. De linker inspringing neemt de eerste regel mee.
- **Klik** in de onderste helft van de liniaal om een tabstop toe te voegen. Sleep een tabstop om hem te verplaatsen; sleep hem omhoog of omlaag van de liniaal af om hem te verwijderen. **Rechtsklik** op een tabstop om hem links, gecentreerd, rechts of decimaal te maken.
- Markeringen springen op kwart centimeters (zestiende inches); houd **Shift** ingedrukt om ze vrij te plaatsen.
- **Dubbelklik** op de liniaal voor het alineavenster. Het rechtsklikmenu schakelt tussen centimeters en inches (ook onder **Bestand > Opties > Bewerken**).

Tabstops worden gemeten vanaf het begin van de eerste regel van de alinea. Zonder eigen tabstops heeft een alinea om de 2,1 cm een tabpositie.

De liniaal toont echte lengtes. Zoom vergroot de letters op het scherm maar niet de inspringingen en tabstops, dus vergelijk afstanden bij 100 %.

### Standaardopmaak

- **Dit project**: **Project > Projectinstellingen… > Opmaak** (ook de knop met schuiven in de opmaakbalk) stelt het lettertype, de grootte, de regelafstand, de inspringing van de eerste regel, de ruimte na alinea's en de uitlijning van de tekst van het hele project in (vier knoppen: links, gecentreerd, rechts, uitgevuld). In het Stijlenpaneel neemt **Geen stijl** > **Opnieuw definiëren vanuit selectie** deze waarden over van de alinea bij de cursor (zie "Stijlen" hieronder). **Toepassen** wijzigt alle documenten tegelijk en laat het venster open, zodat je het resultaat erachter kunt zien; **OK** doet hetzelfde en sluit het venster. **Huidige gebruiken** neemt de waarden over van de alinea waarin de cursor staat.
  - Met **Ook tekst met een eigen lettertype, grootte of regelafstand wijzigen** (standaard aan) volgt ook tekst die met de hand is opgemaakt of met een eigen lettertype uit een import kwam, zolang hij het hoofdlettertype, de hoofdgrootte en de hoofdregelafstand van zijn document heeft. Wat afwijkt van de hoofdtekst blijft zoals het is: een grotere kopregel, een passage in een ander lettertype, een alinea zonder inspringing. Zet het vakje uit om alle met de hand opgemaakte tekst met rust te laten.
  - Elk document dat verandert, krijgt eerst een momentopname (inspector, tabblad Momentopnamen, "Vóór de wijziging van de projectopmaak"), zodat je terug kunt gaan. Scriptdocumenten behouden hun eigen opmaak.
- **Nieuwe projecten**: **Bestand > Opties > Bewerken > Opmaak van nieuwe projecten**, met dezelfde vier uitlijningsknoppen en **Opmaak van huidige editor gebruiken**. Bestaande projecten behouden hun opmaak. De standaard is 11,0 pt, regelafstand 1,09, geen inspringing van de eerste regel en 6 pt na alinea's. Projecten die uit Scrivener zijn geïmporteerd, beginnen ook met deze waarden (met het lettertype dat het meest in het Scrivener-project wordt gebruikt).
- **Terug naar de standaard**: selecteer documenten in de binder en kies **Documenten > Omzetten > Tekst naar standaardopmaak…**. Jij bepaalt of lettertypen en kleuren, inspringingen en ruimte, uitlijning en tabstops worden verwijderd. Vet, cursief, stijlen, notities en koppelingen blijven altijd. Van elk document wordt eerst een momentopname gemaakt (inspector, tabblad Momentopnamen), zodat je terug kunt gaan.

### Stijlen

Een stijl bewaart opmaak onder een naam. De tekst onthoudt de naam, dus wanneer je de stijl wijzigt, verandert alle tekst die hem gebruikt mee, in het hele project.

Er zijn twee soorten:

- **Alineastijlen** gelden voor hele alinea's. Ze bevatten alinea-opmaak (uitlijning, inspringingen, ruimte, regelafstand, tabstops) en, als je dat wilt, tekenopmaak voor de alinea (lettertype, grootte, vet, cursief, kleur). In lijsten zijn ze gemarkeerd met "¶", of "¶a" wanneer ze tekenopmaak bevatten.
- **Tekenstijlen** gelden voor de geselecteerde tekst, of voor het woord bij de cursor. Ze bevatten alleen tekenopmaak. Ze zijn gemarkeerd met "a".

Laat lopende tekst op **Geen stijl** staan. Hij volgt de standaardopmaak van het project, en Samenstellen kan hem elk uiterlijk geven. Gebruik stijlen alleen voor tekst die er anders uit moet zien dan lopende tekst.

**Een stijl toepassen**: kies hem in de lijst links van de opmaakbalk, onder **Opmaak > Stijl**, in het menu van een rechtsklik op de tekst, of in het **Stijlenpaneel** (**Opmaak > Stijl > Stijlenpaneel**), een klein venster dat naast de tekst open blijft; een dubbelklik op een rij past de stijl toe. **Geen stijl** verwijdert een tekenstijl van de selectie; waar er geen is, verwijdert het de alineastijl. Vensters en vragen die het Stijlenpaneel opent, verschijnen boven het paneel.

**De stijllijst** van de opmaakbalk en van het rechtsklikmenu (**Stijl**) toont de stijlen die je kiest, in de volgorde die je kiest: **Stijllijst aanpassen…** (onderaan de lijst, en onder **Opmaak > Stijl**) toont alle items; vink die uit die je niet nodig hebt, sleep ze in een andere volgorde of gebruik de pijlen, en **Alle, in de volgorde van de stijlen** brengt de volledige lijst terug. Met meer dan tien items toont de lijst er tien en scrolt. Daaronder, altijd in beeld: **Nieuwe stijl uit selectie…**, **Stijl opnieuw definiëren vanuit selectie…**, **Stijlenpaneel** en **Stijllijst aanpassen…**. Een stijl die niet in de lijst staat, wordt in de opmaakbalk nog steeds genoemd wanneer de tekst hem gebruikt; **Opmaak > Stijl** toont altijd alle stijlen. De keuze hoort bij het project (een nieuwe stijl verschijnt aan het eind van de lijst) en maakt deel uit van de stijlen in projectinstellingenbestanden.

**Geen stijl is de stijl van de hoofdtekst.** Klik in het Stijlenpaneel met de rechtermuisknop op **Geen stijl** (of selecteer hem en gebruik de knop Opnieuw definiëren) en kies **Opnieuw definiëren vanuit selectie…**: het lettertype, de grootte, de regelafstand, de inspringing van de eerste regel, de ruimte na alinea's en de uitlijning van de alinea bij de cursor worden de stijl van de hoofdtekst van het project (de waarden van **Project > Projectinstellingen… > Opmaak**). Een vraag toont eerst de waarden. Alle tekst zonder stijl in het hele project volgt, ook tekst met een eigen lettertype, grootte of regelafstand; tekst met een stijl behoudt wat zijn stijl instelt. Elk document dat verandert, krijgt eerst een momentopname.

**Een stijl maken**:

1. Maak wat tekst op zoals de stijl eruit moet zien, en klik erin of selecteer een deel ervan.
2. Kies **Opmaak > Stijl > Nieuwe stijl uit selectie…** (of de plusknop van het Stijlenpaneel).
3. Voer een naam in en kies wat je opslaat: alle opmaak (een alineastijl met tekenopmaak), alleen de alinea-opmaak, of de tekenkenmerken (een tekenstijl).
4. Bepaal of het lettertype en de lettergrootte deel van de stijl zijn. Zonder die behoudt de stijl het lettertype van de tekst eromheen.
5. Kies voor alineastijlen welke stijl de volgende alinea krijgt wanneer je aan het eind op Enter drukt.

Een alineastijl bewaart alleen wat afwijkt van lopende tekst. Al het andere blijft de opmaak van het project volgen, zodat een nieuw standaardlettertype ook tekst met een stijl bereikt.

**Een stijl wijzigen**:

- **Opmaak > Stijl > Stijl opnieuw definiëren vanuit selectie…** neemt de opmaak van de tekst bij de cursor over, zoals hierboven. Een alineastijl blijft een alineastijl, een tekenstijl een tekenstijl.
- **Opmaak > Stijl > Stijl bewerken** (of de potloodknop van het Stijlenpaneel) opent een venster met de naam, **Lettertype…**, **Alinea…** en de stijl van de volgende alinea. Een voorbeeld toont het resultaat.

Daarna gaat {{APP}} het hele project langs en werkt de tekst bij, in open documenten en in de bestanden van de andere. Wat de wijziging raakt (lettertype, grootte, vet, cursief, kleur, letterafstand, hoofdletters, uitlijning, inspringingen, ruimte, regelafstand, tabstops) bereikt **alle** tekst van de stijl, ook waar die tekst een eigen waarde had: met de hand ingesteld, of meegebracht door een import (geïmporteerde koppen noemen hun lettertype, grootte en ruimte meestal in elke alinea). Opmaak die de wijziging niet raakt, blijft zoals ze is, bijvoorbeeld een met de hand gekleurd woord in een kop wanneer alleen de grootte van de kop veranderde. Elk document dat verandert, krijgt eerst een momentopname ("Vóór de wijziging van de stijl ..."). Dit kan niet ongedaan worden gemaakt met **Bewerken > Ongedaan maken**; zet een momentopname terug, definieer de stijl opnieuw, of gebruik voor een ingebouwde stijl **Terug naar de ingebouwde definitie** in het Stijlenpaneel.

**Hernoemen en verwijderen**: hernoem een stijl in **Stijl bewerken**. De koppen, Titel, Ondertitel, Blokcitaat, Gecentreerd en Bijschrift behouden hun namen, omdat Samenstellen en de Markdown-bestanden ze bij naam kennen. Wanneer je een stijl verwijdert (**Opmaak > Stijl > Stijl verwijderen**), behoudt tekst die hem gebruikte zijn uiterlijk als met de hand ingestelde opmaak.

**Andere projecten**: **Opmaak > Stijl > Stijlen importeren…** leest de stijlen van een ander project. **Deze stijlen gebruiken in nieuwe projecten** maakt van de stijlen van het geopende project de beginset van elk nieuw project; **Bestand > Opties > Bewerken** gaat terug naar de ingebouwde stijlen.

In het Stijlenpaneel wijzigen de pijlknoppen de volgorde van de stijlen in alle lijsten.

### Markdown of RTF: hoe documenten worden opgeslagen

Elk project bewaart zijn documenten in een van twee indelingen. Je kiest wanneer je een project maakt of importeert, en je kunt later wisselen onder **Project > Projectinstellingen… > Opmaak**.

- **Markdown** (aanbevolen): platte tekstbestanden die elk programma kan openen, nu en over vijftig jaar. Het uiterlijk van de tekst komt uit de standaardopmaak. Lettertypen en alinea-opmaak die je met de hand instelt, worden als kleine markeringen in de tekst geschreven, alleen waar je ze gebruikte (sectie 18); al het andere blijft gewone tekst.
- **RTF**: rijke tekstbestanden zoals een tekstverwerker. Elke alinea draagt zijn volledige opmaak, dus een wijziging van de standaardopmaak bereikt bestaande tekst niet.

Wisselen zet elk document om. Van RTF naar Markdown blijven lettertypen, kleuren, uitlijning, linker en rechter inspringingen en tabstops behouden. Regelafstand, inspringing van de eerste regel en alinea-afstand van tekst die uit andere programma's kwam, volgen daarna het project. Maak eerst een back-up (**Bestand > Back-up > Nu back-up maken**).

Tekst die uit andere programma's in een Markdown-project wordt geplakt, neemt het uiterlijk van de tekst eromheen aan (vet, cursief en koppelingen blijven). Tekst die binnen {{APP}} wordt gekopieerd, behoudt zijn lettertypen. **Bewerken > Plakken met opmaak** (**Ctrl+Alt+Shift+V**) behoudt ook het uiterlijk van tekst uit een tekstverwerker of webpagina: lettertypen, groottes, kleuren, en de uitlijning, inspringingen en ruimte van zijn alinea's (de eerste geplakte alinea brengt zijn alinea-opmaak alleen in een lege alinea). **Bewerken > Plakken en stijl aanpassen** (**Ctrl+Shift+V**) doet het tegenovergestelde en plakt platte tekst. Het rechtsklikmenu van de tekst biedt alle drie onder elkaar aan: **Plakken**, **Plakken en stijl aanpassen** en **Plakken met opmaak**.

Om één los bestand buiten een project om te zetten, gebruik je **Bestand > Markdown ⇄ RTF-bestand omzetten…**.

### Opmerkingen en voetnoten

- **Opmerking** (**Shift+F2**): selecteer wat tekst en voeg een opmerking toe. De tekst wordt gemarkeerd en de opmerking verschijnt in de inspector.
- **Voetnoot** (**Ctrl+Shift+F2**): selecteer het woord waarbij de voetnoot hoort; schrijf de voetnoot in de inspector.
- **Inline-aantekening** (**Ctrl+Shift+A**): een notitie die rechtstreeks in de tekst staat, in kleur getoond in een tekstballon. Aantekeningen worden verwijderd wanneer je samenstelt (tenzij je kiest ze als opmerkingen te exporteren).
- **Inline-voetnoot** (**Ctrl+Alt+F**): een voetnoot die rechtstreeks in de tekst staat.

Om de kleur van een inline-aantekening te wijzigen, selecteer je hem (**Bewerken > Selecteren > Aantekening of notitie selecteren** selecteert de hele notitie bij de cursor) en kies je een tekstkleur; de aantekening behoudt die kleur. **Opmaak > Lettertype > Automatische tekstkleur** geeft hem weer het gebruikelijke rood.

Klik op een opmerking in de inspector om ernaartoe te springen. Klik er met de rechtermuisknop op om de kleur te wijzigen, hem om te zetten in een andere soort notitie of hem te verwijderen.

Een klik op tekst met een opmerking opent de inspector met die opmerking. **Alt+klik** opent de opmerking of voetnoot in plaats daarvan in een klein venster bij de muis, waar je hem kunt lezen en wijzigen zonder de inspector; klik elders of druk op **Esc** om het te sluiten.

### Afbeeldingen en koppelingen

- **Invoegen > Afbeelding uit bestand…** zet een afbeelding in de tekst; je kunt ook een afbeelding in de tekst slepen of er een plakken. Klik met de rechtermuisknop op de afbeelding en kies **Afbeeldingsgrootte…** om te wijzigen hoe groot ze verschijnt.
- **Invoegen > Koppelen aan document…** (**Ctrl+L**) koppelt de geselecteerde tekst aan een ander document in het project. Een klik op de koppeling opent dat document (in de andere editor, dezelfde editor, zijn kopijhouder of een paneel Snelle naslag; zie **Bestand > Opties > Gedrag**).
- **Invoegen > Weblink invoegen…** koppelt aan een webpagina.
- Web- en maillinks (*http*, *https*, *mailto*) openen meteen. Elk ander doel van een koppeling, in een tekst, een mindmap, een bladwijzer of een geïmporteerde webpagina (een bestand, een map, een netwerkadres, de koppelingsvorm van een ander programma), wordt eerst volledig getoond en pas geopend wanneer je op **Openen** klikt; zulke koppelingen kunnen met een project, een webpagina of een Word-bestand van iemand anders meekomen. Voor een bestand of map op je computer noemt de vraag de soort (bijvoorbeeld "tekstdocument zonder opmaak (.txt)"), de hele naam en de map; een lange naam wordt afgebroken, zodat het einde te lezen is, en een bestand dat er niet is, wordt als ontbrekend gemeld. Koppelingen naar programma's, scripts, snelkoppelingen en installatieprogramma's worden nooit geopend; open die zelf vanuit je bestandsbeheer als je ze vertrouwt.

### Tabellen

**Invoegen > Tabel…** vraagt om het aantal rijen en kolommen en het ontwerp, en zet de tabel bij de cursor. In een tabel gaat **Tab** naar de volgende cel (in de laatste cel voegt het een rij toe) en gaat **Shift+Tab** terug. **Opmaak > Tabel**, ook in het rechtsklikmenu in een tabel, heeft de rest:

- **Rij erboven / eronder invoegen**, **Kolom links / rechts invoegen**.
- **Rijen verwijderen** en **Kolommen verwijderen** (die van de geselecteerde cellen, of van de cel met de cursor), **Tabel verwijderen**.
- **Cellen samenvoegen** voegt de geselecteerde cellen samen tot één; **Cel splitsen** verdeelt een samengevoegde cel weer.
- **Celkleur…** geeft de geselecteerde cellen een eigen achtergrond; **Celkleur van het ontwerp** haalt hem weer weg.
- **Tabeleigenschappen…** wijzigt het ontwerp.

**Het ontwerp.** Vijf ontwerpen zijn kant-en-klaar: **Raster**, **Raster met kopregel**, **Gestreepte rijen**, **Lijnen tussen de rijen** en **Geen lijnen**. Kies er een en wijzig wat je wilt; het ontwerp is dan **Mijn eigen**:

- **Lijnen**: breedte (0: geen lijnen), kleur en **Alleen tussen de rijen** (geen lijnen aan de zijkanten).
- **Cellen**: de ruimte in een cel en de ruimte tussen cellen (0: naburige cellen delen één lijn).
- **Kopregels**: hoeveel rijen bovenaan koppen zijn, met hun kleur. Afdrukken en samenstellen herhalen ze bovenaan elke pagina waarop een lange tabel doorloopt. **Om de andere rij** krijgt een eigen kleur als je er een kiest.
- **Plaats in de tekst**: de breedte (een deel van de tekstbreedte, of zo breed als de inhoud), de kolombreedtes (allemaal gelijk, of delen zoals "30, 70"), de uitlijning en waar hij staat: **Op eigen regels**, of **Links** of **Rechts** met de tekst ernaast.

Rijen en kolommen die je later toevoegt, behouden het ontwerp (de strepen gaan door). De tekst in tabelcellen wordt niet uitgevuld, ook niet wanneer de lopende tekst dat wel is (een smalle cel zou zijn woorden uit elkaar trekken); een uitlijning die je een alinea in een cel geeft, blijft. Tabellen worden in beide opslagindelingen en in Scrivener-indeling bewaard; tabellen in RTF-bestanden uit andere programma's (bijvoorbeeld een Word-bestand dat als RTF is opgeslagen) worden ook tabellen.

### Afbeeldingen naast de tekst, bijschriften en beschrijvingen

Klik met de rechtermuisknop op een afbeelding en kies **Afbeelding…** (of zet de cursor ernaast en kies **Opmaak > Afbeelding…**):

- **Breedte op het scherm**: de hoogte volgt; **Oorspronkelijke grootte** gaat terug naar de eigen grootte van de afbeelding.
- **Positie**: **In de regel, als een letter** (zoals voorheen), **Links** of **Rechts** met de tekst eromheen, of **Gecentreerd op eigen regels**.
- **Bijschrift**: getoond onder de afbeelding en ermee samengesteld. Een afbeelding in de regel heeft geen bijschrift; geef haar eerst een positie.
- **Beschrijving**: wat de afbeelding toont, in een zin of twee. Ze wordt niet in de tekst getoond. E-books, Word-bestanden en Markdown bewaren haar bij de afbeelding (als haar "alt-tekst"), en schermlezers lezen haar voor aan mensen die de afbeelding niet kunnen zien.

De afbeelding met haar bijschrift verhuist als één geheel; typ het bijschrift onder de afbeelding zoals elke andere tekst (het heeft de stijl **Bijschrift**).

### Zoeken en vervangen

- **Ctrl+F** (**Bewerken > Zoeken > Zoeken**) opent de zoekbalk boven de tekst. Hij toont hoeveel treffers er zijn en markeert ze; **F3** en **Shift+F3** gaan naar de volgende en vorige treffer. **Hoofdlettergevoelig** en **Hele woorden** verfijnen de zoekopdracht.
- **Ctrl+H** voegt de vervangrij toe: **Vervangen** wijzigt de geselecteerde treffer en gaat naar de volgende, **Alles vervangen** wijzigt elke treffer in de tekst die je ziet (één stap voor **Ongedaan maken**). De nieuwe tekst neemt de opmaak over van de tekst die hij vervangt, en opmerkingen blijven eraan hangen.
- **Ctrl+E** (**Selectie gebruiken om te zoeken**) zet de geselecteerde tekst in de zoekbalk.
- In de gecombineerde tekst werken zoeken en vervangen door alle getoonde documenten heen.
- Terwijl het toetsenbord in een **kopijhouder** staat, doorzoekt Ctrl+F het document van de kopijhouder.
- **Ctrl+Shift+F** doorzoekt het hele project, **Bewerken > Zoeken > Project vervangen** vervangt in het hele project, en **Bewerken > Zoeken > Zoeken op opmaak** zoekt naar markeringen, opmerkingen, kleuren en stijlen (alles in sectie 9).
- Op het prikbord en in het overzicht opent Ctrl+F in plaats daarvan de **filterbalk** (sectie 5).

### Spelling

Verkeerd gespelde woorden krijgen een rode golvende onderstreping; klik met de rechtermuisknop ergens op het onderstreepte woord voor suggesties of om het woord aan je woordenboek toe te voegen (ook bij woorden met een apostrof, zoals "Kayel's" of "D'Nersun": het menu biedt altijd het hele onderstreepte woord aan). Bij een verkeerd gespeld woord is het menu kort, zodat het direct bij de muis opent: de suggesties, **"..." aan woordenboek toevoegen**, **"..." negeren** en **Meer opdrachten** met alles wat het menu elders bevat (Knippen, Kopiëren, Plakken, Stijl, opmerkingen, Uiterlijk ...). De eigen naam en termen van het programma ({{APP}}, overzicht, subdocument, kopijhouder) tellen altijd als juist. Kies de taal onder **Bestand > Opties > Correcties** (pagina Spelling); daar kies je ook de onderstreping (golven, stippen of een rechte lijn), of woorden met cijfers ("Route66") worden gecontroleerd, en **Persoonlijke woordenlijst bekijken...** toont de woorden die je toevoegde en verwijdert de woorden die je per ongeluk toevoegde. Woordenboeken komen van je systeem. Op Ubuntu en Kubuntu is Engels geïnstalleerd; installeer voor andere talen het bijpassende *hunspell*-pakket in Discover (bijvoorbeeld "hunspell-nl" voor Nederlands).

### Speciale invoegingen

Het menu **Invoegen** biedt ook de huidige datum en tijd, speciale tekens (gedachtestreep, halve streep, weglatingsteken, harde spatie, paragraafteken, bullet), **automatische nummers** en **plaatsvervangers**, die Samenstellen vervangt (sectie 13).

### Correcties tijdens het typen

**Bestand > Opties > Correcties** stelt in wat tijdens het typen wordt gecorrigeerd. Een woord wordt gecorrigeerd wanneer je het afmaakt, dat wil zeggen wanneer je er een spatie, een Return of een leesteken achter typt; **Ctrl+Z** direct daarna neemt alleen de correctie terug.

- **Hoofdletters aan het begin van zinnen herstellen**: het eerste woord van een zin krijgt een hoofdletter (niet na afkortingen zoals "Dr." of "e.g."). **"i" met hoofdletter**: "i" en "i'm" worden "I" en "I'm". Beide staan eerst uit.
- **Slimme aanhalingstekens** in de stijl die je kiest (“Engels”, „Duits“, »Duits«, «Frans», ”Zweeds”); een apostrof binnen een woord is altijd ’. Waar het sluitende enkele teken niet de apostrof is (Duits ‚…‘, Frans ‹…›), sluit een ' die direct na een woord wordt getypt een enkel teken dat in de alinea openstaat; als je daarna letters blijft typen ("geht's"), wordt het weer de apostrof. Twee koppeltekens worden een gedachtestreep (—); drie punten kunnen één weglatingsteken worden (eerst uit). In scriptmodus kunnen deze worden uitgezet.
- **Aanhalingstekens die al in het project staan** (sinds 0.61.1): onder **Slimme aanhalingstekens** zet **Omzetten naar typografische aanhalingstekens…** de rechte " en ' van het project om in de tekens van de stijl die naast **Slimme aanhalingstekens gebruiken** is gekozen, en zet **Omzetten naar rechte aanhalingstekens…** elk typografisch aanhalingsteken en elke apostrof terug in " en '. Er opent een venster als Project vervangen (sectie 9): de richting en nogmaals de stijl, waar omgezet moet worden (titels, tekst, notities, samenvattingen, aangepaste metagegevens, opmerkingen en voetnoten), het bereik (het hele project of de in de binder geselecteerde documenten, de Prullenbak alleen wanneer je dat aanvinkt) en een momentopname van elke tekst voordat hij verandert (eerst aangevinkt). **Tellen** toont hoeveel tekens zouden veranderen; **Omzetten** vraagt nog één keer en doet het. De opmaak blijft (een vet aanhalingsteken blijft vet). Een teken opent na niets, een spatie, een haakje, een streep of een openingsteken, en sluit anders; een apostrof binnen een woord of voor een getal ('90s) wordt ’. Een recht teken met aan beide kanten een spatie blijft zoals het is, en een apostrof aan het begin van een woord ('tis) wordt een openingsteken: kijk die daarna na. Zoeken en vervangen kan dit niet, omdat één teken afhankelijk van zijn buren een openend of een sluitend teken moet worden.
- **Gegevensherkenning**: een getypt webadres wordt een koppeling; **[[Titel]]** wordt een koppeling naar het document met die titel, en als dat er niet is, wordt naast de tekst een nieuw leeg document met die naam gemaakt (de "documentkoppelingen" van Scrivener).
- **Extra vervangingen**: een lijst van wat je typt en wat het wordt, bijvoorbeeld "(c)" en ©, "->" en →. **Vervangingen bewerken...** wijzigt de lijst; **Standaardvervangingen** brengt de lijst terug waarmee het programma kwam.
- **Automatisch aanvullen**: in scriptmodus de lijst van personagenamen en scènekoppen die tijdens het typen wordt aangeboden.

### Zoom, typemachinescrollen en vaste breedte

- **Ctrl+muiswiel** zoomt het gebied onder de muis, waar het ook is: de editors, de binder, de inspector, het overzicht, het prikbord (kaarten en hun tekst samen), een mindmap, de schrijfmodus en vensters van zichzelf (Snelle naslag, de schrijfhulpmiddelen); het toetsenbord kan ergens anders zijn. Elk gebied behoudt zijn zoom tot je hem weer wijzigt.
- **Ctrl++ / Ctrl+-** of het zoomvak in de voettekst vergroten de tekst op het scherm zonder de lettergrootte te wijzigen. **Weergave > Zoom herstellen** (**Ctrl+0**) gaat terug naar de standaardtekstzoom.
- **Bestand > Opties > Bewerken** stelt de zoom in waarmee elk gebied begint ("Standaardtekstzoom", voor elk gebied van het venster en elk onderdeel met een eigen zoom: de samenvatting, de notities, de bladwijzerlijsten, de tekst van de bladwijzer, de lijst en tekst van momentopnamen, de objectlijst en het objectblad in de inspector en in het venster Objecten, nieuwe mindmaps) en of de editors opnieuw openen met de zoom die ze het laatst hadden ("Laatst gekozen zoom van de hoofdeditors herladen").
- **Waar je was.** {{APP}} onthoudt met het project voor elk document de plaats van de cursor en het deel van de tekst dat op het scherm stond: opnieuw getoond (vanuit de binder, een koppeling, Terug en Vooruit, in de kopijhouder, een paneel Snelle naslag, of na het heropenen van het project) opent een document waar je het liet. Een map in de gecombineerde tekst onthoudt welk van zijn documenten de cursor had. Een zoekresultaat of een koppeling die naar een bepaalde plek wijst, gaat in plaats daarvan daarheen. Afbeeldingen behouden hun zoom en pdf-bestanden hun zoom en pagina.
- **Typemachinescrollen** (**Weergave > Tekstbewerking**) houdt de regel waarin je typt in het midden van het scherm.
- **Editor met vaste breedte** toont de tekst als een gecentreerde kolom van aangename breedte (500 px bij 100 % tenzij je dat wijzigt), hoe breed het venster ook is. Zet hem uit, of wijzig de breedte, onder **Bestand > Opties > Uiterlijk > Hoofdeditor** ("Editor met vaste breedte gebruiken", "Breedte van de tekstkolom"), of met een rechtsklik op het papier (**Uiterlijk > Breedte van de tekstkolom**). Het papier van de kolom is standaard #d1d495 en het gebied ernaast #999999 ("Papier" en "Naast de tekstkolom" onder de kleuren van de Hoofdeditor); ze blijven hetzelfde wanneer je een ander thema kiest, tot je ze wijzigt of herstelt.
- **Rand naast de tekst.** Links en rechts van de tekst heeft het papier een rand in een eigen kleur, zodat de tekst de rand van het papier niet raakt: 10 px bij 100 % voor nieuwe projecten, en hij groeit mee met de zoom. Hij ligt buiten de tekstkolom, dus de regels breken waar ze braken. Wijzig hem onder **Bestand > Opties > Uiterlijk > Hoofdeditor** ("Rand naast de tekst", 0 tot 100 px) of met een rechtsklik op het papier (**Uiterlijk > Rand naast de tekst**). Projecten die vóór 0.40.2 zijn gemaakt, behouden hun uiterlijk, zonder rand, tot je ze er een geeft. Een klik op de rand zet de cursor aan het begin of eind van de regel ernaast.
- **Klikken en rechtsklikken** verplaatsen de weergave nooit: de cursor gaat waar je klikt, en de tekst blijft waar hij op het scherm staat (vóór 0.40.2 kon een klik of rechtsklik bij de boven- of onderrand, of een rechtsklik naast de tekst, de tekst laten springen).

### Paginaweergave

**Weergave > Tekstbewerking > Paginaweergave** (ook in het menu van de werkbalkknop **Weergave**) toont je tekst op vellen papier in plaats van in één eindeloze kolom. De voettekst toont dan de pagina van de cursor en het aantal pagina's, bijvoorbeeld "Pagina 3 van 12". Kies de opdracht opnieuw om terug te keren naar de kolom. De kleur van het bureau rond de pagina's staat onder **Bestand > Opties > Uiterlijk > Hoofdeditor** (of rechtsklik > **Uiterlijk** terwijl de pagina's worden getoond).

De pagina's zijn een manier om naar je tekst te kijken, niets meer. Ze laten niet zien hoe een samengesteld of afgedrukt manuscript eruit zal zien: je schrijft in stukken, en titels, lettertypen en paginaformaat worden bepaald wanneer je samenstelt (sectie 13). Niets over de pagina's wordt in je documenten opgeslagen.

- **Weergave > Tekstbewerking > Paginaformaat en marges…** stelt het papier in (A4, A5, US Letter en andere, of een eigen formaat), staand of liggend, en de vier marges. De instelling hoort bij het project; vink **Deze pagina ook voor nieuwe projecten gebruiken** aan om haar je standaard te maken.
- Zoom vergroot vellen, marges en letters samen. Een vel dat breder is dan de editor kan opzij worden gescrolld. Zoals in de kolomweergave behouden inspringingen en tabstops hun echte lengte, dus regeleinden kunnen tussen zoomniveaus een beetje verschillen.
- In de gecombineerde tekst begint elk document op een eigen vel, en zijn laatste vel eindigt vlak onder de tekst, zodat korte documenten het scherm niet met leeg papier vullen.
- Paginaweergave geldt voor alles wat je teksten toont in het hoofdvenster en in vensters met snelle naslag: beide editors en de kopijhouders. Notities behouden hun gewone uiterlijk, en de schrijfmodus heeft zijn eigen papier.
- De kleur rond de vellen is **Rond de pagina's (paginaweergave)** onder **Bestand > Opties > Uiterlijk > Hoofdeditor**.

### Scriptschrijven

Scenario's, toneelstukken en stripscripts worden geschreven in **scriptmodus**. Een script bestaat uit *elementen*: scènekop, handeling, personage, tussenzin, dialoog, overgang enzovoort. Elk element heeft zijn plaats op de pagina, en in scriptmodus brengen de toetsen **Tab** en **Enter** je van het ene element naar het volgende, zodat je nooit met de hand een inspringing instelt.

**Aanzetten.** Klik in een document en kies **Opmaak > Scriptschrijven > Scriptmodus** (**Ctrl+8**). Het menu-item noemt de scriptindeling van het project, bijvoorbeeld *Scriptmodus - Scenario*. Scriptmodus hoort bij het document: scripts en gewone teksten staan naast elkaar in één project, en scripts hebben een **geel pictogram** in de binder. Kies de opdracht opnieuw om hem uit te zetten; de tekst behoudt zijn uiterlijk. Een nieuw document dat tussen scripts wordt gemaakt (in een map waarvan alle teksten scripts zijn) is ook een script.

**De voettekst.** In een script toont de voettekst van de editor, in plaats van het aantal woorden, wat **Tab** en **Enter** bij de cursor zullen doen, en rechts een uitklapmenu met het element van de huidige alinea. Kies in dat menu een ander element en de alinea wordt opnieuw opgemaakt. Tekst die geen element van het script is, wordt als **Algemene tekst** getoond.

**Tab en Enter.**

- **Enter** aan het eind van een alinea begint het element dat gewoonlijk volgt: handeling na een scènekop, dialoog na een personagenaam enzovoort.
- **Enter** op een lege regel opent de lijst van elementen bij de cursor; druk op de letter die in de lijst is onderstreept (S voor scènekop, C voor personage ...). **Opmaak > Scriptschrijven > Lijst met elementen tonen** (**Ctrl+\\**) opent dezelfde lijst op elk moment.
- **Tab** op een lege regel verandert de regel in een ander element; druk er nogmaals op voor het volgende, **Shift+Tab** om terug te gaan.
- **Tab** aan het eind van een alinea doet wat bij het element past: na een personagenaam of een regel dialoog begint het een tussenzin, en in een scènekop typt het " - ", klaar voor het tijdstip van de dag.
- **Enter** midden in een alinea splitst hem; beide helften behouden het element. **Shift+Enter** is een regeleinde binnen de alinea.
- Elementen die in hoofdletters worden geschreven (scènekoppen, personagenamen, overgangen) maken van wat je typt hoofdletters. Haakjes en voorvoegsels zoals "SFX: " worden voor je getypt.

**Automatisch aanvullen.** Tijdens het typen biedt een kleine lijst aan wat bij het element past: INT. en EXT. aan het begin van een scènekop, de tijdstippen van de dag na zijn " - ", de plaatsen en personagenamen die al in het document zijn gebruikt, en vaste overgangen zoals CUT TO:. **Enter** of **Tab** neemt het gemarkeerde item, de pijltoetsen kiezen een ander, **Esc** sluit de lijst.

**Elementen wijzigen.** **Opmaak > Scriptschrijven > Element wijzigen in** wijzigt de alinea met de cursor, of alle geselecteerde alinea's. Daar een element kiezen in een gewoon document maakt het document een script. Elk element is een opdracht, dus je kunt je favorieten een sneltoets geven onder **Bestand > Opties > Toetsenbord**.

**Scriptindelingen.** Een project gebruikt voor al zijn scripts één scriptindeling. **Opmaak > Scriptschrijven > Scriptopmaak** toont *Scenario*, *Toneelstuk* en *Stripscript*, de indelingen van add-ons en je eigen. Wanneer je een andere indeling kiest, krijgen de scripts van het project de nieuwe lay-out: elk element wordt het element van dezelfde soort in de nieuwe indeling (een handeling wordt bijvoorbeeld een regieaanwijzing). Een element zonder tegenhanger wordt algemene tekst die zijn uiterlijk behoudt; de vraag vóór de wijziging noemt deze elementen.

**Scriptinstellingen.** **Opmaak > Scriptschrijven > Scriptinstellingen…** wijzigt de indeling van het project of bouwt een nieuwe:

- de naam van de indeling, zijn lettertype (standaard het schrijfmachinelettertype van je systeem: Courier Prime, Courier New of een soortgelijk lettertype) en grootte, en of lengtes in inches of centimeters zijn;
- de lijst van elementen: toevoegen, verwijderen en herschikken (de volgorde is de volgorde van de lijst van elementen en van **Tab** op een lege regel);
- per element: naam, soort (wat het voor andere programma's is: scènekop, handeling, personage ...) en zijn letter in de lijst; vet, cursief, onderstreept, hoofdletters en kleur; uitlijning, inspringingen vanaf beide marges, lege regels ervoor en regelafstand; wat **Enter** en **Tab** doen; een tekst die voor en na de alinea wordt getypt; en de lijsten voor automatisch aanvullen;
- een voorbeeld van alle elementen.

Een hernoemd element behoudt zijn alinea's in elk script van het project. De knop **Beheren** begint vanuit een andere indeling, laadt of bewaart een indelingsbestand (*.qbscript*) en bewaart de indeling voor gebruik in andere projecten.

**Goed om te weten.**

- De toetsen en de lijsten werken hetzelfde in de gecombineerde tekst, in de tweede editor, in kopijhouders, in panelen Snelle naslag en in de schrijfmodus. Alleen de editors van het hoofdvenster hebben de voettekst met de hints; elders toont **Ctrl+\\** de lijst van elementen.
- Paginaweergave (hierboven) toont een script op vellen; met het gebruikelijke schrijfmachinelettertype op 12 punten is een pagina van een scenario ongeveer een minuut film, maar de pagina's op het scherm zijn niet de pagina's van het samengestelde bestand.
- Het lettertype en de inspringingen van de elementen horen bij de scriptindeling, niet bij het document. De bestanden van je scripts bevatten alleen de namen van de elementen (sectie 18).
- Scripts importeren en exporteren, in de indelingen van andere scenarioprogramma's: sectie 14. Een heel script samenstellen: sectie 13.
- Scripts vereisen versie 0.10 of later. Een project dat scripts bevat, zegt dat in zijn projectbestand, en vanaf deze versie vraagt {{APP}} om bevestiging voordat het een project opent dat een nieuwere versie nodig heeft dan de versie die draait. Versies 0.9 en ouder vragen niet: ze tonen scripts als gewone tekst en verliezen de elementen van elk script dat je daar bekijkt.

---

## 5. Drie manieren om naar je project te kijken

Kies de weergave met de drie werkbalkknoppen of met **Ctrl+1**, **Ctrl+2** en **Ctrl+3**.

### Gecombineerde tekst (Ctrl+1): veel documenten als één tekst

Selecteer meerdere documenten in de binder (Ctrl+klik of Shift+klik) of een map, en druk op **Ctrl+1**. Alle teksten verschijnen na elkaar, gescheiden door een dunne gestippelde lijn, en je kunt in allemaal schrijven alsof het één document is. Wijzigingen komen automatisch in de juiste documenten terecht. Toon de documenttitels ertussen met **Weergave > Tekstbewerking > Titels tonen in Gecombineerde tekst**.

### Prikbord (Ctrl+2): indexkaarten

Elk document is een indexkaart met zijn titel en samenvatting; een document zonder samenvatting toont in plaats daarvan de eerste regels van zijn tekst (vervaagd, cursief). De kaart van een afbeelding toont de afbeelding, en dat doet ook de kaart van een document waarvan de samenvatting een afbeelding is (sectie 7). Sleep kaarten om de documenten te herordenen. Dubbelklik op een kaart om zijn titel of samenvatting te bewerken. Druk op **Enter** om de geselecteerde kaarten als tekst te openen.

In de voettekst van het prikbord:

- **Raster** of **vrij**: in de vrije modus kun je kaarten overal plaatsen, zoals op een echt prikbord. **Volgorde vastleggen** herordent dan de documenten in de binder naar de kaarten, van links naar rechts, van boven naar beneden. De indeling die je voor een map kiest, wordt bij die map onthouden (sinds 0.90.0): de volgende keer dat je hem toont, is zijn prikbord op dezelfde manier ingedeeld. Een map waarvoor je nooit een indeling koos, behoudt die welke de editor toont.
- **Labelrijen**: rangschikt de kaarten in rijen per label.
- **Kaartgrootte** van de kaarten, met de schuif, de pijltoetsen of het muiswiel erboven; ze wordt meteen bewaard (**Ctrl+muiswiel** boven het prikbord zoomt de kaarten en hun tekst samen, zoals de andere gebieden, zie sectie 4).

Een dubbelklik op een lege plek van het prikbord maakt een nieuwe kaart (**Bestand > Opties > Gedrag > Dubbelklikken**). Wanneer je een samenvatting op een kaart bewerkt, beëindigt **Return** het bewerken en maakt **Shift+Return** een nieuwe regel (**Bestand > Opties > Gedrag > Returntoets**, waar Return ook kan worden ingesteld om een nieuwe kaart of overzichtsrij na de geselecteerde te maken).

Statusstempels, de vrije modus en de rangschikking op label staan ook onder **Weergave > Prikbordopties**; labeltinten voor kaarten en binder onder **Weergave > Labelkleur gebruiken in**. De kolommen van het overzicht staan ook onder **Weergave > Overzichtsopties**.

### Overzicht (Ctrl+3): een tabel

Toont de documenten als een tabel met titel, samenvatting, label, status, aantallen woorden, doelen, datums en je eigen metagegevens. Klik op de kolomknop in de voettekst om kolommen te kiezen. Klik op een kolomkop om te sorteren (alleen voor de weergave; de volgorde van de binder verandert niet). Sleep rijen om documenten te herordenen.

### Het prikbord en het overzicht filteren

Terwijl het prikbord of het overzicht het toetsenbord heeft, opent **Ctrl+F** (**Bewerken > Zoeken > Zoeken**) een **filterbalk** bovenaan de editor. Terwijl je typt, blijven alleen de kaarten of rijen over die overeenkomen; de balk zegt hoeveel ("2 van 9"). In het overzicht blijven de groepen van overeenkomende documenten staan, zodat je ziet waar ze bij horen.

- Klik op de **driehoek** links van het veld voor de opties: waar gezocht wordt (**Titels**, **Samenvattingen**, **Trefwoorden**, **Tekst**), een **label**, een **status**, een **trefwoord**, of het document **bij het samenstellen is opgenomen**, en **Tonen wat niet overeenkomt**. Meerdere keuzes samen verfijnen de lijst verder.
- Het filter blijft staan terwijl je andere mappen opent, zodat je door het project kunt lopen en bijvoorbeeld alleen de scènes van één personage ziet.
- **Klaar** verwijdert het filter en toont weer alles.

Kaarten slepen herordent de documenten nog steeds: een kaart die tussen twee zichtbare kaarten wordt neergezet, komt in de binder naast hen te staan. **Volgorde vastleggen** van het vrije prikbord staat uit zolang een filter kaarten verbergt.

---

## 6. Twee editors, bladwijzers en snelle naslag

### Gesplitste editor

Klik op de splitsknop rechts van de editorkop om een tweede editor te tonen. Alt+klik splitst in de andere richting. **Weergave > Editor splitsen** biedt dezelfde keuzes. **F6** springt tussen de editors. Een klik in de binder toont het document in de editor die je het laatst gebruikte; **Navigeren > Selectie in de Binder werkt op** wijzigt dat in de linker of bovenste editor, de rechter of onderste editor, of de andere editor, welke je ook aan het schrijven bent.

In een prikbord of overzicht zorgt de voettekstknop **Selectie werkt op de andere editor** (ook in **Weergave > Prikbordopties** en **Overzichtsopties**) ervoor dat de andere editor de kaart toont waarop je klikt. Dit is een snelle manier om scènes te doorbladeren terwijl je het hoofdstukoverzicht behoudt.

**Navigeren > Editor > Vastzetten** houdt een editor bij zijn document, ook wanneer je elders in de binder klikt.

### Kopijhouders

Elke editor heeft een **kopijhouder**: een tweede, eenvoudige weergave binnen de editor die één ander document ter referentie toont, zoals notities of een personageblad. Met twee editors en hun kopijhouders kun je vier documenten tegelijk zien.

- **Om hem te openen** houd je **Alt** ingedrukt en sleep je een document uit de binder, een prikbord of een overzicht naar de kopbalk van de editor. Zonder Alt laadt het neerzetten het document in de editor zelf. **Navigeren > Openen > Openen in kopijhouder** en het rechtsklikmenu van een document doen hetzelfde voor het geselecteerde document.
- De kopijhouder heeft een smalle kopbalk met de titel en geen voettekst. Hij toont de tekst van één document (of een afbeelding of pdf); hij heeft geen prikbord, overzicht of gecombineerde tekst. Je kunt erin schrijven en opmaken zoals in elke editor.
- **Om hem te verplaatsen** klik je met de rechtermuisknop op zijn kopbalk en kies je links, rechts, boven of onder. De eerste keer opent hij naast een brede editor en onder een smalle; daarna waar je hem het laatst neerzette. Sleep de lijn tussen editor en kopijhouder om hun groottes te wijzigen.
- **Om een ander document te tonen** zet je het neer op de kopbalk van de kopijhouder, of sleep je het met Alt opnieuw naar de kopbalk van de editor.
- **Om hem te sluiten** klik je op de **X** rechts van zijn kopbalk, of kies je **Navigeren > Editor > Kopijhouder sluiten**. De knop links van de X sluit hem en opent het document in plaats daarvan als venster met snelle naslag.

**Navigeren met een kopijhouder.** Terwijl een editor een prikbord of overzicht toont en zijn kopijhouder open heeft, heeft de knop **Selectie werkt op** in de voettekst drie standen. De eerste klik laat je selectie zoals gewoonlijk in de andere editor openen. De tweede klik draait de pijl zodat hij in het vak wijst: nu opent de kaart of rij waarop je klikt in de kopijhouder. Een derde klik zet het uit. Een handige opstelling: het manuscript in de linker editor, een overzicht van je onderzoek in de rechter editor met zijn kopijhouder eronder, en **Navigeren > Selectie in de Binder werkt op > Linker of bovenste editor**. De binder stuurt dan het manuscript aan, en het overzicht stuurt de kopijhouder aan.

De kopijhouder, zijn plaats en zijn document worden met het project onthouden.

### De editorkop

De kop boven elke editor toont de documenttitel en bevat de knoppen **terug** en **vooruit** (**Ctrl+[** en **Ctrl+]**), een menu met de secties van het document en knoppen voor het vorige en volgende document (**Alt+Shift+Omhoog / Omlaag**).

- **Geschiedenis**: elke editor onthoudt de documenten die hij heeft getoond. Houd de knop terug of vooruit ingedrukt (of klik er met de rechtermuisknop op) voor een lijst en kies een document om er direct heen te gaan; **Navigeren > Terug naar** en **Vooruit naar** tonen dezelfde lijsten. De geschiedenis wordt met het project bewaard.
- **Het pictogram** naast de titel staat voor het document. Klik erop voor een menu (Vastzetten, In de Binder tonen ...). Sleep het zoals een document in de binder: naar de kopbalk van de andere editor, naar de binder, naar een tabblad van een verzameling, naar de knop Snelle naslag of Bladwijzers van de werkbalk, of naar tekst om een koppeling te maken.
- **Documenten > Groeperen** (**Ctrl+Shift+J**, ook in het contextmenu van de binder) verplaatst de geselecteerde items naar een nieuwe map op de plek van het eerste; de titel is klaar om te typen. Met het toetsenbord in de binder neemt **Bewerken > Ongedaan maken** de hele stap terug.
- **Documenten > Omzetten > Blad naar object** (ook in het contextmenu van de binder) maakt van elk geselecteerd bladdocument een Object (sectie over Objecten), bijvoorbeeld een personage- of locatieblad: de titel is de naam, de tekst gaat naar **Overig**, een afbeelding in de tekst wordt de afbeelding. De naam van de map (Personages, Plaatsen) bepaalt de soort; voor een andere map wordt het je gevraagd. Het binderitem blijft: als je het selecteert, toont de inspector het Object, en Return opent het in het venster Objecten. Het document behoudt zijn tekst (een Scrivener-project ziet een gewoon document); de koppeling wordt bewaard in de eigen Objectengegevens van het project. **Bewerken > Ongedaan maken** met het toetsenbord in de binder neemt het terug.
- **Navigeren > Ga naar** toont de hele binder als menu en laadt het gekozen document in de actieve editor. **Navigeren > Omsluitende groep** toont de map of groep waartoe het document behoort, met het document daar geselecteerd.
- Een **dubbelklik op de scheiding** tussen twee editors maakt ze even groot.

### Slepen en neerzetten in één oogopslag

| Sleep ... | naar ... | en ... |
|---|---|---|
| een document (binder, verzameling, prikbord, overzicht, kopicoon, bladwijzer) | de kopbalk van een editor | het wordt in die editor getoond; met **Alt**: in zijn kopijhouder |
| | tekst in een editor | een koppeling naar het document; op geselecteerde tekst wordt die tekst de koppeling |
| | tekst, met **Alt** ingedrukt | de tekst van het document wordt ingevoegd |
| een afbeelding uit de binder | tekst in een editor | de afbeelding wordt ingevoegd |
| een document | de knop **Snelle naslag** van de werkbalk | elk document opent in een eigen paneel |
| | de knop **Bladwijzers** van de werkbalk | het wordt aan de projectbladwijzers toegevoegd |
| | een tabblad van een verzameling, een bladwijzerlijst | het wordt toegevoegd |
| | de kopbalk van een paneel Snelle naslag | het wordt in dat paneel getoond |
| | het bestandsbeheer of het bureaublad | het wordt daar als bestand opgeslagen (zie "Documenten exporteren") |
| geselecteerde tekst | de binder | een nieuw document met die tekst; de tekst blijft waar hij was |
| bestanden uit het bestandsbeheer | de binder of een prikbord | ze worden geïmporteerd |
| een weblink uit een browser | een map in de binder, het prikbord, het overzicht | de pagina wordt geïmporteerd (add-on Webpagina importeren, sectie 14) |
| een afbeeldingsbestand, of een afbeelding uit de binder | de indexkaart in de inspector (afbeeldingsmodus) | het wordt de afbeelding van de kaart |
| trefwoorden uit **Project > Projecttrefwoorden** | een document in de binder, een verzameling, het prikbord, het overzicht; de trefwoordenlijst van de inspector | ze worden toegewezen; met **Alt**: samen met de groep waartoe ze behoren |

Sommige bureaubladen gebruiken Alt+slepen om vensters te verplaatsen. Als Alt+slepen het programma niet bereikt, wijzig dan de wijzigingstoets voor het verplaatsen van vensters in de instellingen van je bureaublad (KDE: Systeeminstellingen, venstergedrag; de exacte plek verschilt per versie), of gebruik in plaats daarvan de menuopdrachten (**Navigeren > Openen > Openen in kopijhouder**, **Project > Projecttrefwoorden > Toewijzen aan geselecteerde documenten**).

### Panelen Snelle naslag

Een paneel Snelle naslag is een document in een eigen venster, bijvoorbeeld om een personageblad in beeld te houden terwijl je schrijft. Tekst in een paneel kan worden bewerkt en opgemaakt zoals in de editor (de sneltoetsen van de menu's Opmaak en Invoegen werken daar).

**Panelen openen:**

- **Navigeren > Openen > Openen als snelle naslag**, of de knop Snelle naslag van de werkbalk, voor de geselecteerde documenten.
- Druk op **Spatie** op het prikbord of in het overzicht, of in de binder en in een lijst van een verzameling of zoekresultaten wanneer **Weergave > Snelle toetsen in de Binder** aanstaat (zie sectie 2, de toetsen van de binder). Zonder die optie hoort Spatie in de binder bij het typen van een naam om erheen te springen.
- Sleep documenten naar de **knop Snelle naslag** van de werkbalk: elk opent in een eigen paneel, en de editor behoudt zijn document.
- **Navigeren > Snelle naslag openen** toont de hele binder als menu, zodat je een document kunt openen zonder het eerst te selecteren.

**In een paneel:**

- De **titel** in de kopbalk kan worden bewerkt (dat hernoemt het document). Het **pictogram** ernaast opent een menu (In de Binder tonen, Openen in de editor, toevoegen aan of verwijderen uit de projectbladwijzers) en kan worden gesleept zoals het document zelf. Zet een document neer op de kopbalk om het in dit paneel te tonen.
- De knop **rechtsboven** toont de **samenvatting**, de **notities**, de **trefwoorden** of de **metagegevens** van het document onder zijn tekst. Wat je daar wijzigt, wijzigt het document.
- De **voettekstbalk** toont het aantal woorden en laat je **label** en **status** instellen.
- De **bladwijzerknop** links van de voettekstbalk opent de **projectbladwijzers** naast het document. Een klik op een document met een bladwijzer toont het in het paneel, zodat één paneel genoeg is om tussen je notities te wisselen. Sleep bladwijzers in een andere volgorde, zet documenten erin neer en verwijder een bladwijzer met de knop ➖ (het document blijft in de binder). Bestanden en weblinks openen met een dubbelklik.
- De knop ➕ onder de bladwijzers maakt een **nieuw document** en voorziet het van een bladwijzer. De eerste keer wordt om de map gevraagd (bijvoorbeeld een map "Notities" in Onderzoek); vink **Niet meer vragen** aan om die map voortaan te gebruiken. Wijzig dit onder **Project > Projectinstellingen… > Speciale mappen**.
- De knop **Zweven** houdt het paneel boven het hoofdvenster; **Venster > Panelen Snelle naslag laten zweven** doet dat voor alle panelen.

Het menu Venster toont de geopende panelen en sluit ze allemaal tegelijk. De panelen die open zijn wanneer je een project sluit, komen terug wanneer je het opnieuw opent, met hun plaats, grootte, onderste vak en zijbalk (zet dit uit onder **Bestand > Opties > Gedrag**).

### Audio en video

Audio- en videobestanden in de binder (bijvoorbeeld interviews in de map Onderzoek) worden binnen {{APP}} afgespeeld wanneer de add-on **Media afspelen** is geïnstalleerd (sectie 17; hij komt met update 0.12). Zonder die toont zo'n bestand **Openen met standaardtoepassing**, zoals voorheen.

**Afspelen.** Klik op het bestand in de binder. Een video toont zijn beeld; een audiobestand toont zijn titel, artiest en lengte. De **voettekstbalk** van de editor heeft de afspeelknoppen:

- de knop **pauzeren en terugspoelen** links (zie hieronder),
- **naar het begin**, **terugspringen**, **afspelen / pauzeren**, **vooruitspringen**,
- de **positie**: sleep de schuif, of klik naast zijn greep,
- de tijd, de **snelheid** (0,5× tot 2×; langzamere spraak is makkelijker uit te typen; de stem wordt lager of hoger met de snelheid, zoals bij een bandje) en het **volume** (klik: dempen; de pijl: een schuif).

Een klik op het beeld van een video speelt of pauzeert hem; een rechtsklik biedt ook **Openen met standaardtoepassing** aan. Met het toetsenbord in de speler: **Spatie** speelt of pauzeert, de **pijltoetsen links** en **rechts** springen, **Home** gaat naar het begin.

**Uittypen.** Splits de editor, toon het audiobestand in de ene editor en je transcript in de andere, en blijf typen: de opdrachten van **Navigeren > Media** werken waar het toetsenbord ook is, ook in panelen Snelle naslag en in de schrijfmodus.

| Opdracht | Sneltoets |
|---------|----------|
| Afspelen / Pauzeren | Ctrl+Return |
| Terugspringen / Vooruitspringen (5 seconden) | Ctrl+Alt+, / Ctrl+Alt+. |
| Mediatijdstempel invoegen (ook **Invoegen > Mediatijdstempel invoegen**: dezelfde opdracht, in beide menu's getoond) | Ctrl+Shift+Return |
| Stoppen, Naar het begin, Pauzeren en terugspoelen, Sneller, Langzamer, Normale snelheid, Volume hoger, Volume lager, Dempen, Naar tijdstempel bij cursor | geen sneltoets (geef ze er een onder **Bestand > Opties > Toetsenbord**) |

De opdrachten werken op het bestand dat je het laatst opende of gebruikte (in een van beide editors, een kopijhouder of een paneel Snelle naslag).

**Pauzeren en terugspoelen.** Wanneer dit aanstaat (de knop ziet er ingedrukt uit), gaat elke pauze een beetje terug, met 2 seconden tenzij je onder **Bestand > Opties > Gedrag > Afspelen** een andere waarde instelt. Wanneer je op Ctrl+Return drukt om te typen wat je hoorde, hoor je de laatste woorden opnieuw wanneer je verdergaat.

**Tijdstempels.** **Mediatijdstempel invoegen** typt de tijd van de speler bij de cursor, zoals `[03:07]` (met uren vanaf één uur: `[1:02:03]`). **Naar tijdstempel bij cursor** doet het tegenovergestelde: het neemt de tijdstempel in de regel van de cursor (de dichtstbijzijnde ervoor of eromheen) en verplaatst de speler daarheen. Tijdstempels zijn gewone tekst; ze kunnen ook met de hand worden geschreven (`3:07`, `[03:07]`, `1:02:03.5`).

**In een kopijhouder of een paneel Snelle naslag** heeft de speler zijn afspeelknoppen onderaan. Een audiobestand in de kopijhouder van de editor van het transcript is een goede opstelling voor één enkele editor.

**Waar het was.** Wanneer je een audio- of videobestand opnieuw opent, gaat het verder waar je het pauzeerde (zet dit uit onder **Bestand > Opties > Gedrag > Afspelen**). Een ander document in de editor tonen stopt het bestand.

**Wat er wordt afgespeeld.** De add-on *Media afspelen* (2.0) speelt formaten af die vrij te gebruiken zijn: MP3, Ogg Vorbis, Opus, FLAC, ALAC, WAV, AIFF en WavPack voor audio; WebM, MKV en Ogg voor video met VP8-, VP9- of Theora-beeld (ook MJPEG en andere vrije formaten). De add-on brengt alles mee wat hij nodig heeft.

**Meer formaten.** H.264, H.265 (HEVC), AAC (M4A), AC-3, E-AC-3, DTS, MPEG-1/2/4-video, VC-1 en WMV, WMA en AMR vallen in sommige landen onder octrooien; ze komen met de aparte add-on **Extra video- en audioformaten**. De eenvoudigste manier om beide te krijgen is het ene pakket **media-all-formats.qbpkg** op de pagina Releases: geïnstalleerd zoals elke add-on (sectie 17) installeert het *Media afspelen* en *Extra video- en audioformaten* samen. Beide verschijnen dan in **Help > Add-ons en updates > Add-onbeheer…**, waar de extra formaten afzonderlijk kunnen worden uitgezet of verwijderd. Of voor het gebruik van deze formaten een octrooilicentie nodig is, hangt van het land af. Zolang hij is geïnstalleerd en aanstaat, gebruikt *Media afspelen* hem, en worden ook MP4/M4V/MOV-, AVI-, MPEG-, WMV-, FLV- en 3GP-bestanden afgespeeld (niet WMA Pro en WMV 9). Zonder hem toont zo'n bestand onder de speler een melding die zijn formaten en deze add-on noemt; al het andere werkt zoals voorheen. AV1-video wordt met de ingebouwde decodering niet afgespeeld.

Sinds 0.80.0 decodeert de speler altijd met zijn eigen FFmpeg: de Qt die {{APP}} sindsdien gebruikt (de eigen build van The Qt Company van Qt 6.12) kan niet via de **GStreamer** van het systeem afspelen, dus **Bestand > Opties > Gedrag > Afspelen > Decodering** toont die keuze als niet beschikbaar.

### Bladwijzers

**Documentbladwijzers** (inspector, tabblad Bladwijzers) koppelen een document aan andere documenten, bestanden op je schijf of webpagina's. **Projectbladwijzers** zijn beschikbaar vanuit elk document, vanuit de bladwijzerknop in de werkbalk en vanuit de zijbalk van panelen Snelle naslag. Het tabblad Bladwijzers van de inspector toont beide lijsten, boven elkaar: **Documentbladwijzers** en **Projectbladwijzers**, elk met zijn eigen knoppen ➕ en ➖. Een klik op een bladwijzer toont het document onder de lijsten; een document dat de editor al toont, verschijnt daar als alleen-lezen kopie (schrijf in de editor), zodat de editor zijn regelbreedte behoudt. Een dubbelklik opent de bladwijzer. Sleep documenten, bestanden of weblinks in de bladwijzerlijst, of gebruik de knop ➕; een link die uit een webbrowser wordt gesleept, behoudt de titel van de pagina. Sleep een bladwijzer omhoog of omlaag om de volgorde te wijzigen. Documenten die op de **knop Bladwijzers** van de werkbalk worden neergezet, worden projectbladwijzers; dat geldt ook voor de geselecteerde documenten met **Documenten > Aan projectbladwijzers toevoegen**; de inspector toont dan zijn tabblad Bladwijzers met de nieuwe invoer geselecteerd.

---

## 7. De inspector

De knoppen bovenaan de inspector schakelen tussen:

- **Samenvatting en notities**: de samenvatting is een korte samenvatting die op de indexkaart verschijnt. Je kunt een afbeelding gebruiken in plaats van tekst: klik op de afbeeldingsknop van de kaart en zet een afbeeldingsbestand of een afbeelding uit de binder neer op het grijze gebied (of dubbelklik op het gebied om een bestand te kiezen; rechtsklik om de afbeelding te verwijderen). Daaronder staan de **documentnotities** (of, omgeschakeld met het kleine menu, de **projectnotities**, gedeeld door het hele project). **Navigeren > Inspecteren > Naar samenvatting gaan** (**Ctrl+Alt+Y**) en **Naar documentnotities gaan** (**Ctrl+Alt+N**) zetten het toetsenbord vanuit de tekst daarheen; **Alt+2** gaat terug naar de editor. Sinds 0.93.0 beginnen de notities (document- en projectnotities) als platte tekst: geen lettertype, grootte, inspringing van de eerste regel, alinea-afstand of stijl uit de tekstopmaak van het project, in het lettertype van de programma-interface bij de zoom van de notities. Opmaak verschijnt alleen wanneer je haar zelf toepast (de opmaakbalk, de menu's, sneltoetsen) en blijft dan. Notities die al tekst hebben, behouden het uiterlijk dat ze hadden. Tekst die in de notities wordt geplakt, is gewone tekst; **Bewerken > Plakken met opmaak** (**Ctrl+Alt+Shift+V**) behoudt de opmaak die hij meebrengt.
- **Bladwijzers**: de bladwijzers van het document en die van het project. Een klik op een bladwijzer van een document van het project toont zijn tekst onder de lijsten. Je kunt daar schrijven en opmaken zoals in een editor (de opmaakbalk, het menu Opmaak en hun sneltoetsen werken erop terwijl de cursor erin staat): alles gaat meteen in het document en zijn bestand, en een editor die hetzelfde document toont, toont de wijziging meteen. De tekst daar heeft een eigen zoom.
- **Mindmap**: een bord met notities voor het document of de map (zie "Mindmaps" hieronder).
- **Objecten**: de personages, locaties, omgevingen, bezienswaardigheden, weer en andere dingen van je verhaal, elk met een eigen blad en koppelingen ertussen (zie "Objecten" hieronder).
- **Metagegevens**: label, status, sectiesoort, opnemen bij samenstellen, aanmaak- en wijzigingsdatums, **aangepaste metagegevens** en **trefwoorden**.
- **Momentopnamen** (sectie 11).
- **Opmerkingen en voetnoten**.

Schrijfhulpmiddelen van add-ons kunnen eigen tabbladen toevoegen; hun knoppen staan vooraan in de rij, bijvoorbeeld **Transcripties** van Spraaktranscriptie (sectie 10).

**De eigen zoom van de inspector.** **Ctrl+muiswiel** boven een deel van de inspector maakt dat deel groter of kleiner, zonder de editors te wijzigen. De tabbladen met twee delen hebben voor elk een zoom: **Samenvatting en notities** (de kaart, de notities), **Bladwijzers** (de twee lijsten, de tekst van de bladwijzer eronder), **Momentopnamen** (de lijst, de tekst van de momentopname), **Objecten** (de categorieknoppen met de lijst, het blad). De lijst **Zoom** onderaan toont en stelt de zoom in van het deel dat het toetsenbord heeft, of van het deel dat je het laatst zoomde; zijn titel noemt het deel ("Zoom: Notities"), en op het tabblad Mindmap is het de zoom van het bord. De andere tabbladen (Metagegevens, Opmerkingen) en de rijen knoppen bovenaan en onderaan delen één zoom; met **Weergave > Tabbladen van de Inspector apart zoomen** (ook in het rechtsklikmenu van de lijst Zoom) behouden Metagegevens en Opmerkingen er elk één.

**De inspector onthoudt** met het project welk tabblad open was, waar je de scheidingen in de tabbladen zette, de getoonde bladwijzer en het object dat op het tabblad Objecten werd getoond.

**Opmerkingen van een map.** Met een map in de editor (als prikbord of overzicht) toont het tabblad Opmerkingen en voetnoten de opmerkingen en voetnoten van elk document in de map en zijn submappen, gegroepeerd per document. Een klik op de naam van een document opent het; een klik op **Opmerking ▸** (of **Voetnoot ▸**) opent het document en toont die plek; de tekst van een opmerking of voetnoot kan daar meteen worden gewijzigd. In de gecombineerde tekst (meerdere documenten als één tekst) toont het tabblad de notities van alle getoonde documenten.

### Mindmaps

Elk document en elke map kan een **mindmap** hebben: een bord waarop je overal notities zet, ze met lijnen en pijlen verbindt, kleurt en groepeert op achtergrondvormen. Borden kunnen als Scapple-bestanden worden opgeslagen, en Scapple-bestanden kunnen worden gelezen (zie *Bestanden en Scapple* hieronder). Klik op de knop **Mindmap** van de inspector (tussen Bladwijzers en Metagegevens). De knop is onderstreept wanneer het document een mindmap heeft.

**Een vastgezette mindmap (0.92.0).** Normaal toont het tabblad de mindmap van het document of de map die je hebt geselecteerd. Je kunt hem in plaats daarvan op één item vastzetten: het tabblad blijft dan de mindmap van dat item tonen, wat je ook in de binder selecteert of in de editor opent. Dit is voor een bord dat je naast je wilt hebben terwijl je schrijft, zoals een plotkaart op een map. Drie plaatsen doen hetzelfde:

- **In de binder** beweeg je de muis over een rij: een kleine punaise verschijnt vlak bij het rechteruiteinde. Klik erop, en dat item is vastgezet; zijn punaise blijft en is gevuld. Een klik op de gevulde punaise maakt hem los. Op een punaise klikken selecteert niets.
- **In het tabblad Mindmap** zet de punaiseknop rechts van de kleine balk het bord vast dat het tabblad toont en maakt het weer los. Terwijl een bord is vastgezet, noemt de balk zijn item (*Vastgezet: Plot*, direct na de overzichtsknop; een lange titel wordt ingekort, en de hele titel verschijnt wanneer je de muis erop laat rusten), zodat duidelijk is dat het bord niet bij het geselecteerde document hoort.
- **In de menu's**: **Navigeren > Inspecteren > De mindmap van dit item vastzetten** en **Navigeren > Inspecteren > De vastgezette mindmap losmaken** (ze hebben eerst geen sneltoetsen; geef ze er in de opties een als je wilt).

Terwijl een item is vastgezet, zegt een smalle balk onderaan de binder *Mindmap vastgezet:* met zijn titel en heeft hij de knop **Loslaten**. Hooguit één item van een project is vastgezet; een ander vastzetten verplaatst het vastzetten. Een item zonder mindmap kan ook worden vastgezet: het tabblad toont een leeg bord om mee te beginnen. Alleen het tabblad Mindmap is vastgezet; de andere tabbladen van de inspector volgen zoals altijd het geselecteerde document, en een bord dat in een groot venster is geopend, blijft wat het is. Het vastzetten wordt bij het sluiten met het project op deze computer bewaard, en het wordt losgemaakt wanneer het item naar de Prullenbak gaat.

**In de inspector** vult het bord het tabblad. De kleine balk erboven heeft **Nieuwe notitie**, **Nieuwe achtergrondvorm**, **Passend zoomen**, **Menu** (alle opdrachten: Bestand, Bewerken, Weergave, Notities, Opmaak), de knop **overzicht** (zie *Rondkijken*) en **In venster openen**. De balk eronder telt de notities en heeft de zoomschuif. **In venster openen** toont hetzelfde bord in een groot venster met dezelfde menu's in zijn menubalk en een inspector rechts (**Notitieopmaak**, **Notitiestijlen**, **Documentinstellingen**); wijzigingen op de ene plek zijn meteen zichtbaar op de andere. Een rechtsklik op het bord opent de opdrachten voor wat onder de muis ligt. Een nieuwe notitie (knop, menu of een dubbelklik) neemt meteen het toetsenbord, ook wanneer het eerder in de binder stond: wat je typt, vervangt zijn geselecteerde tekst (0.94.0).

De opdrachten van een mindmap staan in deze eigen menu's, niet in de menu's van het hoofdvenster. Hieronder betekent **Menu > Notities > Verbinden**: de knop **Menu** van het bord van de inspector, dan **Notities**, dan **Verbinden** (in het grote venster: **Notities > Verbinden** van zijn menubalk). Terwijl het bord het toetsenbord heeft, gaan zijn toetsen voor: **Ctrl+Z** maakt een wijziging van het bord ongedaan, **Ctrl+F** zoekt in zijn notities, **Ctrl+P** drukt het af.

**Help voor mindmaps.** **Help > Mindmap-help** (ook de knop **?** aan het rechteruiteinde van de balk van het bord, en **Menu > Help** van het bord) toont een korte gids met afbeeldingen: hoe je notities maakt, hoe je een lijn trekt (sleep een notitie naar een andere), hoe je een pijl trekt (houd **Alt** ingedrukt terwijl je loslaat; **Ctrl+Alt** voor een pijl terug, **Ctrl+Alt+Shift** voor beide uiteinden), woorden op een lijn en achtergrondvormen. De knop **Voorbeeldbord openen** (ook **Menu > Help > Voorbeeldbord openen**) opent een oefenbord in een eigen venster, met notities om op elkaar te slepen en kant-en-klare voorbeelden van lijnen en pijlen. Niets op het voorbeeldbord hoort bij je project, en het wordt niet opgeslagen.

**Notities.**

- **Dubbelklik** op een lege plek: een nieuwe notitie, klaar om te typen. **Ctrl+Enter** maakt een nieuwe notitie onder de geselecteerde (gestapeld). Met **Ctrl** bij de dubbelklik wordt de nieuwe notitie met de geselecteerde notities verbonden; met **Alt** door een pijl ernaartoe; met **Ctrl+Alt** door een pijl terug.
- **Dubbelklik** op een notitie om zijn tekst te wijzigen; **Esc** of een klik erbuiten beëindigt het bewerken. Een korte notitie wordt zo smal als zijn tekst; een lange breekt af op de breedte van nieuwe notities (beide in **Menu > Bestand > Mindmapopties…**).
- **Sleep** notities om ze te verplaatsen; met **Ctrl** bij het neerzetten worden ze gekopieerd. De **pijltoetsen** verplaatsen de geselecteerde notities wanneer **Menu > Bewerken > Pijltoetsen verplaatsen notities** (**M**) aanstaat (met **Shift** met 10 px); anders selecteren ze de volgende notitie in die richting. **Enter** of **F2** bewerkt de geselecteerde notitie, **Delete** verwijdert de geselecteerde notities.
- **Sleep een notitie naar een andere** om de twee met een lijn te verbinden; nogmaals om de lijn te verwijderen. Met **Alt** krijgt de lijn een pijl naar de notitie waarop is neergezet; met **Ctrl+Alt** een pijl terug; met **Ctrl+Alt+Shift** pijlen aan beide uiteinden.
- **Dubbelklik op een lijn** om er een nieuwe notitie in te zetten, of op het label van een lijn om het te wijzigen (**Menu > Notities > Verbindingslabel…**).
- **Menu > Notities > Verbinden** (**Ctrl+.**), **Verbinden met pijl** (**Ctrl+Alt+.**), **Pijlen verwijderen** (**Ctrl+,**) en **Verbinding verbreken** werken op de geselecteerde notities: de eerste die je selecteerde, is het middelpunt.
- **Menu > Notities > Nieuwe verbonden notitie** maakt een notitie links van, rechts van, boven of onder de geselecteerde, ermee verbonden (met of zonder pijl).
- **Menu > Notities > Splitsen** (**Ctrl+Shift+K**) verdeelt een notitie bij de cursor; **Samenvoegen** voegt de geselecteerde notities samen.
- **Menu > Notities > Stapelen** (**Ctrl+'**), **Uitlijnen**, **Verdelen**, **Even breed / hoog maken**, **Automatisch aanpassen**, **Naar voren halen**, **Naar achteren sturen**.
- **Menu > Bewerken > Koppeling toevoegen…** (**Ctrl+K**) koppelt tekst van een notitie aan een webpagina of een bestand; documenten die uit de binder op het bord worden gesleept, worden notities met koppelingen ernaartoe. Een klik op een koppeling opent hem: een document opent zoals documentkoppelingen in de tekst dat doen (**Bestand > Opties > Gedrag**).
- **Menu > Notities > Notities aan de Binder toevoegen** zet de geselecteerde notities om in documenten binnen het document of de map waartoe het bord behoort, in de leesvolgorde van het bord.

**Achtergrondvormen** (**Menu > Notities > Nieuwe achtergrondvorm**, **Ctrl+Shift+G**) zijn gekleurde vlakken achter notities. Gemaakt terwijl notities zijn geselecteerd, omringt de vorm ze. Een **magnetische** vorm (**Menu > Notities > Magnetisch**, **Ctrl+Shift+M**) neemt de notities erop mee wanneer hij wordt verplaatst. Een nieuwe vorm begint middelgrijs (#808080) en niet magnetisch; **Bestand > Opties > Extra's > Mindmap** (en **Menu > Bestand > Mindmapopties…**) stelt de vulling, de rand en **Magnetisch** in waarmee nieuwe vormen beginnen. (Vóór 0.93.0 was een nieuwe vorm lichtgrijs en magnetisch; een kleur die je daar had gekozen, blijft, en een bewaarde instelling **Magnetisch** werd één keer uitgezet.)

**Het rechtsklikmenu van een notitie** (of van een achtergrondvorm) wijzigt de geselecteerde notities: **Vulkleur**, **Tekstkleur** en **Randkleur** bieden de kleuren van de labels van je project, de zes kleuren van de opmaakbalk (rood, blauw, geel, groen, wit, zwart) en **Anders...** voor elke kleur, dezelfde keuze overal in {{APP}}; **Vorm** is **Rechthoek**, **Afgeronde rechthoek**, **Wolk** of **Gekarteld** (een notitie zonder rand krijgt een dunne, zodat de vorm zichtbaar is); **Rand** stelt de dikte in; **Pictogrammen** zet maximaal acht pictogrammen op een notitie, één bij elke hoek en één in het midden van elke zijde, gekozen uit de pictogrammen van de binder (**Documenten > Pictogram wijzigen**), je eigen pictogrammen inbegrepen. Klik met de rechtermuisknop vlak bij de plek van een pictogram om alleen dat te verwijderen; **Alle pictogrammen verwijderen** wist ze.

**Het rechtsklikmenu van een lijn** hoort alleen bij die lijn, zodat andere lijnen van dezelfde notities niet worden gewijzigd: zijn pijlen (**Geen pijlen**, een pijl naar een van beide notities of **Pijlen aan beide uiteinden**), **Lijnkleur**, **Pictogram in het midden** (een pictogram dat in het midden van de lijn wordt getekend), het label, **Notitie op de lijn invoegen** en **Verbinding verbreken**. Elke wijziging kan afzonderlijk ongedaan worden gemaakt.

De pictogrammen, lijnkleuren en lijnpictogrammen zijn eigen toevoegingen van {{APP}}; ze worden in het bordbestand bewaard in elementen die Scapple niet leest (een bord dat als Scapple-bestand wordt geëxporteerd, heeft de notities, lijnen en pijlen zonder deze). **Bestand > Opties > Extra's > Mindmap** stelt de grootte van de pictogrammen en de vorm van nieuwe notities in.

**Uiterlijk.** **Menu > Opmaak** heeft vet, cursief, onderstrepen, doorhalen, groter en kleiner (**Ctrl+Shift+>** en **Ctrl+Shift+<**), tekst-, vul- en randkleuren, randdikte (geen, 1 tot 5 px) en stijl (afgerond, vierkant, gekarteld, wolk), schaduw en **Vervagen** (**Ctrl+Shift+F**, de notitie wordt bleek getoond). **Notitiestijlen** bewaren een uiterlijk onder een naam; **Menu > Opmaak > Notitiestijl** past er een toe (**Ctrl+Alt+1** tot **9**, **Ctrl+Alt+0** voor Standaard), maakt er een van de geselecteerde notitie, of kopieert en plakt het uiterlijk van een notitie (**Alt+Shift+C** / **Alt+Shift+V**). **Documentinstellingen** in de inspector van het venster stellen de achtergrond, de tekstkleur, het lettertype en de schrijfrichting van het bord in (en **Als standaard instellen** voor nieuwe borden).

**Achtergrond.** Een rechtsklik op een lege plek van het bord (in de inspector of in het venster), **Menu > Opmaak > Achtergrondkleur** of de achtergrondknop van **Documentinstellingen** biedt de kleuren van de labels van je project, de zes kleuren van de opmaakbalk, de standaard (**Standaard voor lichte thema's** of **Standaard voor donkere thema's** in de menu's, **Standaard gebruiken** in Documentinstellingen) en **Anders...**. Een bord heeft een eigen achtergrond voor lichte thema's en een voor donkere thema's: de kleur die je kiest, wordt bewaard voor de soort thema die in gebruik is, en de andere soort behoudt de zijne. Een bord zonder eigen kleur toont de standaard van **Bestand > Opties > Extra's > Mindmap** ("Achtergrond van borden", een voor lichte en een voor donkere thema's) en volgt die wanneer je hem wijzigt. **Als standaard instellen** in Documentinstellingen maakt de achtergrond van het bord de standaard van de soort thema die in gebruik is. Een bord behoudt zijn achtergrondkleur ook wanneer er nog niets op staat.

**Zoom en plaats.** Elk bord onthoudt met het project zijn zoom en het deel ervan dat op het scherm staat, een voor de inspector en een voor het grote venster. Een bord dat je nooit hebt gezoomd, opent met de standaardzoom van mindmaps (**Bestand > Opties > Bewerken**, "Standaardtekstzoom").

**Selecteren.** Klik, **Shift+klik** of **Ctrl+klik** om toe te voegen, sleep een kader rond notities, **Ctrl+A**. **Menu > Bewerken > Selecteren** heeft notities die eronder gestapeld staan, notities op de geselecteerde vormen (**Alt+A**), verbonden notities (**Ctrl+Shift+A**) en hele verbonden clusters; daaronder staan **Selectie omkeren** (**Alt+Shift+I**) en **Deselecteren** (**Ctrl+D**).

**Rondkijken.** Zoom met **Ctrl+muiswiel**, **Ctrl++** en **Ctrl+-**, **Ware grootte** (**Ctrl+0**), **Passend zoomen** (**Ctrl+9**), **Zoomen naar selectie** (**Ctrl+Shift+8**) en **Zoomen naar willekeurig** (**Ctrl+Shift+R**). Houd **Spatie** ingedrukt en sleep om het bord te verplaatsen; houd **Z** ingedrukt om het hele bord even te zien. **Menu > Bewerken > Zoeken** (**Ctrl+F**) zoekt en vervangt tekst in de notities. **Menu > Weergave > Paginahulplijnen** toont waar de pagina's eindigen bij het afdrukken.

**Het overzicht (0.93.0).** Een kleine kaart in de linkerbovenhoek van het bord toont het hele bord in het klein, en een kader voor het deel dat je nu ziet; het kader beweegt en verandert van grootte terwijl je scrolt en zoomt. Notities, achtergrondvormen en lijnen worden in hun kleuren getekend; de tekst van een notitie wordt alleen getoond waar hij bij die grootte nog leesbaar is, anders is de notitie zijn gekleurde kader. Klik in het overzicht om de weergave naar dat punt te verplaatsen; sleep het kader om de weergave mee te verplaatsen. De zoom blijft, en een bord dat is vergrendeld (**Bewerken voorkomen**) kan ook worden verplaatst. Het muiswiel boven het overzicht werkt als boven het bord (**Ctrl+wiel** zoomt). Sleep de greep in de rechteronderhoek om de breedte te wijzigen (hij begint op een vijfde van de breedte van de weergave, minstens 80 px, en is nooit breder dan de helft van de weergave); de hoogte volgt de vorm van wat hij toont, nooit meer dan de helft van de weergave, en de breedte krimpt mee wanneer de hoogte daar voorbij zou gaan. Het overzicht hoort alleen bij de weergave: het staat niet in exports, afdrukken of de afbeelding van het bord. Het is eerst **automatisch**: het verschijnt wanneer een deel van het bord buiten de weergave ligt, en verdwijnt wanneer alles past (met een kleine marge, zodat het aan de rand niet flikkert). De knop **overzicht** in de balk van het bord van de inspector (direct na **Menu**) schakelt tussen **automatisch** en **altijd getoond** (de knop is ingedrukt); dezelfde schakelaar is het item **Overzicht altijd tonen** in het menu Weergave van het bord (in de inspector: de knop **Menu**, dan **Weergave**; in het grote venster: zijn menubalk). De keuze en de groottes worden op deze computer bewaard, in de instellingen van het programma (één keuze voor alle borden; de grootte van het overzicht van de inspector en van het grote venster apart); er gaat niets in het project.

**Tekst, afbeeldingen en bestanden.** Plak of zet tekst neer om een notitie te maken (tekst met lege regels kan meerdere notities worden), zet een afbeelding neer voor een afbeeldingsnotitie, en zet een tekstbestand neer om van zijn alinea's notities te maken.

**Bestanden en Scapple.** **Menu > Bestand > Exporteren** schrijft het bord als pdf (**Ctrl+Shift+E**), PNG-afbeelding, platte tekst, tekstlijst, RTF, OPML-overzicht, zijn afbeeldingen of een Scapple-bestand (*.scap*); **Menu > Bestand > Mindmap- of Scapple-bestand importeren…** leest een *.scap*-bestand in het bord. **Afdrukken** en **Afdrukvoorbeeld** drukken het af. **Menu > Bestand > Bewerken voorkomen** vergrendelt het bord.

**Mindmapopties** (**Menu > Bestand > Mindmapopties…**, en hetzelfde in **Bestand > Opties > Extra's > Mindmap** van het hoofdvenster): of **Enter** het bewerken van een notitie beëindigt (dan maakt **Shift+Enter** een nieuwe regel), of **Enter** op een geselecteerde notitie een nieuwe maakt, of nieuwe notities het uiterlijk van de geselecteerde notitie overnemen, gestippelde lijnen zonder pijlen, smalle notities voor korte teksten, hoe bleek vervaagde notities zijn, en de grootte, breedte, kleur en rand van nieuwe notities en vormen.

**Waar het bord wordt bewaard.** Een bord wordt vanzelf opgeslagen, als bestand in de indeling van Scapple-bestanden (*content/<ID>/mindmap.scap* in de projectmap; *Files/Data/<UUID>/quillbinder-mindmap.scap* in een Scrivener-project, naast de eigen bestanden van het document). Een document krijgt het bestand pas wanneer er iets op zijn bord wordt gezet. Ongedaan maken (**Ctrl+Z**) werkt voor elk bord afzonderlijk.

{{APP}} leest Scapple-bestanden zoals Scapple 1.x ze schrijft, voor zover hun indeling bekend is; het was niet mogelijk ze met Scapple zelf te testen (zie de notities van versie 0.17.0 in het statusbestand).

### Objecten: personages, locaties en meer

**Objecten** zijn de mensen, plaatsen en dingen van je verhaal, elk met een blad met velden: een personage met ras, lengte, oogkleur, eigenschappen en achtergrondverhaal; een plaats met zijn soort, zijn omgeving en zijn weer; enzovoort. Ze horen bij het hele project, niet bij één document, en worden gescheiden van de binder bewaard. Koppelingen verbinden ze, zodat je je door een achtergrond heen kunt klikken: het personageblad, het dorp waar ze woont, het landschap eromheen, het weer daar.

Klik op de knop **Objecten** van de inspector (rechts van de knop **Mindmap**), of kies **Navigeren > Inspecteren > Objecten van het project: personages, locaties, omgevingen, ...**. **Venster > Objecten…** (of de vensterknop in de balk van het tabblad) toont dezelfde objecten in een groot eigen venster, met de lijst links en het blad rechts. Wijzigingen op de ene plek zijn meteen zichtbaar op de andere.

**Categorieën.** De gekleurde knoppen bovenaan zijn de categorieën: om mee te beginnen **Personages**, **Locaties**, **Omgevingen**, **Bezienswaardigheden** en **Weer**. Klik op een om zijn objecten te zien. Je kunt ze hernoemen, andere kleuren geven, hun volgorde wijzigen, ze verwijderen en eigen categorieën toevoegen (facties, spreuken, schepen ...): zie *Sjablonen en kleuren* hieronder.

**Objecten maken.** **+** in de balk van het tabblad maakt een nieuw object in de gekozen categorie (in de map van het geselecteerde object); typ zijn naam bovenaan het blad. De knop met de map maakt een **map** om te sorteren, bijvoorbeeld "Dorpen" binnen Locaties; mappen kunnen mappen bevatten. Sleep objecten en mappen in de lijst om ze te verplaatsen. **F2** of een rechtsklik hernoemt, en het rechtsklikmenu heeft ook **Dupliceren** en **Verwijderen…** (ook in het menu **≡** van de balk). Het veld **Filter** toont alleen de objecten waarvan de naam of tekst bevat wat je typt.

**Het blad.** Onder de gekleurde strook met de categorie en de naam staan de velden van het sjabloon van de categorie. Alles wat je typt, wordt vanzelf opgeslagen, zoals je documenten. De soorten velden zijn:

- **Korte tekst** (één regel) en **Datum** (in elke vorm die je wilt, zoals "lente 1203" of "de derde winter").
- **Getal** met een eenheid (cm, jaren, °C ...).
- **Lange tekst**, die koppelingen kan bevatten (zie hieronder), en **vette**, *cursieve*, onderstreepte, doorgehaalde, gekleurde en gemarkeerde woorden: met de cursor in het veld werken de opmaakbalk, het menu Opmaak en hun sneltoetsen (**Ctrl+B**, **Ctrl+I**, **Ctrl+U**; ook in het venster Objecten) erop. Versies 0.20 en 0.21 van {{APP}} tonen deze ook.
- **Keuze**: een lijst om uit te kiezen; je kunt ook iets anders typen.
- **Ja / nee**.
- **Afbeelding**: klik op het gebied (of zet een afbeelding erop neer) om er een te kiezen; ze wordt in het project gekopieerd, kleiner gemaakt als ze groter is dan 1600 pixels. Rechtsklikken verwijdert haar.
- **Koppeling naar objecten**: de gekoppelde objecten worden als gekleurde knoppen getoond. **+** voegt er een toe: typ om te zoeken en kies dan een object, of kies **Nieuw in Locaties: "Ashvale"** om een nieuw object met die naam te maken en het meteen te koppelen. Een klik op een gekoppeld object toont zijn blad; een rechtsklik verwijdert de koppeling.
- **Koppeling naar documenten**: documenten van de binder die bij het object horen, zoals "Komt voor in" van een personage of "Scènes die hier spelen" van een plaats. Een klik opent het document in de editor.

**Kleurknop.** De knop **Kleur** van de strook geeft een object een eigen kleur (of geeft hem de kleur van de categorie terug), bijvoorbeeld om de hoofdpersonen te markeren.

**Koppelingen in lange teksten.** Selecteer in een veld met lange tekst een woord en kies **Koppelen aan object…** (of **Koppelen aan document…**) in zijn rechtsklikmenu; zonder selectie wordt de naam van het object als koppeling ingevoegd. Een klik op de koppeling volgt hem; **Koppeling verwijderen** in het rechtsklikmenu haalt hem weer weg. In de tekst van een document voegt **Invoegen > Koppelen aan object…** een koppeling naar een object in de tekst in; een klik erop toont het object in de inspector. Wanneer het manuscript wordt samengesteld, worden deze koppelingen gewone woorden.

**Heen en weer gaan.** **‹** en **›** in de balk van het tabblad gaan terug en vooruit door de objecten die je bekeek, zoals een webbrowser.

**Gekoppeld vanuit.** Onderaan elk blad somt **Gekoppeld vanuit** de objecten op die naar dit object verwijzen (in een koppelingsveld of in een tekst), zodat het dorp laat zien wie erin woont. Een klik gaat daarheen.

**In dit document.** Wanneer het document in de editor aan objecten is gekoppeld (via een documentkoppelingsveld, bijvoorbeeld "Komt voor in"), toont het tabblad **In dit document:** met die objecten, en is de knop **Objecten** onderstreept zoals andere tabbladen met inhoud.

**Verwijderen.** Een object verwijderen vraagt eerst om bevestiging. Koppelingen ernaartoe worden uit de andere objecten verwijderd (in teksten blijft het gekoppelde woord staan, als gewone tekst). Een koppeling ernaartoe in een document zegt, wanneer je erop klikt, dat het object niet meer bestaat. Een map verwijderen laat zijn objecten behouden; ze gaan een niveau omhoog.

**Sjablonen en kleuren.** **Project > Objectsjablonen en kleuren…** (ook in het menu **≡** van het tabblad, en in het rechtsklikmenu van een categorieknop) bewerkt de categorieën en hun velden:

- Links de categorieën: **Toevoegen**, **Verwijderen** en de pijlen voor de volgorde. Een categorie verwijderen die objecten heeft, vraagt of die ook moeten worden verwijderd; **Nee** wijzigt niets.
- Rechts de naam en de **Kleur** van de categorie, en zijn velden: **Veld toevoegen**, **Verwijderen**, **Omhoog**, **Omlaag**. Voor elk veld zijn naam en zijn soort; een getal kan een eenheid hebben, een keuze zijn lijst met keuzes (een per regel) en een koppelingsveld de categorieën waaraan het kan koppelen (geen aangevinkt: alle).
- **Gebruiken voor nieuwe projecten** bewaart deze categorieën en sjablonen voor projecten die je vanaf nu maakt. **Standaardsjablonen** zet een categorie terug op het sjabloon waarmee hij kwam, of voegt de standaardcategorieën toe die je verwijderde.
- Een veld uit een sjabloon verwijderen verbergt wat de objecten erin hebben, maar verwijdert het niet uit hun bestanden. Voor een standaardveld brengt **Standaardsjablonen > Deze categorie terugzetten naar de standaard** het met zijn inhoud terug; een eigen veld komt niet terug, dus hernoem een veld liever dan het te verwijderen als je de inhoud misschien weer wilt hebben.

**Waar objecten worden bewaard.** In de projectmap, in een map *objects*: één klein bestand per object (*objects/characters/<ID>.json* enzovoort), de sjablonen, kleuren en mappen in *objects/schema.json* en de afbeeldingen in *objects/images*. Het projectbestand zelf wordt niet gewijzigd, zodat oudere versies van {{APP}} het project openen zoals voorheen en de objecten met rust laten. Een project in Scrivener-indeling bewaart zijn objecten in zijn werkkopie (sectie 14); Scrivener kent ze niet. Back-ups bevatten ze.

---

## 8. Je project ordenen

### Trefwoorden

Trefwoorden zijn labels, bijvoorbeeld de personages of plaatsen in een scène. Voeg ze toe in de inspector of beheer ze allemaal met **Project > Projecttrefwoorden** (**Ctrl+Alt+K**), waar je ze kleuren kunt geven, ze aan de geselecteerde documenten kunt toewijzen en alle documenten met een trefwoord kunt vinden. Je kunt ook **trefwoorden slepen** uit dat venster naar documenten in de binder, een verzameling, het prikbord of het overzicht (als het document een van meerdere geselecteerde is, krijgen ze allemaal de trefwoorden), of naar de trefwoordenlijst van de inspector of van een paneel Snelle naslag. Houd **Alt** ingedrukt tijdens het neerzetten om ook de groep van een trefwoord toe te wijzen. Sleep in het venster een trefwoord naar een ander om het in die groep te zetten. Trefwoordkleuren verschijnen op indexkaarten.

### Aangepaste metagegevens

Onder **Project > Projectinstellingen… > Aangepaste metagegevens** kun je eigen velden toevoegen: tekst, selectievakje, lijst (een vaste set keuzes) of datum, bijvoorbeeld *Gezichtspunt*, *Locatie* of *Verhaaldag*. Ze verschijnen in de inspector en als overzichtskolommen.

### De instellingen van het project in een bestand opslaan

**Project > Projectinstellingen opslaan in bestand…** schrijft de instellingen van het geopende project in een eigen bestand (*.qbsettings*): labels, status, sectiesoorten, aangepaste metagegevens, trefwoorden, de standaardopmaak, de stijlen, de scriptindeling, de doelen, de samenstellingsinstellingen, paginaweergave en paginaformaat, de afdrukinstellingen, het uiterlijk (thema, kleuren, lettertypen, kolombreedte, rand), de eigen opties van het project (Bestand > Opties met "Dit project") en de sjablonen en kleuren van het blad Objecten. De tekst van je documenten maakt er geen deel van uit. Labels, sectiesoorten en de mappen die de samenstellingsinstellingen noemen, worden bij hun naam bewaard, zodat het bestand in een ander project kan worden gelezen.

**Project > Projectinstellingen laden uit bestand…** leest zo'n bestand in het geopende project. Wanneer de instellingen van het project dezelfde zijn als in het bestand, zegt het dat en wijzigt niets. Wanneer ze verschillen, toont een venster elke sectie met wat verschilt (bijvoorbeeld "alleen in het bestand: Idee; alleen in het project: Herzien") en laat je voor elke sectie kiezen:

- **Overnemen uit het bestand**: de instellingen van het bestand voor deze sectie.
- **Die van het project behouden**: de sectie blijft zoals ze is.
- **Quillbinder-standaard**: wat een nieuw project krijgt (de instellingen voor nieuwe projecten van **Bestand > Opties**, de standaardlabels en -statussen ...).

**Alles overnemen uit het bestand** en **Alles van het project behouden** stellen elke sectie in één keer in. Wat je ook kiest, niets wat je documenten gebruiken gaat verloren: een label, status, sectiesoort, trefwoord, aangepast metagegevensveld, objectcategorie of stijl die documenten nog gebruiken, blijft in het project ook wanneer het bestand hem niet heeft, en de melding aan het eind noemt wat is behouden. Wanneer de opmaak of een stijl verandert, volgt de tekst van alle documenten, zoals wanneer je hem zelf wijzigt. Voordat het laadt, bewaart {{APP}} de instellingen zoals ze waren in een bestand in de map *settings* van het project ("settings-before-loading-*datum*.qbsettings"); laad dat bestand om terug te gaan.

### Sectiesoorten

Sectiesoorten vertellen Samenstellen wat elk document is: *Deel*, *Hoofdstuk*, *Scène*, *Sectie* enzovoort. Stel ze per document in de inspector in, of laat {{APP}} beslissen op basis van de positie (**Projectinstellingen > Sectiesoorten**, "Standaardsoorten naar structuur"), bijvoorbeeld: mappen op het eerste niveau zijn hoofdstukken, tekstdocumenten erin zijn scènes.

### Documentsjablonen

Zet sjabloondocumenten (bijvoorbeeld een personageblad) in een map en kies die map dan onder **Projectinstellingen > Speciale mappen**. De sjablonen verschijnen onder **Project > Nieuw uit sjabloon**. Onder **Documenten > Standaardsjabloon voor subdocumenten** kun je een map een sjabloon laten gebruiken voor elk nieuw document dat je erin maakt.

### Projectsjablonen

**Bestand > Opslaan als sjabloon…** slaat het huidige project (structuur, instellingen en eventuele voorbeelddocumenten) op als sjabloon voor nieuwe projecten.

---

## 9. Zoeken

### Snel zoeken

Typ een paar letters in het veld in de werkbalk. Een lijst toont de documenten waarvan de **titel**, **samenvatting** of **tekst** ze bevat, tot tien van elke soort onder een eigen kop. Kies er een om hem te openen: de plekken in de tekst die bevatten wat je typte, zijn **gemarkeerd**, en de eerste is geselecteerd en in beeld gescrolld. De markeringen verdwijnen wanneer je een ander document in de binder kiest. Dit is de snelste weg naar een document waarvan je de naam kent.

### Projectzoekopdracht

**Ctrl+Shift+F**, of het vergrootglas in de werkbalk, toont een zoekveld boven de binder. Typ, en de binder wordt vervangen door de paarse lijst met resultaten.

- Klik op een resultaat om het te openen. **De treffers worden in de tekst gemarkeerd**, de eerste is geselecteerd en in beeld gescrolld, en **F3** / **Shift+F3** springen van de ene naar de volgende. **Ctrl+F** opent de zoekbalk al gevuld met je zoekterm.
- De **X** in de kop van de resultaten keert terug naar de binder; de editor behoudt het document dat hij toont. Klik nogmaals op het vergrootglas in de werkbalk om het zoekveld te verbergen.
- Met de verzamelingen getoond voert het tabblad **Zoekresultaten** je laatste zoekopdracht opnieuw uit. De laatste zoekopdracht wordt met het project bewaard.

Het vergrootglas links van het zoekveld opent de opties:

| Groep | Items |
|-------|---------|
| **Zoeken in** | **Alles**, of een van: Titel, Tekst, Samenvatting, Notities, Trefwoorden, Label, Status, Aangepaste metagegevens. Een klik kiest die ene plek. Houd **Alt** ingedrukt terwijl je klikt (of terwijl je het menu opent) om meerdere plekken te kiezen. |
| **Operator** | **Alle woorden** (elk woord moet voorkomen, in willekeurige volgorde), **Elk woord**, **Exacte frase**, **Reguliere expressie** |
| **Opties** | Hele woorden, Hoofdlettergevoelig, Resultaten omkeren (de documenten die *niet* overeenkomen), Alleen in Concept zoeken, Alleen in selectie van de Binder zoeken (de in de binder geselecteerde documenten en alles erin), In "opgenomen" documenten zoeken / In "uitgesloten" documenten zoeken (naar **Opnemen bij samenstellen**), Prullenbak meenemen |

Voorbeeld: om alles te zien wat nog de status "Te doen" heeft, zoek je naar `Te doen` en kies je **Status** onder Zoeken in.

**Zoekopdracht als verzameling opslaan…** onderaan het menu bewaart de zoekopdracht, met zijn opties, als een verzameling die elke keer dat je hem opent opnieuw wordt uitgevoerd.

### Project vervangen

**Bewerken > Zoeken > Project vervangen** vervangt een tekst in één keer in het hele project.

1. Voer de tekst in die je wilt **vervangen** en de tekst die in de plaats komt (**Door**). Laat "Door" leeg om de tekst te verwijderen.
2. Kies **Hoofdlettergevoelig**, **Alleen hele woorden** of **Reguliere expressie** (dan mag "Door" `\1` tot `\9` gebruiken voor de groepen van de expressie).
3. Vink aan waar vervangen moet worden: **Titels**, **Tekst**, **Notities**, **Samenvattingen**, **Aangepaste metagegevens** (tekstvelden), **Opmerkingen en voetnoten** (de teksten die in de inspector worden getoond).
4. Kies **Het hele project** of **Alleen geselecteerde documenten** (de in de binder geselecteerde documenten, met wat erin zit). De Prullenbak blijft met rust tenzij je hem aanvinkt.
5. **Tellen** vertelt hoeveel voorkomens zouden veranderen en waar. **Alles vervangen** vraagt nog één keer en doet het dan.

Project vervangen kan niet als geheel ongedaan worden gemaakt. Laat **Van elke tekst een momentopname maken voordat hij wordt gewijzigd** aangevinkt: elke gewijzigde tekst heeft dan een momentopname met de naam "Vóór het vervangen …" om naar terug te gaan (sectie 11). Titels en samenvattingen hebben geen momentopnamen; maak voor een grote wijziging eerst een back-up met **Bestand > Back-up > Nu back-up maken**. Een tekst die in een editor open is, kan daar ook met **Bewerken > Ongedaan maken** worden teruggenomen.

De opmaak blijft zoals ze is: de nieuwe tekst neemt de opmaak over van de tekst die hij vervangt. Een tekst die niet in een editor open is, wordt in zijn bestand gewijzigd, dus **Bewerken > Ongedaan maken** neemt die wijziging niet terug (de eerst gemaakte momentopname wel); hetzelfde geldt voor Aanhalingstekens omzetten, opmerkingskaarten en Vergelijken en overbrengen.

### Zoeken op opmaak

**Bewerken > Zoeken > Zoeken op opmaak** (**Ctrl+F3**) opent een klein venster dat open blijft terwijl je werkt. Kies waarnaar je zoekt:

| Zoeken | Vindt | Extra keuze |
|------|-------|--------------|
| Gemarkeerde tekst | tekst die met de markeerstift is gemarkeerd | beperken tot één kleur |
| Opmerkingen en voetnoten | tekst waaraan een opmerking of voetnoot hangt (inspector) | beperken tot één opmerkingskleur |
| Inline-aantekeningen, inline-voetnoten | notities die in de tekst zijn getypt | aantekeningkleur |
| Revisiekleur | tekst die in revisiemodus is getypt | welke revisie |
| Gekleurde tekst | tekst waarvan je de kleur met de hand instelde | beperken tot één kleur |
| Koppelingen | weblinks en koppelingen naar documenten | |
| Tekenopmaak | vette, cursieve, onderstreepte, doorgehaalde tekst (met de hand ingesteld, niet door een stijl) | welke ervan |
| Stijl | tekst met een alinea- of tekenstijl | welke stijl |
| Afbeeldingen | afbeeldingen in de tekst | |

**Met tekst** beperkt de zoekopdracht tot plekken die een woord bevatten (bij opmerkingen en voetnoten telt ook de eigen tekst van de notitie mee). **Volgende** en **Vorige** selecteren de volgende plek. Met **Zoeken in: Alle documenten** gaat de zoekopdracht verder in de volgende documenten van de binder en opent ze; met **Alleen de huidige editor** blijft ze waar je bent. **Ctrl+G** en **Ctrl+Shift+G** herhalen de zoekopdracht zonder het venster. **Sluiten** (of **Esc**) sluit het venster.

---

## 10. Doelen, statistieken en schrijfhulpmiddelen

- **Project > Projectdoelen** (**Ctrl+Shift+T**): stel een doel in voor het aantal woorden of tekens voor het Manuscript en voor elke schrijfsessie, en eventueel een deadline en je schrijfdagen. {{APP}} kan dan uitrekenen hoeveel je per sessie moet schrijven. In de voettekst zie je je voortgang naast het aantal woorden (sinds 0.94.0): een kleine balk en bijvoorbeeld "83 / 1.000 vandaag" voor de sessie en "12.400 / 80.000 manuscript" voor het Manuscript, elk alleen zolang het bijbehorende doel is ingesteld; een klik erop opent dit venster. De twee dunne balken onder het snelzoekvak (blauw: Manuscript; oranje: sessie) blijven bestaan, en via **Bestand > Opties > Uiterlijk > Algemene interface** kun je ze uitzetten. Woorden die in één keer in het project komen, tellen niet mee als geschreven in de sessie (0.94.0): geïmporteerde tekst (**Bestand > Importeren**, **Importeren en splitsen**, neergezette bestanden, webpagina's), een geplakte tekst van meer dan 200 woorden en de woorden die een project al bevat wanneer je het voor het eerst opent. **Sessie nu opnieuw instellen** begint de telling weer bij 0 en de tooltip van het snelzoekvak toont dat meteen. Het venster met de doelen houdt ruimte vrij voor zijn balken, zodat de velden niet verschuiven wanneer je een doel instelt.
- **Documentdoelen**: klik op het doel in de voettekst van de editor.
- **Project > Projectstatistieken**: woorden en tekens in het Manuscript, in de samengestelde tekst en in het hele project, plus een schatting van het aantal pagina's. In een groot project duurt het tellen even; de bezig-aanwijzer laat dat zien.
- **Project > Schrijfgeschiedenis**: hoeveel woorden je op elke dag hebt geschreven.

De onderstaande schrijfhulpmiddelen zijn add-ons (paragraaf 17): **Naamgenerator**, **Kladblok** en **Tekststatistieken** (ze kwamen met update 0.11 als één pakket) en **Spraaktranscriptie** (0.61); elk daarvan kun je uitzetten of verwijderen via **Help > Add-ons en updates > Add-onbeheer**. Zonder deze add-ons staat in **Bewerken > Schrijfhulpmiddelen** een melding daarover; het Add-onbeheer vind je in het menu Help.

### Tekststatistieken

**Project > Tekststatistieken…** telt een deel van je project. Kies bovenaan welke tekst:

- **Geselecteerde tekst** (gekozen wanneer je het venster opent met geselecteerde tekst),
- **Tekst in de editor** (in de Gecombineerde tekst: alle getoonde documenten),
- **In de Binder geselecteerde documenten**, met de documenten die daarin zitten,
- **Manuscript (opgenomen bij samenstellen)**, of het **Hele project** (zonder de Prullenbak).

Het venster heeft vier tabbladen:

- **Kerncijfers**: woorden (precies zo geteld als in de voetbalk en bij de doelen), tekens met en zonder spaties, alinea's, zinnen, verschillende woorden, pagina's van 250 woorden, leestijd bij 230 woorden per minuut en gemiddelden (letters per woord, woorden per zin en per alinea, de langste zin; wijs hem aan om hem te lezen).
- **Leesbaarheid (schattingen)**, ook op het eerste tabblad. Voor het Engels: de *Flesch Reading Ease* (0 tot 100, hoger is makkelijker; 60 tot 70 is gewoon Engels) en de *Flesch-Kincaid Grade Level* (het Amerikaanse schooljaar dat nodig is om de tekst te volgen). Voor het Duits: de *Flesch-index van Amstad* (dezelfde schaal) en de *Wiener Sachtextformel* (ongeveer het schooljaar, 4 makkelijk tot 15 zeer moeilijk). Voor elke taal: *LIX* (woorden per zin plus het aandeel woorden van meer dan zes letters; onder 30 zeer makkelijk, boven 60 zeer moeilijk). De taal wordt vanzelf herkend of bovenaan door jou gekozen. Deze waarden zijn richtlijnen, geen oordelen: lettergrepen en zinnen worden met vuistregels gevonden en de formules zijn gemaakt voor zakelijke tekst, niet voor dialogen of poëzie.
- **Woordfrequentie**: elk woord met zijn aantal en aandeel, de meest voorkomende eerst. **Veelvoorkomende woorden verbergen** laat lidwoorden, voornaamwoorden, voorzetsels en dergelijke weg (Engels en Duits). Stel een minimale lengte in, zoek een woord of tel hoofdletters apart. Dubbelklik op een woord om het hele project erop te doorzoeken.
- **Herhaalde zinsdelen**: groepen van twee tot zes woorden die je meer dan eens binnen zinnen hebt gebruikt, zoals een beschrijving die je twee keer hebt gegeven. Zinsdelen die alleen uit veelvoorkomende woorden bestaan, blijven weg. Dubbelklik om het project te doorzoeken.
- **Documenten**: een tabel met de documenten, met woorden, tekens, zinnen, woorden per zin en hun aandeel in het geheel. Klik op een kolomkop om te sorteren; dubbelklik op een document om het te openen.

**Tabel exporteren…** slaat de tabel van het huidige tabblad op als CSV-bestand voor een spreadsheet.

### Naamgenerator

**Bewerken > Schrijfhulpmiddelen > Naamgenerator…** bedenkt namen voor personages.

1. Vink de lijsten aan voor de **voornamen** (62 herkomsten uit Europa, Amerika, Azië en Afrika, bijvoorbeeld Engels, Amerikaans, Iers en Iers-Gaelisch, Duits, Oostenrijks, Zwitsers, Nederlands, Frans, Frans-Canadees, Italiaans, Spaans, Mexicaans, Portugees, Braziliaans, Deens, Zweeds, Noors, Fins, IJslands, Pools, Tsjechisch, Hongaars, Russisch, Grieks, Turks, Hebreeuws, Arabisch, Perzisch, Indiaas, Japans, Chinees, Koreaans, Yoruba, Swahili en Zoeloe; er zijn nog geen Schotse of Welshe lijsten). De **achternamen** komen uit dezelfde lijst als de voornaam, of haal het vinkje bij **De lijst van de voornaam** weg en kies andere lijsten (een Italiaanse voornaam met een Ierse achternaam).
2. Kies **Geslacht**, **Aantal namen** en wat je wilt **Tonen** (volledige naam, alleen voornaam, alleen achternaam, initialen en achternaam). **Met een tweede voornaam** voegt een tweede voornaam toe.
3. **Fijnafstelling**: **gewoon ↔ zeldzaam** kiest uit de meest voorkomende namen (links) of uit de zeldzame (rechts); dit werkt met de lijsten die zijn gemarkeerd als *op frequentie* (bijvoorbeeld Amerikaans, Zweeds, Hongaars, Hebreeuws), waarin staat hoe vaak elke naam voorkomt. **Alliteratie** geeft dat deel van de namen een achternaam met de beginletter van de voornaam (Peter Parker); **Dubbele achternamen** verbindt twee achternamen met een koppelteken. **Voornaam begint met** / **Achternaam begint met** beperken de namen tot die met deze beginletters.
4. Klik op **Namen genereren**. Selecteer namen en klik op **Kopiëren** (alle namen als er geen is geselecteerd) of **In tekst invoegen** (ook met een dubbelklik) om ze bij de cursor in de editor te zetten.

Vrouwen krijgen de vrouwelijke vorm van een achternaam wanneer een taal die heeft (Pools *Kowalska*, Tsjechisch *Nováková*, Russisch *Иванова*). Japanse, Chinese, Koreaanse, Hongaarse en Vietnamese namen tonen eerst de familienaam. Sommige lijsten staan in hun eigen schrift (Russisch, Grieks, Hebreeuws, Arabisch en andere); de naam van de lijst geeft dat aan.

**Eigen lijst toevoegen…** maakt van een tekstbestand met één naam per regel een lijst van jezelf (voornamen, vrouwelijke of mannelijke voornamen, of achternamen). Kies de lijst en klik op **Eigen lijst verwijderen** om hem te wissen. De namenlijsten komen uit het opensourceproject *Faker* (MIT-licentie); het zijn lijsten van namen die in gebruik zijn, geen gecontroleerde historische namenlijsten.

### Kladblok

Het kladblok bevat notities buiten alle projecten om: ideeën terwijl een ander project open is, citaten die je van een webpagina hebt gekopieerd, een afbeelding van een deel van het scherm. Open het met **Venster > Kladblok** (**Alt+Shift+Return**).

- **+** begint een notitie (met de pijl ernaast kies je opgemaakte of platte tekst). De titel boven de tekst is de naam van de notitie. **−** verwijdert de geselecteerde notitie.
- Opgemaakte notities kennen **vet**, *cursief* en onderstrepen (knoppen of **Ctrl+B / I / U**) en afbeeldingen: plak een afbeelding, of klik op de **camera** om een afbeelding van een deel van het scherm te maken. De camera is er alleen voor opgemaakte notities.
- Alles wordt tijdens het typen vanzelf opgeslagen. Als een notitie niet kan worden opgeslagen (een volle schijf, een notitiemap die weg is), meldt de statusregel dat en blijft de notitie met haar tekst open: een andere notitie, een nieuwe notitie of een andere notitiemap gaat pas open als de notitie is opgeslagen, en een venster biedt **Tekst kopiëren** en **Opnieuw proberen** aan. Als het programma eindigt terwijl een notitie niet kan worden opgeslagen, gaat de tekst naar de noodmap *~/.local/share/{{APP}}/Emergency* (0.65).
- **Naar project sturen** toont elk open project, ook die in andere {{APP}}-vensters. Voor elk project: **Nieuw document in de map Research**, **Nieuw document in** een map naar keuze, of **Toevoegen aan het einde van** een tekstdocument. Opmaak en afbeeldingen gaan mee. De notitie blijft in het kladblok staan tot je haar verwijdert.

De notities zijn gewone bestanden in een map, normaal *Documents/{{APP}} Scratch Pad* (platte tekst als .txt, opgemaakte notities als .html met hun afbeeldingen in de verborgen submap *.images*). Kies een andere map, het soort nieuwe notities en of het kladblok boven andere vensters blijft staan via **Bestand > Opties > Algemeen > Kladblok**. Notities die elders worden gewijzigd (een ander {{APP}}-venster, een gesynchroniseerde map) verschijnen meteen.

**Vanuit elk programma.** De sneltoets in het programma werkt zolang {{APP}} het toetsenbord heeft. Om het kladblok vanuit elk programma te openen, laat je het bureaublad {{APP}} starten met `--scratch-pad`: in KDE via **Systeeminstellingen > Toetsenbord > Sneltoetsen > Nieuw toevoegen > Opdracht of script**, voer de opdracht in die onder **Bestand > Opties > Toetsenbord** staat (knop **Opdracht kopiëren**) en druk op de toetsen die je wilt. Het kladblok opent dan vanzelf, zonder projectvenster, en stuurt notities naar de projecten die open zijn. Als {{APP}} aan het toepassingsmenu is toegevoegd (**Help > Aan toepassingsmenu toevoegen**), heeft het menu-item ook de actie **Kladblok**.

**De camera** maakt de afbeelding met KDE's schermafbeeldingsprogramma *Spectacle* wanneer dat is geïnstalleerd (nodig onder Wayland, waar programma's het scherm niet zelf mogen lezen): sleep over het deel dat je wilt. Zonder Spectacle toont {{APP}} op X11 het scherm en sleep je over het deel; met **Esc** annuleer je, met **Enter** neem je het hele scherm.

### Spraaktranscriptie

Praat in plaats van te typen: de add-on **Spraaktranscriptie** (versie 0.61 of nieuwer) schrijft op wat je zegt. Alles gebeurt op deze computer; er wordt niets verstuurd terwijl je praat.

**Voor de eerste opname: kies een model.** Spraakherkenning heeft een model nodig, dat één keer wordt gedownload. Open **Bestand > Opties > Spraak** (of de pijl naast de microfoonknop, **Opties voor spraaktranscriptie…**), kies een model en druk op **Downloaden en gebruiken**:

- **Small**: een download van 190 MB, snel; goed voor duidelijke spraak, vertalen alleen globaal.
- **Medium** (aanbevolen): 539 MB; houdt op de meeste processors van de laatste jaren het spreektempo bij; goede herkenning en vertaling.
- **Large v3**: ongeveer 1,1 GB; het beste, maar op een processor vaak trager dan spraak, zodat de tekst bij lange stukken achterblijft. Een videokaart helpt.
- **Large v3 Turbo**: 574 MB; dicht bij Large v3 en sneller, maar kan niet vertalen.

De modellen komen van de modelbestanden van whisper.cpp op Hugging Face. Onder **Bron** kun je in plaats daarvan een ander adres opgeven, of **Een gedownload bestand gebruiken…** voor een modelbestand dat je zelf hebt opgehaald. Voordat een model wordt gebruikt, wordt het gecontroleerd: eerst de controlesom (SHA-256), dan de beschrijving van het model in het bestand en een korte proeftranscriptie. Alleen als alles in orde is, vervangt het het model dat in gebruik is, dat dan wordt verwijderd. Een model uit de officiële bron moet de bekende controlesom hebben; anders wordt het niet geïnstalleerd (de download kan beschadigd zijn). Een bestand van een ander adres, of een bestand dat je zelf hebt gedownload, waarvan de controlesom niet de bekende is, wordt pas gelezen nadat je akkoord gaat, omdat de spraakengine het moet lezen. Andere adressen moeten beginnen met *https://*. Een onderbroken download gaat de volgende keer verder waar hij gebleven was. **Snelheidstest** toont hoe lang deze computer over een korte opname doet met het gekozen model en apparaat.

**Opnemen.** Druk op de microfoonknop in de werkbalk Hulpmiddelen (of op **Bewerken > Schrijfhulpmiddelen > Spraaktranscriptie**). De Inspector opent op zijn tabblad **Transcripties** (de microfoon, de eerste in de rij knoppen van de Inspector) en er begint een nieuwe transcriptie, genoemd naar datum en tijd. Praat; een korte pauze beëindigt een stuk en de tekst verschijnt enkele seconden later. Een langere pauze (3 s tenzij je dat wijzigt) begint een nieuwe alinea. Druk nogmaals op de knop om te stoppen: de stukken die nog wachten worden geschreven, daarna kan de tekst worden bewerkt. Met de pijl naast de knop kies je de microfoon.

**De transcripties** zijn platte tekstbestanden in een eigen map, buiten alle projecten (normaal *Documents/{{APP}} Transcriptions*), zodat je projecten ongemoeid blijven. In het tabblad kun je een transcriptie bewerken (wordt vanzelf opgeslagen), hernoemen, de hele tekst kopiëren om hem elders te plakken, hem verwijderen (hij gaat naar de prullenbak van het bureaublad) of de map openen. Zolang een opname wordt uitgeschreven, is de tekst alleen-lezen. Als een bewerkte transcriptie niet kan worden opgeslagen, blijft hij open (de statusregel zegt waarom); als er intussen een opname begint of het programma eindigt, gaat de tekst naar de noodmap *~/.local/share/{{APP}}/Emergency* (0.65).

**Nu opnemen, de tekst later schrijven** (0.64). Het schrijven van de tekst kost veel rekenkracht; op een laptop op batterij (of een telefoon) kun je alleen opnemen en de tekst later laten schrijven. Kies **Alleen opnemen** onder **Bestand > Opties > Spraak** (*Wanneer de microfoon start*; op Android is dat de standaard). De microfoonknop neemt dan alleen op: er draait verder niets en er wordt geen model geladen. De opname wordt bewaard als geluidsbestand (WAV, ongeveer 1,9 MB per minuut) naast de transcripties, met dezelfde soort naam, en verschijnt in de lijst van het tabblad met een knop **Transcriberen** in zijn regel; **Alles transcriberen** schrijft de tekst van elke opname die er nog geen heeft, een voor een. De tekst verschijnt dan zoals bij een live opname en kan worden bewerkt. Een opname blijft bewaard nadat haar tekst is geschreven, tenzij je **Een opname verwijderen zodra haar tekst is geschreven** aanvinkt; een transcriptie hernoemen of verwijderen neemt haar opname mee. Om de tekst op een andere computer te schrijven, kies je een transcriptiemap die je synchronisatieprogramma op beide computers gelijk houdt. Als een opname wordt gestopt terwijl haar tekst wordt geschreven (of het programma eindigt), houdt ze haar knop **Transcriberen** en kan ze opnieuw worden uitgevoerd.

**Talen.** De **hoofdtaal** is de taal van de tekst; de **terugvaltaal** is een andere taal die je ook spreekt. Met Engels als hoofdtaal wordt wat je in de terugvaltaal zegt in het Engels geschreven (whisper vertaalt alleen naar het Engels). Woorden uit de terugvaltaal binnen Engelse zinnen kunnen verkeerd verstaan worden (in een test werd "Bahnhof" "Bon Huff"); zet de woorden die je gebruikt onder **Hintwoorden**, dan worden ze geschreven zoals je ze zegt ("Bahnhof", "Brötchen"). Bij een andere hoofdtaal, of met Large v3 Turbo, blijft spraak in de taal waarin ze is gesproken. **Het gezegde naast een vertaling bewaren** voegt het origineel tussen haakjes toe na een vertaald stuk (dit duurt langer). **Hintwoorden**: namen en woorden die je gebruikt, zodat ze op jouw manier worden gespeld.

**Andere opties.** **Rekenen op**: de processor, of een videokaart die whisper.cpp via Vulkan kan gebruiken (getoond wanneer er een is gevonden; kaarten met te weinig geheugen voor het model staan in de lijst maar kunnen niet worden gekozen). Op Windows gebruikt de add-on voorlopig alleen de processor: Small of Medium houden het daar het best bij. **Processorthreads**: een minder dan de processor heeft, houdt het typen soepel. **Nauwkeurigheid**: *Zorgvuldig* weegt meerdere mogelijke teksten af en bepaalt eerst de taal van elk stuk: beter, maar langzamer. **Microfoon**, **Gevoeligheid** (laag voor een rumoerige ruimte, hoog voor een zachte stem), **Een pauze die een stuk beëindigt**, **Nieuwe alinea na een pauze van** (*nooit* schakelt het uit), de **map**, en onder **Geavanceerd**: tekst weglaten bij stilte en ruis (whisper verzint daar soms een regel zoals "Thank you for watching") en de voorgaande tekst als context voor het volgende stuk gebruiken.

---

### Vertalen en proeflezen

Sinds 0.92.0 is de add-on **Vertalen en proeflezen** (versie 1.4) een van de eigen add-ons van {{APP}} en zit hij in de volledige versie; voor een {{APP}} dat je al hebt, is het een eigen pakket op de pagina Releases (paragraaf 17). Hij controleert Engelse tekst op spelling en grammatica en vertaalt Duitse tekst naar het Engels en Engelse tekst naar het Duits, op je eigen computer: een tekst verlaat hem nooit. {{APP}} zelf heeft geen AI-functies; deze add-on werkt met taalmodellen en kan, net als elke andere, in het Add-onbeheer worden uitgezet of verwijderd.

- Hij heeft een eigen tabblad in de Inspector en een eigen venster (de opdracht staat in het submenu Schrijfhulpmiddelen van het menu Bewerken en er is een werkbalkknop). De tekst gaat erin, het resultaat komt eruit met elke wijziging gemarkeerd, live terwijl je typt; met een klik op een woord behoud je het origineel of krijg je andere woorden aangeboden.
- Hij heeft een model nodig, dat je één keer kiest en downloadt in de opties van de add-on (enkele honderden megabytes tot enkele gigabytes; een model dat te groot is voor de computer wordt niet aangeboden). De modellen worden opgehaald van de releases *models-1*, *models-2* en *models-3* van de repository van {{APP}}, elk wordt alleen geïnstalleerd als zijn controlesom de bekende is, en daarna werkt de add-on zonder verbinding. Eén model, CoEdIT Large, mag alleen voor niet-commerciële doeleinden worden gebruikt; de add-on zegt dat voordat het wordt gedownload.
- **Een woord alleen** (1.3). Eén Engels woord: het resultaat is het woord zoals het moet zijn, en daaronder wat het betekent (de betekenissen komen uit het woordenboek WordNet 3.0, dat in de add-on is ingebouwd; een woord dat het niet heeft, krijgt een betekenis die het Qwen3-model schrijft dat je hebt gedownload, met een waarschuwing dat die onnauwkeurig kan zijn, en zonder zo'n model zegt de add-on wat je kunt downloaden), zijn spellingsuggesties en woorden met een vergelijkbare betekenis. Eén Duits woord: de add-on zoekt het eerst op in het **Duitse woordenboek**, een uittreksel van Wiktionary dat je apart downloadt in de opties van de add-on (de licentie, CC BY-SA 4.0, wordt getoond voor de download): de grondvorm, de woordsoorten, de betekenissen en de Engelse woorden bij elke betekenis. De eerste vijf betekenissen van een woordsoort worden getoond, de rest staat achter "meer"; een klik op een Engels woord maakt het tot het resultaat, en "Vergelijkbaar" volgt dan dat woord en zijn woordsoort. Voor wat het woordenboek niet zegt, en voor een woord dat het niet heeft, vertaalt het kleine model het woord, en de uitvoer waarschuwt dat dit onnauwkeurig kan zijn. Van Engels naar Duits werkt het op dezelfde manier voor één Engels woord, per woordsoort die WordNet kent.
- **Omwisselen en Terugvertalen** (1.3). **Omwisselen** maakt van Duits naar Engels Engels naar Duits en omgekeerd: het resultaat gaat naar de tekst en wordt de andere kant op vertaald. **Terugvertalen** vertaalt het resultaat terug naar de taal van je tekst en toont dat onder het resultaat, tot je de tekst wijzigt; het heeft het model voor de terugweg nodig (OPUS-MT Engels naar Duits (klein) voor een Duitse tekst) en biedt aan het te downloaden als het ontbreekt. Terwijl je vertaalt, toont de tweede rij bedieningselementen het vertaalmodel (een ander kiezen wijzigt de instelling en de vertaling) en is de lijst met stijlen verborgen.
- **Korte teksten** (1.3). Een enkel woord en een tekst van maximaal drie woorden gaan nooit naar het grote vertaalmodel (dat is beter voor hele zinnen en onbetrouwbaar voor losse woorden): eerst het woordenboek, anders het kleine model, ook als het grote is gekozen. Voor een korte tekst van meerdere woorden toont "Andere vertalingen:" de beste volledige vertalingen van het model, zonder namen die het heeft verzonnen.
- **Versie 1.4.** Een enkel woord in beide richtingen toont nu onder het resultaat de vertalingen van elke woordsoort die het heeft (een Duits woord dat een werkwoord is in kleine letters en een zelfstandig naamwoord met hoofdletter, krijgt beide; een woord dat het woordenboek niet heeft, wordt in beide vormen vertaald), daarna "Vergelijkbare vertalingen" (woorden met een vergelijkbare betekenis als het Engelse resultaat) of, van Engels naar Duits, "Vergelijkbare Duitse woorden": de Duitse woorden waarvan de betekenissen het Engelse woord of een van zijn synoniemen noemen (dit heeft het Duitse woordenboek nodig; zonder dat zegt één regel het), daarna de korte beschrijving. Met "Automatisch" wordt een enkel Duits woord dat geen Engels woord is, als Duits herkend wanneer het Duitse woordenboek is geïnstalleerd. Voordat Duits wordt vertaald, worden de hoofdletters van Duitse zelfstandige naamwoorden rechtgezet in de kopie die naar het model gaat (je tekst blijft zoals hij is): een woord dat het woordenboek alleen als zelfstandig naamwoord kent, krijgt zijn hoofdletter, een woord dat op beide manieren bekend is ("essen") houdt de vorm die je typte, behalve direct na een lidwoord of een voorzetsel met lidwoord ("das Essen", "im Essen"); zonder het woordenboek zegt een opmerking onder de vertaling dat Duitse zelfstandige naamwoorden hoofdletters nodig hebben. Alternatieven die alleen in hoofdletters van elkaar of van de vertaling verschillen, worden één keer getoond. Het woordgebied toont geen regel "Spelling" meer voor woorden van het resultaat (woorden waarop je in je eigen tekst klikt, houden hem), en de achtervoegselregels van de vergelijkbare woorden lezen "number" niet meer als "numb". Waar iets ontbreekt (een vertaalmodel, het Duitse woordenboek), noemt een melding bovenaan het tabblad en venster van de add-on het en heeft een knop die de opties van de add-on opent. Een model dat 15 minuten niet is gebruikt, wordt gelost om geheugen vrij te maken (**Bestand > Opties > Proeflezen > Snelheid > Modellen lossen na**; "nooit" houdt het geladen). De hints in de lege velden worden bij een smalle breedte afgebroken.
- Modellen die je met versie 1.1 hebt gedownload, blijven zoals ze zijn. Versie 1.1 haalde modellen op van de voormalige repository van de add-on; 1.2 kent de nieuwe plek.
- De eigen beschrijving van de add-on, met zijn regels voor het proeflezen (Amerikaanse spelling, gekrulde aanhalingstekens, geen em-streepjes) en waar elk model goed in is, is *addons/translation-proofreading/source/README.md* in de broncode van {{APP}}.

## 11. Momentopnamen

Een momentopname is een bewaarde kopie van een document op een bepaald moment. Maak er een voordat je iets ingrijpend herschrijft:

- Een document met minstens één momentopname toont rechtsboven in zijn pictogram in de Binder een omgevouwen hoekje (licht en grijs).
- Met **Ctrl+5** maak je een momentopname; **Ctrl+Shift+5** vraagt om een titel.
- Met **Bestand > Opties > Algemeen > Opslaan** maakt **Bestand > Opslaan** (**Ctrl+S**) een momentopname van elke tekst die je sinds de vorige keer Bestand > Opslaan hebt gewijzigd (met de titel "Saved"); **Bestand > Opties > Gedrag > Momentopnamen** toont een korte melding in de statusbalk wanneer momentopnamen worden gemaakt.
- In het tabblad Momentopnamen van de Inspector kun je een momentopname met de huidige tekst **vergelijken** (toevoegingen worden in kleur getoond, verwijderingen doorgehaald; kies of je woorden, zinnen, alinea's of tekens vergelijkt) en ernaar **terugzetten**. Voordat je terugzet, kan {{APP}} een momentopname van de huidige tekst maken, zodat er niets verloren gaat. De momentopname wordt eerst gelezen (0.65): een die niet helemaal kan worden gelezen, laat de huidige tekst zoals hij is, een die geen tekst bevat vervangt hem pas na een tweede vraag, en wanneer de momentopname van de huidige tekst is gevraagd maar niet kan worden gemaakt, wordt er niets teruggezet. Terugzetten is ook de manier om een document te herstellen waarvan het bestand beschadigd is of een andere tekencodering heeft (het wordt alleen-lezen getoond): het beschadigde bestand wordt eerst als kopie bewaard in *settings/conflicts* en de Inspector zegt waar (0.81.1).
- **Project > Oude momentopnamen verwijderen…** (sinds 0.61.1, onder Prullenbak legen) verwijdert elke momentopname van het project, ook die van documenten in de Prullenbak, om het project weer kleiner te maken. De teksten blijven zoals ze nu zijn; alleen hun eerdere versies verdwijnen. Een vraag vooraf noemt hoeveel momentopnamen er zijn, van hoeveel documenten en hoeveel ruimte ze innemen; dit kan niet ongedaan worden gemaakt, dus maak eerst een back-up (**Bestand > Back-up**). De opdracht is uit zolang het project geen momentopnamen heeft. In een project in Scrivener-indeling verdwijnen de momentopnamen ook uit de *.scriv* (met het opslaan dat meteen volgt); sinds 0.61.1 geldt dit ook voor een enkele momentopname die je in de Inspector verwijdert.

---

## 12. Schrijfmodus

Met **F11** vult alleen je tekst op een vel papier het scherm. Beweeg de muis naar de onderrand om de bedieningsbalk te tonen: zoom, papierbreedte, achtergronduitfading, typemachinescrollen, aantal woorden en een kleine Inspector met samenvatting en notities. **Alt+Shift+Up / Down** gaat naar het vorige of volgende document, met **Ctrl+[** en **Ctrl+]** ga je terug en vooruit door de documenten die je hebt bekeken. De sneltoetsen van de menu's Opmaak en Invoegen (vet, stijlen, opmerkingen ...) werken hier ook. Met **Esc** of **F11** keer je terug naar het hoofdvenster.

**Bestand > Opties > Gedrag > Schrijfmodus** kiest het scherm waarop de schrijfmodus opent (het huidige, het primaire of een met naam), of het hoofdvenster opzijgaat (geminimaliseerd terwijl de schrijfmodus open is) en of **Esc** hem sluit (**F11** doet dat altijd).

De bedieningsbalk is in elk thema donker, zodat de knoppen, de zoomlijst en hun teksten ook in lichte thema's leesbaar blijven (0.94.0). De kleuren van tekst, papier en achtergrond komen uit het thema. Om ze te wijzigen, bijvoorbeeld in zwart op wit of groen op zwart, ga je naar **Bestand > Opties > Uiterlijk > Schrijfmodus**; de wijziging is meteen zichtbaar, ook terwijl de schrijfmodus open is. Een achtergrondafbeelding kan per project worden ingesteld onder **Projectinstellingen > Speciale mappen**.

---

## 13. Samenstellen: het manuscript maken

**Bestand > Samenstellen…** (**Ctrl+Shift+E**) voegt alles in het Manuscript samen tot één bestand.

### Het venster Samenstellen

Het venster Samenstellen heeft bovenaan **Samenstellen voor**, links de indelingen, in het midden de sectielay-outs en rechts de inhoud, metagegevens, opties en vervangingen (vier tabbladen met pictogrammen; houd de muis boven een tabblad voor zijn naam). De kolom Indelingen is minstens zo breed als haar langste naam; sleep de lijnen tussen de drie kolommen om er een breder te maken, en het venster onthoudt de breedtes voor de volgende keer.

1. **Samenstellen voor**: de uitvoerindeling. Ingebouwd, zonder ander programma: **Word-document (.docx)**, **PDF (.pdf)**, **E-book (.epub)**, en voor scripts **Fountain** en **Final Draft**. PDF en OpenDocument via LibreOffice, EPUB via Pandoc en Markdown verschijnen ook als de bijbehorende plug-in en het programma zijn geïnstalleerd (zie hieronder). Ernaast: hoeveel documenten en ongeveer hoeveel woorden worden samengesteld.
2. **Indelingen** (links), onder **Mijn indelingen** en **Ingebouwde indelingen**: de samenstelindeling bepaalt hoe het manuscript eruitziet: lettertypen, regelafstand, paginaformaat, marges, kop- en voetteksten, hoe hoofdstukken en scènes eruitzien, hoe stijlen, notities en citaten eruit komen. De ingebouwde indelingen (*Manuscript (Times)*, *Manuscript (Courier)*, *Modern*, *Paperback (6 x 9 in)*, *E-book*, *Standaard (opmaak van de editor)* en *Overzicht (samenvattingen)*) kunnen niet worden gewijzigd. De knoppen onder de lijst: **+** (de geselecteerde indeling dupliceren om je eigen versie te maken en te bewerken), het potlood (je eigen indeling bewerken), **-** (verwijderen), **Importeren...** en **Exporteren...** (een *.qbformat*-bestand om op een andere computer te gebruiken of weg te geven; een geïmporteerde indeling waarvan de naam al bestaat, krijgt een nummer).
3. **Sectielay-outs** (midden): **Lettertype** stelt één lettertype in voor het hele manuscript, of laat het aan de lay-outs over ("Bepaald door sectielay-out"). De tabel toont elk sectietype van je project met de lay-out die het in deze indeling krijgt; kies een andere in de lijst. Onder de tabel toont een klein plaatje hoe een sectie met de geselecteerde lay-out begint. **Sectielay-outs toewijzen...** opent een groter venster met links de sectietypen en rechts plaatjes van alle lay-outs van de indeling. De ingebouwde indelingen kennen de gebruikelijke namen van sectietypen: in *Manuscript (Courier)* bijvoorbeeld krijgt *Part* **Part Title**, *Chapter* krijgt **Chapter Heading** en *Scene* of *Section* krijgt **Section Text**.
4. **Inhoud** (eerste tabblad): wat wordt samengesteld, met het sectietype van elk document en zijn lay-out. Kies een map om maar een deel van het Manuscript samen te stellen (bijvoorbeeld één hoofdstuk), voeg voorwerk (titelpagina) en achterwerk (dankwoord) toe. **De naam van de auteur** (0.94.0): de titelpagina van de projectsjablonen leest "door" en de naam van de auteur. De eerste keer dat je samenstelt met een titelpagina, wordt om die naam gevraagd (voor- en achternaam; hij wordt bewaard bij de Auteursgegevens in **Bestand > Opties > Algemeen**, dus er wordt maar één keer naar gevraagd); als je het leeg laat, vervalt de regel met "door" op de titelpagina, in elke indeling. Het veld Auteur op het tabblad Metagegevens van het venster Samenstellen wordt, als er een naam in staat, in plaats daarvan voor het samengestelde bestand gebruikt. Haal het vinkje bij een document weg om het weg te laten; dit schakelt zijn instelling **Opnemen bij samenstellen** uit. **Objectbladen toevoegen** zet na het achterwerk een blad voor elk object van het tabblad Objecten (paragraaf 7): een kop per categorie (Personages, Locaties ...), daarna elk object met zijn velden; de knop ernaast kiest de categorieën. Het staat in het begin uit.
5. **Metagegevens** (tweede tabblad): titel, ondertitel, auteur, taal (bijvoorbeeld *en-US*), onderwerp, trefwoorden, uitgever, datum, beschrijving, de omslag van een e-book (een afbeelding uit de Binder) en de boekidentificatie. Lege velden nemen de projecttitel, je naam en de datum van vandaag. De gegevens komen in de documenteigenschappen van de PDF, in het e-book en in het Word-bestand; plaatsvervangers zoals `<$projecttitle>` werken hier ook.
6. **Opties** (derde tabblad): of de tekst van mappen wordt samengesteld.
7. **Vervangingen** (vierde tabblad): vervangingen van dit project, uitgevoerd na die van de indeling, bijvoorbeeld een naam die in je concepten op de ene manier is gespeld en in het boek op een andere. Elke rij kan aan en uit worden gezet en kan een reguliere expressie zijn, hoofdlettergevoelig, of alleen hele woorden.
8. **Voorbeeld** toont de pagina's van het manuscript als PDF in een eigen venster, met de indeling en inhoud die je hebt gekozen, voordat er iets wordt opgeslagen.

Het lettertype, de vervangingen en de objectbladen worden net als de andere keuzes met het project bewaard (oudere versies van {{APP}} negeren ze).

Klik op **Samenstellen…**, kies waar je het opslaat, en het voltooide bestand opent desgewenst in je tekstverwerker of lezer. **Instellingen opslaan** bewaart de keuzes zonder samen te stellen.

**Met de hand ingestelde lettertypen en alinea-opmaak.** Met de indeling *Standaard (opmaak van de editor)* behoudt het samengestelde bestand ze allemaal: lettertypen, groottes, kleuren, hoofdletters, letterspatiëring, inspringingen, afstanden, regelafstand en tabstops. Elke andere indeling heeft een eigen uiterlijk: die vervangt lettertype, grootte, inspringingen en afstanden, en behoudt tekstkleur, hoofdletters, letterspatiëring, uitlijning en tabstops.

### Een samenstelindeling bewerken

**Dupliceren en bewerken…** en **Bewerken…** openen de indelingseditor. De lijst links bevat zijn pagina's:

- **Tekst**: lettertype, grootte, regelafstand, inspringing van de eerste regel, ruimte na alinea's, uitgevulde tekst, het lettertype en de grootte van koppen, vette koppen. **De opmaak van de editor van het project gebruiken** behoudt in plaats daarvan het uiterlijk van de editor.
- **Sectielay-outs**: de lay-outs van de indeling (toevoegen, dupliceren, verwijderen) met kop, voor- en achtervoegsel (`Chapter <$W:chapter>`), kopstijl en uitlijning, hoofdletters, lege regels voor de kop, de tekst, de samenvatting en de notities, een nieuwe pagina, het scheidingsteken voor de sectie, geen inspringing bij de eerste alinea. Het voorbeeld toont hoe een sectie met deze lay-out begint.
- **Toewijzingen**: welke lay-out elk sectietype in deze indeling krijgt.
- **Stijlen**: hoe een stijl van je project in deze indeling eruit komt. **Een stijl toevoegen…**, kies dan: **Zijn uiterlijk behouden**, **Zijn uiterlijk wijzigen** (lettertype, grootte, vet, cursief, onderstrepen, kleine of alle hoofdletters, kleur, uitlijning, inspringingen, ruimte ervoor en erna, regelafstand, tekst ervoor en erna, hoofdletters, op een nieuwe pagina beginnen; een vakje dat half is aangevinkt, of "ongewijzigd", laat de stijl zoals hij is), **Er lopende tekst van maken** of **De tekst weglaten** (bijvoorbeeld notities aan jezelf in een eigen stijl).
- **Pagina**: papierformaat en marges, gespiegelde marges voor boeken (binnen en buiten), kop- en voetteksten (links, midden, rechts; plaatsvervangers zoals `<$p>` voor het paginanummer en `<$title>`), tegenover elkaar liggende pagina's (linkse teksten op linkerpagina's, rechtse teksten op rechterpagina's), kop- en voettekst op de eerste pagina en op pagina's waar een hoofdstuk begint, hoofdstukken op een rechterpagina, het eerste paginanummer en zijn stijl (1, 2, 3 / i, ii, iii / I, II, III), de grootte van kop- en voetteksten.
- **Voetnoten en opmerkingen**: voetnoten onderaan de pagina (in e-books: notities die opspringen), aan het einde van elk hoofdstuk of aan het einde van de tekst (PDF en e-books; een Word-bestand heeft zijn voetnoten altijd onderaan de pagina), met de titel van de notities en hun grootte; inline-aantekeningen als opmerkingen of verwijderd; gekoppelde opmerkingen als opmerkingen van het Word-bestand; markeringen verwijderd.
- **Vervangingen**: tekst die alleen in het samengestelde bestand wordt vervangen (je project blijft zoals het is): aan of uit, hoofdlettergevoeligheid, hele woorden, reguliere expressies (`\1` tot `\9` in "Door").
- **Transformaties**: rechte of typografische aanhalingstekens, gedachtestrepen als koppeltekens, cursief als onderstreept (de manuscriptconventie), meerdere spaties als één, geen spaties aan het einde van alinea's, platte tekst zonder vet, cursief, lettertypen of kleuren.
- **Inhoud, PDF en e-book**: een inhoudsopgave (zijn titel en hoeveel kopniveaus), PDF-bladwijzers voor de koppen, aanklikbare weblinks in de PDF, een bestand per sectie die in een e-book op een nieuwe pagina begint, en de inhoudsopgave ook als pagina van het e-book.

Je indelingen worden bewaard in *~/.local/share/{{APP}}/CompileFormats* en zijn in elk project beschikbaar.

### PDF

De ingebouwde PDF-uitvoer legt de pagina's zelf op: voetnoten onderaan de pagina waar ernaar wordt verwezen (een lange gaat door op de volgende pagina), kop- en voetteksten, hoofdstukken op rechterpagina's als de indeling dat zegt, koppen die bij de tekst erna blijven, geen losse regels boven- of onderaan een pagina, een inhoudsopgave met aanklikbare paginanummers, bladwijzers voor de koppen in het zijpaneel van de lezer en aanklikbare links. Lettertypen worden ingesloten. Afbeeldingen behouden de grootte die ze in de tekst hebben; een afbeelding die groter is dan het tekstgebied wordt verkleind zodat hij past. Afbeeldingen en tabellen naast de tekst hebben de tekst eromheen lopen, zoals in de editor. Een tabel loopt door over zoveel pagina's als nodig, rij voor rij, met zijn kopregels bovenaan elke pagina.

### E-books

De ingebouwde e-book-uitvoer schrijft EPUB 3 (ook met een inhoudsopgave voor oudere lezers): een bestand per hoofdstuk, de koppen in de inhoudsopgave van de lezer, voetnoten als notities die opspringen (of aan het einde van elk hoofdstuk of van het boek), afbeeldingen en de omslag uit **Metagegevens…**. Tabellen behouden hun ontwerp; een afbeelding met bijschrift is een figuur (naast de tekst als hij dat in de editor ook is), en zijn beschrijving is de alt-tekst van de afbeelding, die schermlezers voorlezen. Het boek krijgt de eerste keer dat het wordt samengesteld een identificatie; die blijft bij het project, zodat een lezer een nieuwe versie voor hetzelfde boek aanziet. Lezers kiezen hun eigen lettertypen en paginaformaat, dus pagina-instellingen, kop- en voetteksten zijn niet van toepassing.

### Word

Het Word-bestand gebruikt benoemde stijlen (Title, Subtitle, Heading 1 tot 3, Quote, Centered, Caption en je eigen alinea- en tekenstijlen), zodat de koppen verschijnen in het navigatiedeelvenster van Word. Met **Een inhoudsopgave opnemen** wordt aan het begin een inhoudsopgaveveld geplaatst, en het bestand vraagt de tekstverwerker om zijn velden bij te werken wanneer het wordt geopend, waardoor de paginanummers worden ingevuld; als een tekstverwerker dat niet doet, werk je het veld daar zelf bij. Tegenover elkaar liggende kopteksten, de eerste pagina en het eerste paginanummer komen uit de **Pagina**-instellingen van de indeling. Tabellen zijn Word-tabellen met hun lijnen, kleuren, samengevoegde cellen en kopregels (in Word herhaald op elke pagina). Een afbeelding naast de tekst komt, samen met zijn bijschrift, in een tabel van één cel zonder lijnen, zodat Word en LibreOffice de tekst om beide laten lopen; de beschrijving van een afbeelding is zijn alt-tekst.

### Koppen en plaatsvervangers

In een sectielay-out kan de kop plaatsvervangers bevatten die Samenstellen vervangt:

| Plaatsvervanger | Resultaat |
|-------------|--------|
| `<$n>` | 1, 2, 3 … |
| `<$w>` / `<$W>` / `<$WW>` | een, twee … / Een, Twee … / EEN, TWEE … |
| `<$R>` / `<$r>` | I, II, III … / i, ii, iii … |
| `<$L>` / `<$l>` | A, B, C … / a, b, c … |
| `<$n:name>` | een aparte teller met de naam *name* (werkt met elke bovenstaande nummerstijl) |
| `<$title>` | de documenttitel |
| `<$projecttitle>` / `<$shorttitle>` | de projecttitel / zijn eerste drie woorden in hoofdletters |
| `<$author>`, `<$forename>`, `<$surname>` | je naam uit **Bestand > Opties** of de projectinstellingen |
| `<$wc>` / `<$wc100>` / `<$wc1000>` | het aantal woorden, exact of afgerond |
| `<$date>` / `<$year>` | de datum van vandaag (jjjj-mm-dd) / het jaar |
| `<$p>` | het paginanummer (alleen kop- en voetteksten) |
| `<$pages>` | het aantal pagina's (kop- en voetteksten van PDF-bestanden en afdrukken) |

Voorbeeld: de kop `Chapter <$W:chapter>` wordt *Chapter One*, *Chapter Two*, enzovoort.

Plaatsvervangers kunnen ook in de tekst zelf worden getypt (**Invoegen > Automatisch nummeren** en **Invoegen > Plaatsvervangers**). **Help > Lijst van alle plaatsvervangers…** toont ze allemaal; selecteer rijen en druk op **Ctrl+C** om ze te kopiëren.

### Stijlen bij het samenstellen

De koppen, Title, Subtitle, Block Quote, Centered en Caption krijgen hun uiterlijk van de samenstelindeling. Elke andere stijl brengt zijn eigen opmaak mee (inspringingen, afstanden, uitlijning, lettertype, kleur), in elke samenstelindeling. Met **Standaard (opmaak van de editor)** worden ook de wijzigingen overgenomen die je in de ingebouwde stijlen hebt aangebracht. De pagina **Stijlen** van een indeling wijzigt dit per stijl (zie *Een samenstelindeling bewerken*).

### Notities bij het samenstellen

Op de pagina **Voetnoten en opmerkingen** van een samenstelindeling kies je waar voetnoten komen in een PDF of e-book (onderaan de pagina, einde van elk hoofdstuk, einde van de tekst), of inline-aantekeningen opmerkingen in het Word-bestand worden of worden verwijderd, of gekoppelde opmerkingen blijven bewaard en of markeringen worden verwijderd. In een Word-bestand zijn voetnoten altijd echte voetnoten onderaan de pagina.

### Meer uitvoerindelingen (plug-ins)

{{APP}} wordt ook met deze indelingen geleverd. Elke vereist een programma dat apart wordt geïnstalleerd; PDF en EPUB via LibreOffice en Pandoc zijn een alternatief voor de ingebouwde (ze gaan uit van het Word-bestand en lijken erop):

| Indeling | Vereist |
|--------|-------|
| PDF (via LibreOffice) | LibreOffice (op de meeste Ubuntu- en Kubuntu-systemen geïnstalleerd) |
| OpenDocument-tekst (.odt, via LibreOffice) | LibreOffice |
| E-book (.epub, via Pandoc) | Pandoc (installeer "pandoc" in Discover) |
| Markdown (.md) | niets |

**Bestand > Opties > Plugins** toont welke indelingen klaar zijn en bij welke een programma ontbreekt. Meer indelingen kunnen als add-ons worden geïnstalleerd (paragraaf 17).

### Scripts samenstellen

Voor scripts worden twee uitvoerindelingen gemaakt die geen ander programma nodig hebben:

- **Fountain-scenario (.fountain)**: een platte tekstbestand dat Highland, Fade In, Beat, Trelby en andere scenarioprogramma's lezen.
- **Final Draft (.fdx)**: de bestandsindeling van Final Draft, die de meeste scenarioprogramma's ook kunnen openen.

In beide behoudt elke alinea wat ze is (scènekop, actie, personage, parenthetical, dialoog, overgang, shot); het andere programma legt het script op zijn eigen manier op. Titels van mappen en documenten die de samenstelindeling toevoegt, worden secties (Fountain) of aktekoppen (Final Draft); tekst die geen scriptelement is, wordt actie. De projecttitel en je naam (Bestand > Opties) vormen de titelpagina. Inline-aantekeningen worden notities in een Fountain-bestand en blijven weg uit een Final Draft-bestand.

Kies voor Word of PDF de samenstelindeling **Standaard (opmaak van de editor)**: die behoudt de inspringingen, hoofdletters en het lettertype van de elementen zoals je ze in de editor ziet. De andere samenstelindelingen vervangen lettertype en grootte maar behouden de inspringingen.

---

## 14. Importeren en exporteren

### Bestanden importeren

**Bestand > Importeren > Bestanden…** (**Ctrl+Shift+I**) voegt tekstbestanden (.txt, .md, .rtf, .docx, .odt; oude Word-bestanden .doc met de onderstaande add-on), scripts (.fountain, .fdx), afbeeldingen, PDF's, audio en video en andere bestanden toe aan de Binder. Je kunt ook bestanden uit het bestandsbeheer naar de Binder slepen. Tekstbestanden worden bewerkbare documenten; afbeeldingen en PDF's worden in de editor getoond; audio en video worden daar afgespeeld met de add-on Media Playback (paragraaf 6). Een bestand dat niet in het project kan worden gekopieerd (een volle schijf, een bestand dat niet kan worden gelezen) wordt in een melding genoemd en niet toegevoegd, zodat er geen leeg item achterblijft (0.92.0).

Hoe bestanden worden gelezen, stel je in onder **Bestand > Opties > Delen > Importeren**: of opmerkingen van Word-, LibreOffice- en RTF-bestanden inline-aantekeningen worden (anders blijven het opmerkingen naast de tekst), of voetnoten inline-voetnoten worden (de standaard) of gekoppelde voetnoten, of de alinea- en tekenstijlen van de bestanden worden weggelaten (de tekst krijgt het uiterlijk van je project; vet, cursief, notities en links blijven), de codering van platte tekstbestanden (automatisch: UTF-8, anders Windows Western) en of *.html*-bestanden tekstdocumenten worden in plaats van webpagina-documenten.

Waar de bestanden heen gaan: selecteer een map in de Binder om erin te importeren (de bestanden komen achteraan), of een document om ze er direct onder te zetten. Het Manuscript neemt alleen tekst aan; andere bestanden die daar worden neergezet, gaan naar Research.

**Importeren en splitsen…** leest één lang bestand en knipt het in veel documenten, bijvoorbeeld bij elke regel die alleen `#` bevat, of bij elke dubbele lege regel. Sinds 0.94.0 kan het ook een .docx-, .odt- of .md-bestand **bij zijn koppen** knippen: Kop 1 wordt een map, Kop 2 een map of een document (je kiest dat in het venster), de tekst tussen koppen wordt documenten en lagere koppen blijven in de tekst. Het venster toont de Binder die zou ontstaan als boom voordat je importeert. De titel van een document dat door geen kop wordt genoemd, volgt een regel die in het venster staat: de eerste niet-lege regel van zijn tekst (ingekort tot 120 tekens als hij langer is), of "<bestandsnaam> <nummer>" wanneer **De eerste regel van elke sectie als titel gebruiken** uit staat.

### Oude Word-bestanden (.doc)

Word 97 tot 2003 bewaarde documenten als *.doc* (sjablonen *.dot*). Ze worden tekstdocumenten met de add-on **Word 97–2003 Import** (paragraaf 17); zonder die wordt een .doc-bestand zoals het is aan de Binder toegevoegd en wordt je verteld welke add-on het leest. De add-on heeft geen ander programma nodig. Hij behoudt:

- de tekst, inclusief Grieks, Cyrillisch, Aziatische schriften en dergelijke;
- koppen (Kop 1 tot 6), titel, ondertitel, citaten en bijschriften, die de stijlen van je project krijgen; een alinea-stijl van het document die je project ook heeft (dezelfde naam) krijgt die stijl;
- vet, cursief, onderstrepen, doorhalen, kleine en alle hoofdletters, superscript en subscript, markering; gecentreerde, rechts uitgelijnde en uitgevulde alinea's;
- lijsten met opsommingstekens en genummerde lijsten met hun niveaus (de nummers zelf worden door {{APP}} gemaakt, dus de nummering "Chapter 1" van koppen wordt niet overgenomen);
- tabellen, als een regel per rij met de cellen gescheiden door tabs;
- voetnoten en eindnoten (als voetnoten), opmerkingen met de becommentarieerde woorden (de naam van de auteur komt eerst in de opmerking);
- links, het resultaat van velden (datums, paginanummers zoals ze het laatst werden getoond), pagina- en sectie-eindes, regeleindes;
- afbeeldingen in de tekst (JPEG, PNG, BMP, TIFF);
- in RTF-projecten ook met de hand ingestelde lettertypen, groottes, kleuren en inspringingen (Markdown-projecten behouden het uiterlijk van het project, zoals bij elke import).

Weggelaten: kop- en voetteksten, zwevende afbeeldingen en tekeningen, afbeeldingen in de oude metabestandindelingen (WMF, EMF), ingesloten objecten, verborgen tekst en tekst die met bijgehouden wijzigingen is verwijderd (ingevoegde tekst blijft). De tekst van tekstvakken wordt aan het einde van het document toegevoegd. Na de import noemt een melding wat is weggelaten.

**Bestand > Opties > Delen > Conversie** kan LibreOffice de eerste keuze maken voor *.doc*-bestanden (het leest ook kopteksten, tekeningen en Word 6/95-bestanden); wanneer de gekozen manier mislukt, wordt de andere geprobeerd. Word 6.0- en Word 95-bestanden, en bestanden die de add-on niet kan lezen, worden met LibreOffice geconverteerd wanneer dat is geïnstalleerd (op Ubuntu: `sudo apt install libreoffice-writer`); anders zegt de melding dat. Met een wachtwoord beveiligde bestanden moeten eerst zonder wachtwoord worden geopend en opgeslagen. Een *.doc*-bestand dat in werkelijkheid een RTF-bestand, een Word 2007-bestand (.docx) of een webpagina is, zoals sommige programma's ze hebben opgeslagen, wordt gelezen als wat het is, ook zonder de add-on.

### Webpagina's importeren

**Bestand > Importeren > Webpagina…** haalt een pagina van het web in je project. De add-on **Web Page Import** (paragraaf 17) doet het werk; zonder hem zegt de opdracht dat.

1. Selecteer de map die de pagina moet ontvangen (anders gaat hij naar **Research**).
2. Kies **Bestand > Importeren > Webpagina…**. Als het klembord een webadres bevat, is dat al ingevuld; typ of plak anders een adres.
3. Klik op **Laden** (of druk op Enter). De pagina en zijn afbeeldingen worden opgehaald en het venster toont wat zal worden geïmporteerd, met het aantal woorden en afbeeldingen, de auteur, de site en de datum.
4. Corrigeer desgewenst de **Titel**, controleer de keuzes eronder en klik op **Importeren**.

**Alleen het artikel** behoudt de tekst waarvoor je kwam en laat menu's, advertenties, cookiemeldingen, deelknoppen, reactiesecties en lijsten met andere artikelen weg. {{APP}} vindt het artikel zoals leeshulpmiddelen dat doen (de tekst met de meeste zinnen en komma's en de minste links wint; namen als "article" of "sidebar" in de pagina tellen voor of tegen een deel). Als er iets ontbreekt, haal je het vinkje weg om de hele pagina te nemen. **Afbeeldingen** haalt de afbeeldingen van het artikel op (zeer grote worden 1600 pixels breed gemaakt). Voor een pagina van internet worden afbeeldingen alleen van openbare adressen opgehaald: een afbeelding die op je eigen computer of op een privénetwerk staat (ook via een omleiding) blijft weg en het resultaat vertelt hoeveel. Een pagina die zelf van je eigen computer of netwerk komt, wordt met al zijn afbeeldingen opgehaald.

**Importeren als**:

- **Tekstdocument** (de standaard): koppen, alinea's, citaten, lijsten, tabellen (een regel per rij), programmacode, bijschriften, vet, cursief en links worden een gewoon document dat je kunt bewerken, annoteren, splitsen en samenstellen.
- **Webpagina-document**: een alleen-lezen kopie van het artikel met zijn afbeeldingen, ter referentie. Het wordt niet meegeteld of samengesteld.

**Een webpagina-document bekijken** (0.81.3). De editor toont de pagina als eenvoudige opgemaakte tekst, met zijn adres en twee knoppen erboven. **Opgeslagen pagina in browser openen** toont de pagina zoals hij is opgeslagen in je webbrowser: {{APP}} zet er een kopie van in je map Downloads (*Quillbinder web pages/<titel>*), omdat een browser die als sandbox-pakket is geïnstalleerd (bijvoorbeeld Firefox op Kubuntu) de eigen mappen van {{APP}} niet kan lezen; de regel onder de knoppen zegt waar de kopie staat, en de volgende klik vervangt hem. Op Windows wordt de kopie in plaats daarvan in een tijdelijke map gemaakt en verwijderd wanneer {{APP}} sluit, zodat er niets buiten de map van het programma wordt geschreven. **Oorspronkelijk adres openen** opent het adres van de pagina, zodat je de pagina ziet zoals hij nu is; de knop is uit wanneer de pagina geen adres heeft. Webpagina's die Scrivener heeft opgeslagen (elk één *.mht*-bestand) worden op dezelfde manier getoond; een browser als Firefox kan *.mht*-bestanden zelf niet openen, dus de kopie is gewone HTML met de afbeeldingen ernaast.

Hoe dan ook wordt het adres van de pagina een **bladwijzer** van het document (Inspector, Bladwijzers), wordt de korte beschrijving van de pagina zijn **samenvatting** en leggen de **notities** het adres, de site, de auteur, de publicatiedatum en het moment waarop je hem hebt opgehaald vast.

**Een link slepen** vanuit een webbrowser naar een map in de Binder, naar het prikbord of naar het overzicht importeert de pagina direct, met de keuzes die je de vorige keer in het venster hebt gemaakt.

Goed om te weten: de pagina wordt opgehaald zoals een browser dat doet, maar zonder zijn scripts uit te voeren. Pagina's die hun tekst met een script opbouwen, dragen die vaak toch in hun gegevens, en die worden dan gebruikt (zonder opmaak en afbeeldingen; een melding zegt dat). Pagina's achter een login of een betaalmuur geven wat ze aan een niet-ingelogde bezoeker tonen. Beveiligde pagina's (https) hebben OpenSSL 3 op je systeem nodig (onderdeel van elke Ubuntu). Een webadres dat naar een PDF-bestand leidt, wordt niet op deze manier geïmporteerd: download het bestand en gebruik **Bestand > Importeren > Bestanden…**.

**Scripts.** Fountain-bestanden (*.fountain*) en Final Draft-bestanden (*.fdx*) worden als scripts geïmporteerd: de alinea's krijgen de elementen van de scriptindeling van je project, het document staat in scriptmodus en de titelpagina van het bestand gaat naar de notities van het document. Vette, cursieve en onderstreepte tekst, notities en geforceerde pagina-einden blijven behouden; scènenummers, revisiemarkeringen en de tweekolomsweergave van gelijktijdige dialoog niet (de twee teksten volgen op elkaar). **Importeren en splitsen…** met **Scènekoppen** knipt een script in één document per scène, genoemd naar zijn kop.

### Een Scrivener-project importeren

**Bestand > Importeren > Scrivener-project…** leest een Scrivener 3- of 2-project (de map *.scriv*; kies het bestand *.scrivx* erin) en maakt er een nieuw {{APP}}-project van. Het project van Scrivener wordt alleen gelezen, nooit gewijzigd. Sinds 0.92.0 noemt het verslag na de import ook elk bestand dat niet kon worden gekopieerd (een afbeelding, een PDF, een audiobestand, de afbeelding van een indexkaart, een mindmap), en zo'n bestand telt niet als geïmporteerd; eerder werd het zonder een woord weggelaten.

De import neemt mee: de structuur van de Binder, teksten, samenvattingen, notities, labels, statussen, trefwoorden, eigen metagegevens, sectietypen, de vlaggen Opnemen bij samenstellen, momentopnamen, opmerkingen en voetnoten, inline-aantekeningen, researchbestanden (afbeeldingen, PDF's en andere), verzamelingen, projectbladwijzers en doelen. Samenstelinstellingen worden niet geïmporteerd; stel ze één keer in het venster Samenstellen in. Kies of het nieuwe project zijn tekst als Markdown of RTF bewaart.

Koppen en stijlen: Scrivener markeert tekst met een stijl in zijn bestanden met codes zoals `<$Scr_Ps::0><$Scr_H::2>`. Die zijn geen tekst; {{APP}} haalt ze eruit. Een kop krijgt **Kop 1** tot **Kop 6**, en een alinea met een van de alinea-stijlen van Scrivener krijgt de stijl van je project met dezelfde naam (**Title**, **Block Quote**; de **Centered Text** van Scrivener wordt **Centered**). Sinds 0.81.3 krijgt tekst met een van de tekenstijlen van Scrivener de tekenstijl van je project met die naam, en een stijl van Scrivener waarvoor je project geen stijl met die naam heeft, wordt een stijl van het project, met het uiterlijk dat Scrivener eraan geeft. Projecten die met versies van vóór 0.14 zijn geïmporteerd, behielden de codes als tekst; {{APP}} haalt ze eruit wanneer zo'n project opent en zegt dat in de statusbalk. Projectbladwijzers werden vóór 0.14 niet geïmporteerd; importeer het Scrivener-project opnieuw om ze te krijgen.

Als iets niet goed is overgekomen, is het oorspronkelijke project onaangeroerd en kun je tekst rechtstreeks uit Scrivener kopiëren.

### Werken in Scrivener-indeling

In plaats van een kopie te maken kan {{APP}} ook zelf aan een Scrivener 3-project werken, zodat je later in Scrivener verder kunt (bijvoorbeeld op een andere computer).

- **Openen**: **Bestand > Project openen…** (**Ander openen…** in het startvenster) en kies het bestand *.scrivx* in de map *.scriv*. De eerste keer vraagt {{APP}}: **Werken in Scrivener-indeling** (je wijzigingen worden in de *.scriv* opgeslagen), **Omzetten…** (er wordt een eigen {{APP}}-project van gemaakt, zoals bij de import hierboven) of **Annuleren**. Daarna opent het project zonder de vraag, ook via **Bestand > Recent geopend**.
- **Nieuwe projecten**: kies **Indeling: Scrivener 3-project (.scriv)** in het venster Nieuw project.
- **Opslaan**: elke keer opslaan (ook automatisch) schrijft naar de *.scriv*. Alleen wat je hebt gewijzigd wordt geschreven; documenten die je niet hebt aangeraakt behouden de bestanden die Scrivener schreef. De venstertitel zegt *(Scrivener-indeling)*, en **Bestand > Project tonen in bestandsbeheer** en de back-ups nemen de map *.scriv*.
- **Grote projecten** (sinds 0.60.5): het automatisch opslaan terwijl je schrijft zet alleen de teksten, notities en samenvattingen die je hebt gewijzigd in de *.scriv* (en de Binder, als die is veranderd), zodat het kort blijft, ook bij honderdduizenden woorden. Al het andere, zoals de zoekindex van Scrivener, momentopnamen en afbeeldingen, volgt wanneer je naar een ander programma overschakelt, **Bestand > Opslaan** (**Ctrl+S**) gebruikt of het project sluit; sluit het project dus hier of schakel eerst over naar een ander programma voordat je het in Scrivener opent.
- **Eén programma tegelijk**: als Scrivener het project open heeft (of niet netjes is afgesloten), zegt {{APP}} dat en vraagt voordat het wordt geopend. Sluit het project in het ene programma voordat je het in het andere opent. Wanneer het project in Scrivener (of door een synchronisatiedienst) is gewijzigd sinds je het hier voor het laatst open had, leest {{APP}} het opnieuw en meldt in de statusbalk hoeveel documenten zijn veranderd; een document dat intussen in Scrivener is gewijzigd, wordt nooit met een oudere tekst overschreven.

Wat in de *.scriv* terechtkomt: de Binder, teksten (als RTF, met hun afbeeldingen), samenvattingen, notities (ook van afbeeldingen, PDF's, audio en video, sinds 0.81.3), labels, statussen, sectietypen, **Opnemen bij samenstellen**, bladwijzers (van het project en van elk document, ook naar webadressen en bestanden), projectdoelen met de deadline, documentdoelen, de schrijfgeschiedenis, het prikbord (de rangschikking van elke map en de plaatsen van vrij geplaatste kaarten), de titel, auteur en taal van het samengestelde bestand (in de samenstelinstellingen van Scrivener), momentopnamen, opmerkingen en voetnoten, inline-aantekeningen en -voetnoten, alinea- en tekenstijlen, trefwoorden, eigen metagegevensvelden en hun waarden, verzamelingen, opgeslagen zoekopdrachten (zoekverzamelingen, sinds 0.90.0), de afbeeldingen van indexkaarten (sinds 0.90.0), afbeeldingen, PDF's, audio en video, webpagina's, pictogrammen (met de namen van Scrivener; je eigen pictogrammen in zijn map *Icons*) en mindmaps (als extra bestand naast de bestanden van het document, dat Scrivener met rust laat). Alles wordt geschreven in de vorm die Scrivener 3 voor Windows gebruikt:

- Trefwoorden, eigen metagegevens en verzamelingen behouden de nummers en namen van Scrivener (sinds 0.81.2), zodat ze in beide programma's hetzelfde zijn.
- Stijlen (sinds 0.81.3): de stijlen van een tekst die je wijzigt, worden erin gemarkeerd zoals Scrivener ze markeert, zodat Scrivener dezelfde stijlen toont. Een stijl die je in {{APP}} hebt gemaakt, wordt de eerste keer dat een tekst hem gebruikt, met zijn uiterlijk aan de stijlen van het Scrivener-project toegevoegd.
- Inline-aantekeningen en voetnoten (sinds 0.81.3) blijven inline, de aantekeningen in hun kleur (eerder gaf een tekst die je wijzigde ze aan Scrivener door als gekoppelde opmerkingen en voetnoten).
- Webpagina's (sinds 0.81.3): een pagina die je in {{APP}} hebt opgeslagen, gaat erin als de eigen opgeslagen webpagina van Scrivener (één *.mht*-bestand met zijn afbeeldingen) en behoudt zijn adres. Andere bestanden waarvoor Scrivener geen soort heeft, gaan erin als tekstdocumenten die het bestand noemen, met het bestand ernaast; {{APP}} leest ze terug zoals ze waren.
- Audio, video en webpagina's die versies van vóór 0.81.2 tot zulke tekstdocumenten maakten, worden met het volgende opslaan weer de eigen audio-, video- of webpagina-documenten van Scrivener.
- Teksten (sinds 0.90.0) worden teruggeschreven met alles wat Scrivener erin bewaart: rechterinspringingen onder nul, lijsten met hun nummering, het onderstrepen van links, afbeeldingen die aan een bestand zijn gekoppeld, lettertypen die met hun snit zijn genoemd (zoals *Cambria-BoldItalic*) en de plaats van een afbeelding waarvan de gegevens ontbreken. Opmerkingen en voetnoten behouden hun ID's, of hun vak is ingeklapt en hun eigen opmaak zolang je ze niet wijzigt.
- Het prikbord (sinds 0.90.0): kaarten die je vrij plaatst, behouden hun plaats in beide programma's. Scrivener bewaart een plaats in kaartbreedtes; {{APP}} zet die om in zijn eigen kaartgrootte, zodat de rangschikking hetzelfde blijft, welke grootte elk programma de kaarten ook toont. Elke map behoudt zijn rangschikking (vrij geplaatst of in een raster), zoals in Scrivener: de keuze van **Vrij** of het raster voor het prikbord van een map wordt voor die map onthouden.
- Labels en statussen (sinds 0.90.0): het standaardlabel en de standaardstatus van nieuwe documenten, en de namen die Scrivener "No Label" en "No Status" geeft, blijven zoals het project ze heeft.
- Opgeslagen zoekopdrachten (sinds 0.90.0): een opgeslagen zoekopdracht van Scrivener is een zoekverzameling in {{APP}}, en een zoekverzameling die je maakt of wijzigt, gaat als opgeslagen zoekopdracht in de *.scriv*. De lijst met recente zoekresultaten van Scrivener blijft van Scrivener zelf. Gecontroleerd met een opgeslagen zoekopdracht naar een van de woorden; hoe Scrivener de andere soorten noemt (alle woorden, de exacte zin, een reguliere expressie, alleen documenten die bij het samenstellen zijn opgenomen of weggelaten) is nog niet in een echt project gezien, dus een opgeslagen zoekopdracht van die soorten kan in Scrivener als zoekopdracht naar alle woorden uitkomen. Waar in de documenten een zoekopdracht kijkt, hoort niet bij wat wordt overgebracht: een opgeslagen zoekopdracht die uit Scrivener is gelezen, kijkt overal.
- De afbeelding van een indexkaart (sinds 0.90.0): een kaart die een afbeelding toont in plaats van zijn samenvatting, behoudt die in beide programma's; hem eraf halen of er een opzetten in {{APP}} bereikt Scrivener.

Wat alleen aan de kant van {{APP}} blijft (in een werkkopie in *~/.local/share/{{APP}}/ScrivenerProjects*): scriptindelingen, samenstelinstellingen (maar de titel, auteur en taal wel), objecten (paragraaf 7) en de vensterindeling; wat de *.scriv* hier al van heeft, blijft erin zoals het is.

De eerste keer dat een Scrivener-project wordt geopend of geïmporteerd, toont zijn Binder de mappen die in Scrivener open waren en de documenten die daar waren geselecteerd (0.90.0; {{APP}} schrijft de eigen vensterinstellingen van Scrivener nooit). De eerste keer dat je een project in Scrivener-indeling opent met 0.81.2 of nieuwer, leest {{APP}} de *.scriv* opnieuw en voegt de trefwoorden, eigen metagegevens en verzamelingen van zijn werkkopie samen met die van het project, zodat niets wat je met een oudere versie hebt gemaakt, verloren gaat; het volgende opslaan zet ze in de *.scriv*. De eerste keer openen met 0.90.0 of nieuwer doet hetzelfde voor bladwijzers, documentdoelen, de deadline, de schrijfgeschiedenis en het prikbord.

Als een opslag in de *.scriv* werd afgebroken (een crash, een volle schijf) nadat er nieuwe documenten in de werkkopie waren geschreven, behoudt het volgende openen die documenten en zegt dat; het volgende opslaan zet ze in de *.scriv*. Documenten die in Scrivener zijn verwijderd, blijven verwijderd.

**Bestand > Opslaan als…** (**Ctrl+Shift+S**) slaat het open project onder een andere naam of op een andere plaats op, en zo verandert een project van indeling. Typ in het dialoogvenster de naam en kies de plaats; het **bestandstype** bepaalt de indeling: *Scrivener 3 Project (\*.scriv)* of *{{APP}} Project (\*.qbx)* (een naam die op *.scriv* of *.qbx* eindigt, bepaalt dat zelf). Een project is een map: het programma maakt die van de naam (*Naam.scriv*, of *Naam.quillbinder* met *Naam.qbx* erin). Een naam die al bestaat, wordt geweigerd.

- Vanaf dan werkt het venster aan het nieuwe project en gaat elke latere opslag daarheen. De titel van het project is de naam die je typte. Wijzigingen die je nog niet had opgeslagen, gaan alleen naar het nieuwe project.
- Het origineel blijft op schijf zoals het op dat moment was (bewaar het als kopie of verwijder het); het staat ook in **Bestand > Recent geopend**, onder het nieuwe project. Zijn vergrendelingsbestand wordt verwijderd wanneer het venster het verlaat en het nieuwe project krijgt zijn eigen.
- Het nieuwe project is een eigen project, met zijn eigen **back-ups**: het begint een nieuwe reeks, en de tot nu toe gemaakte back-ups blijven bij het origineel (paragraaf 14, Back-ups). Een project in Scrivener-indeling krijgt hiervoor een nieuwe identificatie in zijn *.scrivx*.
- Opslaan als werkt hetzelfde van een indeling naar dezelfde indeling (een nieuwe naam of plaats) en naar de andere (de overstap die het menu "Projectindeling" tot 0.92 deed).

**Bestand > Exporteren > Scrivener-project…** schrijft een **kopie** in Scrivener-indeling, en het item eronder in hetzelfde menu, **{{APP}}-project…**, een kopie in de eigen indeling van {{APP}}; beide laten het venster op het origineel: er verandert niets in het project waaraan je werkt (vóór 0.93.0 waren dit "Een kopie opslaan als ..." in het menu "Projectindeling").

### Documenten exporteren

**Bestand > Exporteren > Bestanden…** slaat de geselecteerde documenten op voor andere programma's. Gebruik Samenstellen voor een afgerond manuscript; gebruik Exporteren om losse documenten aan iemand te geven, of om een kopie buiten {{APP}} te bewaren.

1. Selecteer in de Binder de documenten en mappen die je wilt exporteren.
2. Kies **Bestand > Exporteren > Bestanden…**.
3. **Tekstbestanden exporteren als**: Rich Text (.rtf), Word (.docx), Markdown (.md), platte tekst (.txt), Fountain of Final Draft (voor scripts is Fountain voorgeselecteerd). Afbeeldingen, PDF-bestanden, webpagina's, audio en video worden geëxporteerd zoals ze zijn; de indeling telt dan alleen voor hun notities.
4. **Metagegevens**: **Metagegevens exporteren** schrijft naast elk document een tekstbestand, "*naam* (metadata).txt", met zijn titel, soort, label, status, trefwoorden, opnemen bij samenstellen, sectietype, aantal woorden, doel, datums van aanmaken en wijzigen, eigen metagegevens en bladwijzers. **Samenvattingen exporteren** voegt de samenvatting eraan toe (of schrijft "*naam* (synopsis).txt" wanneer de metagegevens niet worden geëxporteerd).
5. **Opties**: **Subdocumenten opnemen** (mappen worden mappen), **Notities exporteren** ("*naam* (notes)" in de indeling van stap 3; platte tekst voor scripts), **Opmerkingen en inline-aantekeningen exporteren** (haal het vinkje weg om de schone tekst door te geven; de becommentarieerde woorden blijven, voetnoten blijven altijd), **Momentopnamen exporteren** (een map "*naam* (snapshots)" met één bestand per momentopname, genoemd naar datum en titel).
6. **Locatie**: de map om in op te slaan. Wanneer meerdere documenten zijn geselecteerd, gaan ze in een nieuwe map waarvan je de naam opgeeft (de titel van het project wordt voorgesteld; als de map bestaat, wordt een nummer toegevoegd).
7. Klik op **Exporteren**. De map opent in het bestandsbeheer.

De keuzes worden voor de volgende export onthouden.

De snelle manier: **sleep documenten uit de Binder** naar het bestandsbeheer of naar het bureaublad. Tekstdocumenten worden opgeslagen als RTF-bestanden (of Word, Markdown of platte tekst: **Bestand > Opties > Gedrag**), afbeeldingen en andere bestanden zoals ze zijn, en een map wordt een map met zijn documenten erin. Als het bestandsbeheer vraagt of het moet verplaatsen of kopiëren, is beide goed: de documenten blijven in je project.

### Vergelijken en overbrengen: een Word- of LibreOffice-bestand en het project

Wanneer een manuscript van een redacteur terugkomt, of je in Word of LibreOffice verder hebt gewerkt, zoekt **Bestand > Vergelijken en overbrengen > Vergelijken met bestand…** uit wat dat bestand en je project van elkaar verschilt en neemt de kant over die je kiest.

1. Kies het bestand: een Word-bestand (*.docx*) of een LibreOffice-bestand (*.odt*). Oude Word-bestanden (*.doc*), RTF-, platte tekst- en Markdown-bestanden kunnen ook worden vergeleken, maar alleen gelezen: hun tekst kan in het project, het bestand zelf kan niet worden gewijzigd.
2. {{APP}} vergelijkt het bestand met **wat Samenstellen van het project maakt**, niet met het hele project: de documenten van de samenstelgroep (het Manuscript, tenzij in het venster Samenstellen een andere groep is gekozen) die **Opnemen bij samenstellen** hebben, in de volgorde van de Binder. Voor- en achterwerk, de map Research en documenten die van Samenstellen zijn uitgesloten, tellen niet mee. Als minder dan 65 % van de tekst van het bestand daarin wordt gevonden, zegt {{APP}} dat en vraagt of het toch moet vergelijken (**Toch vergelijken**); het bestand kan bij een ander project horen, of Samenstellen laat weg wat het bestand heeft. Een woord telt als gevonden wanneer het in een reeks van vier woorden ligt die de samengestelde tekst ook heeft, zodat een paar gewijzigde woorden een bestand niet losstaand maken.
3. Het venster **Vergelijken en overbrengen** toont de verschillen. Kies de kant die wordt bijgewerkt:
   - **Het project bijwerken vanuit het bestand**: de wijzigingen worden in je documenten gemaakt. Daarvoor krijgt elk gewijzigd document een momentopname, "Before Compare and Transfer (*bestandsnaam*)" (**Documenten > Momentopnamen**), zodat je altijd terug kunt. Tekst die alleen in het bestand staat, wordt een nieuw document (genoemd naar zijn kop in het bestand), geplaatst na het document ervoor.
   - **Het bestand bijwerken vanuit het project**: het bestand dat je koos blijft zoals het is. {{APP}} schrijft ernaast een kopie, "*naam* (Quillbinder).docx" (met een nummer wanneer die naam al bestaat). In een Word-bestand is elke wijziging een **bijgehouden wijziging** van de auteur "Quillbinder": open de kopie in Word of LibreOffice en accepteer of verwerp elke wijziging daar (in Word op het tabblad Controleren; in LibreOffice met Bewerken, Wijzigingen bijhouden, Beheren). Een LibreOffice-kopie (*.odt*) krijgt de wijzigingen direct, zonder markeringen.
4. In de weergave komt **groene** tekst erbij, gaat **rode** doorgehaalde tekst eraf, en krijgt **blauwe** tekst het vet, cursief of onderstrepen van de andere kant. Grijze alinea's zijn ongewijzigde tekst rond een wijziging, om te laten zien waar die is. Elke plaats heeft een link **overnemen** of **overslaan**; een klik schakelt hem om.
5. Klik op **... overbrengen naar het project** of **... schrijven in een kopie van het bestand**. Een melding zegt wat er is gedaan; voor het bestand opent **Het bestand openen** de kopie.

**Per hoofdstuk** (**Bestand > Vergelijken en overbrengen > Per hoofdstuk**, ook een vinkvak in het venster) toont links de documenten, elk met een vinkvak en wat verschilt ("2 plaats(en)", "alleen in het bestand", "niet in het bestand"). Haal het vinkje bij een document weg om het te laten zoals het is; een klik toont zijn wijzigingen rechts. Zonder Per hoofdstuk worden alle wijzigingen onder elkaar getoond.

**Stap voor stap** (**Bestand > Vergelijken en overbrengen > Stap voor stap**, ook in het venster) toont één wijziging tegelijk: **Overnemen** of **Overslaan**, of ga **Terug**. Aan het einde zegt het venster hoeveel wijzigingen worden overgenomen; klik dan op Overbrengen. Beide keuzes worden voor de volgende keer onthouden.

Wat wordt vergeleken: de woorden, en vet, cursief en onderstrepen. Elke alinea van het bestand wordt gekoppeld aan het document dat zijn woorden heeft, zodat een scène ook wordt gevonden waar hij in het bestand is verplaatst (hij wordt dan gemarkeerd als "op een andere plaats in het bestand"; de volgorde van de Binder wordt niet gewijzigd). Koppen van het bestand die geen tekst van een document zijn, zoals de hoofdstukkoppen die Samenstellen maakt, worden niet vergeleken.

Wat eerst is aangevinkt: elke wijziging binnen een document. Tekst die maar aan één kant staat, is aangevinkt wanneer hij minstens 100 woorden heeft (een nieuw hoofdstuk van het bestand wordt een nieuw document; een scène die in het bestand ontbreekt, wordt aan de kopie toegevoegd). Een heel document gaat nooit naar de Prullenbak en een heel deel wordt nooit uit het bestand gehaald, tenzij je het aanvinkt.

Beperkingen: nieuwe alinea's nemen het uiterlijk van de alinea ernaast over. Tekst in tabellen wordt vergeleken als andere alinea's (een tabel zelf wordt nooit verwijderd) en velden tellen mee met de tekst die ze tonen. Kop- en voetteksten, tekstvakken en de tekst van de voetnoten en opmerkingen van het bestand worden niet vergeleken en blijven zoals ze zijn; opmerkingen en voetnoten van het project blijven in het project en worden niet in het bestand geschreven. Als het bestand na de vergelijking is gewijzigd, wordt de kopie niet geschreven; vergelijk opnieuw.

### Afdrukken

**Bestand > Huidig document afdrukken…** (**Ctrl+P**) drukt af wat de huidige editor toont:

- **Een tekst** (één document, of meerdere in de Gecombineerde tekst): de tekst met zijn opmaak en afbeeldingen, de titel van elk document boven zijn tekst en voetnoten onderaan de pagina. Met het toetsenbord in een kopijhouder wordt het document van de kopijhouder afgedrukt.
- **Het prikbord**: de indexkaarten in de getoonde volgorde, zoveel op een vel als er passen (met lijnen om langs te knippen), of één kaart op elk vel voor indexkaartkarton.
- **Het overzicht**: de rijen en kolommen zoals het overzicht ze toont, met de kolomtitels bovenaan elke pagina.
- **Een afbeelding** (zo groot als de pagina toestaat), **een PDF-bestand** (zijn pagina's) of **een webpagina** (zijn tekst).

Het afdrukvenster kiest de printer, de pagina's en het aantal exemplaren. **Selectie** drukt alleen de geselecteerde tekst af, of alleen de geselecteerde indexkaarten of overzichtsrijen. Om de afdruk in plaats daarvan als PDF-bestand op te slaan, kies je **Afdrukken naar bestand (PDF)** als printer.

**Bestand > Afdrukvoorbeeld…** toont eerst de pagina's; de printerknop van het voorbeeld drukt ze af, met **Sluiten** (of **Esc**) sluit je het voorbeeld.

**Bestand > Afdrukinstellingen…** bepaalt hoe het op papier komt. Het venster opent op het tabblad voor wat de editor toont; **Voorbeeld…** toont het resultaat voordat je op **OK** klikt. De instellingen horen bij het project; vink **Deze instellingen ook voor nieuwe projecten gebruiken** aan om ze de standaard voor nieuwe projecten te maken.

- **Pagina**: het papierformaat en de afdrukstand (de knop **Papier van de Paginaweergave** neemt het papier en de marges van **Weergave > Tekstbewerking > Paginaformaat en marges…** over), de marges en de kop- en voettekst, elk met een linker-, een midden- en een rechterdeel. Plaatsvervangers: `<$title>` (wat wordt afgedrukt), `<$projecttitle>`, `<$author>`, `<$date>`, `<$p>` (het paginanummer) en `<$pages>` (het aantal pagina's). Haal het vinkje bij **Kop- en voettekst ook op de eerste pagina** weg voor een schone eerste pagina.
- **Tekst**: wat met de tekst wordt afgedrukt (de titel van elk document, de samenvatting eronder, de documentnotities na de tekst, opmerkingen en inline-aantekeningen als notities onderaan de pagina, voetnoten aan het einde van de tekst in plaats van onderaan elke pagina, markeringen); de lettertypen (als in de editor, of **Eén lettertype** naar keuze); en de regelafstand (als in de editor, enkel, 1,5 regel of dubbel, met ruimte voor correcties). Scriptdocumenten behouden hun scriptindeling.
- **Indexkaarten**: de kaartgrootte (3 x 5 in, 4 x 6 in, 5 x 8 in, A7, A6 of een eigen formaat), zoveel kaarten op een vel als er passen of één kaart op elk vel, wat elke kaart toont (titel, samenvatting, de eerste regels van de tekst wanneer er geen samenvatting is, afbeeldingen, de labelkleur als streep bovenaan, status, trefwoorden, de lijnen van een indexkaart, een lijn rond elke kaart) en het lettertype.
- **Overzicht**: de samenvattingen (zoals het overzicht ze toont, altijd of nooit), ook de rijen binnen gesloten groepen, de kolomtitels bovenaan elke pagina, een lijn tussen rijen, een stip in de labelkleur en het lettertype. Met veel kolommen geeft **Liggend** op het tabblad **Pagina** ze meer ruimte.

Huidig document afdrukken drukt een document af zoals je eraan werkt. Voor een afgerond manuscript met het uiterlijk van een samenstelindeling stel je het samen (paragraaf 13) en druk je het PDF- of Word-bestand af.

---

## 15. Back-ups

{{APP}} maakt een back-up (een ZIP-bestand van het hele project) telkens wanneer je een project sluit, en bewaart de laatste vijf per project. Wijzig dit onder **Bestand > Opties > Back-up**, waar je ook de back-upmap kunt kiezen (het best op een andere schijf, of een map die je synchronisatieprogramma kopieert).

Sinds 0.91 staan de back-ups van alle projecten direct in de back-upmap, elk genoemd naar zijn project met de datum en tijd. Sinds 0.92.0 draagt de naam ook de **ID** van het project, vijf tekens na een "#" (*My Novel #K7T2M bak 2026-11-20 15-40-00.zip*). De ID hoort bij het project, niet bij de computer: hij is hetzelfde op elke computer waarop het project wordt geopend, en blijft als je het project hernoemt of zijn map verplaatst. De back-ups van een project zijn de bestanden die zijn titel en zijn ID dragen, en alleen die worden ervoor geteld en opgeruimd, zodat twee projecten met dezelfde titel elk hun eigen back-ups en hun eigen aantal bewaarde back-ups houden. Een kopie of een nieuw project dat het programma zelf maakt, is een eigen project met een eigen ID: **Bestand > Opslaan als…** en het item {{APP}}-project van **Bestand > Exporteren**, een project dat met **Herstellen uit back-up** is hersteld (het krijgt ook een eigen titel) en een project dat is gemaakt van een sjabloon van jezelf. Een kopie die buiten het programma is gemaakt (een map die je met de hand hebt gekopieerd, een back-up die je met de hand hebt uitgepakt) heeft de ID van zijn origineel: geef zo'n kopie een eigen titel. Een project in Scrivener-indeling neemt zijn ID uit de identificatie die Scrivener in de *.scrivx* bewaart. Als je back-upmap in de OneDrive van een organisatie staat, kan de beheerder "#" in bestandsnamen hebben verboden; OneDrive voor thuis staat het toe. Back-ups die door een versie van vóór 0.92.0 zijn gemaakt, hebben geen ID in hun naam: ze worden nooit door het opruimen verwijderd, **Herstellen uit back-up** toont ze nog steeds en je verwijdert ze zelf wanneer je ze niet meer nodig hebt. Wanneer je de titel van een project wijzigt, blijven de back-ups die onder de oude titel zijn gemaakt staan tot je ze verwijdert. Versies 0.70 tot 0.90 bewaarden de back-ups van elk project in een eigen map in de back-upmap (*My Novel 1a2b3c4d*); zo'n map blijft zoals hij is, er wordt niets meer aan toegevoegd of verwijderd, en **Herstellen uit back-up** toont zijn back-ups nog steeds. Een back-up loopt naast het venster: na een moment toont een voortgangsvenster hoever hij is en kan hij worden geannuleerd. Wie het project open heeft (het vergrendelingsbestand met de naam van de computer en van de login) zit niet in de back-up.

- Met **Back-up maken bij elke handmatige opslag** (**Bestand > Opties > Back-up**) maakt elke **Bestand > Opslaan** (**Ctrl+S**) ook een back-up. Twee back-ups binnen dezelfde seconde krijgen een nummer ("... (2)"). Wanneer die back-up niet kan worden gemaakt, onvolledig is of buiten de back-upmap moet wachten, zegt een melding naast de tekst dat (0.92.0; eerder deed alleen de statusbalk dat, een paar seconden). Je tekst wordt hoe dan ook opgeslagen. De melding komt eenmaal per oorzaak tot je het programma afsluit; de statusbalk zegt het bij elke opslag.
- **De berichtenregel** onder de Binder toont korte berichten een voor een, gedurende 5 seconden (10 met een link) of tot je erop klikt; andere wachten. Add-ons gebruiken hem ook (bijvoorbeeld "Model unloaded, RAM usage minimal" wanneer Vertalen en proeflezen of Spraaktranscriptie zijn model na 15 minuten zonder gebruik heeft vrijgegeven; de tijd staat in hun opties). **Help > Probleemoplossing en ontwikkeling > Systeeminformatie** noemt het geheugen dat het programma vasthoudt.
- **Add-ons die niet zijn geïnstalleerd.** **Bewerken > Schrijfhulpmiddelen** toont "Vertalen en proeflezen" en "Spraaktranscriptie" terwijl de add-on ontbreekt of is uitgezet; elk toont een melding met een knop die **Help > Add-ons en updates > Add-onbeheer** opent.
- **Bestand > Back-up > Nu back-up maken** maakt meteen een back-up. Sinds 0.94.0 wordt het succes gemeld in de berichtenregel onder de Binder ("Backup saved." met de link **Map tonen**, die de map van de back-up opent; de tooltip noemt het bestand; een bericht met een link blijft 10 seconden staan); er wacht geen venster op OK. Een venster komt alleen wanneer er iets is mislukt, of wanneer de back-up is gemaakt maar sommige bestanden niet konden worden gelezen of een oudere back-up niet kon worden verwijderd.
- **Een back-up die door een shutdown of een kill-opdracht niet kon worden gemaakt**, wordt genoteerd en bij het volgende openen gemaakt (0.81). Sinds 0.93.0 blijft de notitie staan tot die back-up is gemaakt (in de back-upmap, of wachtend ernaast zoals hierboven beschreven): een opening waarvan de back-up mislukt, laat de notitie voor de volgende opening staan. **Een oudere back-up die het opruimen niet kan verwijderen** (een bestand dat is vergrendeld of beveiligd) blijft staan, en een melding noemt het bestand en de reden één keer; verwijder het met de hand. Terwijl je doorschrijft, is deze melding een kleine mededeling naast de tekst die geen toetsen overneemt.
- **Back-up maken naar…** slaat een back-up op waar je maar wilt, bijvoorbeeld op een USB-stick.
- **Herstellen uit back-up…** (0.70) toont de back-ups van het open project met hun datum, grootte en of ze volledig zijn; kies er een en **Herstellen en openen**: hij wordt uitgepakt in een nieuwe map naast het project, *… (restored 2026-11-10 14-03)*, zijn titel zegt "(restored …)" en hij opent daar. Het project dat je open had, wordt niet gewijzigd. De lijst bevat de back-ups met de ID van dit project, die zonder ID (gemaakt vóór 0.92.0) en die van een ander project met dezelfde titel; de bestandsnaam in de lijst laat zien welke welke is, zodat niets buiten bereik raakt. Sinds 0.93.0 toont hij ook de back-ups die in de eigen werkmap van het programma wachten (gemaakt terwijl de back-upmap niet bereikbaar was; de volgende back-up in de back-upmap zet ze daar): ze zijn gemarkeerd als "waiting (not yet in the backup folder)" en worden op dezelfde manier hersteld. Sinds 0.81 werkt een herstel van elke grootte (back-ups van 2 GB en meer, bestanden van meer dan 1 GB zoals video's, meer dan 65.535 bestanden), wordt elk bestand gecontroleerd terwijl het wordt geschreven, wordt een herstel dat niet op de schijf past niet gestart, kan een voortgangsvenster het annuleren en noemt de melding, wanneer bestanden niet kunnen worden hersteld, elk ervan met de reden (een volle schijf, een beschadigd onderdeel). Sinds 0.93.0 heeft de melding drie vormen: **hersteld** (alles is er), **onvolledig hersteld** (zo getiteld, met een waarschuwingsteken, de ontbrekende bestanden met de reden genoemd: bekijk het herstelde project voordat je erop vertrouwt) en **niet hersteld**; en wanneer de titel van het herstelde project niet als "(restored …)" kon worden gemarkeerd, zegt de melding dat.
- **Oude back-ups naar de back-upmap verplaatsen…** (0.93) verplaatst de back-ups die versies 0.70 tot 0.90 in een eigen map in de back-upmap bewaarden (*My Novel 1a2b3c4d*) naar de back-upmap zelf, onder hun namen, zodat ze bij de andere liggen. Het is alleen aan terwijl het open project zo'n map met back-ups heeft. Het vraagt eerst en zegt hoeveel er zijn; een back-up waarvan de naam al in de back-upmap staat, wordt nooit overschreven en blijft waar hij was; het verslag vertelt wat is verplaatst en wat is blijven staan (met de reden), en de oude map wordt verwijderd wanneer er niets meer in staat. Deze back-ups hebben geen ID in hun naam, dus het opruimen telt of verwijdert ze nooit, zoals voorheen; **Herstellen uit back-up** toont ze op beide plaatsen.
- **Back-upmap tonen** opent de back-upmap.

**Waar een back-up wordt samengesteld (sinds 0.90.3).** Een back-up wordt ingepakt en gecontroleerd in een eigen map van het programma, *~/.local/share/{{APP}}/backup-work* (op Windows *data\backup-work* in de map van het programma), en alleen het voltooide bestand wordt in de back-upmap gezet, in één stap en onder zijn definitieve naam: erheen verplaatst wanneer beide mappen op dezelfde schijf staan, anders rechtstreeks naar zijn definitieve naam gekopieerd en met het origineel vergeleken. Een back-upmap die een synchronisatieprogramma in de gaten houdt, of die op een USB-stick of een netwerkschijf staat, bevat dus nooit een half geschreven back-up of een tijdelijk bestand, en een back-up die is afgebroken, ziet er nooit uit als een voltooide. (Vóór 0.90.3 werd de back-up in de back-upmap geschreven als *… .zip.part* en daar hernoemd.) De werkmap is leeg tussen back-ups, op een kleine notitie na van de back-upmap die hij bedient.

**Wanneer de back-upmap onbereikbaar is** (een schijf die niet is aangesloten, een map die niet kan worden gemaakt of beschreven, geen ruimte), wordt de back-up toch gemaakt en wacht hij in de werkmap. Een melding zegt dat en noemt beide mappen, en de volgende back-up van het project zet de wachtende back-ups op hun plaats voor zijn eigen. Er wachten er zoveel als er back-ups worden bewaard. **Back-up maken naar…** in een map die niet kan worden beschreven zegt dat gewoon met de reden; kies een andere map.

**Kopieën van een synchronisatieprogramma.** Alleen bestanden die zijn genoemd zoals {{APP}} zijn back-ups noemt (*Title #K7T2M bak 2026-11-20 07-56-13.zip*, met "(2)" of "(incomplete)" waar van toepassing) tellen als back-ups en kunnen door het opruimen worden verwijderd. Andere bestanden in de back-upmap van een project blijven met rust en tellen niet mee: bijvoorbeeld de kopieën die een synchronisatieprogramma maakt wanneer het problemen met een bestand heeft (*… -MyComputer-safeBackup-0001.zip*, *… (conflicted copy).zip*), of een kopie die je met de hand hebt gemaakt. (Vóór 0.90.3 telden zulke kopieën als back-ups van hun datum, en een paar ervan konden de echte oudere back-ups verdringen.) **Herstellen uit back-up…** toont ze nog steeds, zodat je uit een kopie kunt herstellen; verwijder kopieën die je niet nodig hebt zelf.

Grote projecten (meer dan 4 GB, bijvoorbeeld met audio en video) worden ook correct geback-upt (sinds versie 0.20). Een bestand dat niet kan worden gelezen (bijvoorbeeld door zijn rechten, of een bestand van een gesynchroniseerde map dat nog niet op deze computer staat) wordt weggelaten; de back-up wordt toch gemaakt, maar heet *… (incomplete).zip*, een melding noemt de bestanden die ontbreken (ook bij de automatische back-ups wanneer een project opent of sluit, 0.65), en er wordt daarvoor geen oudere back-up verwijderd: het aantal te bewaren back-ups telt alleen volledige back-ups. Welke back-ups de oudste zijn, wordt bepaald door de datum en tijd in hun namen, zodat het hernoemen van het project niets verandert aan welke worden bewaard (0.81; eerder zorgde een nieuwe titel die vóór de oude sorteert ervoor dat elke nieuwe back-up als de oudste gold en meteen werd verwijderd). Na het opruimen controleert het programma dat de nieuwe back-up er nog is. Van onvolledige back-ups blijven de nieuwste tot het aantal te bewaren staan (0.81). Een map in het project die een link is naar een map elders, wordt met zijn bestanden geback-upt (0.81); een link die nergens heen wijst, of terug het project in, maakt de back-up onvolledig. Als het programma door een signaal wordt beëindigd (een shutdown die niet wacht, een kill-opdracht) terwijl **Een back-up maken wanneer een project wordt gesloten** aan staat, is er geen tijd voor de back-up; de volgende keer dat het project opent, wordt de back-up gemaakt en zegt de statusbalk dat (0.81). Het project wordt voor elke back-up opgeslagen; als dat opslaan mislukt (een volle schijf, een map die niet kan worden beschreven), wordt er geen back-up gemaakt en zegt een melding waarom, zodat een back-up nooit je laatste wijzigingen mist zonder dat je het weet. De back-up van een project in Scrivener-indeling bevat de *.scriv* en, ernaast, de werkkopie van {{APP}} (*… - working copy of {{APP}}*), met de trefwoorden, stijlen, objecten en andere dingen die alleen {{APP}} kent.

**Een back-up met de hand herstellen** (bijvoorbeeld op een andere computer): open de back-upmap, pak het ZIP-bestand uit (rechtsklik > Uitpakken) en open het project daarin met **Bestand > Project openen**.

---

## 16. Opties

**Bestand > Opties** (**Ctrl+,**) heeft bovenaan een rij pictogrammen voor de onderdelen, links een lijst met pagina's (of tabbladen) en rechts de opties. Er verandert niets tot je op **OK** of **Toepassen** klikt. Linksonder slaat **Beheren** alle opties op in een bestand en laadt ze weer (**Opties opslaan in bestand...**, **Opties laden uit bestand...**, bijvoorbeeld om je instellingen mee te nemen naar een andere computer; recente projecten en vensterplaatsen staan niet in het bestand), slaat het huidige uiterlijk op als themabestand (een bestand dat er al is, wordt alleen vervangen wanneer het nieuwe volledig is geschreven; als het schrijven mislukt, blijft het oude bestand zoals het was en zegt de melding dat) en toont de thema's; **Standaardwaarden** zet elke optie terug op de standaard (je recente projecten, eigen thema's, indelingen, werkbalken en sneltoetsen blijven).

**Waar elke optie wordt bewaard.** Wijs een optie aan om te zien waar zijn waarde wordt bewaard: zijn tooltip zegt of het open project hem bewaart of het programma hem voor alle projecten bewaart, en of die plaats **vast** is (een instelling die alleen het programma kan bewaren, zoals de taal) of **afhangt van de instelling** **Instellingen voor** bovenaan (en, op de pagina Uiterlijk, **Het uiterlijk tonen en wijzigen van**). (0.90.1 toonde voor elke optie een groene P of een blauwe A; 0.90.2 laat dat aan de tooltip over.) Op een klein scherm schuiven de pagina's, zodat OK, Annuleren en Toepassen in beeld blijven.

Sinds 0.70 opent het venster voor **Alle projecten** (de instellingen van het programma); kies bovenaan **Dit project** om iets alleen voor het open project in te stellen. De eigen instellingen van een project kunnen het opslaan veiliger maken maar nooit minder veilig: de kortere vertraging van het automatisch opslaan telt, een back-up die het programma maakt blijft aan, het grotere aantal back-ups wordt bewaard en het automatisch afsluiten is alleen van het programma. De laatste opslag komt hoogstens 10 seconden na een wijziging, wat de vertraging van het automatisch opslaan ook is.

- **Algemeen**: **Opstarten** (het laatste project heropenen, de bestandsdialogen van het bureaublad, de **Taal**: English of Deutsch, vanaf de volgende start; de handleiding en het welkomstproject blijven Engels; **De cijfers van grote getallen groeperen**: aantallen woorden, doelen en statistieken met een scheidingsteken tussen de duizendtallen, zoals je systeem getallen schrijft (44,825 of 44.825), of 44825 wanneer het niet is aangevinkt); **Opslaan** (interval van het automatisch opslaan, het aantal recente projecten, momentopnamen bij elke keer Bestand > Opslaan); **Auteursgegevens** (je naam, adres, telefoon en e-mail voor titelpagina's, gebruikt door de plaatsvervangers `<$author>`, `<$street>`, `<$city>`, `<$state>`, `<$zip>`, `<$country>`, `<$phone>`, `<$email>`, `<$address>`); de opties van de schrijfhulpmiddelen (**Kladblok**); **Gedeelde sjablonen** (een extra map met projectsjablonen, bijvoorbeeld een die tussen je computers is gesynchroniseerd; de sjablonen verschijnen in Bestand > Nieuw project); **Citaten** (het programma dat **Opmaak > Bibliografie/citaten...** start, bijvoorbeeld Zotero of JabRef); **Scheidingstekens** (wat er tussen samengevoegde documenten en tussen aan een document toegevoegde kladblocknotities komt: een enkele return, een lege regel, een pagina-einde, niets of je eigen tekst); **Automatisch afsluiten** (opslaan en sluiten na een aantal minuten zonder toets- of muisinvoer); **Waarschuwingen** (**Interne logconsole tonen** toont de berichten van het programma van deze sessie, **Alle waarschuwingen herstellen** stelt de vragen opnieuw die met "Niet meer vragen" zijn beantwoord).
- **Bewerken**: **Opties** (de laatste zoom van de editors herladen, de standaardtekstzoom van elk gebied, centimeters of inches en de stap van de liniaal, de toets Insert voor overschrijven, een eigen datumnotatie voor Invoegen > Huidige datum, wat de live telling toont, inline-aantekeningen zonder kleur, opmerkingen en voetnoten die vóór de interpunctie stoppen, een blokinvoegpunt); **Opmaak** (de opmaak en stijlen van nieuwe projecten); **Revisies** (de vijf kleuren van Opmaak > Revisiemodus, uit het gedeelde palet).
- **Gedrag**: **Schrijfmodus** (scherm, hoofdvenster opzij, Esc); **Documentlinks** (waar documentlinks openen, terugverwijzingen); **Dubbelklikken** (een dubbelklik op het prikbord maakt een kaart); **Slepen en neerzetten** (documenten die op tekstdocumenten worden neergezet, de indeling van tekst die naar andere programma's wordt gesleept); **Mappen en bestanden** (mappen openen als prikbord; een tekst met subdocumenten opent als een map); **Afspelen** van audio en video (paragraaf 6); **Snelle naslag** (panelen heropenen met hun project); **Entertoets** (Return beëindigt het bewerken van een samenvatting; Return maakt een nieuwe kaart of rij); **Momentopnamen** (een melding wanneer momentopnamen worden gemaakt).
- **Uiterlijk**: het thema en elf delen van het venster (Algemene interface, Binder, Schrijfmodus, Prikbord, Indexkaarten, Inspector en notities, Hoofdeditor, Overzicht, Gecombineerde tekst, Voortgangsbalken van doelen, Tekstmarkeringen), elk met zijn opties, lettertypen en kleuren. Zie "Thema's, kleuren en lettertypen" hieronder. **Algemene interface** heeft ook het volledige pad van het project in de titelbalk en de grootte van de werkbalkpictogrammen.
- **Correcties**: correcties tijdens het typen (paragraaf 4) en **Spelling** (het woordenboek, woorden met cijfers, de onderstreping, de persoonlijke woordenlijst).
- **Toetsenbord**: de sneltoets van elke opdracht (paragraaf 19).
- **Delen**: **Importeren** (opmerkingen, voetnoten, stijlen, de codering van platte tekst, HTML-bestanden; paragraaf 14), **Exporteren** (de bestandsindeling en notities van Bestand > Exporteren > Bestanden), **Conversie** (hoe *.doc*-bestanden worden gelezen).
- **Back-up**: wanneer een back-up wordt gemaakt (bij het openen, bij het sluiten, bij elke keer Bestand > Opslaan), hoeveel back-ups worden bewaard en waar.
- **Extra's**, voor de eigen toevoegingen van {{APP}}: **Mindmap** (de opties van de borden, de vorm en het uiterlijk van nieuwe notities, de achtergrond van borden zonder eigen kleur in lichte en in donkere thema's, de grootte van pictogrammen op notities en lijnen), **Objecten** (of nieuwe projecten je eigen objectsjablonen krijgen of de standaardsjablonen), **Add-ons en plugins** (de geïnstalleerde samenstelindelingen en schrijfhulpmiddelen, de pluginmappen), **Crashrapporten** (na hoeveel seconden zonder antwoord een vastloprapport wordt geschreven, hoeveel rapporten worden bewaard).
- **Spraak** (met de add-on Spraaktranscriptie): het model, het apparaat, talen, microfoon en pauzes (zie paragraaf 10, *Spraaktranscriptie*). Schrijfhulpmiddelen van latere add-ons kunnen ook een eigen onderdeel meebrengen.

Andere vensters van het programma kunnen de opties op een pagina openen, bijvoorbeeld de pagina Afspelen vanuit de mediaspeler.

Updates en add-ons hebben hun eigen vensters: **Help > Add-ons en updates > Update of add-on installeren…** en **Help > Add-ons en updates > Add-onbeheer…** (paragraaf 17).

Projectspecifieke instellingen (labels, status, sectietypen, het uiterlijk van de tekst, eigen metagegevens, speciale mappen, titel en auteur voor het samenstellen) staan onder **Project > Projectinstellingen**.

### Instellingen voor één project

Bovenaan **Bestand > Opties** kiest **Instellingen voor:** waar wat je wijzigt wordt bewaard:

- **Alle projecten (het programma)**: de instellingen van het programma, voor elk project. Hier opent het venster.
- **Dit project**: het open project bewaart wat je hier wijzigt als eigen instelling (in *settings/options.ini* van zijn map). Het geldt zolang dat project open is; wat het project niet instelt, komt uit de instellingen van het programma. De titel van het venster zegt "Options: This Project" en de regel onder de keuze zegt hoeveel eigen instellingen het project heeft (klik op het getal om ze te zien). **Programma-instellingen gebruiken** verwijdert ze allemaal; de instellingen van het programma gelden dan weer voor het project. Een wijziging buiten de opties, bijvoorbeeld een sneltoets die in het toetsenbordvenster is gewijzigd of een werkbalk die is ingedeeld, gaat naar het project wanneer het project daar al een eigen waarde voor heeft, en anders naar het programma.

Een paar instellingen horen alleen bij het programma en worden niet voor een project getoond: de recente projecten, het heropenen van het laatste project, de bestandsdialogen van het bureaublad, de plaatsen van vensters en de indelingen, de tijden van crashrapporten en het uiterlijk voor nieuwe projecten. Het uiterlijk van een project is altijd zijn eigen geweest: de pagina **Uiterlijk** toont het uiterlijk van het open project of het uiterlijk voor nieuwe projecten (**Het uiterlijk tonen en wijzigen van** bovenaan); met hierboven **Dit project** gekozen toont hij alleen het uiterlijk van het project. **Standaardwaarden** en **Beheren > Opties laden uit bestand…** werken alleen op de instellingen van het programma, dus kies daarvoor **Alle projecten**.

**Welke instellingen voorgaan** (**Algemeen > Opstarten**, voor alle projecten): aangevinkt (de standaard) gaan de eigen instellingen van een project voor en vult het programma de rest aan. Niet aangevinkt gelden altijd de instellingen van het programma: de eigen instellingen van projecten worden niet gebruikt (ze blijven voor later in de projecten) en elk project toont het uiterlijk voor nieuwe projecten.

**Project > Projectinstellingen opslaan in bestand…** bevat de eigen opties van het project (het onderdeel **Opties van dit project**), zodat ze naar een ander project kunnen worden meegenomen.

### Thema's, kleuren en lettertypen

Een thema bepaalt de kleuren, en als het dat wil de lettertypen, van elk gebied van het venster. Kies er een onder **Weergave > Thema's**, of met de themaknop van de werkbalk Thema's (zet de werkbalk aan onder **Weergave > Werkbalken**); de wijziging gaat meteen in, zonder herstart. Acht thema's zijn ingebouwd:

| Thema | Uiterlijk |
|-------|------|
| Standaard (bureaublad volgen) | de kleuren van je bureaublad (het KDE-kleurenschema) |
| Licht | lichtgrijs venster, wit papier |
| Donkere modus | donkergrijs, met donkere indexkaarten |
| Sepia | warm crème en bruin |
| Leisteen | grijs venster, wit papier op een grijs bureau |
| Middernacht | donkerblauw met een amberkleurig accent |
| Oceaan | licht blauwgroen |
| Hoog contrast | wit en geel op zwart |

Add-ons kunnen meer thema's meebrengen (de add-on "Extra thema's" heeft Northern Night, Forest, Graphite, Warm Paper en Rose Dawn).

**De themaknop** (0.90.2, gewijzigd in 0.91 en 0.94). Eén knop, **Thema**, op de werkbalk Thema's, die eerst uit staat (**Weergave > Werkbalken > Thema's** toont hem; vóór 0.94 namen vier knoppen en een lijst een derde van de hoofdwerkbalk in beslag). Zijn menu toont:

- **Licht** en **Donker**: een klik toont dat thema meteen, zoals het is.
- **Mijn look 1** en **Mijn look 2** bewaren twee eigen looks. Een lege kiezen (er staat *leeg* in het menu) slaat de look die je nu ziet (het thema met elke kleur en elk lettertype dat je erbovenop hebt gewijzigd) erin op; een gevulde kiezen brengt zijn look terug. **De huidige look opslaan als Mijn look 1** en **...als Mijn look 2**, aan het einde van het menu, slaan de look die je nu ziet daar op (ze vragen voordat ze een look vervangen). De twee looks zijn gewone thema's: ze staan in de lijst hieronder en onder **Weergave > Thema's**, waar ze ook kunnen worden geëxporteerd of verwijderd (een verwijderde laat zijn item leeg).
- **Alle thema's**, daaronder, met het thema in gebruik aangevinkt. Er een kiezen doet wat **Weergave > Thema's** doet: het open project en nieuwe projecten gebruiken het, en de vraag of je je eigen wijzigingen erbovenop wilt houden komt zoals eerder.
- **Je eigen look gaat nooit verloren door een keuze.** Het menu toont een thema zoals het is, zodat wat je ziet echt verandert. Als kleuren of lettertypen van jezelf boven op het getoonde thema liggen, zouden die daardoor verloren gaan, dus ze worden eerst opgeslagen: in een lege Mijn look zonder vraag (de statusbalk zegt waar), en wanneer beide een look bevatten, vraagt {{APP}} of het in Mijn look 1, in Mijn look 2 moet opslaan, of moet overschakelen zonder op te slaan.

Zoals elk werkbalkonderdeel kan de themaknop worden verplaatst of verwijderd onder **Weergave > Werkbalken > Werkbalken aanpassen**, en de hele werkbalk Thema's kan onder **Weergave > Werkbalken** worden aan- of uitgezet.

**De pagina in donkere thema's** (0.70). De standaardpagina van {{APP}} is licht olijfkleurig papier naast een grijs bureau. Bij een donker thema is de pagina in plaats daarvan zeer donkergrijs met witte tekst (of de eigen donkere pagina van het thema, als het die heeft), zodat een donker thema geen felle pagina in een donker kader achterlaat. Een paginakleur die je zelf hebt gekozen (rechtsklik > Uiterlijk, of Bestand > Opties > Uiterlijk) blijft in elk thema.

**Kleuren en lettertypen wijzigen.** **Bestand > Opties > Uiterlijk** (of **Weergave > Thema's > Kleuren en lettertypen…**) toont links de delen van het venster. Elk toont zijn opties, zijn lettertypen en zijn kleuren (een klik op een kleur toont het gedeelde palet: de labelkleuren, de zes standaardkleuren en **Anders...**):

| Onderdeel | Lettertypen | Kleuren |
|------|-------|--------|
| Algemene interface | menu's, knoppen en panelen | venster, Inspectorachtergrond (de lijsten en tekstvelden van de Inspector), afwisselende rijen, tekst, knoppen, selectie, links, tooltips, randen, grijs gemaakte tekst |
| Binder | Binder | achtergrond, titels |
| Schrijfmodus | (geen) | achtergrond, papier, tekst |
| Prikbord | (geen) | prikbord, statusstempel |
| Indexkaarten | kaarttitels, kaarttekst | indexkaart, titel, tekst, lijn onder de titel, regellijnen |
| Inspector en notities | samenvatting | Inspectorstroken (de tabbladenrij, de koppen, de rij Label en Status), labels, samenvattingskaart en -tekst, notities en hun tekst |
| Hoofdeditor | titel van de kopbalk | papier, tekst, de ruimte naast de tekstkolom, het bureau rond de pagina's (alleen getoond in paginaweergave), de kopbalk (normaal, actieve editor, vastgezet) |
| Overzicht | overzicht | achtergrond, afwisselende rijen, tekst |
| Gecombineerde tekst | titels in de Gecombineerde tekst | (geen) |
| Voortgangsbalken van doelen | (geen) | sessie, manuscript, doel bereikt |
| Tekstmarkeringen | (geen) | links in de tekst, zoekresultaten, spellingsonderstreping, de markeringen van Onzichtbare tekens tonen |

**Elk project houdt zijn eigen uiterlijk.** Het thema van de Binder, editor, het prikbord, overzicht, de Inspector en de schrijfmodus, je eigen kleuren en lettertypen daarvoor, de breedte van de tekstkolom en de rand naast de tekst horen bij het project: ze in het ene project wijzigen laat de andere zoals ze zijn, en een update van {{APP}} verandert nooit hoe een bestaand project eruitziet. De lijst **Het uiterlijk tonen en wijzigen van** bovenaan **Bestand > Opties > Uiterlijk** kiest van wie de pagina het uiterlijk toont en wijzigt: **Dit project** (het open project; eerst gekozen) of **Nieuwe projecten** (het uiterlijk waarmee een nieuw project begint). Wisselen tussen de twee behoudt wat je in elk hebt gewijzigd tot je op **Toepassen** of **OK** drukt. De kleuren van menu's en panelen (**Algemene interface**) en het lettertype van menu's, knoppen en panelen zijn in elk project hetzelfde. Een project dat voor het eerst door versie 0.40.2 of nieuwer wordt geopend, behoudt het uiterlijk dat het tot dan had. **Weergave > Thema's** stelt het thema van het open project en van nieuwe projecten tegelijk in.

**Rechtsklikmenu's.** Overal waar je rechtsklikt, heeft het menu een submenu **Uiterlijk** met de kleuren en lettertypen van wat onder de muis ligt: op de tekst en op het papier ernaast het papier, de tekst, de ruimte naast de kolom, links, zoekresultaten, de spellingsonderstreping, de kolombreedte en de rand; op de kopbalk boven een editor zijn kleuren en lettertype; in de Binder zijn achtergrond, titels, lettertype, inspringing en labelkleuren; op het prikbord het kurk, de indexkaarten en hun lettertypen, statusstempels, de kaartbreedte; in het overzicht zijn achtergrond, rijen, tekst en lettertype; in de Inspector en zijn notities de stroken, labels, samenvattingskaart en notities, en de Inspectorachtergrond, die in elk project hetzelfde is; in de schrijfmodus zijn achtergrond, papier en tekst. Een kleur toont het gedeelde palet en **De kleur van het thema**; **Uiterlijkopties...** opent deze pagina van de opties. Net als de opties wijzigen deze menu's het uiterlijk van het open project (de eerste regel van het submenu zegt dat), of, zonder project, het uiterlijk van nieuwe projecten.

Klik op een kleur om hem te wijzigen, op een lettertypeknop om een lettertype te kiezen. Het kleine plaatje bovenaan toont het resultaat voordat je op **Toepassen** of **OK** drukt. Je wijzigingen liggen boven op het thema: een gewijzigd onderdeel wordt vet getoond, de knop **↺** ernaast geeft het terug aan het thema en **Kleuren en lettertypen terugzetten op het thema** doet dat voor alle. Een kleur die je niet hebt gewijzigd en die het thema niet instelt, volgt de andere; de Binder gebruikt bijvoorbeeld de kleur van lijsten tot je hem er een eigen geeft. De **Inspectorachtergrond** kleurt alleen de lijsten en tekstvelden van de Inspector; de velden op de werkbalken, de liniaal en de lijsten van de opties hebben de kleur van het venster en de menu's.

Het lettertype en de grootte van de tekst die je schrijft, maken geen deel uit van het thema: ze horen bij het project (**Project > Projectinstellingen > Opmaak**, paragraaf 4).

**Eigen thema's.** **Opslaan als thema…** in de opties (of **Weergave > Thema's > Huidig uiterlijk als thema opslaan…**) bewaart het thema samen met je wijzigingen onder een nieuwe naam. Het verschijnt dan net als de andere in **Weergave > Thema's**. **Weergave > Thema's > Thema's beheren…** toont alle thema's: **Dit thema gebruiken**, **Dupliceren…** (ook voor ingebouwde thema's, om een eigen beginpunt te krijgen), **Hernoemen…** en **Verwijderen** (alleen je eigen thema's), **Exporteren…** (schrijft een *.qbtheme*-bestand dat je kunt doorgeven) en **Importeren…** (voegt zo'n bestand toe).

Wanneer je een ander thema kiest terwijl je eigen wijzigingen hebt, vraagt {{APP}} of het die met het nieuwe thema moet behouden of het thema moet gebruiken zoals het is.

Links in je tekst worden met één blauw opgeslagen; op donker papier worden ze in een lichtere tint blauw getoond (**Hoofdeditor > Links in de tekst**). Tekst die je zelf hebt gekleurd, behoudt zijn kleur in elk thema.

---

## 17. Updates en add-ons

**Help > Probleemoplossing en ontwikkeling > Ontwikkelaarsgids: updates en add-ons** legt uit, voor mensen die {{APP}} bouwen of add-ons schrijven, hoe updates en add-ons werken en welke hulpmiddelen ze maken; hij bevat ook wat een technisch aangelegde vriend nodig heeft om dingen in te stellen (media-engines, plugins voor samenstelindelingen, pakket-, thema-, lay-out- en scriptindelingsbestanden, de bestanden in een projectmap) en de omgevingsvariabelen die het programma leest.

{{APP}} is één programmabestand (de AppImage). Updates en uitbreidingen komen als kleine pakketbestanden die eindigen op **.qbpkg**. Er zijn drie soorten, en alle worden op dezelfde manier geïnstalleerd:

- **Programma-update**: een nieuwere versie van {{APP}} zelf.
- **Add-on**: een uitbreiding die je kunt aan- en uitzetten, zoals kleurthema's, projectsjablonen, samenstelindelingen, woordenboeken, schrijfhulpmiddelen (paragraaf 10), het afspelen van audio en video en zijn extra indelingen (paragraaf 6), importers voor oude Word-bestanden en webpagina's (paragraaf 14).

Programma-updates en add-ons staan op de pagina **Releases** van de projectpagina (paragraaf 1), naast de AppImage.
- **Updatebundel**: één bestand dat een programma-update en meerdere add-ons bevat, die samen worden geïnstalleerd.

### Op updates controleren

Sinds 0.92.0 kan {{APP}} zelf naar updates zoeken. Het is het enige waarvoor het programma uit zichzelf contact met internet maakt, en je kunt het uitzetten.

- **Help > Add-ons en updates > Bij het opstarten op updates controleren** is de schakelaar (eerst aan). **Help > Add-ons en updates > Nu op updates controleren** kijkt meteen, wat de schakelaar ook zegt.
- **De eerste start** zegt het op zijn pagina, voordat er contact wordt gemaakt: een vinkvak *Naar updates zoeken wanneer het programma start* (aangevinkt) met de opmerking dat er niets over jou wordt verstuurd; één **OK** beëindigt de pagina. Een programma dat vóór 0.94.0 is gestart en de opmerking niet heeft gezien, toont hem als een venster met **OK** en **Uitzetten**.
- **Wat er gebeurt**: een paar seconden na de start, hoogstens eens per dag, haalt het programma één klein bestand op van de pagina van het project (*shelin-boop.github.io/Quillbinder_Collection/updates-1.json*) en de handtekening van dat bestand. Het verzoek noemt het programma ("{{APP}}") en niets anders: geen versie, geen systeem, geen identificatie, geen cookie. Zoals bij elke verbinding ziet de server (GitHub) je internetadres en de tijd. Het bestand toont de nieuwste versie van het programma en van de add-ons van het project; je programma vergelijkt dat op je computer met wat is geïnstalleerd. Het bestand telt alleen mee wanneer het de handtekening van de releasesleutel van het project draagt, en een bestand dat ouder is dan een eerder gezien bestand wordt genegeerd.
- **Wanneer er een update is**, noemt een venster hem: het programma of de add-on, je versie en de nieuwe, wat er nieuw is en de soort. Een **functionele** update heeft drie knoppen: **Downloaden en installeren**, **Deze versie negeren** (geen herinneringen meer voor die versie) en **Uitstellen**. Uitstellen wacht elke keer langer: 1 dag, 1 week, 2 weken, 4 weken; dan volgt een laatste herinnering, en daarna eindigen de herinneringen voor die versie. Een **aanbevolen (beveiligings)**update heeft maar twee knoppen, **Downloaden en installeren** en **Uitstellen**: hij kan niet worden genegeerd en komt na het vierde uitstel elke 4 weken terug. Wanneer beide soorten er zijn, toont één venster ze en volgt de beveiligingsregels. Het venster komt naast je werk en neemt het toetsenbord niet over.
- **Downloaden en installeren** haalt de bestanden op die de lijst noemt, controleert elk aan de hand van de grootte en controlesom in de ondertekende lijst en installeert het: een add-on of een programma-update zoals **Help > Add-ons en updates > Update of add-on installeren** dat doet (alleen pakketten met de handtekening van het project). Daarna biedt het aan het programma opnieuw te starten.
- **Wanneer de nieuwe versie een nieuwe AppImage nodig heeft** (andere bibliotheken dan je AppImage heeft), downloadt {{APP}} die, zet hem op de plaats van de AppImage die je hebt gestart, waar die ook is en onder dezelfde bestandsnaam, bewaart degene die je had ernaast als *…AppImage.before-update* en start opnieuw. Verwijder het bewaarde bestand wanneer de nieuwe versie voor je werkt. Als de map van je AppImage niet kan worden beschreven, gaat het nieuwe bestand naar je map Downloads en zegt een melding wat je moet doen.
- **Op Windows** (0.92.0) komt zo'n versie als zijn setupprogramma. {{APP}} downloadt het in zijn eigen map (*data\update-setup*), controleert het aan de hand van de ondertekende lijst van het project en vraagt. Met **Sluiten en nu bijwerken** sluit {{APP}}, en het setupprogramma pakt de nieuwe versie uit in de map die je gebruikt, zonder zijn pagina's, en start hem; je instellingen, add-ons en al het andere in *data* blijven. Met **Later** verandert er niets en wordt de update opnieuw aangeboden. Als Windows het setupprogramma niet start (het draagt geen codeondertekeningscertificaat), noemt een melding het bestand en kun je de huidige versie ook downloaden van de pagina van het project.
- **Wanneer het adres niet bereikbaar is**, probeert het programma het twee keer, met 5 seconden ertussen, en toont dan een kleine tekst in de voettekstbalk van de eerste editor, in de plaats van de opslagtekst, die twee keer in- en uitfadet. Geen venster, en hoogstens eens per dag. (**Nu op updates controleren** antwoordt in plaats daarvan met een melding.)
- **Uitgezet** maakt het programma met niets contact, zoals vóór 0.92.0. Voor computers die voor anderen zijn ingericht, voorkomt de omgevingsvariabele `QB_NO_UPDATE_CHECK=1` dat de automatische controle draait, wat de schakelaar ook zegt.
- Een AppImage van een versie vóór 0.92.0 die een nieuwere programma-update draait, kan geen beveiligde verbindingen maken; daar zegt de controle dat wanneer je erom vraagt, en zwijgt hij verder. Download de huidige AppImage één keer.
- Wat het programma hierover onthoudt (de schakelaar, het tijdstip van de laatste controle, wat je hebt genegeerd of uitgesteld) staat in *updates-state.json* in de gegevensmap van het programma.

### Een pakket installeren

1. Kies **Help > Add-ons en updates > Update of add-on installeren…** en selecteer het .qbpkg-bestand, of dubbelklik op het bestand in je bestandsbeheer.
2. {{APP}} toont wat het pakket bevat en wat er nieuw is. Klik op **Installeren**.
3. Als het pakket een programma-update bevat, vraagt {{APP}} om opnieuw te starten. Je open project wordt opgeslagen en heropend.

{{APP}} controleert elk pakket voordat het wordt geïnstalleerd: beschadigde bestanden en pakketten die voor een andere versie zijn gemaakt, worden geweigerd. Een programma-update wordt eenmaal onzichtbaar als test gestart; als hij op je computer niet draait, wordt hij niet geïnstalleerd. De delen van een bundel worden samen geïnstalleerd: wanneer een ervan niet kan worden geïnstalleerd, wordt geen enkel deel geïnstalleerd (0.65).

**Handtekeningen** (0.65). Elk pakket op de releasepagina van het project is ondertekend met de sleutel van het project, en {{APP}} draagt de publieke helft van die sleutel. Het installatievenster zegt **Ondertekend door het Quillbinder-project** bij zo'n pakket: het is het pakket dat het project heeft gepubliceerd, ongewijzigd.

- Een pakket waarvan de handtekening niet klopt (gewijzigd nadat het was ondertekend, of beschadigd) wordt nooit geïnstalleerd.
- Een **programma-update** wordt alleen met de handtekening van het project geïnstalleerd.
- Een **add-on** zonder die handtekening (een die jij of iemand anders heeft gemaakt) wordt pas na een duidelijke waarschuwing geïnstalleerd, met **Annuleren** als standaard. Op de opdrachtregel (`--install <file>`) wordt een pakket zonder de handtekening van het project alleen geïnstalleerd wanneer je `--allow-unsigned` toevoegt.

**Add-onsleutels** (0.91.3). Officiële add-ons, ook die van andere auteurs, kunnen worden ondertekend met een eigen sleutel in plaats van de sleutel van het project. Het project staat voor zo'n sleutel in met zijn eigen sleutel, in een certificaat dat in het pakket zit en de sleutel toestaat add-ons te ondertekenen en niets anders. Het installatievenster noemt dan de sleutel: *Ondertekend met de add-onsleutel "Quillbinder official add-ons", waarvoor het Quillbinder-project instaat*.

- Een **programma-update** die met een add-onsleutel is ondertekend, wordt geweigerd zoals een niet-ondertekende: programma-updates hebben de eigen sleutel van het project nodig.
- Een add-onsleutel waarvoor het project niet instaat, telt als geen handtekening: de add-on wordt pas na de waarschuwing geïnstalleerd.
- Add-ons die met de eigen sleutel van het project zijn ondertekend, zoals alle tot 0.91.2, blijven vertrouwd. Vanaf 0.91.3 worden de eigen add-ons van het project ook met de add-onsleutel ondertekend.
- Versies tot 0.91.2 kennen add-onsleutels niet en waarschuwen voordat ze een add-on installeren die met zo'n sleutel is ondertekend: installeer eerst de programma-update (die is met de eigen sleutel van het project ondertekend), daarna de add-ons.

**Het venster dat vraagt voordat een add-on wordt geïnstalleerd** (0.94.0) zegt eerst, in een of twee zinnen, wat de add-on doet. De technische regels (de bibliotheken die hij meebrengt, de spraak- of grafische engine, de runtime waarvoor hij is gebouwd) staan onder **Details**, dat is ingeklapt tot je het opent.

**Een download zelf controleren.** Elke release bevat ook *SHA256SUMS.txt* (de controlesom van elk bestand), zijn handtekening *SHA256SUMS.txt.sig* en de publieke sleutel *quillbinder-signing-public.pem*. In een terminal, in de map met de downloads:

    openssl pkeyutl -verify -pubin -inkey quillbinder-signing-public.pem -rawin -in SHA256SUMS.txt -sigfile SHA256SUMS.txt.sig
    sha256sum -c --ignore-missing SHA256SUMS.txt

De eerste opdracht antwoordt *Signature Verified Successfully*, de tweede *OK* voor elk bestand dat je hebt gedownload. Om zeker te zijn dat het sleutelbestand zelf van het project is, vergelijk je zijn 32 bytes (`openssl pkey -pubin -in quillbinder-signing-public.pem -outform DER | tail -c 32 | od -An -tx1`) met de sleutel in de broncode van het programma (*src/core/Signing.cpp* en *packaging/quillbinder-signing-public.pem*): ze beginnen met *bc 3c 58 00 08 3f 66 e6*.

### Het Add-onbeheer

**Help > Add-ons en updates > Add-onbeheer…** toont wat is geïnstalleerd.

- **Programmaversie**: de versie die in je AppImage is ingebouwd en elke geïnstalleerde update. Degene die is gemarkeerd als *nu actief* is in gebruik. Om naar een eerdere versie terug te gaan, selecteer je hem en klik je op **Deze versie gebruiken**, en start daarna opnieuw. Geïnstalleerde updates die je niet meer nodig hebt, kunnen worden verwijderd.
- **Add-ons**: haal het vinkje bij een add-on weg om hem uit te zetten zonder hem te verwijderen; **Add-on verwijderen** wist hem. Thema's, sjablonen en samenstelindelingen van een add-on verschijnen op de gebruikelijke plaatsen: **Weergave > Thema's**, het venster **Nieuw project** en het venster **Samenstellen**.
- Wanneer een keuze die hier is gemaakt niet kan worden geschreven (een volle schijf, een map die niet kan worden beschreven), zegt een melding dat met de naam van het bestand (0.92.0). Eerder toonde het venster de keuze als gemaakt en werd die bij de volgende start zonder een woord ongedaan gemaakt. Hetzelfde geldt voor een installatie: een add-on of een programmaversie waarvan het item niet kon worden geschreven, wordt als niet geïnstalleerd gemeld, met wat je moet doen. Wanneer een installatie halverwege mislukt en de eerder geïnstalleerde versie niet kan worden teruggezet, noemt de melding de map waarin hij staat en de naam die je die map moet geven.

### Als een update niet start

Als een programma-update twee keer achter elkaar niet start, zet {{APP}} hem zelf uit en start de versie die in de AppImage is ingebouwd, met een melding die dat zegt. Er verandert hierdoor niets in je projecten.

### Wanneer een nieuwe AppImage nodig is

Programma-updates draaien op de bibliotheken in je AppImage. Af en toe heeft een grotere update een nieuwe AppImage nodig; het installatievenster zegt het wanneer een pakket niet bij het jouwe past. Vervang het oude AppImage-bestand door het nieuwe; je instellingen, add-ons en projecten blijven zoals ze zijn.

0.80.0 is zo'n versie: zijn AppImage bevat een nieuwe Qt (de eigen build van Qt 6.12 van The Qt Company in plaats van de Qt 6.4.2 van Ubuntu). Add-ons met programmacode (*Media Playback*, *Extra Video and Audio Formats*, *Speech Transcription*, de schrijfhulpmiddelen en de importers) moeten opnieuw worden geïnstalleerd in hun versies voor 0.80.0 (de bundel en **media-all-formats.qbpkg** op de pagina Releases brengen ze allemaal mee); tot dan markeert het Add-onbeheer de oude als gebouwd voor een oudere versie van het programma, en wordt hun code niet geladen. De volledige versie van 0.80.0 brengt elke add-on mee.

0.93.0 is er nog een (Linux): zijn AppImage bevat Qt 6.12.1, waarvan de ingebouwde bibliotheken (zlib, HarfBuzz, PCRE2, libpng, libjpeg-turbo, FreeType en andere) nieuwer zijn en veiligheidsfixes hebben. Een programma-update kan ze niet meebrengen, dus de controle op updates biedt de nieuwe AppImage aan. De add-ons met programmacode van de eerdere AppImage (de hierboven genoemde) worden op dezelfde manier opnieuw geïnstalleerd in hun versies voor 0.93.0, en de media-add-ons bevatten nu FFmpeg 9.1.

Een nieuwe AppImage brengt ook een eigen nieuwe programmaversie mee (bijvoorbeeld de AppImage 0.20.0). Hij start die versie, ook als je updates in de oude AppImage hebt geïnstalleerd: oudere updates worden uitgezet en blijven vermeld onder **Help > Add-ons en updates > Add-onbeheer…**, waar je ze kunt verwijderen. Als het nieuwe bestand een andere naam heeft of in een andere map staat dan het oude, kies dan eenmaal **Help > Aan toepassingsmenu toevoegen**, zodat het menu-item en dubbelgeklikte projecten het nieuwe bestand gebruiken.

---

## 18. Het Markdown-dialect

Je hoeft dit niet te weten om {{APP}} te gebruiken; de editor toont opgemaakte tekst. Het doet er alleen toe als je de tekstbestanden met een ander programma opent. {{APP}} schrijft Markdown met deze regels:

- **Elke regel is één alinea**, zoals in een tekstverwerker. Een lege regel is een lege alinea.
- Koppen: `# ` tot `###### ` aan het begin van de regel.
- Bloksitaat: `> `. Gecentreerde alinea: `->text<-`. Elke andere alinea-stijl: `::Style Name:: text`.
- Tekenstijlen (vanaf versie 0.4): `<span class="Style Name">text</span>`. Alleen de namen van stijlen worden in de tekst geschreven. Hoe een stijl eruitziet, wordt één keer in het projectbestand bewaard, zodat een gewijzigde stijl geen wijziging in de documenten nodig heeft.
- Lijsten: `- ` en `1. `; twee spaties per nestniveau.
- `**bold**`, `*italic*`, `~~strikethrough~~`, `==highlight==`, `<u>underline</u>`, `<sup>superscript</sup>`, `<sub>subscript</sub>`.
- Revisiekleuren: `<rev1>…</rev1>` tot `<rev5>…</rev5>`.
- Links: `[text](https://example.com)`; links naar documenten in het project: `[text](qb://ID)`.
- Afbeeldingen: `![](images/picture.png){width=300}` (de afbeeldingen worden in de map van het document bewaard). De beschrijving van een afbeelding (0.62) is de tekst tussen de haakjes: `![A brass lamp](images/lamp.png){width=300}`.
- Tabellen (0.62): een regel `:::table` met het ontwerp tussen accolades (JSON), één regel per rij met de cellen tussen `|` (de alinea's van een cel gescheiden door `<p/>`, een cel die door een samengevoegde cel wordt bedekt blijft leeg, een `|` in de tekst geschreven als `\|`) en een regel `:::` aan het einde.
- Een afbeelding naast de tekst of gecentreerd (0.62): `:::figure {"pos":1,"width":168}` (1 links, 2 rechts, 3 gecentreerd), de regel van de afbeelding, de regels van het bijschrift en `:::`.
- Een regeleinde binnen een alinea: `<br>`.
- Met de hand ingestelde lettertypen (vanaf versie 0.3): `<span style="font-family:'DejaVu Sans'; font-size:14pt; color:#aa0000">text</span>`. De declaraties zijn gewone CSS, dus andere Markdown-programma's die HTML toestaan, tonen ze ook. Mogelijke items: `font-family`, `font-size` (pt), `font-weight` (100 tot 900), `color`, `letter-spacing` (em), `font-variant:small-caps`, `text-transform:uppercase`.
- Met de hand ingestelde alinea-opmaak (vanaf versie 0.3): een lijst tussen accolades aan het begin van de regel, vóór elk ander voorvoegsel: `{: align=right left=10mm right=5mm first=0mm before=6pt after=12pt line=1.5 tabs=20mm,50mmR} text`. `line` is een veelvoud (`1.5`), een exacte hoogte (`14pt`) of een minimum (`14pt+`). Tabstops eindigen op `R` (rechts), `C` (gecentreerd) of `D` (decimaalteken); geen letter betekent links. Alleen de items die je hebt ingesteld worden geschreven.
- Scriptelementen (vanaf versie 0.10) zijn alinea-stijlen die naar het element zijn genoemd: `::Script > Scene Heading:: INT. LIGHTHOUSE - NIGHT`. Hoe een element eruitziet, wordt één keer in het projectbestand bewaard, met de scriptindeling.
- Een pagina-einde vóór een alinea (van geïmporteerde scripts en RTF-bestanden, vanaf versie 0.10): `{: page=before} text`.
- Versies vóór 0.62 tonen tabellen en figuren als tekstregels (`:::table` ...). Als je naar een oudere versie teruggaat, bewerk dan geen documenten met tabellen of afbeeldingen naast de tekst.
- Versie 0.2 toont beide markeringen als platte tekst. Als je met het Add-onbeheer naar versie 0.2 teruggaat, bewerk dan geen documenten die ze bevatten.
- Versies vóór 0.4 kennen je eigen stijlen niet: ze tonen `<span class=…>` als platte tekst en behandelen alinea's van je eigen stijlen als lopende tekst. Als je naar een oudere versie teruggaat, bewerk dan geen documenten die je eigen stijlen gebruiken.
- Versies vóór 0.10 behandelen scriptelementen als lopende tekst en verwijderen de namen van de elementen uit een script zodra het in de editor is getoond en het project wordt opgeslagen. Als je naar een oudere versie teruggaat, open je scripts daar dan niet (zie paragraaf 20).
- Een backslash maakt van het volgende teken gewone tekst: `\*` is een sterretje, geen cursief.
- Notities volgen de CriticMarkup-conventie:
  - `{>>inline annotation<<}`
  - `^[inline footnote]`
  - `{==text==}{>>linked comment<<}`
  - `{==text==}^[linked footnote]`
  - Een opmerking kan met een kleur beginnen: `{>>@#ff8800 text<<}`.

**Bestand > Markdown ⇄ RTF-bestand omzetten…** leest zowel dit dialect als standaard-Markdown (waarin alinea's door lege regels worden gescheiden), en vraagt bij het schrijven van Markdown welke van de twee je wilt.

---

## 19. Sneltoetsen

### Sneltoetsen wijzigen

**Bestand > Opties > Toetsenbord** (of **Help > Sneltoetsen**) toont elke opdracht, gegroepeerd per menu, met zijn sneltoets en de standaardsneltoets waarmee het programma kwam. De stijlen van het open project en de lay-outs staan er ook in, zodat een stijl of een lay-out een eigen sneltoets kan hebben.

1. Zoek de opdracht: scrol, of typ een deel van zijn naam of een sneltoets zoals "Ctrl+K" in het zoekveld.
2. Klik op de opdracht, klik in het sneltoetsveld onder de lijst en druk op de nieuwe toetsen.
3. Als een andere opdracht die sneltoets heeft, wordt gevraagd of je hem daar wilt afnemen. De andere opdracht heeft dan geen sneltoets tot je haar een nieuwe geeft.

**Geen sneltoets** verwijdert de sneltoets van de geselecteerde opdracht, **Standaard** geeft de sneltoets terug waarmee ze kwam en **Alles naar standaard** maakt al je wijzigingen ongedaan. Je wijzigingen worden vet getoond. Een sneltoets heeft Ctrl, Alt of Meta nodig, of moet een functietoets zijn, omdat gewone letters nodig zijn om te schrijven. **Exporteren** en **Importeren** slaan je set sneltoetsen op als bestand (*.qbkeys*) en laden er een, bijvoorbeeld om hem naar een andere computer mee te nemen.

Sneltoetsen van stijlen horen bij de naam van de stijl: "Block Quote" heeft zijn sneltoets in elk project dat een stijl met die naam heeft.

### Sneltoetsen en de toetsenbordindeling

Een paar van de standaardsneltoetsen zijn toetsen die op een Duits toetsenbord **AltGr** nodig hebben: **Ctrl+{**, **Ctrl+|** en **Ctrl+}** (links uitlijnen, centreren, rechts uitlijnen), **Ctrl+[** en **Ctrl+]** (terug en vooruit), **Ctrl+\\** (de lijst met scriptelementen) en **Ctrl+=** (subscript). Sinds 0.94.0 kijkt het programma **bij de allereerste start** (wanneer er geen sneltoetsinstellingen en geen eerdere start zijn) naar de toetsenbordindeling van het systeem en bewaart het antwoord. Bij een **Amerikaanse indeling**, of elke andere indeling dan Duits, blijft de gebruikelijke set hierboven. Bij een **Duitse (of Oostenrijkse) indeling** krijgen deze opdrachten toetsen die geen AltGr nodig hebben:

| Opdracht | Duitse set |
|---------|------------|
| Links uitlijnen / centreren / rechts uitlijnen (ook in de mindmap) | Alt+Shift+L / Alt+Shift+E / Alt+Shift+R |
| Uitvullen (mindmap) | Alt+Shift+J |
| Terug / vooruit | Alt+Shift+B / Alt+Shift+F |
| Lijst met scriptelementen | Ctrl+Shift+O |
| Subscript | Ctrl+Shift+D |
| Naar voren halen / naar achteren sturen (mindmap) | Ctrl+Shift+PageUp / Ctrl+Shift+PageDown |

De menu's tonen elke toets zoals de indeling hem noemt. Een programma dat vóór 0.94.0 is gebruikt, behoudt zijn sneltoetsen zoals ze waren (er verandert niets uit zichzelf), en dat geldt ook voor elke sneltoets die je zelf hebt ingesteld onder **Bestand > Opties > Toetsenbord**: een opgeslagen sneltoets wordt nooit vervangen. Waar het systeem zijn indeling niet doorgeeft, wordt de gebruikelijke set gebruikt. (Linux: de toetsenbordinstellingen van het bureaublad of de X-server; Windows: de taal van de toetsenbordindeling; Android: de taal van het toetsenbord.)

### De standaardsneltoetsen

| Actie | Sneltoets |
|--------|----------|
| Nieuw project / project openen | Ctrl+Shift+N / Ctrl+O |
| Opslaan / project sluiten / afsluiten | Ctrl+S / Ctrl+W / Ctrl+Q |
| Nieuw document / nieuwe map | Ctrl+N / Alt+Shift+N |
| Hernoemen / dupliceren | F2 / Ctrl+D |
| Item omhoog / omlaag / naar buiten / naar binnen verplaatsen | Ctrl+Alt+Up / Down / Left / Right |
| Splitsen bij selectie / met selectie als titel | Ctrl+K / Ctrl+Shift+K |
| Documenten samenvoegen | Ctrl+Shift+M |
| Gecombineerde tekst / prikbord / overzicht | Ctrl+1 / Ctrl+2 / Ctrl+3 |
| Binder / Inspector tonen of verbergen | Ctrl+Alt+B / Ctrl+Alt+I |
| Schrijfmodus / volledig scherm | F11 / Ctrl+Shift+F11 |
| Focus op Binder / focus op editor / andere editor | Alt+1 / Alt+2 / F6 |
| Terug / vooruit | Ctrl+[ / Ctrl+] (Duitse indeling: Alt+Shift+B / Alt+Shift+F) |
| Vorig / volgend document | Alt+Shift+Up / Alt+Shift+Down |
| In de Binder tonen | Ctrl+Shift+R |
| Zoeken / vervangen / volgende / vorige | Ctrl+F / Ctrl+H / F3 / Shift+F3 |
| Projectzoekopdracht | Ctrl+Shift+F |
| Selectie gebruiken om te zoeken | Ctrl+E |
| Zoeken op opmaak / volgende / vorige | Ctrl+F3 / Ctrl+G / Ctrl+Shift+G |
| Kladblok (add-on) | Alt+Shift+Return; vanuit elk programma: zie paragraaf 10 |
| Audio en video (add-on): afspelen / pauzeren | Ctrl+Return |
| Audio en video: terugspoelen / vooruitspoelen | Ctrl+Alt+, / Ctrl+Alt+. |
| Audio en video: tijdstempel invoegen | Ctrl+Shift+Return |
| Prikbord of overzicht filteren | Ctrl+F (terwijl een van beide het toetsenbord heeft) |
| Koppelen aan document | Ctrl+L |
| Opmerking / voetnoot | Shift+F2 / Ctrl+Shift+F2 |
| Inline-aantekening / inline-voetnoot | Ctrl+Shift+A / Ctrl+Alt+F |
| Momentopname maken / met titel | Ctrl+5 / Ctrl+Shift+5 |
| Projectdoelen / projecttrefwoorden | Ctrl+Shift+T / Ctrl+Alt+K |
| Bestanden importeren / samenstellen | Ctrl+Shift+I / Ctrl+Shift+E |
| Huidig document afdrukken | Ctrl+P |
| Lettertypevenster / alineavenster | Ctrl+T / Ctrl+Alt+P |
| Lettergrootte groter / kleiner | Ctrl+> / Ctrl+< |
| Opmaak kopiëren / opmaak plakken | Ctrl+Alt+C / Ctrl+Alt+V |
| Plakken met opmaak / plakken en stijl aanpassen | Ctrl+Alt+Shift+V / Ctrl+Shift+V |
| Geen stijl / Kop 1 / Kop 2 / Kop 3 | Ctrl+Alt+0 / Ctrl+Alt+1 / Ctrl+Alt+2 / Ctrl+Alt+3 |
| De volgende zes stijlen van de stijllijst (Opmaak > Stijl > Stijllijst aanpassen) | Ctrl+Alt+4 ... Ctrl+Alt+9 |
| Naar de samenvatting gaan / naar de documentnotities gaan (Navigeren > Inspecteren) | Ctrl+Alt+Y / Ctrl+Alt+N |
| Nu back-up maken | Ctrl+Shift+B |
| Scriptmodus / lijst met scriptelementen | Ctrl+8 / Ctrl+\\ (Duitse indeling: Ctrl+8 / Ctrl+Shift+O) |
| In een script: volgend element / element wijzigen | Enter / Tab (zie paragraaf 4, Scriptschrijven) |
| Inzoomen / uitzoomen / zoom herstellen | Ctrl++ / Ctrl+- / Ctrl+0 |
| Het gebied onder de muis zoomen | Ctrl+muiswiel |
| Liniaal tonen of verbergen | Ctrl+Alt+R |
| Typemachinescrollen | Meta+Ctrl+T |
| Opties | Ctrl+, |
| Deze handleiding | F1 |

---

## 20. Probleemoplossing

### Crashrapporten en systeeminformatie

Als {{APP}} door een fout eindigt, schrijft het een **crashrapport** op je computer voordat het sluit. Als zijn venster 30 seconden niet reageert (het hangt; de tijd stel je in onder **Bestand > Opties > Extra's > Crashrapporten**), schrijft het een **vastloprapport** dat laat zien waar het vastzit; als het herstelt, zegt het rapport na hoe lang. Er wordt niets ergens heen gestuurd. De volgende keer dat je {{APP}} start na een crash, of nadat je een vastgelopen {{APP}} moest beëindigen, vertelt een melding wat er is gebeurd. Als het binnen een minuut na de start eindigde, biedt de melding **Starten zonder het laatste project** aan, voor het geval het project de oorzaak is.

- **Help > Probleemoplossing en ontwikkeling > Crashrapporten…** toont de crash- en vastloprapporten met hun datum en soort fout ("freeze (recovered)" wanneer {{APP}} doorwerkte). **Kopiëren naar klembord** kopieert het geselecteerde, zodat je het in een bericht kunt plakken aan wie {{APP}} voor je beheert; **Map openen** toont de bestanden; **Verwijderen** en **Alles verwijderen…** wissen ze. De nieuwste 20 rapporten blijven bewaard (**Bestand > Opties > Extra's > Crashrapporten** wijzigt het aantal).
- **Bestand > Opties > Algemeen > Waarschuwingen > Interne logconsole tonen** toont de berichten van het programma en van Qt sinds het is gestart (de laatste 1000), met nieuwe zodra ze komen; handig wanneer iets niet werkt en geen enkele melding zegt waarom.
- **Help > Probleemoplossing en ontwikkeling > Systeeminformatie…** toont dezelfde momentopname van het programma en de computer die een rapport bevat. Hij begint met **Dingen om naar te kijken**: wat binnenkort problemen kan geven, zoals een bijna volle of alleen-lezen schijf, een project in een netwerkmap, een back-upschijf met minder vrije ruimte dan het project nodig heeft, weinig vrij geheugen, een lange tijd voor automatisch opslaan, uitgezette automatische back-ups, of recente crash- en vastloprapporten. Dan volgen de programmaversie, de runtime (de set bibliotheken in de AppImage, met de releasedag van zijn Qt en hoe oud die is: een oude runtime betekent dat een nieuwere AppImage veiligheidsfixes kan brengen) en geïnstalleerde updates en add-ons, het besturingssysteem, bureaublad, schermen (sinds 0.93.1 met het gebied dat een venster op elk kan gebruiken), de invoerapparaten (toetsenbord, muis, aanraakscherm, pen) en of het schermtoetsenbord wordt getoond (0.93.1), processor en geheugen, de mappen met hun vrije ruimte, de instellingen voor opslaan en back-ups en het open project. Gebruik **Kopiëren naar klembord** of **Opslaan als…** wanneer je een probleem meldt dat geen crash is.

Een rapport noemt bestanden, mappen en documenten, en de opdrachten en toetsen die kort voor de fout zijn gebruikt (getypte letters worden alleen geteld, nooit opgenomen). Het bevat geen tekst van je documenten. Je thuismap wordt geschreven als **~**.

Als {{APP}} van buitenaf is gestopt of de computer is uitgezet zonder het te sluiten, is er geen rapport, maar de volgende start vertelt het ook. In beide gevallen is je werk opgeslagen tot de laatste automatische opslag, een paar seconden voor het einde; kijk in de momentopnamen van het document of de nieuwste back-up als er iets ontbreekt (paragraaf 11 en 15).

**"Het projectbestand kan niet worden gelezen."** Als het projectbestand (*JeTitel*{{FILE}}) beschadigd is, bijvoorbeeld door een crash van de computer tijdens het opslaan, biedt {{APP}} de kopie aan die het ervan bewaart (van de laatste keer dat het project opende): **De kopie gebruiken**. Het beschadigde bestand blijft ernaast bewaard (*… .damaged-<datum>*). Wijzigingen in de Binder sinds toen (titels, volgorde, labels) ontbreken; de teksten zijn er allemaal, en documenten die sindsdien zijn gemaakt, komen in de map **Hersteld documenten** in Research, genoemd naar hun eerste regel.

**"Het projectbestand bevatte tekens die erin niet zijn toegestaan."** Versies vóór 0.20 konden onzichtbare stuurtekens opslaan (vanuit andere programma's in een titel of samenvatting geplakt), waarna het project niet meer opende. {{APP}} verwijdert ze wanneer het project opent en zegt dat; er verandert verder niets. Sinds 0.20 worden ze nooit opgeslagen.

**Een document wordt leeg getoond, met de opmerking "Het bestand van dit document kan niet worden gelezen".** Het tekstbestand is er maar kan niet worden gelezen (de rechten zijn gewijzigd, of de schijf waarop het staat is verdwenen). {{APP}} toont het leeg en laat je het niet bewerken, zodat het bestand nooit wordt overschreven. Controleer de rechten van het bestand of de schijf, sluit het project en open het opnieuw.

**Met Page Down of Page Up sloot het programma.** Dit was een fout in versies vóór 0.19; werk bij naar 0.19 of nieuwer. Scrol tot dan met de muis of de schuifbalk.

**Het programma start niet wanneer ik erop dubbelklik.** Rechtsklik op het bestand > Eigenschappen > Rechten en vink "Is uitvoerbaar" aan (in Dolphin: "Bestand als programma uitvoeren toestaan"). Dubbelklik er dan opnieuw op.

**De bestandsdialogen zien er anders uit dan in andere KDE-programma's.** Zet **De bestandsdialogen van het bureaublad gebruiken** aan onder **Bestand > Opties > Algemeen** en start {{APP}} opnieuw.

**Werkbalkpictogrammen zijn slecht te zien in een donker of licht thema.** {{APP}} kiest de lichte of donkere set pictogrammen van je bureaublad die bij het thema past. Als het bureaublad geen passende set heeft, gebruikt het zijn eigen pictogrammen. Als losse pictogrammen nog steeds niet passen, kies dan **Standaard (bureaublad volgen)** onder **Weergave > Thema's** en stel de kleuren in in de eigen instellingen van het bureaublad.

**Ik heb kleuren gewijzigd en wil die van het thema terug.** **Bestand > Opties > Uiterlijk > Kleuren en lettertypen terugzetten op het thema**, of de knop **↺** naast een enkele kleur of een enkel lettertype.

**Een programma-update is niet gestart.** {{APP}} zet een falende update zelf uit (paragraaf 17). Je kunt proberen hem opnieuw te installeren, of de ingebouwde versie blijven gebruiken.

**Een samenstelindeling ontbreekt.** Controleer **Bestand > Opties > Plugins**: daar staat welk programma de indeling nodig heeft. Installeer het in Discover en start {{APP}} opnieuw.

**De spellingcontrole kent mijn taal niet.** Installeer het bijbehorende *hunspell*-woordenboekpakket in Discover en kies het onder **Bestand > Opties > Bewerken**.

**Het project "lijkt in een ander venster of op een andere computer open te staan."** Dit gebeurt wanneer het project in een gesynchroniseerde map staat die elders open is, of na een crash op een andere computer. Als je zeker weet dat het nergens anders open is, kies je **Ja**.

**Alt+slepen opent geen kopijhouder.** Sommige bureaubladen gebruiken Alt+slepen voor iets eigens, zoals het verplaatsen van vensters. Gebruik in plaats daarvan **Navigeren > Openen > in kopijhouder** of het rechtsklikmenu van het document, of wijzig de toets in de vensterinstellingen van het bureaublad.

**Na terugkeren naar een oudere programmaversie is een script platte tekst zonder zijn lay-out.** Versies vóór 0.10 kennen scriptelementen niet en slaan een script dat ze hebben getoond op als gewone tekst. Schakel weer over naar de huidige versie (**Help > Add-ons en updates > Add-onbeheer**). De tekst is er helemaal; om de lay-out terug te krijgen, herstel je het document uit een momentopname of een eerder gemaakte back-up (paragraaf 11 en 15), of geef je de alinea's hun elementen opnieuw met het pop-upmenu in de voettekst.

**De sneltoets van het kladblok doet niets in andere programma's.** Sneltoetsen die zijn ingesteld in **Bestand > Opties > Toetsenbord** werken alleen binnen {{APP}}. Voor andere programma's moet het bureaublad de opdracht starten (paragraaf 10, *Vanuit elk programma*). Als je de AppImage verplaatst, voer de opdracht dan opnieuw in (hij noemt de plaats van de AppImage).

**De camera van het kladblok zegt dat hij het scherm niet kan lezen.** Onder Wayland mag alleen het eigen schermafbeeldingsprogramma van het bureaublad dat. Installeer *Spectacle* (`sudo apt install kde-spectacle`) en probeer het opnieuw.

**Bewerken > Schrijfhulpmiddelen zegt "Geen schrijfhulpmiddelen geïnstalleerd".** De hulpmiddelen zijn add-ons: installeer het pakket van update 0.11 of zet ze aan in **Help > Add-ons en updates > Add-onbeheer**.

**Een audio- of videobestand biedt alleen "Openen met standaardtoepassing".** Afspelen binnen {{APP}} heeft de add-on *Media Playback* nodig (in het pakket van update 0.12); installeer hem, of zet hem aan in **Help > Add-ons en updates > Add-onbeheer**. De regel onder de knop zegt wat ontbreekt.

**Een video wordt afgespeeld maar er is geen geluid, of een audiobestand zegt "Geen geluidsuitvoer gevonden".** {{APP}} speelt geluid af via de geluidsserver van het bureaublad (PipeWire of PulseAudio). Controleer of geluid in andere programma's werkt en of de juiste uitvoer is gekozen in de geluidsinstellingen van het bureaublad; {{APP}} volgt een wijziging van de uitvoer meteen.

**"Dit bestand kan hier niet worden afgespeeld: het gebruikt H.264 en AAC ..."** Het bestand heeft een indeling met octrooilicenties, die *Media Playback* weglaat. Installeer de add-on **Extra Video and Audio Formats**, of het pakket **media-all-formats.qbpkg**, dat hem samen met *Media Playback* brengt (beide op de pagina Releases van de projectpagina, paragraaf 1), en start {{APP}} opnieuw.

**Een bestand wordt niet afgespeeld, of zonder beeld.** Sommige soorten maken geen deel uit van de ingebouwde decodering (AV1-video, WMA Pro, WMV 9). Open het bestand met een ander programma (rechtsklik op de speler: **Openen met standaardtoepassing**), of zet het om naar een andere indeling.

**Een .doc-bestand wordt als bestand toegevoegd, niet als tekst.** De add-on **Word 97–2003 Import** is niet geïnstalleerd of staat uit (**Help > Add-ons en updates > Add-onbeheer**); start {{APP}} na het installeren opnieuw. Een melding na de import noemt wat de add-on heeft weggelaten. Word 6.0- en 95-bestanden hebben LibreOffice nodig (`sudo apt install libreoffice-writer`).

**Een webpagina komt leeg binnen of met het verkeerde deel.** Haal het vinkje bij **Alleen het artikel** in het venster Webpagina weg om de hele pagina te nemen en verwijder wat je niet nodig hebt. Pagina's die een login nodig hebben of volledig door scripts worden opgebouwd, kunnen zonder browser niet worden gelezen: open de pagina in je browser, selecteer de tekst en sleep hem naar de Binder (er ontstaat een nieuw document). "Beveiligde webpagina's kunnen niet worden opgehaald" betekent dat OpenSSL 3 ontbreekt (`sudo apt install libssl3`, onderdeel van elke Ubuntu) of dat de add-on beschadigd is (installeer hem opnieuw).

**De eerste seconde van een bestand is stil of de tijd staat eerst stil.** De geluidsuitvoer heeft even nodig om te starten; bij sommige geluidsservers meer dan een seconde. Spoel een stukje terug (Ctrl+Alt+,) als je het begin hebt gemist.

**De pagina's in de paginaweergave komen niet overeen met mijn afgedrukte manuscript.** Dat is ook niet de bedoeling: de paginaweergave toont je tekst op vellen terwijl je schrijft. De samenstelindeling bepaalt de pagina's van het afgeronde bestand (paragraaf 13).

**"Scrivener heeft dit project open."** Scrivener laat een vergrendelingsbestand in de *.scriv* achter terwijl het het project open heeft, en na een crash. Sluit het project in Scrivener (op elke computer) en open het hier opnieuw; als Scrivener nergens draait, kies je **Ja**.

**Een project in Scrivener-indeling toont mijn laatste wijzigingen uit Scrivener niet.** {{APP}} leest de *.scriv* alleen opnieuw wanneer het project wordt geopend. Sluit het hier (**Bestand > Project sluiten**), laat Scrivener of de synchronisatiedienst klaar worden en open het opnieuw.

**De menu's Invoegen, Navigeren, Project, Documenten en Opmaak zijn grijs.** Er is geen project open. Open of maak er een (menu Bestand).

**Waar je een probleem meldt.** **Help > Een probleem melden…** opent de issuepagina van het project (https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose) in je browser, met een formulier voor een probleem en een voor een wens; je hebt een GitHub-account nodig en je kunt in het Engels of Duits schrijven. Issues zijn openbaar: voeg je manuscript of je project niet bij; **Help > Probleemoplossing en ontwikkeling > Systeeminformatie…** (**Kopiëren naar klembord**) bevat wat een melding over het programma en de computer nodig heeft. Een **beveiligingsprobleem** meld je daar niet maar privé, zoals beschreven in *SECURITY.md* van het project (het tabblad Security van de repository, **Report a vulnerability**).

**De inhoudsopgave van een Word-bestand heeft geen paginanummers.** De paginanummers worden door de tekstverwerker ingevuld wanneer die het veld bijwerkt; werk het daar bij.

**Er is iets misgegaan met een document.** Kijk in het tabblad Momentopnamen van de Inspector, of herstel een back-up (paragraaf 15).

## Credits en licentie

- Hoofdontwikkelaar: Claude.AI w. Opus 5.5 Extra

{{APP}} is vrije software onder de Apache License 2.0: iedereen mag het gebruiken, kopiëren, wijzigen en doorgeven, met of zonder vergoeding, mits de licentie en het NOTICE-bestand, dat het project en zijn broncode noemt, behouden blijven; zonder garantie. Versies tot en met 0.70.0 werden gepubliceerd onder de Zero-Clause BSD-licentie (0BSD). Het venster Over (in het menu Help) toont de versie en de licentie.

{{APP}} gebruikt onderdelen van andere projecten (Qt, Hunspell, ICU, de Breeze-pictogrammen en de andere bibliotheken in de AppImage), elk onder zijn eigen licentie. **Help > Licenties van derden…** (ook een knop van het venster Over) toont ze met hun volledige licentieteksten; dezelfde teksten staan in de AppImage, in *usr/share/doc/quillbinder/licenses*. De broncode van elk, precies in de gebruikte versie, staat in de release **Third-party source code** van de projectpagina (https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources); het venster noemt het bestand voor elke bibliotheek, en de licentieopmerkingen van de add-ons doen hetzelfde voor die van hen.

Scrivener en Scapple zijn namen van producten van Literature & Latte. {{APP}} is een onafhankelijk programma, niet gemaakt door of verbonden met Literature & Latte; het leest en schrijft hun bestandsindelingen zodat je projecten tussen de programma's kunnen worden verplaatst.
