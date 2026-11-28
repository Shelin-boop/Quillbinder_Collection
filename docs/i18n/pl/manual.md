# Podręcznik użytkownika {{APP}}

Ten podręcznik został przetłumaczony maszynowo i nie został jeszcze sprawdzony przez osobę władającą językiem polskim. Tam, gdzie tekst jest niejasny, wzorcem jest podręcznik angielski.

{{APP}} to studio pisarskie do długich tekstów: powieści, scenariuszy, sztuk teatralnych, scenariuszy komiksów, prac dyplomowych i książek non-fiction. Piszesz małymi kawałkami (scenami, rozdziałami, notatkami), układasz je dowolnie, a na końcu *kompilujesz* w jeden rękopis.

Ten podręcznik opisuje wersję {{VERSION}}. Naciśnij **F1** w dowolnej chwili, aby go otworzyć; otworzy się na fragmencie pasującym do tego, na co patrzysz: dokumentu, tablicy korkowej, konspektu lub tekstu połączonego, segregatora, karty inspektora, która ma fokus klawiatury, albo otwartego okna dialogowego (okno dialogowe bez własnego fragmentu otwiera początek). Po lewej stronie okna znajduje się **spis treści** (kliknięcie nagłówka przenosi do niego), a nad tekstem **pole wyszukiwania**: naciśnij **Ctrl+F**, aby w nie przejść, wpisz słowo, a **Enter** pokaże następne trafienie, **Shift+Enter** poprzednie (przyciski **Następny** i **Poprzedni** robią to samo); wiersz obok pola mówi, które trafienie z ilu jest pokazane, a każde trafienie jest zaznaczone w tekście. To, co przeznaczone jest dla osób budujących {{APP}} (wtyczki, pakiety, szczegóły folderu projektu, zmienne środowiskowe), znajduje się w przewodniku programisty: **Pomoc > Rozwiązywanie problemów i programowanie > Przewodnik programisty: aktualizacje i dodatki**.

---

## 1. Pierwsze kroki

### Pobieranie programu

{{APP}} pobiera się ze strony **Releases** (wydania) strony projektu: https://github.com/Shelin-boop/Quillbinder_Collection/releases. Każde wydanie zawiera program jako jeden plik (*{{APP}}-&lt;version&gt;-x86_64.AppImage*), aktualizację programu dla tych, którzy mają już AppImage programu {{APP}} (rozdział 17) oraz dodatki (pliki *.qbpkg*, rozdział 17). Nadaj AppImage prawo wykonywania (kliknij prawym przyciskiem > **Właściwości** > **Uprawnienia** > *Jest wykonywalny* albo użyj `chmod +x` w terminalu) i kliknij go dwukrotnie; **Pomoc > Dodaj do menu aplikacji** umieszcza go w menu pulpitu. Plik *SHA256SUMS.txt* każdego wydania zawiera sumy kontrolne jego plików.

**{{APP}} jest oprogramowaniem {{LABEL}}.** Rób kopie zapasowe swoich projektów (rozdział 15) i trzymaj kopię rękopisów poza programem.

### Rozpoczęcie projektu

Gdy {{APP}} uruchamia się bez projektu, otwiera się **okno startowe**:

- **Nowy projekt**: wybierz kategorię i szablon (na przykład Pusty, Powieść, Powieść z częściami lub Opowiadanie), wpisz nazwę, wybierz miejsce zapisu i kliknij **Utwórz**. **Format** określa, jak projekt jest przechowywany: jako projekt {{APP}} (standard) albo jako projekt Scrivener 3 (*.scriv*), dzięki czemu możesz go otworzyć także w programie Scrivener (rozdział 14, *Praca w formacie Scrivener*).
- **Ostatnie projekty**: kliknij dwukrotnie projekt, nad którym pracowałeś(-aś) wcześniej.
- **Otwórz inny…**: znajdź projekt na dysku.
- **Importuj projekt Scrivener…**: utwórz projekt {{APP}} z projektu Scrivener (zob. rozdział 14). Aby pracować dalej na samym projekcie Scrivener, otwórz zamiast tego jego plik *.scrivx* poleceniem **Otwórz inny…**.

{{APP}} otwiera ostatni projekt przy uruchomieniu (możesz to wyłączyć w **Plik > Opcje > Ogólne**).

**Pierwsze uruchomienie** pokazuje jedną stronę z już wypełnionymi ustawieniami (**OK** je zatwierdza): kopie zapasowe (czy projekt jest archiwizowany przy zamknięciu, ile kopii zachować, na początku 25, i gdzie), w przypadku AppImage na Linuksie pytanie, czy dodać {{APP}} do menu aplikacji (i czy przenieść AppImage do *~/Applications*) oraz informację o sprawdzaniu aktualizacji wraz z jego przełącznikiem. Projekt powitalny otwiera się po naciśnięciu **OK**; wcześniej nie pojawia się puste okno programu. Wszystko można później zmienić w **Plik > Opcje**; tam, w **Ogólne > Uruchamianie**, wybierzesz też, które ustawienia mają pierwszeństwo: własne ustawienia projektu czy zawsze ustawienia programu (rozdział 16, *Ustawienia jednego projektu*).

### Projekt powitalny i Przewodnik szybkiego startu

Przy pierwszym uruchomieniu {{APP}} otwiera **projekt powitalny**: krótki projekt, który w osobnych dokumentach pokazuje, co {{APP}} oferuje ponad zwykłe możliwości (pasek ikon, powiększenie dla każdej części okna, mapy myśli, karty obiektów, komentarze całego folderu, Porównaj i przenieś, projekty Scrivener, motywy, aktualizacje i dodatki oraz to, co chroni twój tekst). Zacznij od dokumentu **Witaj** na górze segregatora; jego odnośniki prowadzą do każdego przystanku wycieczki. Nic, co zrobisz w projekcie powitalnym, nie może zaszkodzić twoim własnym projektom. (Projekt podany w wierszu poleceń lub dwukrotnie kliknięty plik projektu otwiera się w jego miejsce; projekt powitalny pojawi się wtedy przy następnym uruchomieniu.)

Projekt powitalny jest zapisany jako „Welcome to {{APP}}” w twoim folderze *Documents*. **Pomoc > Projekt powitalny…** otwiera go ponownie w stanie, w jakim go zostawiłeś(-aś), albo tworzy świeżą kopię, by zacząć od nowa (**Utwórz świeżą kopię**; stara kopia trafia do kosza pulpitu). Możesz go usunąć, kiedy chcesz; polecenie utworzy nowy.

**Pomoc > Przewodnik szybkiego startu** pokazuje na jednej stronie, gdzie są ważne ustawienia (motywy i tryb ciemny, kolory i czcionki, formatowanie, powiększenie, paski narzędzi, skróty, kopie zapasowe ...) i podsumowuje, co potrafi {{APP}}.

### Zapisywanie

Nigdy nie musisz zapisywać ręcznie. {{APP}} zapisuje automatycznie kilka sekund po zakończeniu pisania, po przełączeniu się do innego programu i przy zamykaniu projektu. Jeśli piszesz dalej, zapisuje w pierwszej krótkiej przerwie (oddech między słowami), gdy minie 5 sekund, a najpóźniej po 10 sekundach nawet bez żadnej przerwy. **Ctrl+S** zapisuje natychmiast, jeśli chcesz mieć pewność.

Każdy plik jest najpierw zapisywany pod nazwą tymczasową i zastępuje stary dopiero wtedy, gdy został zapisany w całości, a komputer umieścił go na dysku. Jeśli dysk jest pełny albo napęd zawiedzie podczas zapisu, stary plik pozostaje taki, jaki był, a twoje zmiany zostają w oknie, aż zapis znów zadziała.

**Stan zapisu jest zawsze widoczny na ekranie** (0.70). Pasek stopki pierwszego edytora (puste miejsce między sumą folderu a liczbą słów, a w pozostałych paskach stopki po lewej stronie) pokazuje **Zapisano**, **Jeszcze nie zapisano** (kilka sekund po tym, jak pisałeś(-aś)) albo, na czerwonym tle, **Nie zapisano: ostatni zapis nie powiódł się o 14:03**; gdy zapisy zawodzą, tytuł okna również nosi znacznik. Nieudany zapis pokazuje też komunikat, który wymienia niezapisane dokumenty i oferuje **Zapisz teksty gdzie indziej…**; nigdy nie przejmuje klawiatury, więc następny wpisany znak nadal trafia do twojego tekstu. Program próbuje ponownie sam (najpierw co 15 sekund, potem co minutę), komunikat wraca po pięciu minutach niepowodzeń, a czerwony znak pozostaje, aż zapis się powiedzie. Mapy myśli są zapisywane i sprawdzane tak samo: tablica, której nie da się zapisać, pozostaje w oknie, przy zamykaniu projektu jest liczona jako niezapisana i trafia do kopii awaryjnej.

**Krótkie komunikaty** (0.93.0). Na dole okna nie ma wiersza paska stanu. Komunikaty, które dawniej tam się pojawiały („Zapisano ponownie: wszystkie zmiany są na dysku.”, wynik kopii zapasowej i tym podobne), pokazują się na kilka sekund w tym samym miejscu co tekst zapisu, w pasku stopki pierwszego edytora (lewego lub górnego, gdy są dwa), a potem wraca tekst zapisu. Komunikat zbyt długi na to miejsce jest skracany; zatrzymaj nad nim wskaźnik, aby przeczytać całość. W bardzo wąskim edytorze tekst jest zastąpiony małym kolorowym znacznikiem (zielony: zapisano, żółty: jeszcze nie zapisano, czerwony: nie powiodło się), a tekst jest jego podpowiedzią. W trybie pisania i gdy żaden projekt nie jest otwarty, nic nie jest pokazywane.

**Pliki, których {{APP}} nie nadpisze** (0.70). Plik tekstowy, który jest uszkodzony (plik RTF z zepsutym początkiem lub ucięty) albo który inny program zapisał w innym kodowaniu znaków (na przykład plik Markdown zapisany jako Windows-1252 lub UTF-16, np. po edycji poza programem), jest pokazywany w takim zakresie, w jakim da się go odczytać, i nie można go edytować; komunikat o tym informuje, a plik nigdy nie jest zmieniany. Skopiuj jego tekst do nowego dokumentu, aby pracować dalej, albo zapisz plik w UTF-8 w programie, który go utworzył, po czym zamknij i otwórz projekt ponownie; możesz też cofnąć go do jednej z jego migawek (od 0.81.1): uszkodzony plik zostaje najpierw zachowany jako kopia w *settings/conflicts* w folderze projektu, a tekst migawki staje się tekstem dokumentu, który można edytować i który jest znowu zapisywany. Scalanie dokumentów i Podział odmawiają działania na takim dokumencie, podając powód, a Zamiana w projekcie, Konwersja cudzysłowów i zmiany stylu pomijają go i go wymieniają. Jeśli jakieś polecenie mimo to go zmieni, zmieniony tekst nie przepada: zostaje zachowany jako plik RTF w *settings/conflicts*, a komunikat mówi gdzie (0.81.1). Plik mapy myśli, którego nie da się odczytać, jest traktowany tak samo. Znaki, których nie da się zapisać (znaki sterujące z wklejonych danych), są pomijane jak dawniej; krótki komunikat w pasku stopki mówi o tym teraz raz na dokument.

**Zapis ląduje w całości** (0.65). Gdy jeden zapis obejmuje kilka plików (tekst nowego dokumentu i plik projektu, tekst i jego obrazy, arkusz Obiekty), wszystkie są najpierw zapisywane obok swoich miejsc, następnie wpisywane do małego dziennika (*settings/journal.json*), a dopiero potem wspólnie umieszczane na miejscu. Jeśli program lub komputer zatrzyma się w połowie, przy następnym otwarciu zapis zostaje dokończony na podstawie dziennika, a jeśli dziennik nie był jeszcze kompletny, projekt pozostaje taki, jaki był po poprzednim zapisie; powiadomienie mówi, które z tych dwóch. Dziennik, którego nie da się odczytać (awaria dysku), nigdy nie jest brany za pusty: on i pliki jego zapisu są zachowywane w folderze w *settings/conflicts*, nic nie jest umieszczane na miejscu ani usuwane, a powiadomienie o tym informuje (0.81.1). W folderze synchronizacji dziennik zapisu z drugiego komputera jest pozostawiany w spokoju przez 15 minut (ten zapis może tam jeszcze trwać albo jego pliki jeszcze docierać): ten komputer w tym czasie nie zapisuje projektu i próbuje ponownie sam, a jeśli drugi komputer zatrzymał się w trakcie tego zapisu, ten dokończy go po upływie tych 15 minut (0.81.1). Jeśli jednego pliku zapisu nie da się zapisać, żaden z nich niczego nie zastępuje, a twoje zmiany zostają w oknie do następnej próby. Od 0.93.0 napęd, który zgłasza błąd przy synchronizacji folderu po umieszczeniu plików na miejscu (prawdziwy błąd, a nie system plików, który nie umie synchronizować), powoduje, że zapis jest liczony jako nieudany: dziennik pozostaje, komunikat mówi, że napęd zgłosił błąd, a następny zapis lub otwarcie kończy zapis na podstawie dziennika. Dopóki dziennika, którego nie da się odczytać, nie udało się odłożyć na bok (do folderu *settings/conflicts* nie można zapisywać), żaden zapis nie tworzy nowego dziennika w jego miejsce: zapis czeka, jego komunikat wymienia plik, a program zapisuje sam, gdy tylko plik da się odłożyć. Tymczasowy plik wcześniejszego zapisu, który stoi na przeszkodzie, jest odkładany na bok, nigdy nadpisywany. Synchronizacja folderu, która zawodzi, zanim cokolwiek zostanie zastąpione, jest czystym niepowodzeniem: nic nie zostało zmienione. Gdy błąd pojawia się po umieszczeniu plików na miejscu, program zapamiętuje, co jest na dysku (dzięki czemu ponowne próby nie zgłaszają fałszywych konfliktów „zmieniono gdzie indziej” i nie zapisują obrazów dwa razy) i synchronizuje folder ponownie przy każdym zapisie: „Zapisano” zawsze następuje po sprawdzonej synchronizacji folderu. System plików, który odpowiada błędem na synchronizację folderu już przy otwieraniu projektu, jest zapamiętywany jako taki, który nie umie synchronizować folderów, a jego zapisy nie są zgłaszane jako nieudane.

Jeśli wylogujesz się lub wyłączysz komputer, gdy {{APP}} jest otwarty, program zapisze wszystko, zanim się zakończy (kopia zapasowa nie jest wtedy tworzona). Jeśli ten zapis się nie powiedzie (pełny dysk, napęd, który zniknął), niezapisane teksty i notatki (a od 0.93.0 także plik projektu w takim stanie, w jakim jest w pamięci) są zapisywane jako pliki RTF w folderze awaryjnym, *~/.local/share/{{APP}}/Emergency*, a przy następnym uruchomieniu program mówi, gdzie one są i ile plików zapisano (powiadomienie mówi „niekompletne” i wymienia, czego brakuje, gdy plik nie dał się zapisać). Jeśli program zakończy się nieoczekiwanie, następne uruchomienie ci o tym powie (rozdział 20, *Raporty o awariach*).

**Zamknięcie projektu, którego nie da się zapisać**, pyta, co zrobić; domyślne jest **Zostaw otwarty**. **Zapisz teksty gdzie indziej…** zapisuje jeszcze niezapisane teksty i notatki jako pliki RTF do wybranego folderu (inny dysk, pamięć USB), a projekt pozostaje otwarty. Od 0.93.0 ta kopia awaryjna zawiera także plik projektu w takim stanie, w jakim jest w oknie (segregator, tytuły i etykiety), tablice map myśli i zmienione obiekty, i jest tworzona także wtedy, gdy zmienił się tylko segregator lub Obiekty. Komunikat mówi, ile plików zapisano, w postaci „n z m”, i wymienia brakujące pliki oraz powody; kopia, której brakuje plików, ma nagłówek **niekompletna** (ze znakiem ostrzegawczym) i nigdy nie wygląda jak gotowa. Istniejący już folder nigdy nie jest użyty ponownie: druga kopia w tej samej sekundzie dostaje numer. Plik projektu w kopii nosi nazwę pliku projektu z dopisanym „.copy” (aby folder nie został omyłkowo wzięty za projekt); plik README w kopii mówi, jak ją przywrócić: zamknij projekt, zmień nazwę pliku projektu w folderze projektu (pozostaje taki, jaki był), skopiuj tam plik z kopii awaryjnej, zmień jego nazwę na nazwę pliku projektu i otwórz projekt ponownie. W przypadku projektu w formacie Scrivener README mówi, że plik należy do kopii roboczej, i nie podaje drogi powrotu. Gdy okazało się, że nic nie było niezapisane (ponowna próba w międzyczasie się powiodła), komunikat mówi, że wszystko jest zapisane i kopia nie była potrzebna. **Zamknij bez zapisywania** zamyka projekt i traci te zmiany.

### Gdzie mieszka twoja praca

Projekt to folder o nazwie *TwójTytuł*{{SUFFIX}}. W środku każdy dokument jest zwykłym plikiem tekstowym, więc twoje pisanie pozostaje czytelne nawet bez programu {{APP}}. Aby otworzyć projekt, kliknij dwukrotnie plik *TwójTytuł*{{FILE}} w tym folderze. Nie zmieniaj ręcznie nazw ani położenia plików w folderze projektu.

Aby znaleźć plik dokumentu poza programem {{APP}}, otwórz plik *contents-by-title.txt* w folderze projektu (0.65): wymienia on każdy dokument segregatora z jego tytułem, wcięty według miejsca, oraz pliki zawierające jego tekst, notatki lub multimedia (na przykład *content/<id>/text.md*). {{APP}} zapisuje go przy każdym zapisie zmieniającym segregator i sam nigdy go nie czyta. (Projekt w formacie Scrivener nie ma takiej listy; plik *.scrivx* programu Scrivener wymienia jego dokumenty.)

Projekt powinien być otwarty tylko w jednym oknie naraz. Jeśli jest już otwarty w innym oknie lub na innym komputerze (na przykład przez synchronizowany folder), {{APP}} ostrzega cię przed otwarciem. Ponowne uruchomienie programu {{APP}} z projektem, który jest otwarty w oknie programu (z menedżera plików lub skrótu na pulpicie), wysuwa to okno na wierzch zamiast otwierać projekt dwa razy; inny projekt otwiera się we własnym oknie.

**Projekt w synchronizowanym folderze** (OneDrive, Dropbox, Nextcloud, Syncthing lub dowolny inny program synchronizujący) może być używany na kilku komputerach, jednym naraz. Zamknij go na jednym komputerze i poczekaj, aż program synchronizujący skończy, zanim otworzysz go na drugim. {{APP}} pomaga w tym na poziomie plików, niezależnie od programu synchronizującego:

- Gdy projekt jest otwarty, mały plik blokady w nim wskazuje komputer i jest odnawiany co kilka minut. Zawiera nazwę komputera, twoją nazwę użytkownika na nim, wersję programu i numer procesu, kiedy projekt został otwarty i kiedy blokada była ostatnio odnowiona oraz kod odróżniający komputery (skrót, nieczytelny jako nazwa). Widzi to każdy, kto ma dostęp do wspólnego folderu. W **Plik > Opcje > Ogólne > Zapisywanie** ustawienie **Nazwa tego komputera w folderach współdzielonych** pozwala nadać własną nazwę („Laptop”); jest ona używana zamiast nazwy komputera w plikach blokady, w nazwach zachowanych wersji i w *settings/ui-<computer>.json*, a twoja nazwa użytkownika jest wtedy pomijana. Na innym komputerze otwarcie projektu pokazuje wtedy nazwę tamtego komputera, od kiedy ma otwarty projekt oraz jego ostatni znak życia; **Otwórz mimo to** lub **Anuluj**. Blokada, która nie była odnawiana dłużej niż 15 minut (projekt nie został tam zamknięty albo tamten komputer jest wyłączony), jest pokazywana jako pozostałość, a domyślne jest **Otwórz**.
- Jeśli projekt zostanie otwarty na innym komputerze, gdy jest otwarty tutaj, powiadomienie o tym informuje i proponuje zamknięcie go tutaj.
- Jeśli plik projektu został zmieniony gdzie indziej (na innym komputerze przez program synchronizujący lub przez inny program) po tym, jak {{APP}} go odczytał, a został zmieniony także tutaj, druga wersja jest zachowywana, zanim zapisana zostanie twoja: jako **migawka** dokumentu (porównasz ją na karcie Migawki inspektora) i jako kopia w folderze projektu *settings/conflicts*. Powiadomienie mówi, których dokumentów to dotyczy. Jeśli drugiej wersji nie da się zachować (na przykład dlatego, że dysk jest pełny lub do *settings/conflicts* nie można zapisywać), twoja nie jest zapisywana na jej miejsce: zapis kończy się niepowodzeniem z komunikatem wymieniającym plik, a twoje zmiany pozostają niezapisane, dopóki drugiej wersji nie da się zachować. Dokument, który tylko oglądałeś(-aś) tutaj, nigdy nie jest zapisywany nad nowszą wersją z innego miejsca.
- Otwarcie i zamknięcie projektu bez zmian nie zmienia żadnego z jego plików; układ okna i miejsca, w których byłeś(-aś) w każdym dokumencie, są przechowywane dla każdego komputera osobno (*settings/ui-<computer>.json*). W formacie Scrivener plik *.scriv* również nie jest dotykany, a w czasie, gdy jest otwarty, dostaje plik blokady programu {{APP}} (*quillbinder.lock*); Scrivener go nie odczytuje. Przed każdym zapisem do *.scriv* {{APP}} szuka własnej blokady programu Scrivener: jeśli Scrivener otworzył projekt po programie {{APP}}, nic więcej nie jest zapisywane do *.scriv* (powiadomienie mówi o tym raz, potem krótki komunikat w pasku stopki); twoje zmiany pozostają w kopii roboczej programu {{APP}}. Zamknij projekt w programie Scrivener i zapisz ponownie (**Plik > Zapisz**). Jeśli najpierw zamkniesz projekt tutaj, zmiany trafią do *.scriv* przy następnym otwarciu tutaj; tam, gdzie Scrivener w międzyczasie coś zmienił, odczytywana jest jego wersja.
- Jeśli projekt wymienia tekst dokumentu, którego plik nie dotarł jeszcze na ten komputer (program synchronizujący nie skończył), powiadomienie po otwarciu go wymienia; ten dokument jest pokazywany jako pusty, nie można go edytować i nic nie jest zapisywane w jego miejsce. Zamknij projekt, poczekaj, aż program synchronizujący skończy, i otwórz go ponownie. To samo dotyczy tekstu, który udało się odczytać tylko w części (0.65).
- Plik zmieniony gdzie indziej jest zauważany także wtedy, gdy zachował rozmiar i czas (niektóre programy synchronizujące same ustawiają czas): {{APP}} porównuje jego zawartość z tym, co odczytał lub zapisał (0.65).
- Zawartość decyduje także w drugą stronę (0.92.0): plik, w którym zmienił się **sam czas**, nie jest inną wersją. Niektóre programy synchronizujące ustawiają czas pliku ponownie kilka sekund po tym, jak {{APP}} go zapisał, gdy skończy się ich wysyłanie; nic nie jest wtedy zachowywane, nie pojawia się powiadomienie i plik nie jest zapisywany ponownie. Plik projektu, który już zawiera to, co zapisałby zapis, również jest pozostawiany w spokoju, więc zapis, który nic nie zmienia, nie daje programowi synchronizującemu nic do przeniesienia.
- Powiadomienie **Zachowano inne wersje** ma przycisk **Nie pokazuj ponownie do zamknięcia programu {{APP}}**. Inne wersje są nadal zachowywane jako migawki i jako kopie w *settings/conflicts*; tylko powiadomienie nie pojawia się, aż zamkniesz program. Następne uruchomienie pokazuje je znowu.
- **Kłódka przy tym, nad czym pracuje inne urządzenie (0.92.0).** Gdy projekt jest otwarty, każde urządzenie zapisuje też obok pliku blokady, które dokumenty ma otwarte w edytorze lub zmieniło od otwarcia projektu. Gdy drugie urządzenie otwiera projekt, jego segregator pokazuje małą kłódkę na prawym końcu wierszy tych dokumentów oraz folderów wokół nich. Wskaż kłódkę: podpowiedź wymienia urządzenie i czas, kiedy otworzyło projekt. To wskazówka, a nie zabezpieczenie: nic nie jest blokowane i nadal możesz edytować dokument. Znacznik wędruje przez twój program synchronizujący, więc może mieć kilka minut; wymaga 0.92.0 lub nowszej na obu urządzeniach (Scrivener niczego nie ogłasza). Lista urządzenia, które nie dało znaku życia dłużej niż 15 minut, nie jest pokazywana. Notatka to mały plik na urządzenie (*settings/lock-work-…json*; dla projektu w formacie Scrivener *quillbinder.lock-work-…json* w *.scriv*); zawiera nazwę urządzenia taką jak w pliku blokady, czas i wewnętrzne numery dokumentów, bez tytułów i bez tekstu. Jest zapisywana tylko wtedy, gdy lista się zmienia, nigdy przy każdym naciśnięciu klawisza, jest usuwana przy zamknięciu projektu i jest pomijana w kopiach zapasowych.
- **Format Scrivener, dwa komputery** (0.65): dokumenty dodane do *.scriv* gdzie indziej (Scrivener na innym komputerze), gdy projekt jest tu otwarty, są zachowywane, a segregator, który je wymienia, nie jest zastępowany; powiadomienie prosi o zamknięcie projektu i otwarcie go ponownie, co pokaże zarówno ich dokumenty, jak i twoje. Obrazy, pliki PDF, pliki audio i wideo oraz mapy myśli zmienione w *.scriv* gdzie indziej są zachowywane w takim stanie, w jakim są (dawniej wersja stąd była kopiowana na nie); gdy jeden został zmieniony po obu stronach, druga wersja jest najpierw zachowywana w *settings/conflicts*.

Projekt w **formacie Scrivener** to zamiast tego folder *TwójTytuł.scriv*; otwórz go jego plikiem *.scrivx*. {{APP}} zapisuje w nim twoje zmiany, więc projekt pozostaje projektem Scrivener. **Plik > Zapisz jako…** zapisuje projekt pod inną nazwą lub w innym miejscu, w obu formatach (rozdział 14).

### Prywatność: co opuszcza twój komputer

{{APP}} łączy się z internetem tylko w jednym celu: aby sprawdzić aktualizacje. Pobiera jeden mały plik, nie wysyła niczego o tobie i możesz to wyłączyć w menu Pomoc. (Rozdział 17, *Sprawdzanie aktualizacji*, mówi dokładnie, co jest pobierane i kiedy. Przed 0.92.0 program w ogóle nie sprawdzał aktualizacji.) Nie ma statystyk użycia ani raportów o awariach. Trzy inne rzeczy trafiają do internetu, i to tylko wtedy, gdy je uruchomisz:

- **Plik > Import > Strona internetowa…** pobiera podaną stronę i jej obrazy z tej witryny. Jak przeglądarka, wysyła nazwę popularnej przeglądarki (Firefox na Linuksie), języki twojego systemu (dla języka strony) i, jak każde połączenie, adres internetowy twojego komputera.
- **Pobieranie modelu językowego** dla dodatku Pomocnik tłumaczenia (rozdział 10) pobiera go z wydań repozytorium programu {{APP}} na GitHubie. Wysyła nazwę dodatku („Quillbinder-Translation-Addon”) i, jak każde połączenie, twój adres internetowy.
- **Pobieranie modelu mowy** (dodatek Speech Transcription, rozdział 10) pobiera model z Hugging Face (huggingface.co, firma w USA) lub z adresu ustawionego w jego opcjach. Wysyła „{{APP}}/<version>” jako nazwę programu i, jak każde połączenie, twój adres internetowy. Model jest instalowany tylko wtedy, gdy jego suma kontrolna jest znaną sumą; plik z innego adresu lub pobrany przez ciebie samodzielnie jest używany dopiero po twojej zgodzie.

Co inni mogą zobaczyć z twojej pracy:

- **Projekty we współdzielonych folderach** noszą plik blokady, gdy są otwarte (zob. wyżej): nazwę komputera, nazwę użytkownika i czasy. Ustaw własną nazwę w **Plik > Opcje > Ogólne > Zapisywanie**, aby je pominąć.
- **Dane autora** (imię i nazwisko, adres, telefon, e-mail w **Plik > Opcje**) pozostają w ustawieniach programu na tym komputerze; nigdy nie trafiają do własnych ustawień projektu. Docierają do rękopisu tylko tam, gdzie format kompilacji używa symboli zastępczych (<$author>, <$address> ...).
- **Usunięty tekst nie znika wszędzie**: migawki zachowują wcześniejsze wersje dokumentu (**Projekt > Usuń stare migawki** je usuwa), a kopie zapasowe zachowują cały projekt w takim stanie, w jakim był (rozdział 15). Kopie zapasowe to zwykłe pliki ZIP bez hasła; folder kopii zapasowych wewnątrz synchronizowanego folderu jest wysyłany przez program synchronizujący.
- **Twoje pliki nie są szyfrowane, dopóki o to nie poprosisz** (0.99.0: **Plik > Opcje > Szyfrowanie**, zob. rozdział 15; wcześniej żadne nie były): teksty, notatki, ustawienia projektu i kopie zapasowe to zwykłe pliki, czytelne dla każdego, kto może odczytać folder, w którym się znajdują. Aby rękopis pozostał prywatny, trzymaj projekt i jego kopie zapasowe na zaszyfrowanym dysku (instalator Kubuntu oferuje szyfrowanie całego dysku; dla pamięci USB lub dysku zewnętrznego: Plasma Vaults z KDE albo dysk szyfrowany LUKS) i trzymaj folder kopii zapasowych poza współdzielonymi folderami, których nie kontrolujesz.
- **Raporty o awariach** pozostają na tym komputerze, dopóki sam ich nie skopiujesz lub nie wyślesz (rozdział 20). Opisują stan programu i wymieniają otwarty projekt oraz jego folder. W systemie Windows obok każdego raportu przechowywany jest zrzut pamięci; może on zawierać tekst, który pisałeś(-aś), i raport o tym informuje.

### Dodawanie programu {{APP}} do menu aplikacji

Przy pierwszym uruchomieniu pliku programu ({{APP}}.AppImage) strona pierwszego uruchomienia proponuje dodanie programu {{APP}} do menu aplikacji. Potem możesz go uruchamiać jak każdy inny program, a dwukrotne kliknięcie projektu go otwiera. Żadne okienko nie potwierdza wpisu; niepowodzenie jest zgłaszane. Ta sama strona (i **Pomoc > Dodaj do menu aplikacji**) proponuje **przeniesienie AppImage do ~/Applications** (domyślnie zaznaczone, gdy pliku jeszcze tam nie ma), aby wpis w menu uruchamiał go ze stałego miejsca, w którym plik pozostaje (pole wyboru mówi to tymi słowami, bez nawiasu); jeśli przeniesienie się nie powiedzie (pełny dysk, plik o takiej nazwie już tam jest), nic nie ginie: plik zostaje tam, gdzie był, wpis wskazuje to miejsce, a komunikat mówi dlaczego. Jeśli później sam(a) przeniesiesz AppImage, wpis nadal wskazuje stare miejsce: użyj ponownie **Pomoc > Dodaj do menu aplikacji**, co zapisuje bieżące miejsce pliku (starszy wpis jest zastępowany, nie dublowany). Po przeniesieniu aktualizacje programu, kopia *.before-update* i **Pomoc > Rozwiązywanie problemów i programowanie > Informacje o systemie** od razu używają nowego miejsca.

Jedno polecenie menu Pomoc, **Pomoc > Dodaj do menu aplikacji**, podąża za stanem wpisu: dodaje wpis, dopóki go nie ma, a gdy już jest, nazywa się *Usuń z menu aplikacji* i usuwa to, co dodało dodawanie (plik wpisu, ikonę i typy plików), pozostawiając AppImage tam, gdzie jest. Jeśli sam(a) przeniesiesz plik programu, usuń wpis i dodaj go ponownie.

### W systemie Windows: wszystko w jednym folderze

Wersja programu {{APP}} dla systemu Windows (eksperymentalna, publikowana od 0.91.1) to folder, który rozpakowujesz, gdzie chcesz, na przykład *C:\Users\<your name>\Quillbinder*; nie wymaga instalacji ani praw administratora. Trzyma swoje ustawienia, dodatki, aktualizacje programu, zainstalowane czcionki i raporty o awariach w folderze *data* wewnątrz tego folderu i nic w folderach systemu Windows ani w rejestrze. Cały folder można przenieść lub skopiować (na pamięć USB lub inny komputer); usunięcie go usuwa program. Folder, do którego program nie może zapisywać (taki jak *Program Files*), jest odrzucany z komunikatem mówiącym, gdzie go zamiast tego umieścić. Aktualizacje programu instaluje się poleceniem **Pomoc > Dodatki i aktualizacje > Zainstaluj aktualizację lub dodatek**; aby przejść na nowo pobraną wersję, rozpakuj ją i skopiuj folder *data* do nowego folderu. Twoje projekty zostają tam, gdzie je trzymasz.

Są dwa sposoby na uzyskanie tego folderu (0.90.4). **Program instalacyjny** (*...-setup.exe*) pyta o folder i rozpakowuje do niego {{APP}}; nie wymaga praw administratora i niczego więcej na komputerze nie zmienia: żadnego wpisu w Aplikacjach, nic w rejestrze, żadnego wpisu w menu Start (oferuje je opisany niżej przewodnik). Rozpakowanie nowszej wersji do tego samego folderu zachowuje *data*, czyli twoje ustawienia i dodatki. **Plik zip** zawiera ten sam folder: rozpakuj go w całości. System Windows może ostrzegać przed każdym z tych plików, ponieważ nie jest podpisany (SmartScreen: *Więcej informacji*, potem *Uruchom mimo to*).

Dwie rzeczy różnią się od Linuksa z powodu samego systemu Windows (0.90.4). Klawisz **AltGr** wpisuje swój znak, tak jak oczekujesz: na niemieckiej klawiaturze AltGr+7, 8, 9, 0 i < dają { [ ] } oraz |, chociaż {{APP}} ma polecenia na Ctrl+Alt z tymi klawiszami; dla poleceń naciśnij lewe klawisze Ctrl i Alt. Pliki potrzebne tylko na chwilę (zapisana strona internetowa rozpakowana do wyświetlenia lub jej kopia dla przeglądarki) są trzymane w folderze tymczasowym systemu Windows i usuwane, gdy {{APP}} się zamyka. Transkrypcja mowy używa w systemie Windows procesora, a nie karty graficznej.

Przy pierwszym uruchomieniu przewodnik mówi, gdzie program trzyma swoje pliki, i ostrzega, gdy jego folder leży wewnątrz folderu usługi synchronizacji (OneDrive, Dropbox, Google Drive ...) lub ma długą ścieżkę. Pyta też o poniższe rzeczy, które ponownie pokazują **Pomoc > Rozwiązywanie problemów i programowanie > Ten komputer** i **Plik > Opcje > Ogólne > Ten komputer**:

- **Kopie zapasowe**: nie ma standardowego folderu kopii zapasowych. Dopóki żadnego nie wybierzesz, żadna automatyczna kopia nie jest tworzona; pasek stopki pierwszego edytora pokazuje **Brak kopii zapasowych** obok tekstu zapisu (kliknij go, aby wybrać folder), a pierwsza kopia, która miałaby się wykonać, prosi o folder. Najlepiej inny dysk.
- **Notatki Brudnopisu i transkrypcje**: w Documents (standard), w folderze programu lub we własnym folderze. Zmiana daje oba narzędzia nowe miejsce; to, co zapisały wcześniej, pozostaje tam, gdzie jest.
- **Windows**: wpis w menu Start, skrót na pulpicie i typy plików projektów (pliki *.qbx* otwierają się w programie {{APP}}, a on jest oferowany w **Otwórz za pomocą** dla plików *.scrivx* programu Scrivener). To jedyne rzeczy, które program umieszcza poza swoim folderem, każda tylko wtedy, gdy ją dodasz, i każda z przyciskiem **Usuń**. Usuń je, zanim skasujesz lub przeniesiesz folder.

Czcionki zainstalowane poleceniem **Format > Czcionka > Zainstaluj pliki czcionek** pozostają w folderze programu i są tylko programu {{APP}}; inne programy ich nie widzą.

Dodatki z kodem programu mają własne wersje dla systemu Windows (ich nazwy plików kończą się na *-windows*): **Media Playback** odtwarza to, co odtwarza sam Windows, **Extra Video and Audio Formats** dodaje FFmpeg, a **Speech Transcription** zapisuje mowę na procesorze (karty graficzne nie są jeszcze w systemie Windows używane). **Pomocnik tłumaczenia** jest w systemie Windows także częścią pełnej wersji (0.92.0). Dodatki bez kodu programu (motywy, szablony, formaty kompilacji) to te same pliki w każdym systemie.

### Na tabletach z Androidem

Wersja programu {{APP}} dla Androida (eksperymentalna, publikowana od 0.91.3) to aplikacja na tablety z Androidem 9 lub nowszym (64-bitowy ARM). Ma te same okna co na komputerze, przygotowane pod palce: palec przewija i dotyka, dwa palce powiększają, palec spoczywający pół sekundy na czymś otwiera jego menu prawego przycisku (w tekście robi to dotknięcie zaznaczonego tekstu), a paski narzędzi zawijają się w tyle rzędów, ile potrzebuje ekran. Pobierz plik APK na tablet i otwórz go; Android zapyta, czy przeglądarka lub aplikacja Pliki może instalować aplikacje, na co zezwalasz na potrzeby tej instalacji. Nowa wersja przychodzi jako nowy APK i instaluje się nad starą, zachowując twoje ustawienia i dodatki; **Pomoc > Dodatki i aktualizacje > Sprawdź aktualizacje teraz** mówi, kiedy jest nowa, i otwiera jej pobieranie w przeglądarce (nowsze dodatki są instalowane wewnątrz programu {{APP}}).

Z zewnętrzną klawiaturą i myszą (od 0.92.0): Ctrl + kółko myszy powiększa, a Ctrl + kliknięcie lub Shift + kliknięcie wybiera kilka dokumentów, jak na komputerze. Linia między dwoma panelami pokazuje uchwyt; mysz naciśnięta kilka pikseli obok niego bierze go ze sobą. Przycisk **Wstecz** Androida zamyka menu, podpowiedź lub okno na wierzchu; każde okno ma przycisk, który je zamyka, a okno większe niż ekran się przewija. Na tablecie pinezka przypiętej mapy myśli pokazuje się na zaznaczonym wierszu segregatora (kliknij ją albo użyj menu prawego przycisku wiersza: *Przypnij mapę myśli tej pozycji*, *Zwolnij przypiętą mapę myśli*), a dotknięcie kłódki wiersza pokazuje, kto nad nim pracuje. Palec na przeglądzie mapy myśli działa jak mysz: dotknięcie przenosi tam widok, palec przeciąga prostokąt pokazanej części, a uchwyt w jego rogu zmienia jego rozmiar. **Plik > Zapisz jako…** pyta na tablecie najpierw o format, a potem otwiera okno plików.

Przy pierwszym uruchomieniu {{APP}} prosi o dostęp do plików tabletu. Dzięki niemu projekty trafiają do *Documents/Quillbinder*, kopie zapasowe do *Documents/Quillbinder/Backups*, eksporty do *Documents/Quillbinder/Exports*, a notatki Brudnopisu do *Documents/Quillbinder/Scratch Pad*, gdzie mogą do nich sięgnąć inne aplikacje, aplikacja synchronizująca i komputer podłączony kablem; przycisk otwiera stronę Androida *Dostęp do wszystkich plików*, na której włączasz go dla programu {{APP}}. Bez niego wszystko zostaje we własnym folderze aplikacji, którego żadna inna aplikacja nie widzi i który Android usuwa razem z aplikacją; *Plik > Pliki na tym tablecie* pyta ponownie i mówi, gdzie co jest. Gdy Android odkłada aplikację (przycisk Home, inna aplikacja), {{APP}} zapisuje natychmiast.

Słowniki pisowni (angielski i niemiecki; pozostałe języki to dodatki, zob. sekcja 4), narzędzia pisarskie, importery stron internetowych i starych plików Worda oraz (od 0.92.0) Pomocnik tłumaczenia są częścią aplikacji; Pomocnik tłumaczenia jest instalowane przy pierwszym uruchomieniu wersji i pobiera model językowy tylko wtedy, gdy jakiś wybierzesz. Dodatki dla Androida (ich nazwy plików kończą się na *-android-arm64*) instaluje się poleceniem **Pomoc > Dodatki i aktualizacje > Zainstaluj aktualizację lub dodatek**; okno plików otwiera się w folderze Pobrane, a {{APP}} uruchamia się ponownie, gdy dodatek tego wymaga. Pakiety zrobione dla Linuksa lub Windows oraz aktualizacje programu są odrzucane z podaniem powodu: sama aplikacja jest aktualizowana wyłącznie przez nowy APK. W wersji na Androida jeszcze nie ma: Transkrypcji mowy, dźwięku i wideo w edytorze oraz raportów o awariach. Tekst skopiowany wewnątrz programu {{APP}} zachowuje po wklejeniu formatowanie; schowek Androida przenosi tylko zwykły tekst, więc tekst skopiowany do innych aplikacji trafia tam jako zwykły.

---

## 2. Okno główne

Okno ma trzy kolumny:

| Obszar | Co robi |
|------|--------------|
| **Segregator** (po lewej) | Spis treści całego projektu: każdy dokument i folder. |
| **Edytor** (pośrodku) | Pokazuje to, co zaznaczysz w segregatorze: tekst, tablicę korkową z fiszkami albo konspekt. |
| **Inspektor** (po prawej) | Informacje o bieżącym dokumencie: streszczenie, notatki, zakładki, metadane, migawki i komentarze. |

Gdy {{APP}} uruchamia się po raz pierwszy na ekranie węższym niż 1600 px, okno otwiera się zmaksymalizowane. Rozmiar, położenie i stan okna (zmaksymalizowane lub nie) są zapisywane chwilę po każdej zmianie, więc awaria ich nie gubi. Ukryj segregator za pomocą **Ctrl+Alt+B**, a inspektor za pomocą **Ctrl+Alt+I**, aby zyskać więcej miejsca. **Część z klawiaturą.** Wyraźna linia o grubości 2 px w kolorze podświetlenia motywu otacza tę część okna, która ma klawiaturę (segregator, edytor, tablicę korkową, konspekt, inspektor lub podgląd pomocniczy wewnątrz edytora), w każdym motywie; naraz jest jedna linia. Tak wygląda okno, które jest szersze niż wysokie; okno wyższe niż szerokie układa te same trzy obszary inaczej (następna sekcja). Kolory i czcionki wszystkich trzech oraz tablicy korkowej, konspektu i trybu pisania pochodzą z motywu (**Widok > Motywy**) i można je zmieniać obszar po obszarze (rozdział 16), także prawym przyciskiem myszy na obszarze (**Wygląd**). Każdy projekt zachowuje własny wygląd.

**Pasek narzędzi** u góry zawiera, od lewej do prawej: przycisk **Widok** (segregator, kolekcje, inspektor, układy), nowy dokument (z menu dla folderów, szablonów i importu), nowy folder, kosz, **cofnij** i **ponów**, przycisk **wyszukiwania** (wyszukiwanie w projekcie; strzałka obok niego wymienia pozostałe narzędzia wyszukiwania: Znajdź, Zamiana w projekcie, Znajdź według formatowania), pole **szybkiego wyszukiwania**, trzy przyciski widoku (tekst, tablica korkowa, konspekt), zakładki projektu, Szybki podgląd, komentarze i przypisy, kompilację, tryb pisania, **mikrofon** Transkrypcji mowy (gdy ten dodatek jest zainstalowany) oraz przycisk inspektora. To ty decydujesz, co zawiera (zob. „Własne paski narzędzi” niżej).

**Pasek formatowania** pod nim zawiera styl, czcionkę, rozmiar, interlinię, domyślne formatowanie tekstu, okno czcionek, pogrubienie, kursywę, podkreślenie, przekreślenie, wyróżnienie, kolor tekstu, wyrównanie, listy, odstęp przed akapitem i po nim, wcięcie pierwszego wiersza i całego akapitu, komentarze, przypisy, adnotacje śródtekstowe i obrazy. Oba paski pokażesz lub ukryjesz poleceniem **Widok > Paski narzędzi**.

**Okna dialogowe.** Okno otwierane przez program (Opcje, Kompilacja, Cele projektu ...) nigdy nie jest większe niż ekran bez paneli pulpitu: na małym ekranie laptopa jego zawartość się przewija lub zmniejsza, a przyciski pozostają widoczne. Gdy się zamyka, klawiatura wraca tam, gdzie była, zwykle do tekstu, więc możesz od razu pisać dalej.

**Gdzie jestem?** Kliknij dwukrotnie miejsce obok tekstu (papier obok kolumny lub obszar wokół niej): segregator zaznacza dokument obok kliknięcia, otwiera foldery nad nim i dwukrotnie miga jego wierszem na pomarańczowo. Edytor zachowuje to, co pokazuje. **Nawigacja > Pokaż w segregatorze** (**Ctrl+Shift+R**) robi to samo dla dokumentu przy kursorze.

**Stopka** każdego edytora pokazuje liczbę słów i znaków (zaznacz tekst, aby policzyć tylko zaznaczenie) oraz cel dla dokumentu. Kliknij liczbę, aby zobaczyć szczegóły. Gdy edytor pokazuje folder (lub dokument z poddokumentami), stopka pokazuje też **sumę** folderu: słowa jego własnego tekstu i wszystkiego, co jest w środku, na przykład „Folder: 12,345 words”. Tablica korkowa i konspekt pokazują tę samą sumę pośrodku stopki; gdy pokazywanych jest kilka pozycji, jest to suma wszystkich. Kliknięcie sumy pokazuje jej szczegóły (słowa, znaki, znaki bez spacji, akapity i strony), tak jak liczba tekstu. Inspektor pokazuje ją pod **Razem** w metadanych folderu, a zatrzymanie myszy na folderze w segregatorze też ją pokazuje (z własnymi słowami folderu, gdy ma własny tekst).

### Menu

| Menu | Co zawiera |
|------|-------------------|
| **Plik** | Projekty, zapisywanie, format projektu ({{APP}} lub Scrivener), kopie zapasowe, import i eksport, porównanie i przeniesienie z plikiem Word lub LibreOffice, kompilacja, drukowanie, konwersja plików, opcje |
| **Edycja** | Cofanie, schowek, zaznaczanie, wyszukiwanie i zamiana (w tekście, w projekcie, według formatowania), przesuwanie dokumentów w segregatorze, narzędzia pisarskie |
| **Wstaw** | Obrazy, tabele, odnośniki do dokumentów i obiektów, odnośniki internetowe, komentarze, przypisy, znaczniki czasu multimediów, autonumeracja, symbole zastępcze, znaki specjalne, data i godzina |
| **Widok** | Trzy tryby widoku, podział edytora (**Podziel edytor**), położenia segregatora, edytora i inspektora oraz zapisane układy (**Układ**), motywy kolorystyczne (**Motywy**), segregator, kolekcje i inspektor (oraz **Klawisze szybkiego dodawania w segregatorze**), paski narzędzi, opcje edycji tekstu (widok strony, linijka, przewijanie jak w maszynie do pisania), opcje tablicy korkowej i konspektu, kolory etykiet, powiększenie, tryb pisania, pełny ekran |
| **Nawigacja** | Wstecz i dalej, poprzedni i następny dokument, otwieranie dokumentów w drugim edytorze, w podglądzie pomocniczym lub jako Szybki podgląd, blokada edytora, do którego edytora ładuje segregator, karty inspektora (wszystkie, pod nazwami ich podpowiedzi: **Streszczenie i notatki**, **Zakładki**, **Metadane**, **Migawki**, **Komentarze i przypisy**, mapa myśli, obiekty i karty narzędzi pisarskich) |
| **Projekt** | Nowe dokumenty, ustawienia projektu (także zapisywane do pliku i wczytywane z pliku), cele, słowa kluczowe, statystyki, historia pisania, kosz, usuwanie starych migawek |
| **Dokumenty** | Migawki, zmiana nazwy, duplikowanie, dzielenie, scalanie, konwertowanie, ikona, etykieta, status, zakładki |
| **Format** | Czcionka (z instalowaniem czcionek), akapit, styl, listy, wyróżnienie, kopiowanie formatowania, tryb rewizji |
| **Okno** | Brudnopis, okna Szybkiego podglądu |
| **Pomoc** | **Znajdź polecenie**, ten podręcznik, Przewodnik szybkiego startu, projekt powitalny, lista wszystkich symboli zastępczych, lista skrótów klawiaturowych, pomoc do map myśli, podmenu **Dodatki i aktualizacje** (Menedżer dodatków, Zainstaluj aktualizację lub dodatek, sprawdzanie aktualizacji), dodawanie programu {{APP}} do menu aplikacji (i jego usuwanie), podmenu **Rozwiązywanie problemów i programowanie** (informacje o systemie, Ten komputer, raporty o awariach, Otwórz folder wtyczek, przewodnik programisty), Zgłoś problem, licencje stron trzecich, O programie |

**Znajdź polecenie** (**Pomoc > Znajdź polecenie…**) otwiera okno z polem wyszukiwania i listą wszystkich poleceń menu, każde z jego menu i skrótem. Wpisz część nazwy (kilka słów zawęża wynik, w dowolnej kolejności); **Enter** lub dwukrotne kliknięcie uruchamia wybrane polecenie. Polecenie, które w danej chwili jest wyłączone, jest wyszarzone i nie uruchamia się. Nie ma własnego skrótu.

Podmenu pokazuje krótkie nazwy: **Plik > Import > Pliki**, **Dokumenty > Podziel > W miejscu zaznaczenia**, **Nawigacja > Otwórz > Jako Szybki podgląd**. Skrót polecenia jest pokazany obok niego i można go zmienić (rozdział 19). Każda pozycja menu ma własną literę (podkreśloną, gdy przytrzymasz **Alt**), więc **Alt** i litery sięgają każdego polecenia. Polecenie kończy się znakiem „…” tylko wtedy, gdy najpierw otwiera się okno i o coś pyta, zanim polecenie zadziała; polecenia, które tylko coś pokazują (**Podgląd wydruku**, **Statystyki projektu**, **Informacje o systemie** i tym podobne), go nie mają. Polecenie, które nie może zadziałać, jest w menu wyszarzone; menu prawego przycisku pozycji segregatora pomija natomiast **Zamień na folder**, **Zamień na dokument tekstowy**, **Zamień kartę na obiekt** i **Scal**, gdy nie mogą zadziałać.

Bez otwartego projektu menu **Wstaw**, **Nawigacja**, **Projekt**, **Dokumenty** i **Format** są wyszarzone, podobnie jak polecenia innych menu, które potrzebują tekstu lub segregatora (Znajdź, Powiększenie, układ edytora, Aktywuj segregator ...). Bez projektu działa: tworzenie, otwieranie i importowanie projektów, konwertowanie plików, instalowanie aktualizacji i dodatków, opcje, motywy i układy, narzędzia pisarskie niewymagające projektu oraz Pomoc.

### Panele według kształtu okna

Okno układa segregator, edytor i inspektor według swojego kształtu i zmienia się samo, gdy kształt się zmienia (przeciągasz okno tak, że jest wyższe niż szerokie, albo obracasz tablet). Okno kwadratowe liczy się jako szerokie.

- **Okno szerokie** (szersze niż wysokie lub kwadratowe): trzy kolumny. Ich kolejność wybierzesz w **Widok > Układ > Okno szerokie**: dowolna z sześciu kolejności segregatora, edytora i inspektora. Szerokości ustawiają linie między kolumnami.
- **Okno pionowe** (wyższe niż szerokie): dwa rzędy pod paskami narzędzi. Jeden rząd to edytor, na pełną szerokość okna; drugi zawiera segregator i inspektor obok siebie. W **Widok > Układ > Okno pionowe** **Edytor na środku** umieszcza rząd edytora nad drugim, **Edytor na dole** pod nim; **Segregator po lewej** i **Segregator po prawej** wybierają stronę segregatora (inspektor zajmuje drugą). Wysokości obu rzędów oraz szerokości segregatora i inspektora ustawiają linie między nimi. Gdy widoczna jest klawiatura ekranowa (Android), rząd segregatora i inspektora zachowuje swoją wysokość; przesuń linię, jeśli chcesz inaczej.
- Ukryty segregator lub inspektor oddaje swoje miejsce drugiemu w rzędzie pionowym, na pełną szerokość; gdy oba są ukryte, edytor ma całe okno. Dwa edytory (**Widok > Podziel edytor**) pozostają w miejscu edytora.
- **Widok > Układ > Przywróć układ** przywraca kształt, który okno ma teraz, do wartości początkowych: segregator, edytor, inspektor w oknie szerokim; edytor pośrodku i segregator po lewej w pionowym.

Obie grupy są zawsze w menu; każda zaczyna działać, gdy okno ma dany kształt, a każdy kształt wraca tak, jak go zostawiłeś(-aś) (miejsca i rozmiary). Wybory należą do tego komputera lub tabletu i są przechowywane w ustawieniach programu, nigdy w projekcie: projekt współdzielony między komputerami zachowuje układ każdego urządzenia, a nic z tego nie jest zapisywane w plikach projektu.

### Układy

Układ zapamiętuje, jak okno jest ułożone: czy segregator, kolekcje, inspektor, pasek narzędzi i pasek formatowania są pokazane, jak szerokie są kolumny, jak podzielony jest edytor, jaki widok ma każdy edytor i do którego edytora ładuje segregator. Nie mówi nic o dokumentach, więc każdy układ działa w każdym projekcie.

Układ wybierzesz w **Widok > Układ** lub przyciskiem **Widok** paska narzędzi. Wbudowanych jest siedem układów:

| Układ | Rozmieszczenie |
|--------|-------------|
| **Domyślna** | Segregator, jeden edytor, inspektor |
| **Segregator i edytor** | Segregator i tekst, bez inspektora |
| **Tylko edytor** | Sam tekst z dwoma paskami |
| **Konspekt i edytor** | Konspekt po lewej, tekst po prawej. Segregator ładuje grupy do konspektu; kliknięcie wiersza otwiera ten dokument po prawej |
| **Tablica korkowa i edytor** | Tablica korkowa na górze, tekst na dole, połączone w ten sam sposób |
| **Dwa edytory** | Dwa teksty obok siebie |
| **Planowanie** | Segregator, tablica korkowa i inspektor ze streszczeniem i notatkami |

**Widok > Układ > Zapisz bieżący układ** zapisuje okno takie, jakie jest teraz, pod wybraną nazwą; zaznacz „Zapamiętaj także rozmiar okna”, jeśli układ ma zmieniać rozmiar okna. **Zarządzaj układami** pokazuje wszystkie układy z małym obrazkiem i pozwala używać własnych układów, aktualizować je (zastąpić bieżącym oknem), zmieniać ich nazwy, usuwać, eksportować i importować. Wbudowanych układów nie można zmieniać; zapisz zmienione okno pod nową nazwą. Każdy układ może mieć skrót klawiaturowy (rozdział 19, grupa „Układy”).

### Mały interfejs

Dla małego ekranu albo do pisania bez klawiatury **mały interfejs** oddaje tekstowi całe okno i umieszcza kilka okrągłych przycisków przy jego krawędzi: w oknie wyższym niż szerokim jako rząd na dole, w oknie szerokim jako kolumnę przy prawej krawędzi. Włączasz i wyłączasz go poleceniem **Widok > Mały interfejs** (pozycja z polem wyboru) albo skrótem **Ctrl+Alt+S**. Pasek menu jest wtedy ukryty: naciśnij **Alt** i puść go, aby otworzyć menu przycisku menu, albo naciśnij **Alt** z literą menu (na przykład **Alt+I** dla Widoku), aby otworzyć to menu przy przycisku. Litera Alt, której nie używa żadne polecenie, nie wpisuje nic do tekstu. Przy uruchomieniu programu jest wyłączony i przełączasz go tylko ty; **Widok > Profile** zapamiętuje go jak każde inne ustawienie, więc profil może zaczynać w małym interfejsie. Na Androidzie pozycja ma zamiast tego trzy opcje: **Pełny**, **Mały** i **Automatyczny**; tryb automatyczny pokazuje mały interfejs na ekranie z mniej niż 1920 x 1080 pikseli fizycznych (zatrzymany w obu przypadkach).

Przyciski, od pierwszego: **Menu** (każde menu paska menu oraz **Przełącz na pełny interfejs**), **Nowy tekst** (nowy dokument), **Pogrubienie**, **Kursywa**, **Styl** (style akapitu projektu), **Segregator**, **Etykiety i ikony** (etykieta i ikona zaznaczonych dokumentów), **Tylko nagrywaj** i **Notatki**. Przyciski są okrągłe, używają kolorów twojego motywu, mają rozmiar zgodny z ustawieniem twoich pasków narzędzi (Widok > Paski narzędzi > Rozmiar pasków narzędzi, Duży na Androidzie) i nigdy nie są mniejsze niż 9 mm w szerokości i wysokości na twoim ekranie. Klawiatura nadal działa: do przycisków można dojść klawiszem Tab, skróty poleceń działają jak dotąd, a linia wokół części, która ma klawiaturę, jest pokazana.

- **Segregator i Notatki** otwierają się jako panele wsuwające się nad tekst: w szerokim oknie z lewej (segregator) i z prawej (notatki), zajmując dużą część szerokości; w oknie wyższym niż szerokim jako arkusz nad całym obszarem tekstu. Naraz otwarty jest jeden panel. Panel segregatora to sam segregator (kolekcje, wyszukiwanie, przyciski pod nim), panel notatek to inspektor z jego kartami, otwarty na notatkach dokumentu. **Zamknij**, **Esc** lub ponowne naciśnięcie przycisku zamyka panel; w oknie wyższym niż szerokim wybranie dokumentu w arkuszu segregatora zamyka go, aby pokazał się tekst.
- **Tylko nagrywaj** rozpoczyna i kończy nagrywanie bez zapisywania jego tekstu (tekst można zapisać później poleceniem **Transkrybuj** na liście transkrypcji). Wymaga dodatku **Speech Transcription**; bez niego lub z jego starszą wersją przycisk jest wyszarzony, a jego podpowiedź mówi dlaczego.
- Nie ma przycisku migawek, paski narzędzi są ukryte, a razem z nimi pasek menu, gdy działa mały interfejs. **Pokaż segregator**, **Pokaż inspektor**, **Układ** i **Paski narzędzi** w menu Widok są w tym czasie wyszarzone.

Wyłączenie małego interfejsu przywraca pełny interfejs taki, jaki był: paski narzędzi i ich miejsca, pasek menu, szerokości i wysokości segregatora, edytora i inspektora oraz to, czy segregator i inspektor były pokazane.

### Paski narzędzi i ich miejsca

Od 0.91 okno ma osiem pasków narzędzi w dwóch rzędach (od 0.95 dziesięć, z dwoma, które na początku są wyłączone). Pierwszy rząd zawiera **Główny** (Widok, nowy tekst, nowy folder, kosz, Cofnij, Ponów), **Motywy** (przycisk motywu, na początku wyłączony), **Profile** (używany profil i menu do jego przełączania, na początku wyłączony; od 0.95), **Wyszukiwanie** (przycisk wyszukiwania, pole szybkiego wyszukiwania, tryby widoku) i **Narzędzia** (zakładki, Szybki podgląd, komentarze, Kompiluj, Pokaż znaki niedrukowane, Tryb pisania, przyciski narzędzi pisarskich, inspektor). Drugi rząd zawiera **Styl i czcionka**, **Znak i wyrównanie**, **Akapit i wstawianie**, **pasek ikon** oraz **Etykiety** (jeden przycisk na każdy kolor etykiety otwartego projektu i **Bez etykiety**; na początku wyłączony; od 0.95).

- **Przesuwanie paska narzędzi.** Każdy pasek ma z przodu uchwyt (dwie kolumny kropek; wskaźnik zmienia się w krzyż ze strzałkami). Przeciągnij uchwyt, a pasek pójdzie tam, gdzie go zabierzesz:
  - **Wzdłuż swojego rzędu.** Dopóki wskaźnik pozostaje między środkami dwóch sąsiednich pasków, pasek podąża za nim od razu. Wolne miejsce przed paskiem jest tak szerokie, jak je zrobisz. Przesuwając w prawo, pasek najpierw zajmuje wolne miejsce za sobą; gdy dojdzie do następnego paska, popycha go dalej, tak daleko, jak paski za nim mogą się zmieścić razem na końcu rzędu. Przesuwając w lewo, zatrzymuje się tam, gdzie kończy się poprzedni pasek, a paski za nim zostają tam, gdzie były (przeciągnij je do niego, jeśli chcesz je znów mieć razem). Pierwszy pasek rzędu pozostaje przy krawędzi okna.
  - **Na inne miejsce w kolejności.** Przesuń wskaźnik poza środek sąsiedniego paska albo na dowolny inny pasek rzędu. Pasek na razie zostaje, gdzie jest, a linia w kolorze wyróżnienia pokazuje, gdzie stanie; puść przycisk myszy, aby go tam umieścić. Możliwa jest więc dowolna kolejność, także dla pierwszego i ostatniego paska rzędu.
  - **Do innego rzędu.** Przesuń wskaźnik do drugiego rzędu: linia pokazuje tam miejsce. Każdy pasek może stać w każdym rzędzie, pasek formatowania obok paska głównego albo pasek wyszukiwania wśród pasków formatowania.
  - **Do własnego rzędu lub na dolną krawędź.** Przesuń wskaźnik tuż pod rzędy pasków (pojawi się linia przez całe okno), aby otworzyć nowy rząd, albo na dolną krawędź okna, aby tam umieścić pasek. Dolna krawędź przyjmuje paski obok siebie i w rzędach, tak jak górna.
  - **Esc** lub puszczenie tam, gdzie nie widać linii (na przykład na środku okna), zostawia wszystko tak, jak było.

  Paski za dawnym miejscem się zsuwają, a wolne miejsce, które było przed paskiem, pozostaje w jego rzędzie; na nowym miejscu pasek stoi tuż za poprzednim paskiem i możesz go stamtąd przeciągnąć dalej w prawo. W oknie zbyt wąskim na pozostawione wolne miejsce miejsce się kurczy, zanim pasek straci przyciski; wraca, gdy okno znów jest szerokie. Pasek, który jest wyłączony, zachowuje swoje miejsce. Pasek nie może stać po bokach ani pływać jako własne okno. {{APP}} zapamiętuje rzędy, kolejność i odstępy.
- **Wyłączanie paska narzędzi.** **Widok > Paski narzędzi** ma przełącznik dla każdego paska; ta sama lista pojawia się po kliknięciu prawym przyciskiem na pasku lub na wolnym miejscu obok pasków. Lista jest podzielona na grupy: **Pokaż pasek narzędzi** z paskami pierwszego rzędu, **Pokaż pasek formatowania** z trzema paskami formatowania, potem **Pasek ikon** i **Etykiety**, **Przywróć położenie pasków narzędzi** oraz dwa polecenia, które dostosowują. **Pokaż pasek narzędzi** i **Pokaż pasek formatowania** przełączają, jak dawniej, cały rząd (cztery paski pierwszego rzędu, trzy paski formatowania), a układy używają tych dwóch; przełącznik pojedynczego paska jest niedostępny, gdy jego rząd jest wyłączony.
- **Widok > Paski narzędzi > Przywróć położenie pasków narzędzi** przywraca wszystkie paski na ich standardowe miejsca (dwa rzędy w standardowej kolejności, nic na dolnej krawędzi) i zamyka odstępy między nimi.

### Własne paski narzędzi

**Widok > Paski narzędzi > Dostosuj paski narzędzi** (lub prawy przycisk na pasku) otwiera okno z dwiema listami: dostępne polecenia po lewej, zawartość paska po prawej. Pasek wybierasz u góry.

- Dodaj polecenie przyciskiem ze strzałką lub dwukrotnym kliknięciem. Dostępne jest każde polecenie menu, także narzędzi pisarskich (Edycja > Narzędzia pisarskie); znajdzie je pole wyszukiwania nad listą.
- Usuń przycisk drugą strzałką lub dwukrotnym kliknięciem, a przesuń go przyciskami w górę i w dół albo przeciągając.
- **Separator** i **Elastyczny odstęp** można używać tyle razy, ile chcesz. Elastyczny odstęp przesuwa to, co za nim, w prawo; dwa z nich wyśrodkowują to, co stoi między nimi.
- Niektóre pozycje to więcej niż jedno polecenie: **Widok**, **Dodaj**, **Wyszukiwanie**, **Pole szybkiego wyszukiwania**, **Tryb widoku**, **Zakładki projektu**, **Komentarze i przypisy**, **Układy** i **Motywy** (przycisk motywu z jego menu) dla czterech pasków pierwszego rzędu; listy stylu, czcionki, rozmiaru i interlinii oraz cztery przyciski akapitu dla trzech pasków formatowania. Taki element istnieje raz: dodany do paska, przenosi się tam z paska, który go miał. Element pierwszego rzędu nie może trafić na pasek formatowania ani odwrotnie.
- Pod listami wybierasz rozmiar wszystkich pasków narzędzi (Mały, Normalny lub Duży; zob. niżej) oraz to, czy paski pierwszego rzędu pokazują tekst obok ikon, czy pod nimi.
- **Resetuj** przywraca na pasek przyciski, z którymi przyszedł program (o nic nie pyta; liczy się po **Zastosuj** lub **OK**).

**Rozmiar pasków narzędzi.** **Widok > Paski narzędzi > Rozmiar pasków narzędzi** (także w menu prawego przycisku każdego paska i w Dostosuj paski narzędzi) ustawia jeden rozmiar dla wszystkich pasków naraz: głównych pasków narzędzi, pasków formatowania, paska ikon oraz pasków Etykiet i Profili. **Mały** ma ikony 16 pikseli, **Normalny** 22, a **Duży** 32 (domyślny na tabletach z Androidem). Listy i pola na paskach (styl, czcionka, rozmiar, interlinia, szybkie wyszukiwanie, motywy) rosną i maleją razem z nim, dzięki czemu paski jednego rzędu mają jedną wysokość. Rozmiar skalowania ekranu w twoim systemie dochodzi do tego dodatkowo. Rozmiar należy do używanego profilu (zob. Profile) i jest zapamiętywany po zamknięciu programu.

Jeśli pasek zawiera więcej, niż mieści się w oknie, strzałka na jego końcu otwiera resztę. Pasek pozostawiony bez żadnego przycisku jest wyłączany; włączony ponownie w **Widok > Paski narzędzi** ma swój zestaw standardowy. Pasek ułożony w wersji przed 0.91 został podzielony na nowe paski: każdy przycisk trafił na pasek, który go zawiera w zestawach standardowych, a to, co zdjąłeś(-aś), pozostaje zdjęte.

### Pasek ikon

**Pasek ikon** zawiera do dziesięciu ikon dokumentów, których często używasz. Kliknięcie jednej z nich nadaje tę ikonę zaznaczonym dokumentom i folderom (w segregatorze albo w tablicy korkowej lub konspekcie, w którym pracujesz); przycisk **↺** za nimi przywraca im ikonę standardową, a ostatni przycisk wybiera ikony paska. Pasek pokażesz lub ukryjesz poleceniem **Widok > Paski narzędzi > Pasek ikon**.

**Widok > Paski narzędzi > Wybierz ikony dla paska ikon…** (także w menu prawego przycisku paska) otwiera okno ze wszystkimi ikonami po lewej, w tym twoimi własnymi ikonami projektu, i ikonami paska po prawej. Dodawaj przyciskiem **Dodaj →** lub dwukrotnym kliknięciem, usuwaj przyciskiem **← Usuń** lub dwukrotnym kliknięciem, a kolejność zmieniaj przyciskami **W górę** i **W dół**. **Zestaw standardowy** przywraca dziesięć ikon, z którymi przyszedł program (Zielona książka, Korekta, Zaznaczone, Niezaznaczone, Znak zapytania, Czerwona flaga, Żółty notes, Karta postaci, Karta miejsca, Ostrzeżenie).

---

## 3. Segregator

Każdy projekt ma trzy specjalne foldery:

- **Szkic** (w niektórych szablonach *Manuskrypt*): wszystko, co stanie się twoją książką. Kompilowane jest tylko to, co jest wewnątrz Szkicu.
- **Materiały**: notatki, karty postaci, obrazy, pliki PDF i wszystko inne, co zbierasz.
- **Kosz**: usunięte pozycje czekają tutaj, aż wybierzesz **Projekt > Opróżnij kosz**. Jego pytanie, jak każde pytanie przed czymś, czego nie da się cofnąć, nazywa krok na swoim przycisku i ma **Anuluj** jako domyślne, więc klawisz Enter nigdy niczego nie usuwa (0.70).

### Praca z pozycjami

- **Nowy dokument**: **Ctrl+N** (lub przycisk ➕). Jest dodawany pod zaznaczoną pozycją. Dokument o nazwie „Bez tytułu” (lub z pustym tytułem) przejmuje pierwszy wiersz swojego tekstu jako tytuł, gdy tylko zaczniesz pisać: co najwyżej 60 znaków, ucięte na granicy słowa, bez adnotacji śródtekstowych, przypisów śródtekstowych i obrazów. Tytuł podąża za pierwszym wierszem, dopóki sam nie zmienisz nazwy dokumentu; dokument, którego tekst usuniesz, znów jest „Bez tytułu”. Dokumenty „Bez tytułu”, które mają już tekst, dostają nazwę przy otwarciu projektu. Tytuł jest zwykłym tytułem: jest zapisywany, kompilowany (gdy tytuły są kompilowane, pierwszy wiersz będący nagłówkiem pojawia się dwa razy) i zapisywany w projektach Scrivener.
- **Nowy folder**: **Alt+Shift+N**.
- **Zmiana nazwy**: **F2** albo powolne dwukrotne kliknięcie tytułu.
- **Przenoszenie**: przeciągaj pozycje myszą albo użyj **Ctrl+Alt+klawisze strzałek**: w górę i w dół przesuwa w obrębie folderu; w lewo wyprowadza z folderu; w prawo wprowadza do pozycji powyżej. Dokument upuszczony *między* dwiema pozycjami trafia w to miejsce; upuszczony *na* pozycję staje się jej poddokumentem. Jeśli przypadkowo zagnieździsz dokumenty, wyłącz upuszczanie na dokumenty tekstowe w **Plik > Opcje > Zachowanie** (foldery zawsze przyjmują upuszczane pozycje).
- **Otwieranie lub zamykanie całej gałęzi**: przytrzymaj **Alt** i kliknij strzałkę folderu lub grupy.
- **Usuwanie**: naciśnij **Delete**. Pozycja trafia do Kosza; tablica korkowa lub konspekt, które pokazywały ją jako jeden z kilku wybranych dokumentów, zwalnia ją od razu (0.94.0).
- **Duplikowanie**: **Ctrl+D**.
- **Dzielenie i scalanie**: **Dokumenty > Podziel > W miejscu zaznaczenia** (**Ctrl+K**) przecina dokument w miejscu kursora na dwa; nie działa wewnątrz tabeli ani podpisu obrazu (ustaw kursor przed nią lub za nią). **Dokumenty > Scal** (**Ctrl+Shift+M**) łączy zaznaczone dokumenty w pierwszy, razem z ich streszczeniami, słowami kluczowymi i notatkami; pozostałe trafiają do Kosza.
- **Zmiana ikony**: prawy przycisk > **Zmień ikonę** (lub **Dokumenty > Zmień ikonę**) albo pasek ikon (rozdział 2). Zob. „Ikony” niżej.

**Prawy przycisk myszy.** Na dokumencie menu prawego przycisku zawiera polecenia dla zaznaczonych dokumentów (zmiana nazwy, duplikowanie, przeniesienie do Kosza, migawki, etykieta, status i inne). Na pustym miejscu pod dokumentami oferuje tylko to, co tworzy coś nowego: **Nowy tekst**, **Nowy folder**, **Nowy z szablonu** oraz **Wygląd** segregatora.

Dokument może mieć poddokumenty, tak jak folder. Foldery i dokumenty zachowują się tak samo; foldery po prostu wyglądają inaczej i domyślnie otwierają się jako tablica korkowa.

**Klawisze segregatora (0.94.0).** Gdy klawiatura jest w segregatorze, **Enter** otwiera zaznaczony dokument w edytorze i przenosi tam klawiaturę (jak w menedżerze plików), a klawisze strzałek poruszają się po liście. **Spacja** jest częścią wpisywania nazwy: wpisz pierwsze litery tytułu, także ze spacjami („Rozdział t”), a zaznaczenie przeskoczy do pierwszego tytułu, który tak się zaczyna. Pozycja **Widok > Klawisze szybkiego dodawania w segregatorze** jest na początku wyłączona (także po aktualizacji). Po włączeniu segregator ma klawisze szybkiego dodawania: **Enter** tworzy nowy pusty dokument pod zaznaczonym, a **Spacja** otwiera zaznaczone dokumenty jako panele Szybkiego podglądu (i na liście kolekcji lub wyszukiwania).

Kliknięcie w segregatorze ładuje dokument do edytora. Po **Wstecz**, po odnośniku do dokumentu lub przy zablokowanym edytorze edytor może pokazywać inny dokument niż zaznaczony w segregatorze: **szary pasek** oznacza wtedy w segregatorze dokument edytora. **Nawigacja > Pokaż w segregatorze** (**Ctrl+Shift+R**) go zaznacza.

**Wąski segregator, głębokie foldery.** Każdy poziom folderów jest wcięty o 12 px (o połowę mniej niż dawniej; zmienisz to w **Plik > Opcje > Wygląd > Segregator > Wcięcie na poziom**). Gdy tytuły są dłuższe niż szerokość segregatora, na dole segregatora pojawia się pasek przewijania, dzięki czemu możesz przeczytać koniec długich tytułów bez poszerzania segregatora.

### Ikony

**Dokumenty > Zmień ikonę** (także w menu prawego przycisku segregatora, tablicy korkowej i konspektu) jest zbudowane jak menu ikon programu Scrivener 3 dla Windows:

- **Przywróć ikonę domyślną**: wraca standardowa ikona dokumentu, folderu lub pliku.
- **Ikona z tekstu…**: tworzy ikonę z maksymalnie trzech znaków (na przykład *PR*, *Ch1* lub *A*). Wybierz kolor pola, liter i obramowania; mały obrazek pokazuje wynik. Ikona jest przechowywana w projekcie i od razu nadawana zaznaczonym dokumentom.
- **Zarządzaj ikonami…**: ikony tego projektu. **Dodaj obraz…** zamienia obrazy (PNG, SVG, JPEG i inne) w ikony, **Z tekstu…** to to samo co wyżej, **Zmień nazwę…** i **Usuń** zmieniają listę (dokumenty z usuniętą ikoną dostają z powrotem ikonę standardową), **Użyj dla zaznaczonych dokumentów** nadaje wybraną ikonę zaznaczonym dokumentom. Ikony są przechowywane w projekcie (folder *icons*), więc podróżują razem z nim.
- **Ostatnie**: ostatnie pięć wybranych ikon.
- **Książka**, **Postacie**, **Flaga**, **Miejsca**, **Notatki**, **Do zrobienia**, **TV**, **Przystanek**: podmenu z ikonami każdej grupy w kilku kolorach (na przykład Niebieska, Zielona, Czerwona i Żółta książka; siedem flag; pięć notesów; Zaznaczone i Niezaznaczone, Zaliczone, Niezaliczone i Niejasne w Do zrobienia).
- Dalej pojedyncze ikony: Końcowe części książki, Tablica, Kalendarz, Klaps filmowy, Zegar, Koło zębate, Oko, Liczby i wykresy, Szafa na akta, Początkowe części książki, Informacja, Mównica, Żarówka, Lupa, Muzyka, Nuty, Konspekt, Ołówek, Korekta, Znak zapytania, Uwagi recenzenta, Nożyczki, Dymek mowy, Cudzysłowy, Statystyki, Folder synchronizacji, Tabele, Probówka, Dymek myśli i Ostrzeżenie.
- **Ikony projektu**: twoje własne ikony (gdy projekt jakieś ma) oraz **Więcej ikon (pulpit)**: ikony motywu pulpitu, które oferowały wcześniejsze wersje.

**Kolor etykiety w ikonach.** Białe części wielu ikon (kartki notesu, płótno białej flagi, papier kartki, pole ikony zrobionej z tekstu) przyjmują kolor etykiety dokumentu. Włączysz to lub wyłączysz w **Widok > Używaj koloru etykiety w > Ikonach** (lub **Plik > Opcje > Wygląd > Segregator**).

**Ikony i Scrivener.** Ikony programu {{APP}} są narysowane od nowa, ale każda z nich nosi nazwę ikony programu Scrivener, którą zastępuje. Projekt w formacie Scrivener (rozdział 14) zachowuje te nazwy, więc Scrivener pokazuje dla nich własną ikonę; ikony zrobione w programie Scrivener wracają tutaj jako pasująca ikona. Twoje własne ikony trafiają do folderu *Icons* w *.scriv*. Ikona, dla której ten program nie ma obrazka, zachowuje swoją nazwę i jest tu pokazywana jako najbliższa ikona.

### Etykiety, status i „Uwzględnij w kompilacji”

Kliknij pozycję prawym przyciskiem, aby ustawić jej **etykietę** (kolor, na przykład dla postaci, z której perspektywy toczy się narracja) i **status** (na przykład *Pierwsza wersja* lub *Gotowe*). Listy zmienisz w **Projekt > Ustawienia projektu**.

Dokument lub folder z etykietą pokazuje jej kolor także w inspektorze: karta streszczenia i notatki dokumentu przyjmują jasny odcień koloru etykiety (15 % jego nasycenia), dzięki czemu od razu widać, jaką etykietę ma pozycja. Pozycje bez etykiety lub z szarą zachowują normalny kolor karty.

Pasek narzędzi **Etykiety** (**Widok > Paski narzędzi > Etykiety**, na początku wyłączony) pokazuje obok siebie wszystkie kolory etykiet otwartego projektu, z przyciskiem dla braku etykiety. Kliknięcie od razu ustawia etykietę zaznaczonych dokumentów, tak jak **Dokumenty > Etykieta**; podpowiedź każdego koloru podaje nazwę etykiety. Etykiety, z którymi startuje nowy projekt, ustawisz w **Plik > Opcje > Wygląd > Kolory standardowe**.

Pozycje z wyłączonym **Uwzględnij w kompilacji** pozostają w Szkicu, ale są pomijane przy kompilacji.

### Kolekcje

Kolekcje to dodatkowe listy dokumentów, na przykład „Do sprawdzenia” lub „Sceny z Anną”. Pokaż je przyciskiem **Kolekcje**. Użyj przycisku ➕ nad segregatorem, aby utworzyć kolekcję z zaznaczonych dokumentów, albo kliknij dokumenty prawym przyciskiem i wybierz **Dodaj do kolekcji**. Usunięcie dokumentu z kolekcji go nie kasuje. Zapisane wyszukiwania (rozdział 9) są również kolekcjami; aktualizują się same.

- **Dodawaj dokumenty**, przeciągając je z segregatora (lub z ikony nagłówka edytora, tablicy korkowej, konspektu) na kartę kolekcji.
- **Wewnątrz listy** kolekcji przeciągaj dokumenty w wybraną kolejność (kolejność w segregatorze się nie zmienia), upuszczaj dokumenty w wybranym miejscu i naciśnij **Delete**, aby wyjąć zaznaczone dokumenty z kolekcji. Kolekcji wyszukiwań nie można w ten sposób zmieniać.
- **Przeciągaj karty**, aby ustawić kolekcje w innej kolejności.
- Dokumenty na liście można przeciągać jak te w segregatorze, a **Spacja** otwiera zaznaczone jako panele Szybkiego podglądu.

---

## 4. Pisanie

### Formatowanie tekstu

| Czynność | Skrót |
|--------|----------|
| Pogrubienie / kursywa / podkreślenie | Ctrl+B / Ctrl+I / Ctrl+U |
| Wyróżnienie | Ctrl+Shift+H |
| Indeks górny / indeks dolny | Ctrl+Shift++ / Ctrl+= (układ niemiecki: Ctrl+Shift+D dla indeksu dolnego, zob. „Skróty a układ klawiatury” niżej) |
| Wyrównanie do lewej / wyśrodkowanie / do prawej | Ctrl+{ / Ctrl+\| / Ctrl+} (układ niemiecki: Alt+Shift+L / Alt+Shift+E / Alt+Shift+R) |
| Okno czcionek | Ctrl+T |
| Rozmiar czcionki większy / mniejszy | Ctrl+> / Ctrl+< |
| Okno akapitu | Ctrl+Alt+P |
| Kopiuj formatowanie / wklej formatowanie | Ctrl+Alt+C / Ctrl+Alt+V |
| Powiększ / pomniejsz / przywróć powiększenie | Ctrl++ / Ctrl+- / Ctrl+0 |
| Powiększenie obszaru pod myszą | Ctrl+kółko myszy |
| Pokaż lub ukryj linijkę | Ctrl+Alt+R |
| Wklej bez formatowania | Ctrl+Shift+V |
| Wklej z formatowaniem (z innych programów) | Ctrl+Alt+Shift+V |
| Cofnij / ponów | Ctrl+Z / Ctrl+Shift+Z (zob. „Cofanie i ponawianie” niżej) |

Pogrubienie, kursywa, indeks górny i tym podobne działają na zaznaczenie. Bez zaznaczenia, wewnątrz słowa zmieniają całe słowo; na końcu słowa (lub gdy wokół kursora nic nie ma) zmieniają to, co wpiszesz dalej: wpisz „H”, naciśnij Ctrl+=, wpisz „2”, naciśnij ponownie Ctrl+= i wpisz „O”, aby otrzymać H₂O.

**Style** (Nagłówek 1–6, Tytuł, Podtytuł, Cytat blokowy i twoje własne) wybiera się z listy po lewej stronie paska formatowania. Style mówią Kompilacji, czym jest akapit, więc używaj ich zamiast powiększać i pogrubiać tekst ręcznie. Zob. „Style” niżej.

**Co pokazuje pasek formatowania.** Pasek pokazuje tekst tak, jak go widzisz, dla całego zaznaczenia. **Pogrubienie**, **Kursywa**, **Podkreślenie**, **Przekreślenie**, indeks górny i indeks dolny są włączone, gdy cały zaznaczony tekst tak wygląda, także wtedy, gdy czyni to jego styl (pogrubienie nagłówka). Kliknięcie przełącza to, co widzisz: **Pogrubienie** na pogrubionym nagłówku sprawia, że nie jest pogrubiony, i pozostaje niepogrubiony, gdy projekt zostanie zapisany i otwarty ponownie. Czcionka, rozmiar, interlinia, przyciski wyrównania i lista stylów też pokazują zaznaczenie; gdy zaznaczenie zawiera dwie czcionki, dwa rozmiary, dwa wyrównania lub akapity dwóch stylów, to pole pozostaje puste (żaden przycisk wyrównania nie jest włączony). Rozmiar wpisany w puste pole nadaje ten rozmiar całemu zaznaczeniu.

**Listy**: użyj przycisków list albo wpisz „- ” lub „1. ” na początku akapitu.

**Inteligentna typografia**: proste cudzysłowy zamieniają się w zakrzywione, a dwa łączniki w pauzę podczas pisania. Wyłączysz to w **Plik > Opcje > Edycja**.

**Tryb rewizji** (**Format > Tryb rewizji**): wszystko, co wpiszesz, jest kolorowane jednym z pięciu kolorów rewizji, dzięki czemu widać, co zmieniło się w rundzie redakcji. Wybierz **Wyłączony**, aby przestać.

**Odstępy i wcięcia w pasku formatowania.** Cztery przyciski za przyciskami list ustawiają akapity, w których jest kursor lub zaznaczenie: **odstęp przed** i **odstęp po** akapicie (0, 3, 6, 9, 12 lub 18 pkt), **wcięcie pierwszego wiersza** i **wcięcie całego akapitu** (w centymetrach lub calach, tak jak pokazuje linijka). Wartość akapitu przy kursorze jest zaznaczona; **Inna...** pyta o dowolną wartość. **Edycja > Cofnij** cofa zmianę. Pasek formatowania ułożony przez ciebie dostaje te cztery przyciski raz, za przyciskami list; **Widok > Paski narzędzi > Dostosuj paski narzędzi** przesuwa je lub usuwa.

**Cofanie i ponawianie.** **Edycja > Cofnij** i **Edycja > Ponów** działają w polu tekstowym, które ma klawiaturę (edytor, streszczenie, notatki, tytuł) i mówią, co cofną: „Cofnij pisanie”, „Cofnij wklejenie”, „Cofnij pogrubienie”, „Ponów usunięcie”. Gdy klawiatura jest gdzie indziej, na przykład w segregatorze, są wyszarzone: przenoszenia dokumentów do Kosza, opróżniania Kosza i usuwania migawek nie da się w ten sposób cofnąć (dokument w Koszu wraca po wyciągnięciu go stamtąd; migawki zob. rozdział 11). Strzałka obok przycisków **Cofnij** i **Ponów** na pasku narzędzi wymienia kroki, następny na początku; wybranie jednego cofa (lub przywraca) ten krok i każdy krok przed nim na liście. Każde polecenie menu Format jest jednym krokiem.

**Kolor tekstu i wyróżnienie w pasku formatowania.** Dwa przyciski z kolorowym paskiem (pióro dla wyróżnienia, litera *T* dla koloru tekstu) działają jak te w programach Scrivener i Word: kliknięcie nakłada kolor pokazany w pasku na zaznaczony tekst albo, bez zaznaczenia, na to, co wpiszesz dalej. Zatrzymaj mysz na przycisku na chwilę, a pod nim otworzy się małe okno z kolorami **czerwonym, niebieskim, żółtym, zielonym, białym i czarnym** oraz **Bez wyróżnienia** (lub **Automatyczny** dla koloru tekstu) i **…** dla dowolnego innego koloru. Kliknięcie koloru nakłada go i czyni go kolorem przycisku, który zachowuje go na następne kliknięcie (także po ponownym uruchomieniu). Wyróżnienie jest rysowane dokładnie w kolorze, który wybrałeś(-aś), zarówno na jasnej, jak i na ciemnej stronie. Tam, gdzie tekst na nim byłby trudny do odczytania (poniżej kontrastu 4,5 : 1), tekst jest pokazywany ciemno lub jasno tylko na ekranie; jego zapisany kolor oraz wyróżnienie w zapisanych i skompilowanych plikach pozostają takie, jak wybrano.

### Czcionki

Cały tekst projektu ma jeden wygląd: **formatowanie domyślne** (zob. niżej). Na nim możesz nadać dowolnemu fragmentowi tekstu własną czcionkę.

- **Pasek formatowania**: lista czcionek i pole rozmiaru zmieniają zaznaczony tekst od razu. Bez zaznaczenia zmieniają **cały akapit**, w którym jest kursor (i to, co wpiszesz tam dalej). Strzałki pola rozmiaru zmieniają tekst przy każdym kliknięciu; wpisany rozmiar zaczyna działać po **Enter**. Pasek formatowania, menu Format i ich skróty działają na tekst, w którym kursor był ostatnio: edytor, notatki w inspektorze, tekst zakładki w inspektorze albo długie pole tekstowe obiektu (tam: pogrubienie, kursywa, podkreślenie, przekreślenie, kolor tekstu i wyróżnienie). Kliknięcie w polu rozmiaru tego nie zmienia.
- Po zmianie w pasku formatowania pasek nad edytorem proponuje to samo dla całego projektu: w akapicie ze stylem (na przykład nagłówku) **Zmień styl „Nagłówek 1”** nadaje stylowi nową czcionkę lub rozmiar, a cały tekst tego stylu w każdym dokumencie za nim podąża; w akapicie bez stylu **Użyj dla całego tekstu** czyni ją czcionką lub rozmiarem projektu (zob. „Formatowanie domyślne” niżej). Kliknij **×** albo po prostu pisz dalej gdzie indziej, aby zostawić zmianę tam, gdzie jest. Każdy dokument, który się zmienia, dostaje wcześniej migawkę.
- **Format > Czcionka > Pokaż czcionki…** (**Ctrl+T**) otwiera okno czcionek: czcionka, krój (cienki, zwykły, pogrubiony, kursywa i cokolwiek jeszcze oferuje czcionka), rozmiar, podkreślenie, przekreślenie, kapitaliki (małe lub wielkie litery), odstępy między literami w procentach i kolor tekstu. Stosowane jest tylko to, co zmienisz w oknie, więc zaznaczenie ze słowami pogrubionymi i zwykłymi je zachowuje. **Czcionka projektu** przywraca tekst do czcionki, rozmiaru i koloru projektu.
- **Pogrubienie, Kursywa, Podkreślenie, Przekreślenie**: przycisk jest włączony, gdy *cały* zaznaczony tekst ma ten format, niezależnie od tego, jak go zaznaczyłeś(-aś) (także od prawej do lewej). Zaznaczenie pogrubione tylko częściowo pokazuje Pogrubienie wyłączone, a **Pogrubienie** pogrubia wtedy całość; drugie kliknięcie czyni całość zwykłą. W oknie czcionek wybrany krój jest również ustawiany dla całego zaznaczenia.
- **Format > Czcionka > Większa / Mniejsza** (**Ctrl+>** / **Ctrl+<**) zmieniają rozmiar o jeden punkt; każda część zaznaczenia zachowuje własny rozmiar i rośnie lub maleje.
- **Format > Czcionka > Kolor tekstu…** i **Automatyczny kolor tekstu**.

Wybranie czcionki lub rozmiaru samego projektu jest zapisywane jako „brak własnej czcionki”: taki tekst podąża za projektem, gdy później zmienisz formatowanie domyślne.

**Czcionki z kilkoma wzorami.** Niektóre czcionki trzymają kilka wzorów jednego kroju pod jedną nazwą: na przykład Sitka z Windows 11 zawiera rozmiary optyczne Small, Text, Subheading, Heading, Display i Banner (każdy narysowany dla własnego zakresu rozmiarów, większe węższe i z mniejszymi małymi literami), a niektóre czcionki w ten sam sposób zawierają wersje wąskie lub szerokie. {{APP}} wymienia każdy wzór jako osobną czcionkę (na przykład **Sitka Small**, **Sitka Text**, **Sitka Banner**), tylko z krojami tego wzoru, dzięki czemu pogrubienie i kursywa pozostają w obrębie wybranego wzoru; przed 0.40.1 pogrubiony tekst mógł wyjść w innym wzorze i wyglądać na mniejszy i węższy. Pliki czcionek nie są zmieniane. Czcionka o pierwotnej nazwie zachowuje własny wzór, więc tekst w niej złożony wcześniej zachowuje swój wygląd.

**Instalowanie czcionek.** **Format > Czcionka** ma polecenia dla czcionek, których nie ma jeszcze na twoim komputerze:

- **Zainstaluj pliki czcionek…**: wybierz pliki czcionek (TTF, OTF, TTC, PFB, WOFF lub plik ZIP, który je zawiera). Są kopiowane do twojego własnego folderu czcionek (*~/.local/share/fonts*), a listy czcionek programu {{APP}} znają je od razu; inne programy zobaczą je po następnym uruchomieniu. Hasło nie jest potrzebne, a czcionki są tylko dla twojego użytkownika.
- **Otwórz folder czcionek**: otwiera ten folder w menedżerze plików; czcionki, które sam tam włożysz, zostaną znalezione po **Wczytaj ponownie listę czcionek**.
- **Otwórz menedżer czcionek pulpitu**: otwiera menedżer czcionek KDE (Ustawienia systemowe > Tekst i czcionki > Zarządzanie czcionkami) lub program do czcionek innego pulpitu, w którym możesz też instalować czcionki dla wszystkich użytkowników i je usuwać.
- **Wczytaj ponownie listę czcionek**: szuka nowych czcionek i je nazywa.

### Formatowanie akapitu

**Format > Akapit > Akapit…** (**Ctrl+Alt+P**) ustawia dla akapitów, w których jest kursor lub zaznaczenie: wyrównanie, wcięcie lewe i prawe, wcięcie pierwszego wiersza (wartość ujemna daje wcięcie wiszące), odstęp przed i po, interlinię (wielokrotność wysokości wiersza, dokładną wysokość lub wysokość minimalną) oraz tabulatory (lewy, wyśrodkowany, prawy, dziesiętny). Stosowane jest tylko to, co zmienisz. **Przywróć domyślne** usuwa formatowanie ustawione ręcznie; akapit znów podąża za swoim stylem i projektem.

Szybkie sposoby: przyciski wyrównania, lista interlinii w pasku formatowania, **Format > Akapit > Interlinia** oraz **Zwiększ wcięcie / Zmniejsz wcięcie**.

Zastosowanie stylu akapitu zastępuje formatowanie ustawione ręcznie w akapitach, których dotyczy: wcięcia, odstępy, wyrównanie i tabulatory oraz czcionki, rozmiary, kolory, odstępy między literami i kapitaliki tekstu w nich. Pogrubienie, kursywa, podkreślenie, przekreślenie, wyróżnienia i style znaków pozostają. Zaznacz tekst obejmujący kilka akapitów, aby nadać styl im wszystkim, każdemu w całości; zaznaczenie kończące się tuż na początku akapitu (na przykład po **Shift+Down**) pomija ten akapit.

**Format > Kopiuj formatowanie** (**Ctrl+Alt+C**) zapamiętuje formatowanie czcionki i akapitu przy kursorze; **Zastosuj skopiowane formatowanie** (**Ctrl+Alt+V**) stosuje je do zaznaczenia albo do całego akapitu, gdy nic nie jest zaznaczone.

### Znaki niewidoczne

**Widok > Edycja tekstu > Pokaż znaki niedrukowane** (także przycisk ¶ na pasku narzędzi) pokazuje znaki, których nie widzisz: kropkę dla każdej spacji, strzałkę dla tabulatora, małe kółko dla spacji nierozdzielającej, zagiętą strzałkę dla złamania wiersza (**Shift+Enter**) i ¶ na końcu każdego akapitu. Są rysowane tylko na ekranie, w głównym tekście każdego edytora; nigdy nie są częścią tekstu, wydruku ani kompilacji. Wybór jest zapamiętywany. Ich kolor to kolor tekstu, wyblakły w papier; własny kolor ustawisz w **Plik > Opcje > Wygląd > Znaczniki tekstowe** (lub kliknij tekst prawym przyciskiem > **Wygląd**) i należy on do wyglądu projektu tak jak papier i tekst (tryb pisania zachowuje wyblakły kolor tekstu).

### Linijka

**Widok > Edycja tekstu > Pokaż linijkę** (**Ctrl+Alt+R**) pokazuje linijkę nad tekstem. Należy do akapitu, w którym jest kursor. **Pokaż pasek formatowania** jest w tym samym menu, zaraz po nim (0.99.0), oraz w **Widok > Paski narzędzi**.

- Znacznik u góry to **wcięcie pierwszego wiersza**, znaczniki na dole to **wcięcie lewe** i **wcięcie prawe**. Przeciągaj je. Wcięcie lewe zabiera ze sobą pierwszy wiersz.
- **Kliknij** w dolnej połowie linijki, aby dodać tabulator. Przeciągnij tabulator, aby go przesunąć; przeciągnij go w górę lub w dół poza linijkę, aby go usunąć. **Kliknij prawym przyciskiem** tabulator, aby uczynić go lewym, wyśrodkowanym, prawym lub dziesiętnym.
- Znaczniki przyciągają się do ćwiartek centymetra (szesnastek cala); przytrzymaj **Shift**, aby ustawiać je swobodnie.
- **Dwukrotne kliknięcie** linijki otwiera okno akapitu. Menu prawego przycisku przełącza między centymetrami i calami (także w **Plik > Opcje > Edycja**).

Tabulatory są mierzone od początku pierwszego wiersza akapitu. Bez własnych tabulatorów akapit ma pozycję tabulacji co 2,1 cm.

Linijka pokazuje rzeczywiste długości. Powiększenie powiększa litery na ekranie, ale nie wcięcia i tabulatory, więc porównuj odległości przy 100 %.

### Formatowanie domyślne

- **Ten projekt**: **Projekt > Ustawienia projektu > Formatowanie** (także przycisk z suwakami w pasku formatowania) ustawia czcionkę, rozmiar, interlinię, wcięcie pierwszego wiersza, odstęp po akapitach i wyrównanie tekstu całego projektu (cztery przyciski: do lewej, wyśrodkowane, do prawej, wyjustowane). W Panelu stylów **Bez stylu** > **Zdefiniuj na nowo z zaznaczenia** bierze te wartości z akapitu przy kursorze (zob. „Style” niżej). **Zastosuj** zmienia wszystkie dokumenty naraz i pozostawia okno otwarte, abyś mógł(-a) zobaczyć wynik za nim; **OK** robi to samo i zamyka okno. **Użyj bieżących** bierze wartości z akapitu, w którym jest kursor.
  - Dzięki **Zmieniaj także tekst z własną czcionką, rozmiarem lub odstępami** (domyślnie włączone) tekst sformatowany ręcznie lub taki, który przyszedł z importu z własną czcionką, też podąża, o ile ma główną czcionkę, rozmiar i odstępy swojego dokumentu. To, co różni się od tekstu głównego, pozostaje: większy wiersz nagłówka, fragment w innej czcionce, akapit bez wcięcia. Wyłącz pole, aby zostawić w spokoju cały tekst sformatowany ręcznie.
  - Każdy dokument, który się zmienia, dostaje wcześniej migawkę (inspektor, karta migawek, „Przed zmianą formatowania projektu”), więc możesz wrócić.  Dokumenty scenariuszy zachowują własny format.
- **Nowe projekty**: **Plik > Opcje > Edycja > Formatowanie > Styl tekstu głównego dla nowych projektów**, z tymi samymi czterema przyciskami wyrównania i **Użyj formatowania z bieżącego edytora**. Istniejące projekty zachowują swoje formatowanie. Standard to 11,0 pkt, interlinia 1,09, brak wcięcia pierwszego wiersza i 6 pkt po akapitach. Projekty zaimportowane ze Scrivenera zaczynają także od tych wartości (z czcionką najczęściej używaną w projekcie Scrivener).
- **Powrót do domyślnego**: zaznacz dokumenty w segregatorze i wybierz **Dokumenty > Konwertuj > Tekst na formatowanie domyślne…**. Decydujesz, czy czcionki i kolory, wcięcia i odstępy, wyrównanie i tabulatory są usuwane. Pogrubienie, kursywa, style, notatki i odnośniki zawsze pozostają. Wcześniej wykonywana jest migawka każdego dokumentu (inspektor, karta migawek), więc możesz wrócić.

### Style

Styl zapisuje formatowanie pod nazwą. Tekst pamięta nazwę, więc gdy zmienisz styl, zmienia się cały tekst, który go używa, w całym projekcie.

Są dwa rodzaje:

- **Style akapitu** dotyczą całych akapitów. Zawierają formatowanie akapitu (wyrównanie, wcięcia, odstępy, interlinię, tabulatory) i, jeśli chcesz, formatowanie znaków dla akapitu (czcionka, rozmiar, pogrubienie, kursywa, kolor). Na listach są oznaczone „¶”, a „¶a”, gdy zawierają formatowanie znaków.
- **Style znaków** dotyczą zaznaczonego tekstu lub słowa przy kursorze. Zawierają tylko formatowanie znaków. Są oznaczone „a”.

Zostaw tekst główny jako **Bez stylu**. Podąża za formatowaniem domyślnym projektu, a Kompilacja może nadać mu dowolny wygląd. Używaj stylów tylko dla tekstu, który ma wyglądać inaczej niż tekst główny.

**Stosowanie stylu**: wybierz go z listy po lewej stronie paska formatowania, w **Format > Styl**, w menu prawego przycisku na tekście albo w **Panelu stylów** (**Format > Styl > Panel stylów**), małym oknie, które pozostaje otwarte obok tekstu; dwukrotne kliknięcie wiersza stosuje styl. **Bez stylu** usuwa z zaznaczenia styl znaków; gdy go nie ma, usuwa styl akapitu. Okna i pytania otwierane przez Panel stylów pojawiają się nad panelem.

**Lista stylów** paska formatowania, menu prawego przycisku (**Styl**) i **Format > Styl** (0.99.0; przy więcej niż dziesięciu stylach lista się przewija, jak w pasku formatowania) pokazuje wybrane przez ciebie style, w wybranej kolejności: **Dostosuj listę stylów…** (na dole listy i w **Format > Styl**) wymienia wszystkie pozycje; odznacz te, których nie potrzebujesz, przeciągnij je w inną kolejność lub użyj strzałek, a **Wszystkie, w kolejności stylów** przywraca pełną listę. Przy więcej niż dziesięciu pozycjach lista pokazuje dziesięć i się przewija. Pod nią, zawsze widoczne: **Nowy styl z zaznaczenia…**, **Zdefiniuj styl na nowo z zaznaczenia…**, **Panel stylów** i **Dostosuj listę stylów…**. Styl, którego nie ma na liście, jest nadal nazwany w pasku formatowania, gdy tekst go używa; **Format > Styl** zawsze wymienia wszystkie style. Wybór należy do projektu (nowy styl pojawia się na końcu listy) i jest częścią stylów w plikach ustawień projektu.

**Bez stylu to styl tekstu głównego.** W Panelu stylów kliknij prawym przyciskiem **Bez stylu** (lub zaznacz go i użyj przycisku Zdefiniuj na nowo) i wybierz **Zdefiniuj na nowo z zaznaczenia…**: czcionka, rozmiar, interlinia, wcięcie pierwszego wiersza, odstęp po akapitach i wyrównanie akapitu przy kursorze stają się stylem tekstu głównego projektu (wartościami **Projekt > Ustawienia projektu > Formatowanie**). Pytanie najpierw pokazuje wartości. Cały tekst bez stylu w całym projekcie za tym podąża, także tekst z własną czcionką, rozmiarem lub odstępami; tekst ze stylem zachowuje to, co ustawia jego styl. Każdy dokument, który się zmienia, dostaje wcześniej migawkę.

**Tworzenie stylu**:

1. Sformatuj jakiś tekst tak, jak ma wyglądać styl, i kliknij w niego lub zaznacz jego część.
2. Wybierz **Format > Styl > Nowy styl z zaznaczenia…** (lub przycisk plusa w Panelu stylów).
3. Wpisz nazwę i wybierz, co zapisać: całe formatowanie (styl akapitu z formatowaniem znaków), tylko formatowanie akapitu albo atrybuty znaków (styl znaków).
4. Zdecyduj, czy czcionka i jej rozmiar są częścią stylu. Bez nich styl zachowuje czcionkę otaczającego tekstu.
5. Dla stylów akapitu wybierz, jaki styl dostaje następny akapit po naciśnięciu Enter na końcu.

Styl akapitu zapisuje tylko to, co różni się od tekstu głównego. Wszystko inne nadal podąża za formatowaniem projektu, więc nowa czcionka domyślna dociera także do tekstu ze stylem.

**Zmiana stylu**:

- **Format > Styl > Zdefiniuj styl ponownie z zaznaczenia…** bierze formatowanie tekstu przy kursorze, jak wyżej. Styl akapitu pozostaje stylem akapitu, styl znaków stylem znaków.
- **Format > Styl > Edytuj styl** (lub przycisk ołówka w Panelu stylów) otwiera okno z nazwą, **Czcionka…**, **Akapit…** i stylem następnego akapitu. Podgląd pokazuje wynik.

Potem {{APP}} przechodzi przez cały projekt i aktualizuje tekst, w otwartych dokumentach i w plikach pozostałych. To, czego dotyka zmiana (czcionka, rozmiar, pogrubienie, kursywa, kolor, odstępy między literami, kapitaliki, wyrównanie, wcięcia, odstępy, interlinia, tabulatory), dociera do **całego** tekstu stylu, także tam, gdzie ten tekst miał własną wartość: ustawioną ręcznie lub przyniesioną przez import (zaimportowane nagłówki zwykle podają czcionkę, rozmiar i odstępy w każdym akapicie). Formatowanie, którego zmiana nie dotyka, pozostaje takie, jakie jest, na przykład słowo pokolorowane ręcznie w nagłówku, gdy zmienił się tylko rozmiar nagłówka. Każdy dokument, który się zmienia, dostaje wcześniej migawkę („Przed zmianą stylu ...”). Tego nie da się cofnąć poleceniem **Edycja > Cofnij**; przywróć migawkę, zdefiniuj styl ponownie albo użyj **Przywróć definicję wbudowaną** w Panelu stylów dla stylu wbudowanego.

**Zmiana nazwy i usuwanie**: zmień nazwę stylu w **Edytuj styl**. Nagłówki, Tytuł, Podtytuł, Cytat blokowy, Wyśrodkowany i Podpis zachowują swoje nazwy, ponieważ Kompilacja i pliki Markdown znają je po nazwie. Gdy usuniesz styl (**Format > Styl > Usuń styl**), tekst, który go używał, zachowuje swój wygląd jako formatowanie ustawione ręcznie.

**Inne projekty**: **Format > Styl > Importuj style…** odczytuje style innego projektu. **Używaj tych stylów w nowych projektach** czyni style otwartego projektu zestawem początkowym każdego nowego projektu; **Plik > Opcje > Edycja** wraca do stylów wbudowanych.

W Panelu stylów przyciski strzałek zmieniają kolejność stylów we wszystkich listach.

### Markdown czy RTF: jak przechowywane są dokumenty

Każdy projekt przechowuje swoje dokumenty w jednym z dwóch formatów. Wybierasz przy tworzeniu lub importowaniu projektu i możesz zmienić później w **Projekt > Ustawienia projektu > Formatowanie**.

- **Markdown** (zalecany): pliki zwykłego tekstu, które otworzy każdy program, teraz i za pięćdziesiąt lat. Wygląd tekstu pochodzi z formatowania domyślnego. Czcionki i formatowanie akapitu ustawione ręcznie są zapisywane w tekście jako małe znaczniki, tylko tam, gdzie ich użyłeś(-aś) (rozdział 18); wszystko inne pozostaje zwykłe.
- **RTF**: pliki tekstu sformatowanego jak w procesorze tekstu. Każdy akapit niesie swoje pełne formatowanie, więc zmiana formatowania domyślnego nie dociera do istniejącego tekstu.

Przełączenie konwertuje każdy dokument. Z RTF do Markdown czcionki, kolory, wyrównanie, wcięcia lewe i prawe oraz tabulatory są zachowywane. Interlinia, wcięcie pierwszego wiersza i odstępy akapitów tekstu pochodzącego z innych programów podążają potem za projektem. Najpierw zrób kopię zapasową (**Plik > Kopia zapasowa > Utwórz kopię zapasową teraz**).

Tekst wklejony z innych programów do projektu Markdown przyjmuje wygląd otaczającego tekstu (pogrubienie, kursywa i odnośniki pozostają). Tekst skopiowany wewnątrz programu {{APP}} zachowuje swoje czcionki. **Edycja > Wklej z formatowaniem** (**Ctrl+Alt+Shift+V**) zachowuje także wygląd tekstu z procesora tekstu lub strony internetowej: czcionki, rozmiary, kolory oraz wyrównanie, wcięcia i odstępy jego akapitów (pierwszy wklejony akapit przenosi swoje formatowanie akapitu tylko do pustego akapitu). **Edycja > Wklej i dopasuj styl** (**Ctrl+Shift+V**) robi odwrotnie i wkleja zwykły tekst. Menu prawego przycisku tekstu oferuje wszystkie trzy, jedno pod drugim: **Wklej**, **Wklej i dopasuj styl** i **Wklej z formatowaniem**.

Aby skonwertować pojedynczy plik poza projektem, użyj **Plik > Konwertuj plik Markdown ⇄ RTF…**.

### Komentarze i przypisy

- **Komentarz** (**Shift+F2**): zaznacz tekst i dodaj komentarz. Tekst jest wyróżniony, a komentarz pojawia się w inspektorze.
- **Przypis** (**Ctrl+Shift+F2**): zaznacz słowo, do którego należy przypis; napisz przypis w inspektorze.
- **Adnotacja śródtekstowa** (**Ctrl+Shift+A**): notatka napisana bezpośrednio w tekście, pokazana w kolorze wewnątrz dymka. Adnotacje są usuwane przy kompilacji (chyba że wybierzesz ich eksport jako komentarzy).
- **Przypis śródtekstowy** (**Ctrl+Alt+F**): przypis napisany bezpośrednio w tekście.

Aby zmienić kolor adnotacji śródtekstowej, zaznacz ją (**Edycja > Zaznacz adnotację lub notatkę** zaznacza całą notatkę przy kursorze) i wybierz kolor tekstu; adnotacja zachowuje ten kolor. **Format > Czcionka > Automatyczny kolor tekstu** nadaje jej znów zwykły czerwony.

Kliknij komentarz w inspektorze, aby do niego przeskoczyć. Kliknij go prawym przyciskiem, aby zmienić jego kolor, zamienić go w inny rodzaj notatki albo go usunąć.

Kliknięcie tekstu z komentarzem otwiera inspektor z tym komentarzem. **Alt+kliknięcie** otwiera zamiast tego komentarz lub przypis w małym oknie przy myszy, w którym możesz go czytać i zmieniać bez inspektora; kliknij gdzie indziej lub naciśnij **Esc**, aby je zamknąć.

### Obrazy i odnośniki

- **Wstaw > Obraz z pliku…** umieszcza obraz w tekście; możesz też przeciągnąć obraz do tekstu lub go wkleić. Kliknij obraz prawym przyciskiem i wybierz **Rozmiar obrazu…**, aby zmienić, jak duży się wyświetla.
- **Wstaw > Odnośnik do dokumentu…** (**Ctrl+L**) łączy zaznaczony tekst z innym dokumentem projektu. Kliknięcie odnośnika otwiera ten dokument (w drugim edytorze, w tym samym edytorze, w jego podglądzie pomocniczym lub w panelu Szybkiego podglądu; zob. **Plik > Opcje > Zachowanie**).
- **Wstaw > Wstaw odnośnik internetowy…** tworzy odnośnik do strony internetowej.
- Odnośniki internetowe i pocztowe (*http*, *https*, *mailto*) otwierają się od razu. Każdy inny cel odnośnika, w tekście, mapie myśli, zakładce lub zaimportowanej stronie internetowej (plik, folder, adres sieciowy, odnośnik innego rodzaju programu), jest najpierw pokazywany w całości i otwierany dopiero po kliknięciu **Otwórz**; takie odnośniki mogą przyjść z cudzym projektem, stroną internetową lub plikiem Word. Dla pliku lub folderu na twoim komputerze pytanie podaje jego rodzaj (na przykład „dokument tekstowy (.txt)”), jego pełną nazwę i jego folder; długa nazwa się zawija, więc można przeczytać jej koniec, a plik, którego nie ma, jest opisany jako brakujący. Odnośniki do programów, skryptów, skrótów i instalatorów nigdy nie są otwierane; otwórz je sam w menedżerze plików, jeśli im ufasz.

### Tabele

**Wstaw > Tabela…** pyta o liczbę wierszy i kolumn oraz o wzór i umieszcza tabelę przy kursorze. Wewnątrz tabeli **Tab** przechodzi do następnej komórki (w ostatniej komórce dodaje wiersz), a **Shift+Tab** wraca. Aby opuścić tabelę za pomocą klawiatury, naciśnij **Ctrl+Enter** (poniżej tabeli) albo **Strzałkę w dół** w ostatnim wierszu (poniżej niej) lub **Strzałkę w górę** w pierwszym wierszu (powyżej niej); gdy w komórce jest jeszcze jeden wiersz, strzałki w górę i w dół najpierw przechodzą przez jej wiersze. Gdy obok tabeli nie ma akapitu, {{APP}} tworzy tam pusty (jeden krok dla Cofnij). **Format > Tabela**, także w menu prawego przycisku wewnątrz tabeli, ma resztę:

- **Wstaw wiersz powyżej / poniżej**, **Wstaw kolumnę po lewej / po prawej**.
- **Usuń wiersze** i **Usuń kolumny** (zaznaczonych komórek lub komórki z kursorem), **Usuń tabelę**.
- **Scal komórki** łączy zaznaczone komórki w jedną; **Podziel komórkę** ponownie dzieli scaloną komórkę.
- **Kolor komórki…** nadaje zaznaczonym komórkom własne tło; **Kolor komórki z projektu** je odbiera.
- **Właściwości tabeli…** zmienia wzór.

**Wzór.** Gotowych jest pięć wzorów: **Siatka**, **Siatka z wierszem nagłówka**, **Paski w wierszach**, **Linie między wierszami** i **Bez linii**. Wybierz jeden i zmień, co chcesz; wzór jest wtedy **Własny**:

- **Linie**: grubość (0: brak linii), kolor i **Tylko między wierszami** (bez linii po bokach).
- **Komórki**: odstęp wewnątrz komórki i odstęp między komórkami (0: sąsiednie komórki dzielą jedną linię).
- **Wiersze nagłówka**: ile wierszy na górze jest nagłówkami, z ich kolorem. Drukowanie i kompilacja powtarzają je na górze każdej strony, na którą przechodzi długa tabela. **Co drugi wiersz** dostaje własny kolor, jeśli go wybierzesz.
- **Miejsce w tekście**: szerokość (udział w szerokości tekstu lub tak szeroka jak jej zawartość), szerokości kolumn (wszystkie równe lub udziały, takie jak „30, 70”), wyrównanie i miejsce, w którym stoi: **We własnych wierszach** lub **Po lewej** albo **Po prawej** z tekstem opływającym ją obok.

Wiersze i kolumny dodane później zachowują wzór (paski są kontynuowane). Tekst w komórkach tabeli nie jest wyjustowany, także gdy tekst główny jest (wąska komórka rozciągałaby swoje słowa); wyrównanie nadane akapitowi w komórce pozostaje. Tabele są zachowywane w obu formatach przechowywania i w formacie Scrivener; tabele w plikach RTF z innych programów (na przykład plik Word zapisany jako RTF) również stają się tabelami.

### Obrazy obok tekstu, podpisy i opisy

Kliknij obraz prawym przyciskiem i wybierz **Obraz…** (lub ustaw kursor obok niego i wybierz **Format > Obraz…**):

- **Szerokość na ekranie**: wysokość za nią podąża; **Rozmiar oryginalny** wraca do własnego rozmiaru obrazu.
- **Położenie**: **W wierszu, jak litera** (jak dawniej), **Po lewej** lub **Po prawej** z tekstem opływającym obraz albo **Wyśrodkowany we własnych wierszach**.
- **Podpis**: pokazywany pod obrazem i kompilowany razem z nim. Obraz w wierszu nie ma podpisu; najpierw nadaj mu położenie.
- **Opis**: co pokazuje obraz, w zdaniu lub dwóch. Nie jest pokazywany w tekście. E-booki, pliki Word i Markdown zachowują go razem z obrazem (jako jego „tekst alternatywny”), a czytniki ekranu czytają go osobom, które nie widzą obrazu.

Obraz z podpisem przesuwa się jako jedna całość; wpisz podpis pod obrazem jak każdy inny tekst (ma styl **Podpis**).

### Znajdź i zamień

- **Ctrl+F** (**Edycja > Znajdź > Znajdź**) otwiera pasek wyszukiwania nad tekstem. Pokazuje, ile jest trafień, i je wyróżnia; **F3** i **Shift+F3** przechodzą do następnego i poprzedniego trafienia. **Uwzględnij wielkość liter** i **Całe słowa** zawężają wyszukiwanie.
- **Ctrl+H** dodaje wiersz zamiany: **Zamień** zmienia zaznaczone trafienie i przechodzi do następnego, **Zamień wszystko** zmienia każde trafienie w widocznym tekście (jeden krok dla **Cofnij**). Nowy tekst przejmuje formatowanie tekstu, który zastępuje, a komentarze pozostają przyłączone.
- **Ctrl+E** (**Użyj zaznaczenia do wyszukiwania**) wstawia zaznaczony tekst do paska wyszukiwania.
- W tekście połączonym wyszukiwanie i zamiana działają we wszystkich pokazanych dokumentach.
- Gdy klawiatura jest w **podglądzie pomocniczym**, Ctrl+F przeszukuje dokument podglądu pomocniczego.
- **Ctrl+Shift+F** przeszukuje cały projekt, **Edycja > Znajdź > Zamiana w projekcie** zamienia w całym projekcie, a **Edycja > Znajdź > Znajdź według formatowania** szuka wyróżnień, komentarzy, kolorów i stylów (wszystko w rozdziale 9).
- Na tablicy korkowej i w konspekcie Ctrl+F otwiera zamiast tego **pasek filtrowania** (rozdział 5).

### Pisownia

Błędnie napisane słowa dostają czerwone faliste podkreślenie; kliknij prawym przyciskiem w dowolnym miejscu podkreślonego słowa, aby zobaczyć propozycje lub dodać słowo do słownika (także przy słowach z apostrofem, takich jak „Kayel's” czy „D'Nersun”: menu zawsze oferuje całe podkreślone słowo). Przy błędnie napisanym słowie menu jest krótkie, aby otwierało się tuż przy myszy: propozycje, **Dodaj „...” do słownika**, **Ignoruj „...”** oraz **Więcej poleceń** z wszystkim, co menu zawiera w innych miejscach (Wytnij, Kopiuj, Wklej, Styl, komentarze, Wygląd ...). Własna nazwa programu i jego terminy ({{APP}}, konspekt, poddokument, podgląd pomocniczy) zawsze są uznawane za poprawne. Język wybierzesz w **Plik > Opcje > Poprawki** (strona Pisownia); na tej liście są słowniki zainstalowane jako dodatki (po jednym na język programu, każdy ze swoją licencją); tam wybierzesz też podkreślenie (fale, kropki lub linia prosta), czy słowa z cyframi („Route66”) są sprawdzane, a **Wyświetl osobistą listę słów...** pokazuje dodane przez ciebie słowa i usuwa te, które dodałeś(-aś) omyłkowo. Słowniki pochodzą z twojego systemu i z dodatków. W Ubuntu i Kubuntu zainstalowany jest angielski; dla innych języków zainstaluj odpowiedni pakiet *hunspell* w Discover (na przykład „hunspell-pl” dla polskiego).

### Wstawki specjalne

Menu **Wstaw** oferuje też bieżącą datę i godzinę, znaki specjalne (pauzę, półpauzę, wielokropek, spację nierozdzielającą, znak paragrafu, punktor), **autonumerację** i **symbole zastępcze**, które Kompilacja zamienia (rozdział 13).

### Poprawki podczas pisania

**Plik > Opcje > Poprawki** ustawia, co jest poprawiane podczas pisania. Słowo jest poprawiane, gdy je skończysz, czyli gdy wpiszesz po nim spację, Enter lub znak interpunkcyjny; **Ctrl+Z** zaraz potem cofa samą poprawkę.

- **Popraw wielkość liter na początku zdań**: pierwsze słowo zdania dostaje wielką literę (nie po skrótach takich jak „Dr.” czy „e.g.”). **Pisz „i” wielką literą**: „i” i „i'm” zamieniają się w „I” i „I'm”. Oba są na początku wyłączone.
- **Inteligentne cudzysłowy** w wybranym stylu (“angielski”, „niemiecki“, »niemiecki«, «francuski», ”szwedzki”); apostrof wewnątrz słowa to zawsze ’. Tam, gdzie zamykający pojedynczy znak nie jest apostrofem (niemiecki ‚…‘, francuski ‹…›), ' wpisany tuż po słowie zamyka pojedynczy znak otwarty w akapicie; jeśli potem piszesz dalej litery („geht's”), znów staje się apostrofem. Dwa łączniki zamieniają się w pauzę (—); trzy kropki mogą stać się jednym znakiem wielokropka (na początku wyłączone). W trybie scenariusza można to wyłączyć.
- **Cudzysłowy już w projekcie** (od 0.61.1): pod **Inteligentne cudzysłowy** polecenie **Zamień na cudzysłowy typograficzne…** zamienia proste " i ' projektu na znaki stylu wybranego obok **Używaj inteligentnych cudzysłowów**, a **Zamień na cudzysłowy proste…** zamienia każdy typograficzny cudzysłów i apostrof z powrotem na " i '. Otwiera się okno jak Zamiana w projekcie (rozdział 9): kierunek i jeszcze raz styl, gdzie zamieniać (tytuły, tekst, notatki, streszczenia, własne metadane, komentarze i przypisy), zakres (cały projekt lub dokumenty zaznaczone w segregatorze, Kosz tylko po zaznaczeniu) oraz migawka każdego tekstu przed zmianą (na początku zaznaczona). **Policz** pokazuje, ile znaków by się zmieniło; **Zamień** pyta jeszcze raz i to robi. Formatowanie pozostaje (pogrubiony cudzysłów pozostaje pogrubiony). Znak otwiera się po niczym, spacji, nawiasie, myślniku lub znaku otwierającym, a w pozostałych przypadkach zamyka; apostrof wewnątrz słowa lub przed liczbą ('90s) zamienia się w ’. Prosty znak ze spacją po obu stronach pozostaje, jaki jest, a apostrof na początku słowa ('tis) staje się znakiem otwierającym: obejrzyj je potem. Znajdź i zamień nie potrafi tego zrobić, ponieważ jeden znak musi stać się otwierającym lub zamykającym zależnie od sąsiadów.
- **Wykrywanie danych**: wpisany adres internetowy staje się odnośnikiem; **[[Tytuł]]** staje się odnośnikiem do dokumentu o takim tytule, a jeśli go nie ma, obok tekstu powstaje nowy pusty dokument o tej nazwie („odnośniki do dokumentów” programu Scrivener).
- **Dodatkowe zamiany**: lista tego, co wpisujesz, i tego, czym się staje, na przykład „(c)” i ©, „->” i →. **Edytuj zamiany...** zmienia listę; **Zamiany standardowe** przywraca listę, z którą przyszedł program.
- **Autouzupełnianie**: w trybie scenariusza lista nazw postaci i nagłówków scen proponowana podczas pisania.

### Powiększenie, przewijanie jak w maszynie do pisania i stała szerokość

- **Ctrl+kółko myszy** powiększa obszar pod myszą, gdziekolwiek jest: edytory, segregator, inspektor, konspekt, tablicę korkową (karty i ich tekst razem), mapę myśli, tryb pisania i własne okna (Szybki podgląd, narzędzia pisarskie); klawiatura może być gdzie indziej. Każdy obszar zachowuje swoje powiększenie, dopóki go ponownie nie zmienisz.
- **Ctrl++ / Ctrl+-** lub pole powiększenia w stopce powiększają tekst na ekranie bez zmiany rozmiaru czcionki. **Widok > Przywróć powiększenie** (**Ctrl+0**) wraca do domyślnego powiększenia tekstu.
- **Plik > Opcje > Edycja** ustawia powiększenie, z którym zaczyna każdy obszar („Domyślne powiększenie tekstu”, dla każdego obszaru okna i każdej części z własnym powiększeniem: streszczenie, notatki, listy zakładek, tekst zakładki, lista i tekst migawek, lista i karta obiektów w inspektorze i w oknie Obiekty, nowe mapy myśli) oraz to, czy edytory otwierają się ponownie z ostatnio użytym powiększeniem („Wczytaj ostatnio wybrane powiększenie głównych edytorów”).
- **Gdzie byłeś(-aś).** {{APP}} zapamiętuje razem z projektem miejsce kursora i część tekstu na ekranie dla każdego dokumentu: pokazany ponownie (z segregatora, odnośnika, przez Wstecz i Dalej, w podglądzie pomocniczym, panelu Szybkiego podglądu lub po ponownym otwarciu projektu), dokument otwiera się tam, gdzie go zostawiłeś(-aś). Folder w tekście połączonym pamięta, który z jego dokumentów miał kursor. Wynik wyszukiwania lub odnośnik wskazujący określone miejsce prowadzi zamiast tego tam. Obrazy zachowują swoje powiększenie, a pliki PDF powiększenie i stronę.
- **Przewijanie jak w maszynie do pisania** (**Widok > Edycja tekstu**) utrzymuje pisany wiersz pośrodku ekranu.
- **Edytor o stałej szerokości** pokazuje tekst jako wyśrodkowaną kolumnę o wygodnej szerokości (500 px przy 100 %, chyba że ją zmienisz), niezależnie od szerokości okna. Wyłączysz go lub zmienisz szerokość w **Plik > Opcje > Wygląd > Główny edytor** („Używaj edytora o stałej szerokości”, „Szerokość kolumny tekstu”) albo prawym przyciskiem na papierze (**Wygląd > Szerokość kolumny tekstu**). Papier kolumny to standardowo #d1d495, a obszar obok #999999 („Papier” i „Obok kolumny tekstu” wśród kolorów Głównego edytora); pozostają takie same po wybraniu innego motywu, dopóki ich nie zmienisz lub nie zresetujesz.
- **Obramowanie obok tekstu.** Po lewej i po prawej stronie tekstu papier ma obramowanie we własnym kolorze, aby tekst nie dotykał krawędzi papieru: 10 px przy 100 % dla nowych projektów, rośnie z powiększeniem. Leży poza kolumną tekstu, więc wiersze łamią się tam, gdzie się łamały. Zmienisz je w **Plik > Opcje > Wygląd > Główny edytor** („Obramowanie obok tekstu”, od 0 do 100 px) lub prawym przyciskiem na papierze (**Wygląd > Obramowanie obok tekstu**). Projekty utworzone przed 0.40.2 zachowują dawny wygląd, bez obramowania, dopóki żadnego im nie nadasz. Kliknięcie obramowania ustawia kursor na początku lub końcu wiersza obok.
- **Kliknięcie i kliknięcie prawym przyciskiem** nigdy nie przesuwają widoku: kursor idzie tam, gdzie klikniesz, a tekst pozostaje tam, gdzie jest na ekranie (przed 0.40.2 kliknięcie lub kliknięcie prawym przyciskiem blisko górnej lub dolnej krawędzi albo kliknięcie prawym przyciskiem obok tekstu mogło sprawić, że tekst podskakiwał).

### Widok strony

**Widok > Edycja tekstu > Widok strony** (także w menu przycisku paska narzędzi **Widok**) pokazuje tekst na arkuszach papieru zamiast jednej nieskończonej kolumny. Stopka pokazuje wtedy stronę kursora i liczbę stron, na przykład „Strona 3 z 12”. Wybierz polecenie ponownie, aby wrócić do kolumny. Kolor biurka wokół stron znajduje się w **Plik > Opcje > Wygląd > Główny edytor** (lub kliknij prawym przyciskiem > **Wygląd**, gdy strony są pokazane).

Strony są tylko sposobem patrzenia na twój tekst, niczym więcej. Nie pokazują, jak będzie wyglądał skompilowany lub wydrukowany rękopis: piszesz kawałkami, a tytuły, czcionki i rozmiar strony ustala się przy kompilacji (rozdział 13). Nic o stronach nie jest zapisywane w twoich dokumentach.

- **Widok > Edycja tekstu > Rozmiar strony i marginesy…** ustawia papier (A4, A5, US Letter i inne lub własny rozmiar), orientację pionową lub poziomą oraz cztery marginesy. Ustawienie należy do projektu; zaznacz **Użyj tej strony także dla nowych projektów**, aby uczynić je swoim standardem.
- Powiększenie powiększa razem arkusze, marginesy i litery. Arkusz szerszy niż edytor można przewijać na boki. Podobnie jak w widoku kolumny wcięcia i tabulatory zachowują rzeczywistą długość, więc łamanie wierszy może się nieco różnić między poziomami powiększenia.
- W tekście połączonym każdy dokument zaczyna się na własnym arkuszu, a jego ostatni arkusz kończy się tuż pod tekstem, dzięki czemu krótkie dokumenty nie wypełniają ekranu pustym papierem.
- Widok strony dotyczy wszystkiego, co pokazuje twoje teksty w oknie głównym i w oknach Szybkiego podglądu: obu edytorów i podglądów pomocniczych. Notatki zachowują zwykły wygląd, a tryb pisania ma własny papier.
- Kolor wokół arkuszy to **Wokół stron (widok strony)** w **Plik > Opcje > Wygląd > Główny edytor**.

### Pisanie scenariuszy

Scenariusze filmowe, sztuki teatralne i scenariusze komiksów pisze się w **trybie scenariusza**. Scenariusz składa się z *elementów*: nagłówek sceny, akcja, postać, didaskalium, dialog, przejście i tak dalej. Każdy element ma swoje miejsce na stronie, a w trybie scenariusza klawisze **Tab** i **Enter** prowadzą od jednego elementu do następnego, więc nigdy nie ustawiasz wcięcia ręcznie.

**Włączanie.** Kliknij w dokumencie i wybierz **Format > Pisanie scenariuszy > Tryb scenariusza** (**Ctrl+8**). Pozycja menu nazywa format scenariusza projektu, na przykład *Tryb scenariusza - Scenariusz filmowy*. Tryb scenariusza należy do dokumentu: scenariusze i zwykłe teksty żyją obok siebie w jednym projekcie, a scenariusze mają w segregatorze **żółtą ikonę**. Wybierz polecenie ponownie, aby go wyłączyć; tekst zachowuje wygląd. Nowy dokument utworzony wśród scenariuszy (w folderze, którego wszystkie teksty są scenariuszami) też jest scenariuszem.

**Stopka.** W scenariuszu stopka edytora pokazuje zamiast liczby słów to, co zrobią **Tab** i **Enter** przy kursorze, a po prawej menu rozwijane z elementem bieżącego akapitu. Wybierz inny element z tego menu, a akapit zostanie sformatowany od nowa. Tekst, który nie jest elementem scenariusza, jest pokazany jako **Tekst ogólny**.

**Tab i Enter.**

- **Enter** na końcu akapitu zaczyna element, który zwykle następuje: akcję po nagłówku sceny, dialog po nazwie postaci i tak dalej.
- **Enter** w pustym wierszu otwiera przy kursorze listę elementów; naciśnij literę podkreśloną na liście (S dla nagłówka sceny, C dla postaci ...). **Format > Pisanie scenariuszy > Pokaż listę elementów** (**Ctrl+\\**) otwiera tę samą listę w dowolnej chwili.
- **Tab** w pustym wierszu zmienia wiersz na inny element; naciśnij go ponownie, aby przejść do następnego, **Shift+Tab**, aby wrócić.
- **Tab** na końcu akapitu robi to, co pasuje do elementu: po nazwie postaci lub wierszu dialogu zaczyna didaskalium, a w nagłówku sceny wpisuje „ - ”, gotowe na porę dnia.
- **Enter** w środku akapitu dzieli go; obie połowy zachowują element. **Shift+Enter** to złamanie wiersza wewnątrz akapitu.
- Elementy pisane wielkimi literami (nagłówki scen, nazwy postaci, przejścia) zamieniają to, co wpisujesz, na wielkie litery. Nawiasy i przedrostki takie jak „SFX: ” są wpisywane za ciebie.

**Autouzupełnianie.** Podczas pisania mała lista proponuje to, co pasuje do elementu: INT. i EXT. na początku nagłówka sceny, pory dnia po jego „ - ”, miejsca i nazwy postaci już użyte w dokumencie oraz typowe przejścia, takie jak CUT TO:. **Enter** lub **Tab** przyjmuje zaznaczoną pozycję, klawisze strzałek wybierają inną, **Esc** zamyka listę.

**Zmiana elementów.** **Format > Pisanie scenariuszy > Zmień element na** zmienia akapit z kursorem lub wszystkie zaznaczone akapity. Wybranie tam elementu w zwykłym dokumencie czyni dokument scenariuszem. Każdy element jest poleceniem, więc ulubionym możesz nadać skrót w **Plik > Opcje > Klawiatura**.

**Formaty scenariusza.** Projekt używa jednego formatu scenariusza dla wszystkich swoich scenariuszy. **Format > Pisanie scenariuszy > Format scenariusza** wymienia *Scenariusz filmowy*, *Sztukę teatralną* i *Scenariusz komiksu*, formaty z dodatków oraz twoje własne. Gdy wybierzesz inny format, scenariusze projektu dostają nowy układ: każdy element staje się elementem tego samego rodzaju w nowym formacie (akcja staje się na przykład didaskaliami scenicznymi). Element bez odpowiednika staje się tekstem ogólnym, który zachowuje wygląd; pytanie przed zmianą wymienia te elementy.

**Ustawienia scenariusza.** **Format > Pisanie scenariuszy > Ustawienia scenariusza…** zmienia format projektu lub buduje nowy:

- nazwa formatu, jego czcionka (domyślnie czcionka maszyny do pisania twojego systemu: Courier Prime, Courier New lub podobna) i rozmiar oraz to, czy długości są w calach, czy w centymetrach;
- lista elementów: dodawanie, usuwanie i zmiana kolejności (kolejność jest kolejnością listy elementów i **Tab** w pustym wierszu);
- dla każdego elementu: nazwa, rodzaj (czym jest dla innych programów: nagłówek sceny, akcja, postać ...) i jego litera na liście; pogrubienie, kursywa, podkreślenie, wielkie litery i kolor; wyrównanie, wcięcia od obu marginesów, puste wiersze przed nim i interlinia; co robią **Enter** i **Tab**; tekst wpisywany przed akapitem i po nim; oraz listy dla autouzupełniania;
- podgląd wszystkich elementów.

Element, któremu zmieniono nazwę, zachowuje swoje akapity w każdym scenariuszu projektu. Przycisk **Zarządzaj** zaczyna od innego formatu, wczytuje lub zapisuje plik formatu (*.qbscript*) i zapisuje format do użycia w innych projektach.

**Warto wiedzieć.**

- Klawisze i listy działają tak samo w tekście połączonym, w drugim edytorze, w podglądach pomocniczych, w panelach Szybkiego podglądu i w trybie pisania. Tylko edytory okna głównego mają stopkę z podpowiedziami; gdzie indziej **Ctrl+\\** pokazuje listę elementów.
- Widok strony (wyżej) pokazuje scenariusz na arkuszach; przy zwykłej czcionce maszyny do pisania 12-punktowej strona scenariusza filmowego to mniej więcej minuta filmu, ale strony na ekranie nie są stronami skompilowanego pliku.
- Czcionka i wcięcia elementów należą do formatu scenariusza, a nie do dokumentu. Pliki twoich scenariuszy zawierają tylko nazwy elementów (rozdział 18).
- Wprowadzanie i wyprowadzanie scenariuszy w formatach innych programów do pisania scenariuszy: rozdział 14. Kompilowanie całego scenariusza: rozdział 13.
- Scenariusze wymagają wersji 0.10 lub nowszej. Projekt zawierający scenariusze mówi o tym w swoim pliku projektu, a od tej wersji {{APP}} pyta, zanim otworzy projekt wymagający nowszej wersji niż uruchomiona. Wersje 0.9 i starsze nie pytają: pokazują scenariusze jako zwykły tekst i tracą elementy każdego scenariusza, który tam oglądasz.

---

## 5. Trzy sposoby patrzenia na projekt

Widok wybierzesz trzema przyciskami paska narzędzi albo **Ctrl+1**, **Ctrl+2** i **Ctrl+3**.

### Tekst połączony (Ctrl+1): wiele dokumentów jako jeden tekst

Zaznacz kilka dokumentów w segregatorze (Ctrl+klik lub Shift+klik) albo folder i naciśnij **Ctrl+1**. Wszystkie teksty pojawiają się jeden za drugim, rozdzielone cienką przerywaną linią, a ty możesz pisać we wszystkich, jakby były jednym dokumentem. Zmiany trafiają do właściwych dokumentów automatycznie. Tytuły dokumentów między nimi pokażesz poleceniem **Widok > Edycja tekstu > Pokaż tytuły w tekście połączonym**.

### Tablica korkowa (Ctrl+2): fiszki

Każdy dokument to fiszka z jego tytułem i streszczeniem; dokument bez streszczenia pokazuje zamiast tego pierwsze wiersze swojego tekstu (wyblakłe, kursywą). Fiszka obrazu pokazuje obraz, podobnie fiszka dokumentu, którego streszczeniem jest obraz (rozdział 7). Przeciągaj fiszki, aby zmienić kolejność dokumentów. Kliknij fiszkę dwukrotnie, aby edytować jej tytuł lub streszczenie. Naciśnij **Enter**, aby otworzyć zaznaczone fiszki jako tekst.

W stopce tablicy korkowej:

- **Siatka** lub **dowolny**: w trybie dowolnym możesz umieszczać fiszki gdziekolwiek, jak na prawdziwej tablicy korkowej. **Zatwierdź kolejność** zmienia wtedy kolejność dokumentów w segregatorze tak, by odpowiadała fiszkom od lewej do prawej, od góry do dołu. Układ wybrany dla folderu jest zapamiętywany razem z tym folderem (od 0.90.0): następnym razem jego tablica korkowa będzie ułożona tak samo. Folder, dla którego nigdy nie wybrano układu, zachowuje ten, który pokazuje edytor.
- **Wątki etykiet**: układa fiszki w rzędach według etykiety.
- **Rozmiar fiszek** za pomocą suwaka, klawiszy strzałek lub kółka myszy nad nim; jest zapamiętywany od razu (**Ctrl+kółko myszy** nad tablicą korkową powiększa fiszki i ich tekst razem, jak inne obszary, zob. rozdział 4).

Dwukrotne kliknięcie pustego miejsca tablicy korkowej tworzy nową fiszkę (**Plik > Opcje > Zachowanie > Dwukrotne kliknięcie**). Gdy edytujesz streszczenie na fiszce, **Enter** kończy edycję, a **Shift+Enter** tworzy nowy wiersz (**Plik > Opcje > Zachowanie > Klawisz Enter**, gdzie Enter można też ustawić tak, by tworzył nową fiszkę lub wiersz konspektu po zaznaczonym).

Stemple statusu, tryb dowolny i układ według etykiety są także w **Widok > Opcje tablicy korkowej**; odcienie etykiet dla fiszek i segregatora w **Widok > Używaj koloru etykiety w**. Kolumny konspektu są również wymienione w **Widok > Opcje konspektu**.

### Konspekt (Ctrl+3): tabela

Pokazuje dokumenty jako tabelę z tytułem, streszczeniem, etykietą, statusem, liczbami słów, celami, datami i twoimi własnymi metadanymi. Kliknij przycisk kolumn w stopce, aby wybrać kolumny. Kliknij nagłówek kolumny, aby sortować (tylko do oglądania; kolejność w segregatorze się nie zmienia). Przeciągaj wiersze, aby zmienić kolejność dokumentów.

### Filtrowanie tablicy korkowej i konspektu

Gdy tablica korkowa lub konspekt ma klawiaturę, **Ctrl+F** (**Edycja > Znajdź > Znajdź**) otwiera **pasek filtrowania** na górze edytora. W miarę pisania zostają tylko pasujące fiszki lub wiersze; pasek mówi ile („2 z 9”). W konspekcie grupy pasujących dokumentów pozostają, abyś widział(a), gdzie należą.

- Kliknij **trójkąt** po lewej od pola, aby zobaczyć opcje: gdzie szukać (**Tytuły**, **Streszczenia**, **Słowa kluczowe**, **Tekst**), **etykieta**, **status**, **słowo kluczowe**, czy dokument jest **uwzględniony w kompilacji** oraz **Pokaż to, co nie pasuje**. Kilka wyborów razem zawęża listę jeszcze bardziej.
- Filtr pozostaje, gdy otwierasz inne foldery, więc możesz przechodzić przez projekt, oglądając tylko, powiedzmy, sceny jednej postaci.
- **Gotowe** usuwa filtr i znów pokazuje wszystko.

Przeciąganie fiszek nadal zmienia kolejność dokumentów: fiszka upuszczona między dwiema widocznymi fiszkami ląduje obok nich w segregatorze. **Zatwierdź kolejność** tablicy korkowej w trybie dowolnym jest wyłączone, gdy filtr ukrywa fiszki.

---

## 6. Dwa edytory, zakładki i szybki podgląd

### Podzielony edytor

Kliknij przycisk podziału po prawej stronie nagłówka edytora, aby pokazać drugi edytor. Alt+kliknięcie dzieli w drugą stronę. **Widok > Podziel edytor** oferuje te same wybory. **F6** przeskakuje między edytorami. Kliknięcie w segregatorze pokazuje dokument w edytorze użytym ostatnio; **Nawigacja > Zaznaczenie w segregatorze wpływa na** zmienia to na lewy lub górny edytor, prawy lub dolny edytor albo drugi edytor, ten, w którym akurat piszesz.

Na tablicy korkowej lub w konspekcie przycisk stopki **Zaznaczenie wpływa na drugi edytor** (także w **Widok > Opcje tablicy korkowej** i **Opcje konspektu**) sprawia, że drugi edytor pokazuje fiszkę, którą klikniesz. To szybki sposób przeglądania scen przy zachowaniu przeglądu rozdziału.

**Nawigacja > Edytor > Zablokuj w miejscu** utrzymuje edytor na jego dokumencie, nawet gdy klikniesz gdzie indziej w segregatorze.

### Podglądy pomocnicze

Każdy edytor ma **podgląd pomocniczy**: drugi, prosty widok wewnątrz edytora, który trzyma jeden inny dokument do wglądu, taki jak notatki lub karta postaci. Z dwoma edytorami i ich podglądami pomocniczymi możesz widzieć cztery dokumenty naraz.

- **Aby go otworzyć**, przytrzymaj **Alt** i przeciągnij dokument z segregatora, tablicy korkowej lub konspektu na pasek nagłówka edytora. Bez Alt upuszczenie ładuje dokument do samego edytora. **Nawigacja > Otwórz > W podglądzie pomocniczym** i menu prawego przycisku dokumentu robią to samo dla zaznaczonego dokumentu.
- Podgląd pomocniczy ma wąski pasek nagłówka z tytułem i nie ma stopki. Pokazuje tekst jednego dokumentu (lub obraz albo PDF); nie ma tablicy korkowej, konspektu ani tekstu połączonego. Możesz w nim pisać i formatować jak w każdym edytorze.
- **Aby go przesunąć**, kliknij prawym przyciskiem jego pasek nagłówka i wybierz lewo, prawo, górę lub dół. Za pierwszym razem otwiera się obok szerokiego edytora i pod wąskim; potem tam, gdzie go ostatnio umieściłeś(-aś). Przeciągnij linię między edytorem a podglądem pomocniczym, aby zmienić ich rozmiary.
- **Aby pokazać inny dokument**, upuść go na pasek nagłówka podglądu pomocniczego albo ponownie przeciągnij go z Alt na pasek nagłówka edytora.
- **Aby go zamknąć**, kliknij **X** po prawej stronie jego paska nagłówka albo wybierz **Nawigacja > Edytor > Zamknij podgląd pomocniczy**. Przycisk po lewej od X zamyka go i otwiera dokument zamiast tego jako okno szybkiego podglądu.

**Nawigacja z podglądem pomocniczym.** Gdy edytor pokazuje tablicę korkową lub konspekt i ma otwarty podgląd pomocniczy, przycisk **Zaznaczenie wpływa na** w stopce ma trzy położenia. Pierwsze kliknięcie sprawia, że twoje zaznaczenie otwiera się w drugim edytorze, jak zwykle. Drugie kliknięcie obraca strzałkę tak, że wskazuje do pola: teraz fiszka lub wiersz, które klikniesz, otwierają się w podglądzie pomocniczym. Trzecie kliknięcie to wyłącza. Przydatny układ: rękopis w lewym edytorze, konspekt twoich materiałów w prawym edytorze z jego podglądem pomocniczym poniżej oraz **Nawigacja > Zaznaczenie w segregatorze wpływa na > Lewy lub górny edytor**. Segregator steruje wtedy rękopisem, a konspekt steruje podglądem pomocniczym.

Podgląd pomocniczy, jego miejsce i jego dokument są zapamiętywane razem z projektem.

### Nagłówek edytora

Nagłówek nad każdym edytorem pokazuje tytuł dokumentu i zawiera przyciski **wstecz** i **dalej** (**Ctrl+[** i **Ctrl+]**), menu sekcji dokumentu oraz przyciski poprzedniego i następnego dokumentu (**Alt+Shift+Up / Down**).

- **Historia**: każdy edytor pamięta dokumenty, które pokazał. Przytrzymaj przycisk wstecz lub dalej (albo kliknij go prawym przyciskiem), aby zobaczyć listę, i wybierz dokument, aby przejść wprost do niego; **Nawigacja > Wstecz do** i **Dalej do** pokazują te same listy. Historia jest przechowywana razem z projektem.
- **Ikona** obok tytułu oznacza dokument. Kliknij ją, aby zobaczyć menu (Zablokuj w miejscu, Pokaż w segregatorze ...). Przeciągnij ją jak dokument w segregatorze: na pasek nagłówka drugiego edytora, do segregatora, na kartę kolekcji, na przycisk Szybki podgląd lub Zakładki paska narzędzi albo na tekst, aby utworzyć odnośnik.
- **Dokumenty > Grupuj** (**Ctrl+Shift+J**, także w menu kontekstowym segregatora) przenosi zaznaczone pozycje do nowego folderu w miejscu pierwszej z nich; jego tytuł jest gotowy do wpisania. Gdy klawiatura jest w segregatorze, **Edycja > Cofnij** cofa cały krok.
- **Dokumenty > Konwertuj > Zamień kartę na obiekt** (także w menu kontekstowym segregatora) tworzy Obiekt (rozdział o Obiektach) z każdego zaznaczonego dokumentu-karty, na przykład karty postaci lub miejsca: tytuł jest nazwą, tekst trafia do **Inne**, obraz w tekście staje się obrazem. Nazwa folderu (Postacie, Miejsca) wybiera rodzaj; dla innego folderu jesteś o to pytany(-a). Pozycja segregatora pozostaje: jej zaznaczenie pokazuje Obiekt w inspektorze, a Enter otwiera go w oknie Obiekty. Dokument zachowuje swój tekst (projekt Scrivener widzi zwykły dokument); powiązanie jest przechowywane we własnych danych Obiektów projektu. **Edycja > Cofnij** z klawiaturą w segregatorze to cofa.
- **Nawigacja > Przejdź do** wymienia cały segregator jako menu i ładuje wybrany dokument do aktywnego edytora. **Nawigacja > Grupa nadrzędna** pokazuje folder lub grupę, do której należy dokument, z zaznaczonym tam dokumentem.
- **Dwukrotne kliknięcie separatora** między dwoma edytorami nadaje im ten sam rozmiar.

### Przeciąganie i upuszczanie w skrócie

| Przeciągnij ... | na ... | i ... |
|---|---|---|
| dokument (segregator, kolekcja, tablica korkowa, konspekt, ikona nagłówka, zakładka) | pasek nagłówka edytora | jest pokazany w tym edytorze; z **Alt**: w jego podglądzie pomocniczym |
| | tekst w edytorze | odnośnik do dokumentu; na zaznaczonym tekście ten tekst staje się odnośnikiem |
| | tekst, z przytrzymanym **Alt** | tekst dokumentu zostaje wstawiony |
| obraz z segregatora | tekst w edytorze | obraz zostaje wstawiony |
| dokument | przycisk **Szybki podgląd** paska narzędzi | każdy dokument otwiera się w osobnym panelu |
| | przycisk **Zakładki** paska narzędzi | zostaje dodany do zakładek projektu |
| | kartę kolekcji, listę zakładek | zostaje dodany |
| | pasek nagłówka panelu Szybkiego podglądu | jest pokazany w tym panelu |
| | menedżer plików lub pulpit | zostaje tam zapisany jako plik (zob. „Eksportowanie dokumentów”) |
| zaznaczony tekst | segregator | nowy dokument z tym tekstem; tekst pozostaje tam, gdzie był |
| pliki z menedżera plików | segregator lub tablicę korkową | zostają zaimportowane |
| odnośnik internetowy z przeglądarki | folder w segregatorze, tablicę korkową, konspekt | strona zostaje zaimportowana (dodatek Web Page Import, rozdział 14) |
| plik obrazu lub obraz z segregatora | fiszkę w inspektorze (tryb obrazu) | staje się obrazem fiszki |
| słowa kluczowe z **Projekt > Słowa kluczowe projektu** | dokument w segregatorze, kolekcję, tablicę korkową, konspekt; listę słów kluczowych inspektora | zostają przypisane; z **Alt**: razem z grupą, do której należą |

Niektóre pulpity używają Alt+przeciągnięcia do przesuwania okien. Jeśli Alt+przeciągnięcie nie dociera do programu, zmień klawisz modyfikujący przesuwanie okien w ustawieniach pulpitu (KDE: Ustawienia systemowe, zachowanie okien; dokładne miejsce różni się między wersjami) albo użyj zamiast tego poleceń menu (**Nawigacja > Otwórz > W podglądzie pomocniczym**, **Projekt > Słowa kluczowe projektu > Przypisz do zaznaczonych dokumentów**).

### Panele Szybkiego podglądu

Panel Szybkiego podglądu to dokument we własnym oknie, na przykład aby mieć kartę postaci w zasięgu wzroku podczas pisania. Tekst w panelu można edytować i formatować jak w edytorze (działają tam skróty menu Format i Wstaw).

**Aby otworzyć panele:**

- **Nawigacja > Otwórz > Jako Szybki podgląd** albo przycisk Szybki podgląd paska narzędzi, dla zaznaczonych dokumentów.
- Naciśnij **Spację** na tablicy korkowej lub w konspekcie albo w segregatorze i na liście kolekcji lub wyników wyszukiwania, gdy **Widok > Klawisze szybkiego dodawania w segregatorze** jest włączone (zob. rozdział 2, klawisze segregatora). Bez tej opcji Spacja w segregatorze służy do wpisywania nazwy, aby do niej przeskoczyć.
- Przeciągnij dokumenty na **przycisk Szybki podgląd** paska narzędzi: każdy otwiera się w osobnym panelu, a edytor zachowuje swój dokument.
- **Nawigacja > Otwórz Szybki podgląd** pokazuje cały segregator jako menu, więc możesz otworzyć dokument bez wcześniejszego zaznaczania go.

**W panelu:**

- **Tytuł** w pasku nagłówka można edytować (zmienia nazwę dokumentu). **Ikona** obok niego otwiera menu (Pokaż w segregatorze, Otwórz w edytorze, dodaj do zakładek projektu lub usuń z nich) i można ją przeciągać jak sam dokument. Upuść dokument na pasek nagłówka, aby pokazać go w tym panelu.
- Przycisk **w prawym górnym rogu** pokazuje pod tekstem dokumentu jego **streszczenie**, **notatki**, **słowa kluczowe** lub **metadane**. To, co tam zmienisz, zmienia dokument.
- **Pasek stopki** pokazuje liczbę słów i pozwala ustawić **etykietę** i **status**.
- **Przycisk zakładek** po lewej stronie paska stopki otwiera **zakładki projektu** obok dokumentu. Kliknięcie dokumentu z zakładki pokazuje go w panelu, więc jeden panel wystarcza, aby poruszać się między notatkami. Przeciągaj zakładki w inną kolejność, upuszczaj dokumenty i usuwaj zakładkę przyciskiem ➖ (dokument pozostaje w segregatorze). Pliki i odnośniki internetowe otwierają się dwukrotnym kliknięciem.
- Przycisk ➕ pod zakładkami tworzy **nowy dokument** i dodaje go do zakładek. Za pierwszym razem jesteś pytany(-a) o folder (na przykład folder „Notatki” w Materiałach); zaznacz **Nie pytaj ponownie**, aby od tej pory używać tego folderu. Zmienisz go w **Projekt > Ustawienia projektu > Foldery specjalne**.
- Przycisk **Odłącz** utrzymuje panel nad oknem głównym; **Okno > Odłącz panele Szybkiego podglądu** robi to dla wszystkich.

Menu Okno wymienia otwarte panele i zamyka je wszystkie naraz. Panele otwarte, gdy zamykasz projekt, wracają po jego ponownym otwarciu, z miejscem, rozmiarem, dolnym panelem i paskiem bocznym (wyłączysz to w **Plik > Opcje > Zachowanie**).

### Audio i wideo

Pliki audio i wideo w segregatorze (na przykład wywiady w folderze Materiały) są odtwarzane wewnątrz programu {{APP}}, gdy zainstalowany jest dodatek **Media Playback** (rozdział 17; przychodzi z aktualizacją 0.12). Bez niego taki plik pokazuje **Otwórz w domyślnej aplikacji**, jak dawniej.

**Odtwarzanie.** Kliknij plik w segregatorze. Wideo pokazuje swój obraz; plik audio pokazuje tytuł, wykonawcę i długość. **Pasek stopki** edytora ma przyciski odtwarzania:

- przycisk **wstrzymaj i przewiń** po lewej (zob. niżej),
- **przejdź na początek**, **przeskocz w tył**, **odtwarzaj / wstrzymaj**, **przeskocz w przód**,
- **pozycja**: przeciągnij suwak lub kliknij obok jego uchwytu,
- czas, **prędkość** (0,5× do 2×; wolniejsza mowa łatwiej się przepisuje; głos staje się niższy lub wyższy z prędkością, jak w taśmie) i **głośność** (kliknięcie: wyciszenie; strzałka: suwak).

Kliknięcie obrazu wideo odtwarza je lub wstrzymuje; kliknięcie prawym przyciskiem oferuje także **Otwórz w domyślnej aplikacji**. Gdy klawiatura jest w odtwarzaczu: **Spacja** odtwarza lub wstrzymuje, **strzałki w lewo** i **w prawo** przeskakują, **Home** idzie na początek.

**Transkrypcja.** Podziel edytor, pokaż plik audio w jednym edytorze, a swoją transkrypcję w drugim i pisz dalej: polecenia **Nawigacja > Multimedia** działają, gdziekolwiek jest klawiatura, także w panelach Szybkiego podglądu i w trybie pisania.

| Polecenie | Skrót |
|---------|----------|
| Odtwarzaj / Wstrzymaj | Ctrl+Enter |
| Przeskocz w tył / Przeskocz w przód (5 sekund) | Ctrl+Alt+, / Ctrl+Alt+. |
| Wstaw znacznik czasu multimediów (także **Wstaw > Wstaw znacznik czasu multimediów**: to samo polecenie, pokazane w obu menu) | Ctrl+Shift+Enter |
| Zatrzymaj, Przejdź na początek, Wstrzymaj i przewiń, Szybciej, Wolniej, Normalna prędkość, Głośniej, Ciszej, Wyciszenie, Przejdź do znacznika czasu przy kursorze | brak skrótu (nadaj im go w **Plik > Opcje > Klawiatura**) |

Polecenia działają na plik, który otworzyłeś(-aś) lub użyłeś(-aś) ostatnio (w obu edytorach, podglądzie pomocniczym lub panelu Szybkiego podglądu).

**Wstrzymaj i przewiń.** Po włączeniu (przycisk wygląda na wciśnięty) każda pauza cofa się trochę, o 2 sekundy, chyba że ustawisz inną wartość w **Plik > Opcje > Zachowanie > Odtwarzanie**. Gdy naciskasz Ctrl+Enter, aby wpisać to, co usłyszałeś(-aś), przy kontynuacji słyszysz ostatnie słowa jeszcze raz.

**Znaczniki czasu.** **Wstaw znacznik czasu multimediów** wpisuje przy kursorze czas odtwarzacza, taki jak `[03:07]` (z godzinami od jednej godziny: `[1:02:03]`). **Przejdź do znacznika czasu przy kursorze** robi odwrotnie: bierze znacznik czasu w wierszu kursora (najbliższy przed nim lub wokół niego) i przenosi tam odtwarzacz. Znaczniki czasu to zwykły tekst; można je też pisać ręcznie (`3:07`, `[03:07]`, `1:02:03.5`).

**W podglądzie pomocniczym lub panelu Szybkiego podglądu** odtwarzacz ma przyciski odtwarzania u dołu. Plik audio w podglądzie pomocniczym edytora transkrypcji to dobry układ dla jednego tylko edytora.

**Gdzie był.** Gdy otworzysz plik audio lub wideo ponownie, kontynuuje on tam, gdzie go wstrzymałeś(-aś) (wyłączysz to w **Plik > Opcje > Zachowanie > Odtwarzanie**). Pokazanie innego dokumentu w edytorze zatrzymuje plik.

**Co się odtwarza.** Dodatek *Media Playback* (2.0) odtwarza formaty, z których można korzystać swobodnie: dźwięk MP3, Ogg Vorbis, Opus, FLAC, ALAC, WAV, AIFF i WavPack; wideo WebM, MKV i Ogg z obrazem VP8, VP9 lub Theora (także MJPEG i inne wolne formaty). Dodatek przynosi wszystko, czego potrzebuje.

**Więcej formatów.** H.264, H.265 (HEVC), AAC (M4A), AC-3, E-AC-3, DTS, wideo MPEG-1/2/4, VC-1 i WMV, WMA oraz AMR są w niektórych krajach objęte licencjami patentowymi; przychodzą z osobnym dodatkiem **Extra Video and Audio Formats**. Najprostszym sposobem na uzyskanie obu jest jeden pakiet **media-all-formats.qbpkg** na stronie Releases: zainstalowany jak każdy dodatek (rozdział 17), instaluje *Media Playback* i *Extra Video and Audio Formats* razem. Oba pojawiają się wtedy w **Pomoc > Dodatki i aktualizacje > Menedżer dodatków**, gdzie dodatkowe formaty można osobno wyłączyć lub usunąć. To, czy do używania tych formatów potrzebna jest licencja patentowa, zależy od kraju. Gdy jest zainstalowany i włączony, *Odtwarzanie multimediów* z niego korzysta i odtwarzane są także pliki MP4/M4V/MOV, AVI, MPEG, WMV, FLV i 3GP (nie WMA Pro i WMV 9). Bez niego taki plik pokazuje pod odtwarzaczem komunikat, który wymienia jego formaty i ten dodatek; wszystko inne działa jak dawniej. Wideo AV1 nie jest odtwarzane przy użyciu wbudowanego dekodowania.

Od 0.80.0 odtwarzacz zawsze dekoduje własnym FFmpeg: Qt używane od tego czasu przez {{APP}} (własna kompilacja Qt 6.12 firmy The Qt Company) nie ma możliwości odtwarzania przez **GStreamer** systemu, więc **Plik > Opcje > Zachowanie > Odtwarzanie > Dekodowanie** pokazuje ten wybór jako niedostępny.

### Zakładki

**Zakładki dokumentu** (inspektor, karta zakładek) łączą dokument z innymi dokumentami, plikami na dysku lub stronami internetowymi. **Zakładki projektu** są dostępne z każdego dokumentu, z przycisku zakładek na pasku narzędzi i z paska bocznego paneli Szybkiego podglądu. Karta zakładek inspektora pokazuje obie listy, jedną nad drugą: **Zakładki dokumentu** i **Zakładki projektu**, każdą z własnymi przyciskami ➕ i ➖. Kliknięcie zakładki pokazuje dokument pod listami; dokument, który edytor już pokazuje, pojawia się tam jako kopia tylko do odczytu (pisz w edytorze), aby edytor zachował szerokość wiersza. Dwukrotne kliknięcie otwiera zakładkę. Przeciągaj dokumenty, pliki lub odnośniki internetowe na listę zakładek albo użyj przycisku ➕; odnośnik przeciągnięty z przeglądarki zachowuje tytuł strony. Przeciągnij zakładkę w górę lub w dół, aby zmienić kolejność. Dzielniki między dwiema listami a tekstem pod nimi można przesuwać; nadane im rozmiary są zachowywane w projekcie i wracają po ponownym uruchomieniu lub aktualizacji programu, także gdy w danej chwili nie jest wybrana żadna zakładka. Dokumenty upuszczone na **przycisk Zakładki** paska narzędzi stają się zakładkami projektu; tak samo zaznaczone dokumenty po **Dokumenty > Dodaj do zakładek projektu**; inspektor pokazuje wtedy swoją kartę zakładek z zaznaczonym nowym wpisem.

---

## 7. Inspektor

Przyciski u góry inspektora przełączają między:

- **Streszczenie i notatki**: streszczenie to krótkie podsumowanie, które pojawia się na fiszce. Zamiast tekstu możesz użyć obrazu: kliknij przycisk obrazu na fiszce i upuść plik obrazu lub obraz z segregatora na szary obszar (albo kliknij obszar dwukrotnie, aby wybrać plik; kliknij prawym przyciskiem, aby usunąć obraz). Poniżej są **notatki dokumentu** (lub, po przełączeniu małym menu, **notatki projektu**, wspólne dla całego projektu). **Nawigacja > Inspekcja > Przejdź do streszczenia** (**Ctrl+Alt+Y**) i **Przejdź do notatek dokumentu** (**Ctrl+Alt+N**) przenoszą tam klawiaturę z tekstu; **Alt+2** wraca do edytora. Od 0.93.0 notatki (dokumentu i projektu) zaczynają jako zwykły tekst: bez czcionki, rozmiaru, wcięcia pierwszego wiersza, odstępów akapitów i stylu z formatowania tekstu projektu, w czcionce interfejsu programu przy powiększeniu notatek. Formatowanie pojawia się tylko wtedy, gdy sam je zastosujesz (pasek formatowania, menu, skróty) i wtedy pozostaje. Notatki, które mają już tekst, zachowują dotychczasowy wygląd. Tekst wklejony do notatek jest zwykły; **Edycja > Wklej z formatowaniem** (**Ctrl+Alt+Shift+V**) zachowuje formatowanie, które ze sobą niesie.
- **Zakładki**: zakładki dokumentu i zakładki projektu. Kliknięcie zakładki dokumentu projektu pokazuje jego tekst pod listami. Możesz tam pisać i formatować jak w edytorze (pasek formatowania, menu Format i ich skróty działają na niego, gdy kursor jest w nim): wszystko trafia od razu do dokumentu i jego pliku, a edytor pokazujący ten sam dokument od razu pokazuje zmianę. Tekst tam ma własne powiększenie.
- **Mapa myśli**: tablica notatek dla dokumentu lub folderu (zob. „Mapy myśli” niżej).
- **Obiekty**: postacie, lokalizacje, środowiska, miejsca godne uwagi, pogoda i inne elementy twojej historii, każdy z własną kartą i powiązaniami między nimi (zob. „Obiekty” niżej).
- **Metadane**: etykieta, status, typ sekcji, uwzględnij w kompilacji, daty utworzenia i zmiany, **własne metadane** i **słowa kluczowe**.
- **Migawki** (rozdział 11).
- **Komentarze i przypisy**.

Narzędzia pisarskie z dodatków mogą dodawać własne karty; ich przyciski stoją w rzędzie na początku, na przykład **Transkrypcje** dodatku Speech Transcription (rozdział 10).

**Własne powiększenie inspektora.** **Ctrl+kółko myszy** nad częścią inspektora powiększa lub pomniejsza tę część, nie zmieniając edytorów. Karty z dwiema częściami mają powiększenie dla każdej: **Streszczenie i notatki** (fiszka, notatki), **Zakładki** (lista zakładek dokumentu, lista zakładek projektu, tekst zakładki pod nimi: trzy powiększenia), **Migawki** (lista, tekst migawki), **Obiekty** (przyciski kategorii z listą, karta). Lista **Powiększenie** na dole pokazuje i ustawia powiększenie części z klawiaturą lub części, którą powiększyłeś(-aś) ostatnio; jej tytuł nazywa część („Powiększenie: Notatki”), a na karcie Mapa myśli jest to powiększenie tablicy. Pozostałe karty (Metadane, Komentarze) oraz rzędy przycisków u góry i u dołu mają jedno wspólne powiększenie; z **Widok > Powiększaj karty inspektora osobno** (także w menu prawego przycisku listy Powiększenie) Metadane i Komentarze mają po jednym.

**Inspektor pamięta** razem z projektem, która karta była otwarta, gdzie ustawiłeś(-aś) przegrody wewnątrz kart, pokazaną zakładkę i obiekt pokazany na karcie Obiekty.

**Komentarze folderu.** Gdy w edytorze jest folder (jako tablica korkowa lub konspekt), karta komentarzy i przypisów wymienia komentarze i przypisy każdego dokumentu wewnątrz folderu i jego podfolderów, pogrupowane według dokumentu. Kliknięcie nazwy dokumentu otwiera go; kliknięcie **Komentarz ▸** (lub **Przypis ▸**) otwiera dokument i pokazuje to miejsce; tekst komentarza lub przypisu można zmienić od razu tam. W tekście połączonym (kilka dokumentów jako jeden tekst) karta wymienia notatki wszystkich pokazanych dokumentów.

### Mapy myśli

Każdy dokument i folder może mieć **mapę myśli**: tablicę, na której umieszczasz notatki gdziekolwiek, łączysz je liniami i strzałkami, kolorujesz i grupujesz na kształtach tła. Tablice można zapisywać jako pliki Scapple, a pliki Scapple można odczytywać (zob. *Pliki i Scapple* niżej). Kliknij przycisk **Mapa myśli** w inspektorze (między Zakładkami a Metadanymi). Przycisk jest podkreślony, gdy dokument ma mapę myśli.

**Przypięta mapa myśli (0.92.0).** Zwykle karta pokazuje mapę myśli zaznaczonego dokumentu lub folderu. Możesz zamiast tego przypiąć ją do jednej pozycji: karta pokazuje wtedy dalej mapę myśli tej pozycji, cokolwiek zaznaczysz w segregatorze lub otworzysz w edytorze. Służy to tablicy, którą chcesz mieć obok siebie podczas pisania, na przykład mapie fabuły trzymanej na folderze. Trzy miejsca robią to samo:

- **W segregatorze** przesuń mysz nad wiersz: mała pinezka pojawia się blisko jego prawego końca. Kliknij ją, a ta pozycja jest przypięta; jej pinezka pozostaje i jest wypełniona. Kliknięcie wypełnionej pinezki ją zwalnia. Kliknięcie pinezki niczego nie zaznacza.
- **Na karcie Mapa myśli** przycisk pinezki po prawej stronie małego paska przypina tablicę pokazywaną przez kartę i znów ją zwalnia. Gdy tablica jest przypięta, pasek nazywa jej pozycję (*Przypięta: Fabuła*, zaraz po przycisku przeglądu; długi tytuł jest skracany, a cały tytuł pokazuje się po zatrzymaniu myszy na nim), aby było jasne, że tablica nie należy do zaznaczonego dokumentu.
- **W menu**, **Nawigacja > Inspekcja > Przypnij mapę myśli tej pozycji** i **Nawigacja > Inspekcja > Zwolnij przypiętą mapę myśli** (na początku nie mają skrótów; nadaj im je w opcjach, jeśli chcesz).

Gdy pozycja jest przypięta, wąski pasek na dole segregatora mówi *Mapa myśli przypięta:* z jej tytułem i ma przycisk **Zwolnij**. Co najwyżej jedna pozycja projektu jest przypięta; przypięcie innej przenosi przypięcie. Można też przypiąć pozycję bez mapy myśli: karta pokazuje pustą tablicę do rozpoczęcia. Przypięta jest tylko karta Mapa myśli; pozostałe karty inspektora podążają za zaznaczonym dokumentem jak zawsze, a tablica otwarta w dużym oknie pozostaje taka, jaka jest. Przypięcie jest zachowywane razem z projektem na tym komputerze po jego zamknięciu i jest zwalniane, gdy pozycja trafia do Kosza.

**W inspektorze** tablica wypełnia kartę. Mały pasek nad nią ma **Nowa notatka**, **Nowy kształt tła**, **Dopasuj powiększenie**, **Menu** (wszystkie polecenia: Plik, Edycja, Widok, Notatki, Format), przycisk **przeglądu** (zob. *Rozglądanie się*) i **Otwórz w oknie**. Pasek pod nią liczy notatki i ma suwak powiększenia. **Otwórz w oknie** pokazuje tę samą tablicę w dużym oknie z tymi samymi menu w pasku menu i inspektorem po prawej (**Format notatki**, **Style notatek**, **Ustawienia dokumentu**); zmiany w jednym miejscu są od razu widoczne w drugim. Kliknięcie tablicy prawym przyciskiem otwiera polecenia dla tego, co jest pod myszą. Nowa notatka (przycisk, menu lub dwukrotne kliknięcie) od razu przejmuje klawiaturę, także gdy była ona wcześniej w segregatorze: to, co wpiszesz, zastępuje jej zaznaczony tekst (0.94.0).

Polecenia mapy myśli są w jej własnych menu, a nie w menu okna głównego. Poniżej **Menu > Notatki > Połącz** oznacza: przycisk **Menu** tablicy inspektora, potem **Notatki**, potem **Połącz** (w dużym oknie: **Notatki > Połącz** jego paska menu). Gdy tablica ma klawiaturę, jej klawisze mają pierwszeństwo: **Ctrl+Z** cofa zmianę tablicy, **Ctrl+F** szuka w jej notatkach, **Ctrl+P** ją drukuje.

**Pasek formatowania działa na notatkach** (0.99.0). Gdy tablica ma fokus klawiatury, pasek formatowania działa na jej notatkach: przy jednej lub kilku zaznaczonych notatkach na całym tekście każdej z nich (**Pogrubienie**, **Kursywa**, **Podkreślenie**, **Przekreślenie**, **Kolor tekstu**, **Automatyczny kolor tekstu**, cztery wyrównania, **Większa**, **Mniejsza**, **Wyczyść formatowanie**); przy edytowanej notatce pogrubienie, kursywa, podkreślenie i przekreślenie działają tylko na zaznaczonych słowach. Przyciski pokazują stan zaznaczenia (gdy jest mieszany, pokazują stan neutralny). To, czego notatka nie może mieć, jest wyszarzone, a nie ukryte: lista stylów, pola czcionki i rozmiaru (użyj **Większa** i **Mniejsza** albo pola rozmiaru w części Format notatki tablicy), interlinia, wyróżnienie, indeks górny, indeks dolny, listy, wcięcia, tabele i obrazy. Każda zmiana to jeden krok cofania nazwany jej imieniem.

**Kilka notatek naraz** (0.99.0). Każde polecenie menu prawego przycisku notatki (kolory, kształt, obramowanie, styl, ikony i reszta) zmienia wszystkie zaznaczone notatki jednym krokiem cofania, także gdy klikniesz prawym przyciskiem jedną z nich. Notatka z kolorem wypełnienia i bez własnego koloru tekstu pokazuje czarny lub biały tekst, zależnie od tego, który lepiej się czyta na wypełnieniu, niezależnie od koloru tablicy.

**Pomoc do map myśli.** **Pomoc > Pomoc do map myśli** (także przycisk **?** na prawym końcu paska tablicy i **Menu > Pomoc** tablicy) pokazuje krótki przewodnik z obrazkami: jak tworzyć notatki, jak narysować linię (przeciągnij notatkę na inną), jak narysować strzałkę (przytrzymaj **Alt** przy puszczaniu; **Ctrl+Alt** dla strzałki wstecz, **Ctrl+Alt+Shift** dla obu końców), słowa na linii i kształty tła. Jego przycisk **Otwórz tablicę przykładową** (także **Menu > Pomoc > Otwórz tablicę przykładową**) otwiera tablicę do ćwiczeń we własnym oknie, z notatkami do przeciągania na siebie i gotowymi przykładami linii i strzałek. Nic na przykładowej tablicy nie należy do twojego projektu i nie jest zapisywane.

**Notatki.**

- **Dwukrotne kliknięcie** pustego miejsca: nowa notatka, gotowa do pisania. **Ctrl+Enter** tworzy nową notatkę pod zaznaczoną (w stosie). Z **Ctrl** przy dwukrotnym kliknięciu nowa notatka jest połączona z zaznaczonymi notatkami; z **Alt** strzałką w jej kierunku; z **Ctrl+Alt** strzałką wstecz.
- **Dwukrotne kliknięcie** notatki zmienia jej tekst; **Esc** lub kliknięcie poza nią kończy edycję. Krótka notatka staje się wąska jak jej tekst; długa zawija się na szerokości nowych notatek (oba w **Menu > Plik > Opcje map myśli…**).
- **Przeciągaj** notatki, aby je przesuwać; z **Ctrl** przy upuszczeniu są kopiowane. **Klawisze strzałek** przesuwają zaznaczone notatki, gdy włączone jest **Menu > Edycja > Strzałki przesuwają notatki** (**M**) (z **Shift** o 10 px); w przeciwnym razie zaznaczają następną notatkę w tym kierunku. **Enter** lub **F2** edytuje zaznaczoną notatkę, **Delete** usuwa zaznaczone notatki.
- **Przeciągnij notatkę na inną**, aby połączyć obie linią; ponownie, aby usunąć linię. Z **Alt** linia dostaje strzałkę w stronę notatki, na którą upuszczono; z **Ctrl+Alt** strzałkę wstecz; z **Ctrl+Alt+Shift** strzałki na obu końcach.
- **Kliknij linię dwukrotnie**, aby umieścić w niej nową notatkę, albo etykietę linii, aby ją zmienić (**Menu > Notatki > Etykieta połączenia…**).
- **Menu > Notatki > Połącz** (**Ctrl+.**), **Połącz strzałką** (**Ctrl+Alt+.**), **Usuń strzałki** (**Ctrl+,**) i **Rozłącz** działają na zaznaczone notatki: pierwsza zaznaczona jest środkiem.
- **Menu > Notatki > Nowa połączona notatka** tworzy notatkę po lewej, po prawej, powyżej lub poniżej zaznaczonej, połączoną z nią (ze strzałką lub bez).
- **Menu > Notatki > Podziel** (**Ctrl+Shift+K**) dzieli notatkę w miejscu kursora; **Scal** łączy zaznaczone notatki.
- **Menu > Notatki > Stos** (**Ctrl+'**), **Wyrównaj**, **Rozmieść równomiernie**, **Ustaw tę samą szerokość / wysokość**, **Dopasuj automatycznie**, **Przesuń na wierzch**, **Przesuń na spód**.
- **Menu > Edycja > Dodaj odnośnik…** (**Ctrl+K**) łączy tekst notatki ze stroną internetową lub plikiem; dokumenty przeciągnięte z segregatora na tablicę stają się notatkami z odnośnikami do nich. Kliknięcie odnośnika go otwiera: dokument otwiera się tak jak odnośniki do dokumentów w tekście (**Plik > Opcje > Zachowanie**).
- **Menu > Notatki > Dodaj notatki do segregatora** zamienia zaznaczone notatki w dokumenty wewnątrz dokumentu lub folderu, do którego należy tablica, w kolejności czytania tablicy.

**Kształty tła** (**Menu > Notatki > Nowy kształt tła**, **Ctrl+Shift+G**) to kolorowe obszary za notatkami. Utworzony przy zaznaczonych notatkach kształt je otacza. Kształt **magnetyczny** (**Menu > Notatki > Magnetyczna**, **Ctrl+Shift+M**) zabiera ze sobą leżące na nim notatki, gdy jest przesuwany. Nowy kształt zaczyna jako średnioszary (#808080) i niemagnetyczny; **Plik > Opcje > Dodatkowe > Mapa myśli** (i **Menu > Plik > Opcje map myśli…**) ustawia wypełnienie, obramowanie i **Magnetyczna**, z którymi zaczynają nowe kształty. (Przed 0.93.0 nowy kształt był jasnoszary i magnetyczny; wybrany tam kolor pozostaje, a zapisane **Magnetyczna** zostało raz wyłączone.)

**Menu prawego przycisku notatki** (lub kształtu tła) zmienia zaznaczone notatki: **Kolor wypełnienia**, **Kolor tekstu** i **Kolor obramowania** oferują kolory etykiet twojego projektu, sześć kolorów paska formatowania (czerwony, niebieski, żółty, zielony, biały, czarny) oraz **Inny...** dla dowolnego koloru, ten sam wybór wszędzie w programie {{APP}}; **Kształt** to **Prostokąt**, **Zaokrąglony prostokąt**, **Chmura** lub **Postrzępiony** (notatka bez obramowania dostaje cienkie, aby kształt było widać); **Obramowanie** ustawia grubość; **Ikony** umieszcza na notatce do ośmiu ikon, po jednej w każdym rogu i jednej pośrodku każdego boku, wybranych spośród ikon segregatora (**Dokumenty > Zmień ikonę**), łącznie z twoimi własnymi ikonami. Kliknij prawym przyciskiem blisko miejsca ikony, aby usunąć tylko tę jedną; **Usuń wszystkie ikony** czyści je.

**Menu prawego przycisku linii** dotyczy tylko tej linii, aby inne linie tych samych notatek nie były zmieniane: jej strzałki (**Bez strzałek**, strzałka do którejkolwiek notatki lub **Strzałki na obu końcach**), **Kolor linii**, **Ikona pośrodku** (ikona rysowana pośrodku linii), etykieta, **Wstaw notatkę na linii** i **Rozłącz**. Każdą zmianę można cofnąć osobno.

Ikony, kolory linii i ikony linii to własne dodatki programu {{APP}}; są zachowywane w pliku tablicy w elementach, których Scapple nie czyta (tablica wyeksportowana jako plik Scapple ma notatki, linie i strzałki bez nich). **Plik > Opcje > Dodatkowe > Mapa myśli** ustawia rozmiar ikon i kształt nowych notatek.

**Wygląd.** **Menu > Format** ma pogrubienie, kursywę, podkreślenie, przekreślenie, większe i mniejsze (**Ctrl+Shift+>** i **Ctrl+Shift+<**), kolory tekstu, wypełnienia i obramowania, grubość obramowania (brak, od 1 do 5 px) i styl (zaokrąglony, kwadratowy, postrzępiony, chmura), cień oraz **Wyblakłe** (**Ctrl+Shift+F**, notatka jest pokazywana blado). **Style notatek** zapisują wygląd pod nazwą; **Menu > Format > Styl notatki** wymienia style notatek (kliknięcie stosuje jeden; 0.99.0), a potem **Nowy styl notatki z zaznaczenia**, **Zdefiniuj styl notatki ponownie z zaznaczenia…** i **Usuń styl notatki…** (te dwa pytają, o który styl chodzi); stosuje jeden (**Ctrl+Alt+1** do **9**, **Ctrl+Alt+0** dla Domyślnego), tworzy go z zaznaczonej notatki lub kopiuje i wkleja wygląd notatki (**Alt+Shift+C** / **Alt+Shift+V**). **Ustawienia dokumentu** w inspektorze okna ustawiają tło, kolor tekstu, czcionkę i kierunek pisania tablicy (oraz **Ustaw jako domyślne** dla nowych tablic).

**Tło.** Kliknięcie prawym przyciskiem pustego miejsca tablicy (w inspektorze lub w oknie), **Menu > Format > Kolor tła** albo przycisk tła w **Ustawieniach dokumentu** oferuje kolory etykiet twojego projektu, sześć kolorów paska formatowania, domyślny (**Domyślny dla jasnych motywów** lub **Domyślny dla ciemnych motywów** w menu, **Użyj domyślnego** w Ustawieniach dokumentu) i **Inny...**. Tablica ma własne tło dla jasnych motywów i jedno dla ciemnych: wybrany kolor jest zachowywany dla rodzaju używanego motywu, a drugi rodzaj zachowuje swój. Tablica bez własnego koloru pokazuje domyślny z **Plik > Opcje > Dodatkowe > Mapa myśli** („Tło tablic”, jedno dla jasnych i jedno dla ciemnych motywów) i podąża za nim, gdy go zmienisz. **Ustaw jako domyślne** w Ustawieniach dokumentu czyni tło tablicy domyślnym dla rodzaju używanego motywu. Tablica zachowuje swój kolor tła także wtedy, gdy nic na niej jeszcze nie ma.

**Powiększenie i miejsce.** Każda tablica pamięta razem z projektem swoje powiększenie i część widoczną na ekranie, osobno dla inspektora i dla dużego okna. Tablica, której nigdy nie powiększałeś(-aś), otwiera się z domyślnym powiększeniem map myśli (**Plik > Opcje > Edycja**, „Domyślne powiększenie tekstu”).

**Zaznaczanie.** Kliknij, **Shift+kliknięcie** lub **Ctrl+kliknięcie**, aby dodać, przeciągnij ramkę wokół notatek, **Ctrl+A**. **Menu > Edycja > Zaznacz** ma notatki ułożone w stos poniżej, notatki na zaznaczonych kształtach (**Alt+A**), połączone notatki (**Ctrl+Shift+A**) i całe połączone grupy; pod nim są **Odwróć zaznaczenie** (**Alt+Shift+I**) i **Odznacz** (**Ctrl+D**).

**Rozglądanie się.** Powiększaj za pomocą **Ctrl+kółko myszy**, **Ctrl++** i **Ctrl+-**, **Rozmiar rzeczywisty** (**Ctrl+0**), **Dopasuj powiększenie** (**Ctrl+9**), **Powiększ do zaznaczenia** (**Ctrl+Shift+8**) i **Powiększ losowo** (**Ctrl+Shift+R**). Przytrzymaj **Spację** i przeciągaj, aby przesuwać tablicę; przytrzymaj **Z**, aby na chwilę zobaczyć całą tablicę. **Menu > Edycja > Znajdź** (**Ctrl+F**) znajduje i zamienia tekst w notatkach. **Menu > Widok > Prowadnice strony** pokazuje, gdzie kończą się strony przy drukowaniu.

**Przegląd (0.93.0).** Mała mapa w lewym górnym rogu tablicy pokazuje całą tablicę w miniaturze i ramkę dla części, którą teraz widzisz; ramka przesuwa się i zmienia rozmiar, gdy przewijasz i powiększasz. Notatki, kształty tła i linie są rysowane w swoich kolorach; tekst notatki jest pokazany tylko tam, gdzie da się go jeszcze przeczytać w tym rozmiarze, w przeciwnym razie notatka jest swoją kolorową ramką. Kliknij w przeglądzie, aby przenieść widok do tego punktu; przeciągnij ramkę, aby przesunąć widok razem z nią. Powiększenie pozostaje, a tablicę zablokowaną (**Zablokuj edycję**) też można w ten sposób przemieszczać. Kółko myszy nad przeglądem działa jak nad tablicą (**Ctrl+kółko** powiększa). Przeciągnij uchwyt w jego prawym dolnym rogu, aby zmienić szerokość (zaczyna od jednej piątej szerokości widoku, co najmniej 80 px, i nigdy nie jest szerszy niż połowa widoku); wysokość podąża za kształtem tego, co pokazuje, nigdy nie więcej niż połowa widoku, a szerokość maleje razem z nią, gdy wysokość miałaby to przekroczyć. Przegląd należy tylko do widoku: nie ma go w eksportach, wydrukach ani obrazie tablicy. Na początku jest **automatyczny**: pokazuje się, gdy część tablicy leży poza widokiem, i znika, gdy mieści się cała (z małym marginesem, aby nie migał na krawędzi). Przycisk **przeglądu** na pasku tablicy inspektora (zaraz po **Menu**) przełącza między **automatycznym** a **zawsze pokazanym** (przycisk jest wciśnięty); ten sam przełącznik to pozycja **Przegląd zawsze pokazany** w menu Widok tablicy (w inspektorze: przycisk **Menu**, potem **Widok**; w dużym oknie: jego pasek menu). Wybór i rozmiary są zachowywane na tym komputerze, w ustawieniach programu (jeden wybór dla wszystkich tablic; rozmiar przeglądu inspektora i dużego okna osobno); nic nie trafia do projektu.

**Tekst, obrazy i pliki.** Wklej lub upuść tekst, aby utworzyć notatkę (tekst z pustymi wierszami może stać się kilkoma notatkami), upuść obraz, aby utworzyć notatkę z obrazem, i upuść plik tekstowy, aby zrobić notatki z jego akapitów.

**Pliki i Scapple.** **Menu > Plik > Eksport** zapisuje tablicę jako PDF (**Ctrl+Shift+E**), obraz PNG, zwykły tekst, listę tekstową, RTF, konspekt OPML, jej obrazy lub plik Scapple (*.scap*); **Menu > Plik > Importuj mapę myśli lub plik Scapple…** wczytuje plik *.scap* do tablicy. **Drukuj** i **Podgląd wydruku** ją drukują. **Menu > Plik > Zablokuj edycję** blokuje tablicę.

**Opcje map myśli** (**Menu > Plik > Opcje map myśli…** oraz to samo w **Plik > Opcje > Dodatkowe > Mapa myśli** okna głównego): czy **Enter** kończy edycję notatki (wtedy **Shift+Enter** tworzy nowy wiersz), czy **Enter** na zaznaczonej notatce tworzy nową, czy nowe notatki przyjmują wygląd zaznaczonej notatki, linie przerywane bez strzałek, wąskie notatki dla krótkich tekstów, jak blade są wyblakłe notatki oraz rozmiar, szerokość, kolor i obramowanie nowych notatek i kształtów.

**Gdzie przechowywana jest tablica.** Tablica jest zapisywana sama, jako plik w układzie plików Scapple (*content/<ID>/mindmap.scap* w folderze projektu; *Files/Data/<UUID>/quillbinder-mindmap.scap* w projekcie Scrivener, obok własnych plików dokumentu). Dokument dostaje plik dopiero wtedy, gdy coś zostanie położone na jego tablicy. Cofanie (**Ctrl+Z**) działa dla każdej tablicy osobno.

{{APP}} odczytuje pliki Scapple tak, jak zapisuje je Scapple 1.x, w zakresie, w jakim znany jest ich układ; nie dało się ich przetestować samym programem Scapple (zob. uwagi do wersji 0.17.0 w pliku stanu).

### Obiekty: postacie, lokalizacje i więcej

**Obiekty** to ludzie, miejsca i rzeczy twojej historii, każdy z kartą pól: postać z rasą, wzrostem, kolorem oczu, cechami i historią; miejsce z jego rodzajem, środowiskiem i pogodą; i tak dalej. Należą do całego projektu, a nie do jednego dokumentu, i są przechowywane osobno od segregatora. Powiązania łączą je, więc możesz przeklikać się przez tło: karta postaci, wieś, w której mieszka, otaczający ją krajobraz, jego pogoda.

Kliknij przycisk **Obiekty** w inspektorze (na prawo od przycisku **Mapa myśli**) albo wybierz **Nawigacja > Inspekcja > Obiekty**. **Okno > Obiekty** (lub przycisk okna na pasku karty) pokazuje te same obiekty w dużym własnym oknie, z listą po lewej i kartą po prawej. Zmiany w jednym miejscu są od razu widoczne w drugim.

**Kategorie.** Kolorowe przyciski u góry to kategorie: na początek **Postacie**, **Lokalizacje**, **Środowiska**, **Miejsca godne uwagi** i **Pogoda**. Kliknij jeden, aby zobaczyć jego obiekty. Możesz zmieniać ich nazwy, nadawać im inne kolory, zmieniać kolejność, usuwać je i dodawać własne kategorie (frakcje, zaklęcia, statki ...): zob. *Szablony i kolory* niżej.

**Tworzenie obiektów.** **+** na pasku karty tworzy nowy obiekt w wybranej kategorii (w folderze zaznaczonego obiektu); wpisz jego nazwę u góry karty. Przycisk z folderem tworzy **folder** do sortowania, na przykład „Wsie” wewnątrz Lokalizacji; foldery mogą zawierać foldery. Przeciągaj obiekty i foldery na liście, aby je przenosić. **F2** lub kliknięcie prawym przyciskiem zmienia nazwę, a menu prawego przycisku ma też **Duplikuj** i **Usuń…** (także w menu **≡** paska). Pole **Filtr** pokazuje tylko obiekty, których nazwa lub tekst zawiera wpisane przez ciebie słowa.

**Karta.** Pod kolorowym paskiem z kategorią i nazwą znajdują się pola szablonu kategorii. Wszystko, co wpiszesz, jest zapisywane samo, jak twoje dokumenty. Rodzaje pól to:

- **Krótki tekst** (jeden wiersz) i **Data** (w dowolnej postaci, na przykład „wiosna 1203” lub „trzecia zima”).
- **Liczba** z jednostką (cm, lata, °C ...).
- **Długi tekst**, który może zawierać odnośniki (zob. niżej) oraz **pogrubione**, *pochylone*, podkreślone, przekreślone, kolorowane i wyróżnione słowa: gdy kursor jest w polu, działają na nie pasek formatowania, menu Format i ich skróty (**Ctrl+B**, **Ctrl+I**, **Ctrl+U**; także w oknie Obiekty). Wersje 0.20 i 0.21 programu {{APP}} też je pokazują.
- **Wybór**: lista do wyboru; możesz też wpisać coś innego.
- **Tak / nie**.
- **Obraz**: kliknij obszar (lub upuść na niego obraz), aby wybrać jeden; jest kopiowany do projektu i zmniejszany, jeśli jest większy niż 1600 pikseli. Kliknięcie prawym przyciskiem go usuwa.
- **Odnośnik do obiektów**: powiązane obiekty są pokazane jako kolorowe przyciski. **+** dodaje jeden: wpisz, aby wyszukać, a potem wybierz obiekt albo wybierz **Nowy w Lokalizacjach: „Ashvale”**, aby utworzyć nowy obiekt o tej nazwie i od razu go powiązać. Kliknięcie powiązanego obiektu pokazuje jego kartę; kliknięcie prawym przyciskiem usuwa powiązanie.
- **Odnośnik do dokumentów**: dokumenty segregatora należące do obiektu, takie jak „Pojawia się w” postaci lub „Sceny rozgrywające się tutaj” miejsca. Kliknięcie otwiera dokument w edytorze.

**Przycisk koloru.** Przycisk **Kolor** paska nadaje obiektowi własny kolor (albo przywraca kolor kategorii), na przykład aby oznaczyć główne postacie.

**Odnośniki w długich tekstach.** W polu długiego tekstu zaznacz słowo i wybierz **Odnośnik do obiektu…** (lub **Odnośnik do dokumentu…**) w jego menu prawego przycisku; bez zaznaczenia nazwa obiektu jest wstawiana jako odnośnik. Kliknięcie odnośnika za nim podąża; **Usuń odnośnik** w menu prawego przycisku znów go usuwa. W tekście dokumentu **Wstaw > Odnośnik do obiektu…** wstawia do tekstu odnośnik do obiektu; kliknięcie go pokazuje obiekt w inspektorze. Gdy rękopis jest kompilowany, te odnośniki stają się zwykłymi słowami.

**W tę i z powrotem.** **‹** i **›** na pasku karty cofają i przesuwają w przód przez obiekty, które oglądałeś(-aś), jak w przeglądarce internetowej.

**Powiązane z.** Na dole każdej karty **Powiązane z** wymienia obiekty, które łączą się z tym (w polu odnośnika lub w tekście), więc wieś pokazuje, kto w niej mieszka. Kliknięcie przenosi tam.

**W tym dokumencie.** Gdy dokument w edytorze jest powiązany z obiektami (na przykład przez pole odnośnika do dokumentu „Pojawia się w”), karta pokazuje **W tym dokumencie:** z tymi obiektami, a przycisk **Obiekty** jest podkreślony jak inne karty z zawartością.

**Usuwanie.** Usunięcie obiektu najpierw pyta. Odnośniki do niego są usuwane z innych obiektów (w tekstach powiązane słowo pozostaje jako zwykły tekst). Odnośnik do niego w dokumencie mówi po kliknięciu, że obiekt już nie istnieje. Usunięcie folderu zachowuje jego obiekty; przenoszą się o jeden poziom wyżej.

**Szablony i kolory.** **Projekt > Szablony i kolory obiektów…** (także w menu **≡** karty i w menu prawego przycisku przycisku kategorii) edytuje kategorie i ich pola:

- Po lewej kategorie: **Dodaj**, **Usuń** i strzałki do zmiany kolejności. Usunięcie kategorii, która ma obiekty, pyta, czy mają zostać usunięte także one; **Nie** niczego nie zmienia.
- Po prawej nazwa i **Kolor** kategorii oraz jej pola: **Dodaj pole**, **Usuń**, **W górę**, **W dół**. Dla każdego pola jego nazwa i rodzaj; liczba może mieć jednostkę, wybór swoją listę wyborów (jeden na wiersz), a pole odnośnika kategorie, z którymi może się łączyć (żadna zaznaczona: wszystkie).
- **Użyj dla nowych projektów** zachowuje te kategorie i szablony dla projektów, które tworzysz od teraz. **Szablony standardowe** przywraca kategorię do szablonu, z którym przyszła, albo dodaje standardowe kategorie, które usunąłeś(-aś).
- Usunięcie pola z szablonu ukrywa to, co obiekty w nim mają, ale nie usuwa tego z ich plików. Dla pola standardowego **Szablony standardowe > Przywróć tę kategorię do standardu** przywraca je z zawartością; własne pole nie wraca, więc zmień nazwę pola zamiast je usuwać, jeśli możesz jeszcze potrzebować jego zawartości.

**Gdzie przechowywane są obiekty.** W folderze projektu, w folderze *objects*: jeden mały plik na obiekt (*objects/characters/<ID>.json* i tak dalej), szablony, kolory i foldery w *objects/schema.json*, a obrazy w *objects/images*. Sam plik projektu nie jest zmieniany, więc starsze wersje programu {{APP}} otwierają projekt jak dawniej i zostawiają obiekty w spokoju. Projekt w formacie Scrivener przechowuje swoje obiekty w swojej kopii roboczej (rozdział 14); Scrivener ich nie zna. Kopie zapasowe je zawierają.

---

## 8. Organizowanie projektu

### Słowa kluczowe

Słowa kluczowe to tagi, na przykład postacie lub miejsca w scenie. Dodawaj je w inspektorze albo zarządzaj wszystkimi w **Projekt > Słowa kluczowe projektu** (**Ctrl+Alt+K**), gdzie możesz nadawać im kolory, przypisywać je zaznaczonym dokumentom i znajdować wszystkie dokumenty z danym słowem kluczowym. Możesz też **przeciągać słowa kluczowe** z tego okna na dokumenty w segregatorze, kolekcji, tablicy korkowej lub konspekcie (jeśli dokument jest jednym z kilku zaznaczonych, wszystkie dostają słowa kluczowe) albo na listę słów kluczowych inspektora lub panelu Szybkiego podglądu. Przytrzymaj **Alt** przy upuszczaniu, aby przypisać także grupę słowa kluczowego. Wewnątrz okna przeciągnij słowo kluczowe na inne, aby umieścić je w tej grupie. Kolory słów kluczowych pojawiają się na fiszkach.

### Własne metadane

W **Projekt > Ustawienia projektu > Własne metadane** możesz dodawać własne pola: tekst, pole wyboru, listę (stały zestaw wyborów) lub datę, na przykład *Narrator*, *Miejsce* lub *Dzień fabuły*. Pojawiają się w inspektorze i jako kolumny konspektu.

### Zapisywanie ustawień projektu do pliku

**Projekt > Zapisz ustawienia projektu do pliku…** zapisuje ustawienia otwartego projektu do osobnego pliku (*.qbsettings*): etykiety, statusy, typy sekcji, własne metadane, słowa kluczowe, formatowanie domyślne, style, format scenariusza, cele, ustawienia kompilacji, widok strony i rozmiar strony, ustawienia wydruku, wygląd (motyw, kolory, czcionki, szerokość kolumny, obramowanie), własne opcje projektu (Plik > Opcje z „Ten projekt”) oraz szablony i kolory karty Obiektów. Tekst twoich dokumentów nie jest jego częścią. Etykiety, typy sekcji i foldery, które nazywają ustawienia kompilacji, są przechowywane według nazw, więc plik można wczytać do innego projektu.

**Projekt > Wczytaj ustawienia projektu z pliku…** wczytuje taki plik do otwartego projektu. Gdy ustawienia projektu są takie same jak w pliku, mówi o tym i niczego nie zmienia. Gdy się różnią, okno wymienia każdą sekcję z tym, co się różni (na przykład „tylko w pliku: Pomysł; tylko w projekcie: Korekta”) i pozwala wybrać dla każdej sekcji:

- **Weź z pliku**: ustawienia pliku dla tej sekcji.
- **Zachowaj ustawienia projektu**: sekcja pozostaje taka, jaka jest.
- **Domyślne Quillbinder**: to, co dostaje nowy projekt (ustawienia dla nowych projektów z **Plik > Opcje**, standardowe etykiety i statusy ...).

**Weź wszystko z pliku** i **Zachowaj wszystko z projektu** ustawiają każdą sekcję naraz. Cokolwiek wybierzesz, nic, czego używają twoje dokumenty, nie ginie: etykieta, status, typ sekcji, słowo kluczowe, pole własnych metadanych, kategoria obiektów lub styl, których dokumenty nadal używają, pozostają w projekcie, nawet gdy pliku ich nie ma, a komunikat na końcu wymienia, co zachowano. Gdy zmienia się formatowanie lub styl, tekst wszystkich dokumentów za tym podąża, tak jak przy samodzielnej zmianie. Przed wczytaniem {{APP}} zachowuje ustawienia takimi, jakie były, w pliku w folderze *settings* projektu („settings-before-loading-*data*.qbsettings”); wczytaj ten plik, aby wrócić.

### Typy sekcji

Typy sekcji mówią Kompilacji, czym jest każdy dokument: *Część*, *Rozdział*, *Scena*, *Sekcja* i tak dalej. Ustawiaj je dla każdego dokumentu w inspektorze albo pozwól programowi {{APP}} zdecydować według położenia (**Ustawienia projektu > Typy sekcji**, „Domyślne typy według struktury”), na przykład: foldery na pierwszym poziomie to rozdziały, dokumenty tekstowe w nich to sceny.

### Szablony dokumentów

Umieść dokumenty szablonów (na przykład kartę postaci) w folderze, a potem wybierz ten folder w **Ustawienia projektu > Foldery specjalne**. Szablony pojawiają się w **Projekt > Nowy z szablonu**. W **Dokumenty > Domyślny szablon dla poddokumentów** możesz sprawić, że folder używa szablonu dla każdego nowego dokumentu, który w nim utworzysz.

### Szablony projektów

**Plik > Zapisz jako szablon…** zapisuje bieżący projekt (strukturę, ustawienia i ewentualne dokumenty przykładowe) jako szablon dla nowych projektów.

---

## 9. Wyszukiwanie

### Szybkie wyszukiwanie

Wpisz kilka liter w pole na pasku narzędzi. Lista pokazuje dokumenty, których **tytuł**, **streszczenie** lub **tekst** je zawiera, do dziesięciu każdego rodzaju pod własnym nagłówkiem. Wybierz jeden, aby go otworzyć: miejsca w tekście zawierające to, co wpisałeś(-aś), są **zaznaczone**, a pierwsze jest wybrane i przewinięte do widoku. Znaczniki znikają, gdy wybierzesz inny dokument w segregatorze. To najszybsza droga do dokumentu, którego nazwę znasz.

### Wyszukiwanie w projekcie

**Ctrl+Shift+F** lub lupa na pasku narzędzi pokazuje pole wyszukiwania nad segregatorem. Wpisz, a segregator zostanie zastąpiony fioletową listą wyników.

- Kliknij wynik, aby go otworzyć. **Trafienia są wyróżnione w tekście**, pierwsze jest wybrane i przewinięte do widoku, a **F3** / **Shift+F3** przeskakują od jednego do następnego. **Ctrl+F** otwiera pasek wyszukiwania już wypełniony twoim hasłem.
- **X** w nagłówku wyników wraca do segregatora; edytor zachowuje dokument, który pokazuje. Kliknij lupę na pasku narzędzi ponownie, aby ukryć pole wyszukiwania.
- Gdy kolekcje są pokazane, karta **Wyniki wyszukiwania** uruchamia ostatnie wyszukiwanie ponownie. Ostatnie wyszukiwanie jest zachowywane razem z projektem.

Lupa po lewej stronie pola wyszukiwania otwiera opcje:

| Grupa | Pozycje |
|-------|---------|
| **Szukaj w** | **Wszystko** lub jedno z: Tytuł, Tekst, Streszczenie, Notatki, Słowa kluczowe, Etykieta, Status, Własne metadane. Kliknięcie wybiera to jedno miejsce. Przytrzymaj **Alt** przy kliknięciu (lub przy otwieraniu menu), aby wybrać kilka miejsc. |
| **Operator** | **Wszystkie słowa** (każde słowo musi wystąpić, w dowolnej kolejności), **Dowolne słowo**, **Dokładna fraza**, **Wyrażenie regularne** |
| **Opcje** | Całe słowa, Uwzględnij wielkość liter, Odwróć wyniki (dokumenty, które *nie* pasują), Szukaj tylko w Szkicu, Szukaj tylko w zaznaczeniu segregatora (dokumenty zaznaczone w segregatorze i wszystko w nich), Szukaj w dokumentach „uwzględnionych” / Szukaj w dokumentach „wykluczonych” (według **Uwzględnij w kompilacji**), Uwzględnij Kosz |

Przykład: aby zobaczyć wszystko, co nadal ma status „Do zrobienia”, wyszukaj `To Do` i wybierz **Status** w Szukaj w.

**Zapisz wyszukiwanie jako kolekcję…** na dole menu zachowuje wyszukiwanie wraz z jego opcjami jako kolekcję, która uruchamia się ponownie za każdym razem, gdy ją otworzysz.

### Zamiana w projekcie

**Edycja > Znajdź > Zamiana w projekcie** zamienia tekst w całym projekcie naraz.

1. Wpisz tekst do **Zamiany** i tekst, który ma go zastąpić (**Na**). Pozostaw „Na” puste, aby usunąć tekst.
2. Wybierz **Uwzględnij wielkość liter**, **Tylko całe słowa** lub **Wyrażenie regularne** (wtedy „Na” może używać `\1` do `\9` dla grup wyrażenia).
3. Zaznacz, gdzie zamieniać: **Tytuły**, **Tekst**, **Notatki**, **Streszczenia**, **Własne metadane** (pola tekstowe), **Komentarze i przypisy** (teksty pokazywane w inspektorze).
4. Wybierz **Cały projekt** lub **Tylko zaznaczone dokumenty** (dokumenty zaznaczone w segregatorze, wraz z tym, co w nich jest). Kosz jest pomijany, chyba że go zaznaczysz.
5. **Policz** mówi, ile wystąpień by się zmieniło i gdzie. **Zamień wszystko** pyta jeszcze raz, a potem to robi.

Zamiany w projekcie nie można cofnąć jako całości. Pozostaw zaznaczone **Wykonaj migawkę każdego tekstu przed jego zmianą**: każdy zmieniony tekst ma wtedy migawkę o nazwie „Przed zamianą …”, do której można wrócić (rozdział 11). Tytuły i streszczenia nie mają migawek; przy dużej zmianie najpierw zrób kopię zapasową poleceniem **Plik > Kopia zapasowa > Utwórz kopię zapasową teraz**. Tekst otwarty w edytorze można też cofnąć tam poleceniem **Edycja > Cofnij**.

Formatowanie pozostaje takie, jakie jest: nowy tekst przejmuje formatowanie tekstu, który zastępuje. Tekst, który nie jest otwarty w edytorze, jest zmieniany w swoim pliku, więc **Edycja > Cofnij** nie cofa tej zmiany (cofa ją wcześniej wykonana migawka); to samo dotyczy Konwersji cudzysłowów, kart komentarzy oraz Porównaj i przenieś.

### Znajdź według formatowania

**Edycja > Znajdź > Znajdź według formatowania** (**Ctrl+F3**) otwiera małe okno, które pozostaje otwarte podczas pracy. Wybierz, czego szukać:

| Znajdź | Znajduje | Dodatkowy wybór |
|------|-------|--------------|
| Wyróżniony tekst | tekst zaznaczony zakreślaczem | ogranicz do jednego koloru |
| Komentarze i przypisy | tekst z dołączonym komentarzem lub przypisem (inspektor) | ogranicz do jednego koloru komentarza |
| Adnotacje śródtekstowe, przypisy śródtekstowe | notatki wpisane w tekst | kolor adnotacji |
| Kolor rewizji | tekst wpisany w trybie rewizji | która rewizja |
| Kolorowany tekst | tekst, którego kolor ustawiłeś(-aś) ręcznie | ogranicz do jednego koloru |
| Odnośniki | odnośniki internetowe i odnośniki do dokumentów | |
| Format znaków | tekst pogrubiony, pochylony, podkreślony, przekreślony (ustawiony ręcznie, nie przez styl) | który z nich |
| Styl | tekst ze stylem akapitu lub znaków | który styl |
| Obrazy | obrazy w tekście | |

**Zawierający tekst** zawęża wyszukiwanie do miejsc zawierających słowo (dla komentarzy i przypisów liczy się także własny tekst notatki). **Następny** i **Poprzedni** zaznaczają następne miejsce. Przy **Szukaj w: Wszystkich dokumentach** wyszukiwanie jest kontynuowane w kolejnych dokumentach segregatora i je otwiera; przy **Tylko w bieżącym edytorze** pozostaje tam, gdzie jesteś. **Ctrl+G** i **Ctrl+Shift+G** powtarzają wyszukiwanie bez okna. **Zamknij** (lub **Esc**) zamyka okno.

---

## 10. Cele, statystyki i narzędzia pisarskie

- **Projekt > Cele projektu** (**Ctrl+Shift+T**): ustaw cel słów lub znaków dla Szkicu i dla każdej sesji pisania, a opcjonalnie także termin i dni, w których piszesz. {{APP}} może wtedy policzyć, ile musisz napisać w każdej sesji. Stopka pokazuje postęp obok liczby słów (od wersji 0.94.0): mały pasek i na przykład „83 / 1,000 dzisiaj” dla sesji oraz „12,400 / 80,000 szkic” dla Szkicu, każdy tylko wtedy, gdy jego cel jest ustawiony; kliknięcie go otwiera to okno. Dwa cienkie paski pod polem szybkiego wyszukiwania (niebieski: Szkic; pomarańczowy: sesja) pozostają, a **Plik > Opcje > Wygląd > Ogólne ustawienia interfejsu** pozwala je wyłączyć. Słowa, które trafiają do projektu jednym krokiem, nie są liczone jako napisane w sesji (0.94.0): zaimportowany tekst (**Plik > Import**, **Importuj i podziel**, upuszczone pliki, strony internetowe), wklejenie ponad 200 słów oraz słowa, które projekt już zawiera przy pierwszym otwarciu. **Zeruj sesję teraz** zaczyna liczenie od 0, a podpowiedź pola szybkiego wyszukiwania pokazuje to od razu. Okno celów rezerwuje miejsce na swoje paski, dzięki czemu pola nie przesuwają się, gdy ustawiasz cel.
- **Cele dokumentu**: kliknij cel w stopce edytora.
- **Cele z odejmowaniem** (0.95.0): cel może być liczbą ujemną, w każdym polu celu (Szkic, sesja i dokument). To cel dla skracania: „-2,000” dla sesji oznacza, że dziś trzeba wyciąć 2000 słów, „-4,000” dla dokumentu oznacza, że trzeba go zmniejszyć do 4000 słów. Jego pasek wypełnia się w miarę spadku liczby słów (pasek sesji według usuniętych słów, pasek dokumentu lub Szkicu aż liczba spadnie do podanej wartości), a liczba ujemna jest pokazywana na czerwono. Przy ujemnym celu sesji dzisiejsza liczba jest netto: liczy usunięte słowa jako liczbę poniżej 0, na czerwono; przy celu dodatnim pozostaje jak dawniej i nigdy nie spada poniżej 0. Scrivener nie przyjmuje ujemnych celów: ujemny cel jest zachowywany tylko we własnej kopii {{APP}} projektu w formacie Scrivenera; plik *.scriv* otrzymuje dla niego 0 („brak celu”).
- **Projekt > Statystyki projektu**: słowa i znaki w Szkicu, w skompilowanym tekście i w całym projekcie, a także szacunkowa liczba stron. W dużym projekcie liczenie chwilę trwa; pokazuje to kursor zajętości.
- **Projekt > Historia pisania**: ile słów napisałeś(-aś) każdego dnia.

Poniższe narzędzia pisarskie są dodatkami (sekcja 17): **Generator nazw**, **Brudnopis** i **Statystyki tekstu** (przyszły z aktualizacją 0.11 jako jeden pakiet) oraz **Speech Transcription** (0.61); każdy z nich można wyłączyć lub usunąć w **Pomoc > Dodatki i aktualizacje > Menedżer dodatków**. Bez nich **Edycja > Narzędzia pisarskie** informuje o tym; Menedżer dodatków znajdziesz w menu Pomoc.

### Statystyki tekstu

**Projekt > Statystyki tekstu…** liczy wybraną część projektu. U góry wybierasz, który tekst:

- **Zaznaczony tekst** (wybrany, gdy otwierasz okno przy zaznaczonym tekście),
- **Tekst w edytorze** (w tekście połączonym: wszystkie pokazane dokumenty),
- **Dokumenty zaznaczone w segregatorze**, razem z dokumentami w nich zawartymi,
- **Szkic (uwzględniony w kompilacji)** albo **Cały projekt** (bez Kosza).

Okno ma cztery karty:

- **Przegląd**: słowa (liczone dokładnie tak jak na pasku stopki i w celach), znaki ze spacjami i bez, akapity, zdania, różne słowa, strony po 250 słów, czas czytania przy 230 słowach na minutę oraz średnie (liter na słowo, słów na zdanie i na akapit, najdłuższe zdanie; wskaż je, aby je przeczytać).
- **Czytelność (szacunki)**, także na pierwszej karcie. Dla angielskiego: *Flesch Reading Ease* (od 0 do 100, im wyżej, tym łatwiej; 60 do 70 to zwykły, prosty angielski) i *Flesch-Kincaid Grade Level* (klasa w szkole amerykańskiej potrzebna, by zrozumieć tekst). Dla niemieckiego: *indeks Flescha według Amstada* (ta sama skala) i *Wiener Sachtextformel* (mniej więcej klasa szkolna, od 4 łatwy do 15 bardzo trudny). Dla każdego języka: *LIX* (słów na zdanie plus udział słów dłuższych niż sześć liter; poniżej 30 bardzo łatwy, powyżej 60 bardzo trudny). Język jest rozpoznawany automatycznie albo wybierany u góry. Te wyniki są wskazówkami, nie wyrokami: sylaby i zdania są wykrywane według reguł przybliżonych, a wzory powstały dla tekstów rzeczowych, nie dla dialogów czy poezji.
- **Częstość słów**: każde słowo z liczbą wystąpień i udziałem, od najczęstszych. **Ukryj słowa pospolite** pomija rodzajniki, zaimki, przyimki i tym podobne (angielski i niemiecki). Możesz ustawić minimalną długość, wyszukać słowo albo liczyć wielkie litery osobno. Dwukrotne kliknięcie słowa wyszukuje je w projekcie.
- **Powtarzające się frazy**: grupy od dwóch do sześciu słów, których użyłeś(-aś) w zdaniach więcej niż raz, na przykład opis podany dwa razy. Frazy złożone wyłącznie ze słów pospolitych są pomijane. Dwukrotne kliknięcie wyszukuje w projekcie.
- **Dokumenty**: tabela dokumentów ze słowami, znakami, zdaniami, słowami na zdanie i ich udziałem w całości. Kliknij nagłówek kolumny, aby posortować; dwukrotne kliknięcie dokumentu otwiera go.

**Eksportuj tabelę…** zapisuje tabelę bieżącej karty jako plik CSV dla arkusza kalkulacyjnego.

### Generator nazw

**Edycja > Narzędzia pisarskie > Generator nazw…** tworzy imiona i nazwiska postaci.

1. Zaznacz listy **imion** (62 pochodzenia z Europy, obu Ameryk, Azji i Afryki, na przykład angielskie, amerykańskie, irlandzkie i irlandzkie gaelickie, niemieckie, austriackie, szwajcarskie, holenderskie, francuskie, francusko-kanadyjskie, włoskie, hiszpańskie, meksykańskie, portugalskie, brazylijskie, duńskie, szwedzkie, norweskie, fińskie, islandzkie, polskie, czeskie, węgierskie, rosyjskie, greckie, tureckie, hebrajskie, arabskie, perskie, indyjskie, japońskie, chińskie, koreańskie, joruba, suahili i zulu; list szkockich i walijskich jeszcze nie ma). **Nazwiska** pochodzą z tej samej listy co imię albo odznacz **Lista imienia** i wybierz inne listy (włoskie imię z irlandzkim nazwiskiem).
2. Wybierz **Płeć**, **Liczbę nazw** i to, co **Pokazać** (pełne imię i nazwisko, tylko imię, tylko nazwisko, inicjały i nazwisko). **Z drugim imieniem** dodaje drugie imię.
3. **Dostrajanie**: **pospolite ↔ rzadkie** wybiera spośród najczęstszych nazw (po lewej) albo rzadkich (po prawej); działa to z listami oznaczonymi *wg częstości* (na przykład amerykańska, szwedzka, węgierska, hebrajska), które zawierają informację, jak często dana nazwa występuje. **Aliteracja** nadaje taki odsetek nazw nazwisko zaczynające się na pierwszą literę imienia (Peter Parker); **Podwójne nazwiska** łączą dwa nazwiska dywizem. **Imię zaczyna się na** / **Nazwisko zaczyna się na** ograniczają nazwy do zaczynających się od podanych liter.
4. Kliknij **Generuj nazwy**. Zaznacz nazwy i kliknij **Kopiuj** (wszystkie nazwy, gdy żadna nie jest zaznaczona) albo **Wstaw do tekstu** (także dwukrotnym kliknięciem), aby wstawić je w miejscu kursora w edytorze.

Kobiety dostają żeńską formę nazwiska tam, gdzie język ją ma (polska *Kowalska*, czeska *Nováková*, rosyjska *Иванова*). Nazwy japońskie, chińskie, koreańskie, węgierskie i wietnamskie pokazują najpierw nazwisko rodowe. Niektóre listy są zapisane własnym pismem (rosyjska, grecka, hebrajska, arabska i inne); mówią o tym ich nazwy na liście.

**Dodaj własną listę…** zamienia plik tekstowy z jedną nazwą w wierszu w twoją własną listę (imiona, imiona żeńskie lub męskie albo nazwiska). Wybierz listę i kliknij **Usuń własną listę**, aby ją skasować. Listy nazw pochodzą z projektu open source *Faker* (licencja MIT); są to listy nazw używanych współcześnie, a nie sprawdzone historycznie listy nazw.

### Brudnopis

Brudnopis przechowuje notatki spoza wszystkich projektów: pomysły, gdy otwarty jest inny projekt, cytaty skopiowane ze strony internetowej, obraz fragmentu ekranu. Otwierasz go przez **Okno > Brudnopis** (**Alt+Shift+Enter**).

- **+** zaczyna notatkę (strzałka obok: tekst sformatowany lub zwykły). Tytuł nad tekstem to nazwa notatki. **−** usuwa zaznaczoną notatkę.
- Notatki sformatowane obsługują **pogrubienie**, *kursywę* i podkreślenie (przyciski lub **Ctrl+B / I / U**) oraz obrazy: wklej obraz albo kliknij **aparat**, aby zrobić zrzut fragmentu ekranu. Aparat jest dostępny tylko dla notatek sformatowanych.
- Wszystko jest zapisywane samo podczas pisania. Jeśli notatki nie da się zapisać (pełny dysk, brak folderu notatek), mówi o tym wiersz stanu, a notatka pozostaje otwarta ze swoim tekstem: inna notatka, nowa notatka lub inny folder notatek otworzą się dopiero po jej zapisaniu, a okno oferuje **Kopiuj tekst** i **Spróbuj ponownie**. Jeśli program zakończy działanie, gdy notatki nie da się zapisać, jej tekst trafia do folderu awaryjnego *~/.local/share/{{APP}}/Emergency* (0.65).
- **Wyślij do projektu** wymienia każdy otwarty projekt, także te otwarte w innych oknach {{APP}}. Dla każdego: **Nowy dokument w folderze Materiałów**, **Nowy dokument w** wybranym folderze albo **Dodaj na końcu** dokumentu tekstowego. Formatowanie i obrazy są przenoszone. Notatka pozostaje w brudnopisie, dopóki jej nie usuniesz.

Notatki to zwykłe pliki w folderze, normalnie *Documents/{{APP}} Scratch Pad* (zwykły tekst jako .txt, notatki sformatowane jako .html, z obrazami w ukrytym podfolderze *.images*). Inny folder, rodzaj nowych notatek i to, czy brudnopis pozostaje nad innymi oknami, wybierzesz w **Plik > Opcje > Ogólne > Brudnopis**. Notatki zmienione gdzie indziej (w innym oknie {{APP}}, w synchronizowanym folderze) pojawiają się od razu.

**Z każdego programu.** Skrót w programie działa, gdy klawiaturę ma {{APP}}. Aby otworzyć brudnopis z dowolnego programu, każ pulpitowi uruchamiać {{APP}} z `--scratch-pad`: w KDE przez **Ustawienia systemowe > Klawiatura > Skróty > Dodaj nowy > Polecenie lub skrypt**, wpisz polecenie pokazane w **Plik > Opcje > Klawiatura** (przycisk **Kopiuj polecenie**) i naciśnij wybrane klawisze. Brudnopis otworzy się wtedy sam, bez okna projektu, i wysyła notatki do dowolnego otwartego projektu. Jeśli {{APP}} został dodany do menu aplikacji (**Pomoc > Dodaj do menu aplikacji**), wpis w menu ma także akcję **Brudnopis**.

**Aparat** robi zrzut za pomocą programu do zrzutów ekranu KDE, *Spectacle*, jeśli jest zainstalowany (wymagany w Waylandzie, gdzie programy nie mogą same czytać ekranu): przeciągnij po wybranym fragmencie. Bez Spectacle, w X11, {{APP}} pokazuje ekran, a ty przeciągasz po fragmencie; **Esc** anuluje, **Enter** robi zrzut całego ekranu.

### Transkrypcja mowy

Mów zamiast pisać: dodatek **Speech Transcription** (wersja 0.61 lub nowsza) zapisuje to, co mówisz. Wszystko dzieje się na tym komputerze; podczas mówienia nic nie jest nigdzie wysyłane.

**Wersja 1.10 (z {{APP}} 0.99.0)** wykonuje rozpoznawanie w **silniku AI**, który {{APP}} współdzieli między swoimi funkcjami AI. Dodatek wymaga {{APP}} 0.99.0 lub nowszego (starszy program zachowuje wersję 1.9, która ma własny whisper.cpp); strona opcji zaczyna się tym samym panelem co Czytanie na głos i Pismo odręczne, w którym raz instalujesz silnik, a później modele z listy programu. Pobieranie modeli przez sam dodatek (niżej) nadal działa. Liczba rdzeni i czas, po którym nieużywany model jest zwalniany, nie są tu już ustawiane, lecz dla wszystkich funkcji AI razem w **Plik > Opcje > Ogólne > Modele AI**.

**Przed pierwszym nagraniem: wybierz model.** Rozpoznawanie mowy wymaga modelu, który pobiera się jednorazowo. Otwórz **Plik > Opcje > Mowa** (albo strzałkę obok przycisku mikrofonu, **Opcje transkrypcji mowy…**), wybierz model i naciśnij **Pobierz i użyj**:

- **Small**: pobranie 190 MB, szybki; dobry dla wyraźnej mowy, tłumaczenie tylko zgrubne.
- **Medium** (zalecany): 539 MB; nadąża za mową na większości procesorów z ostatnich lat; dobre rozpoznawanie i tłumaczenie.
- **Large v3**: około 1,1 GB; najlepszy, ale na procesorze często wolniejszy od mowy, więc przy długich fragmentach tekst się spóźnia. Pomaga karta graficzna.
- **Large v3 Turbo**: 574 MB; zbliżony do Large v3 i szybszy, ale nie potrafi tłumaczyć.

Modele pochodzą z plików modeli whisper.cpp na Hugging Face. W polu **Źródło** możesz podać inny adres albo użyć **Użyj pobranego pliku…** dla pliku modelu, który pobrałeś(-aś) samodzielnie. Przed użyciem model jest sprawdzany: najpierw jego suma kontrolna (SHA-256), potem opis modelu wewnątrz pliku i krótka próbna transkrypcja. Dopiero gdy wszystko jest w porządku, zastępuje używany model, który następnie jest usuwany. Model z oficjalnego źródła musi mieć znaną sumę kontrolną; w przeciwnym razie nie jest instalowany (pobranie mogło zostać uszkodzone). Plik z innego adresu albo pobrany przez ciebie, którego suma kontrolna nie jest znana, jest czytany dopiero po twojej zgodzie, ponieważ silnik mowy musi go odczytać. Inne adresy muszą zaczynać się od *https://*. Przerwane pobieranie jest kontynuowane od miejsca, w którym stanęło, przy następnej próbie. **Test szybkości** pokazuje, ile czasu ten komputer potrzebuje na krótkie nagranie z wybranym modelem i urządzeniem.

**Nagrywanie.** Naciśnij przycisk mikrofonu na pasku narzędzi Narzędzia (albo **Edycja > Narzędzia pisarskie > Transkrypcja mowy**). Inspektor otwiera się na karcie **Transkrypcje** (mikrofon, pierwszy w rzędzie przycisków inspektora) i zaczyna się nowa transkrypcja, nazwana według daty i godziny. Mów; krótka pauza kończy fragment, a jego tekst pojawia się kilka sekund później. Dłuższa pauza (3 s, o ile tego nie zmienisz) zaczyna nowy akapit. Naciśnij przycisk ponownie, aby zatrzymać: fragmenty, które jeszcze czekają, zostaną zapisane, po czym tekst można edytować. Strzałka obok przycisku wybiera mikrofon.

**Transkrypcje** to zwykłe pliki tekstowe we własnym folderze, poza wszystkimi projektami (normalnie *Documents/{{APP}} Transcriptions*), więc twoje projekty pozostają bez zmian. Na karcie możesz edytować transkrypcję (zapisuje się sama), zmienić jej nazwę, skopiować cały tekst, aby wkleić go, gdzie potrzebujesz, wstawić go do swojego tekstu poleceniem **Wstaw przy kursorze** (zaznaczona część transkrypcji, albo cała, gdy nic nie jest zaznaczone, trafia do kursora edytowanego tekstu jako jeden krok dla Cofnij), usunąć ją (trafia do kosza pulpitu) albo otworzyć folder. Gdy nagranie jest zapisywane, jego tekst jest tylko do odczytu. Jeśli edytowanej transkrypcji nie da się zapisać, pozostaje otwarta (wiersz stanu mówi dlaczego); gdy w międzyczasie zacznie się nagranie lub program zakończy działanie, jej tekst trafia do folderu awaryjnego *~/.local/share/{{APP}}/Emergency* (0.65).

**Nagraj teraz, tekst zapisz później** (0.64). Zapisywanie tekstu pochłania dużo mocy obliczeniowej; na laptopie na baterii (albo na telefonie) możesz tylko nagrywać, a tekst zapisać później. Wybierz **Tylko nagrywaj** w **Plik > Opcje > Mowa** (*Gdy mikrofon się włącza*; na Androidzie jest to ustawienie standardowe). Przycisk mikrofonu tylko wtedy nagrywa: nic więcej nie działa, żaden model nie jest ładowany. Nagranie jest zachowywane jako plik dźwiękowy (WAV, około 1,9 MB na minutę) obok transkrypcji, z nazwą tego samego rodzaju, i pojawia się na liście karty z przyciskiem **Transkrybuj** w swoim wierszu; **Transkrybuj wszystko** zapisuje tekst każdego nagrania, które jeszcze go nie ma, jedno po drugim. Tekst pojawia się wtedy tak jak przy nagraniu na żywo i można go edytować. Nagranie jest zachowywane po zapisaniu jego tekstu, chyba że zaznaczysz **Usuń nagranie po zapisaniu jego tekstu**; zmiana nazwy lub usunięcie transkrypcji zabiera ze sobą jej nagranie. Aby zapisać tekst na innym komputerze, wybierz folder transkrypcji, który twój program synchronizacji utrzymuje jednakowy na obu. Jeśli nagranie zostanie zatrzymane w trakcie zapisywania jego tekstu (albo program zakończy działanie), zachowuje swój przycisk **Transkrybuj** i można je uruchomić ponownie.

**Języki.** **Język główny** to język tekstu; **język zapasowy** to inny język, którym także mówisz (oba wybiera się spośród jedenastu języków programu: angielski, niemiecki, hiszpański, francuski, portugalski, włoski, rosyjski, indonezyjski, turecki, polski i niderlandzki). Gdy językiem głównym jest angielski, to, co powiesz w języku zapasowym, jest zapisywane po angielsku (whisper tłumaczy tylko na angielski). Słowa języka zapasowego wewnątrz angielskich zdań mogą zostać źle usłyszane (w teście „Bahnhof” zamieniło się w „Bon Huff”); wpisz te, których używasz, w **Słowa podpowiedzi**, a będą zapisywane tak, jak je mówisz („Bahnhof”, „Brötchen”). Przy innym języku głównym albo z modelem Large v3 Turbo mowa pozostaje w języku, w którym została wypowiedziana. **Zachowaj oryginał obok tłumaczenia** dodaje oryginał w nawiasie po przetłumaczonym fragmencie (trwa to dłużej). **Słowa podpowiedzi**: nazwy i słowa, których używasz, aby były pisane po twojemu.

**Inne opcje.** **Obliczenia na**: **Automatyczny** (karta graficzna, której wolna pamięć mieści model, w przeciwnym razie procesor), procesorze albo jednej z kart graficznych znalezionych przez silnik AI (NVIDIA, AMD i Intel przez Vulkan; lista pokazuje je po zainstalowaniu silnika). Pola wersji 1.9 dla wątków procesora i czasu zwalniania modelu zniknęły: to jedno ustawienie programu w **Plik > Opcje > Ogólne > Modele AI**. **Dokładność**: *Staranna* rozważa kilka możliwych tekstów (przeszukiwanie wiązkowe) i najpierw rozpoznaje język każdego fragmentu, spośród twoich dwóch języków: lepsza, ale wolniejsza (wymaga to pakietu silnika AI z {{APP}} 0.99.0 lub nowszego; starszy pakiet silnika ją ignoruje). **Wpisuj do tekstu w trakcie mówienia** (na początku wyłączone): każdy fragment mowy na żywo jest też wstawiany przy kursorze edytowanego tekstu; transkrypcja nadal jest zachowywana na karcie jak dotąd. **Mikrofon**, **Czułość** (niska dla hałaśliwego pokoju, wysoka dla cichego głosu), **Pauza kończąca fragment**, **Nowy akapit po pauzie** (*nigdy* to wyłącza), **folder** oraz w **Zaawansowane**: pomijanie tekstu w ciszy i hałasie (whisper czasem wymyśla tam wiersz w rodzaju „Thank you for watching”) oraz używanie poprzedzającego tekstu jako kontekstu dla następnego fragmentu.

---

### Pomocnik tłumaczenia

Od wersji 0.92.0 dodatek **Pomocnik tłumaczenia** (wersja 1.6; do 1.4 nazywał się Tłumaczenie i korekta) jest jednym z własnych dodatków {{APP}} i wchodzi w skład pełnej wersji; dla już posiadanego {{APP}} jest osobnym pakietem na stronie Releases (sekcja 17). Tłumaczy między dowolnymi dwoma z jedenastu języków i sprawdza angielski tekst pod kątem pisowni i gramatyki, na twoim własnym komputerze: tekst nigdy go nie opuszcza. Sam {{APP}} nie ma funkcji AI; ten dodatek korzysta z modeli językowych i, jak każdy inny, można go wyłączyć lub usunąć w Menedżerze dodatków.

- **To narzędzie daje wyłącznie propozycje. Jego tłumaczenia nie są precyzyjne: sprawdzaj każde z nich, co wymaga dobrej znajomości obu języków.** To samo zdanie stoi w oknie dodatku i w jego opisie.
- Ma własną kartę w inspektorze i własne okno (jego polecenie jest w podmenu Narzędzia pisarskie menu Edycja, a także ma przycisk na pasku narzędzi). Tekst wchodzi, wynik wychodzi z zaznaczonymi wszystkimi zmianami, na żywo podczas pisania; kliknięcie słowa zachowuje oryginał albo proponuje inne słowa.
- Potrzebuje modelu, który wybierasz i pobierasz raz w opcjach dodatku (od kilkuset megabajtów do kilku gigabajtów; model zbyt duży dla komputera nie jest oferowany). Modele są pobierane z wydań *models-1*, *models-2*, *models-3* i *models-4* repozytorium {{APP}}, każdy jest instalowany tylko wtedy, gdy jego suma kontrolna jest znana, a potem dodatek działa bez żadnego połączenia. Jeden model, CoEdIT Large, może być używany wyłącznie do celów niekomercyjnych; dodatek mówi o tym przed pobraniem.
- **Pojedyncze słowo** (1.3). Jedno angielskie słowo: wynikiem jest słowo w poprawnej postaci, a pod nim jego znaczenie (znaczenia pochodzą ze słownika WordNet 3.0, wbudowanego w dodatek; słowo, którego w nim nie ma, dostaje znaczenie napisane przez pobrany przez ciebie model Qwen3, z ostrzeżeniem, że może być niedokładne, a bez takiego modelu dodatek mówi, co możesz pobrać), podpowiedzi pisowni i słowa o podobnym znaczeniu. Jedno niemieckie słowo: dodatek szuka go najpierw w **słowniku niemieckim**, wyciągu z Wikisłownika, który pobierasz osobno w opcjach dodatku (jego licencja, CC BY-SA 4.0, jest pokazywana przed pobraniem): forma podstawowa, części mowy, znaczenia i angielskie słowa każdego znaczenia. Pierwsze pięć znaczeń danej części mowy jest pokazanych, reszta jest pod „więcej”; kliknięcie angielskiego słowa robi z niego wynik, a „Podobne” podąża wtedy za tym słowem i jego częścią mowy. Dla tego, czego słownik nie podaje, i dla słowa, którego nie ma, mały model tłumaczy słowo, a wynik ostrzega, że może być niedokładny. Z angielskiego na niemiecki działa to tak samo dla jednego angielskiego słowa, według części mowy znanych WordNetowi.
- **Zamień i Przetłumacz z powrotem** (1.3). **Zamień** zmienia kierunek z niemieckiego na angielski w kierunek z angielskiego na niemiecki i odwrotnie: wynik przechodzi do tekstu i jest tłumaczony w drugą stronę. **Przetłumacz z powrotem** tłumaczy wynik z powrotem na język twojego tekstu i pokazuje go pod wynikiem, dopóki nie zmienisz tekstu; potrzebuje modelu dla drogi powrotnej (OPUS-MT angielski–niemiecki (mały) dla tekstu niemieckiego) i proponuje jego pobranie, gdy go brakuje. Podczas tłumaczenia drugi rząd elementów sterujących pokazuje model tłumaczenia (wybór innego zmienia ustawienie i tłumaczenie), a lista stylów jest ukryta.
- **Krótkie teksty** (1.3). Pojedyncze słowo i tekst do trzech słów nigdy nie trafiają do dużego modelu tłumaczenia (jest lepszy dla całych zdań i zawodny dla pojedynczych słów): najpierw słownik, w przeciwnym razie mały model, także gdy wybrany jest duży. Dla krótkiego tekstu z kilku słów „Inne tłumaczenia:” wymienia najlepsze pełne tłumaczenia modelu, bez nazw przez niego wymyślonych.
- **Wersja 1.4.** Pojedyncze słowo w obu kierunkach pokazuje teraz pod wynikiem tłumaczenia każdej części mowy, jaką ma (niemieckie słowo, które małą literą jest czasownikiem, a wielką rzeczownikiem, dostaje oba; słowo, którego słownik nie ma, jest tłumaczone w obu formach), potem „Podobne tłumaczenia” (słowa o podobnym znaczeniu do angielskiego wyniku) albo, z angielskiego na niemiecki, „Podobne słowa niemieckie”: niemieckie słowa, których znaczenia nazywają angielskie słowo lub jedno z jego synonimów (wymaga to słownika niemieckiego; bez niego jeden wiersz o tym mówi), a na końcu krótki opis. Przy ustawieniu „Automatycznie” pojedyncze niemieckie słowo, które nie jest słowem angielskim, jest rozpoznawane jako niemieckie, gdy zainstalowany jest słownik niemiecki. Przed tłumaczeniem niemieckiego wielkie litery niemieckich rzeczowników są poprawiane w kopii, która trafia do modelu (twój tekst pozostaje bez zmian): słowo, które słownik zna tylko jako rzeczownik, dostaje wielką literę, słowo znane w obu postaciach („essen”) zachowuje formę, którą wpisałeś(-aś), z wyjątkiem miejsca tuż po rodzajniku lub przyimku z rodzajnikiem („das Essen”, „im Essen”); bez słownika notatka pod tłumaczeniem mówi, że niemieckie rzeczowniki wymagają wielkich liter. Alternatywy, które różnią się od siebie lub od tłumaczenia tylko wielkimi literami, są pokazywane raz. Obszar słowa nie pokazuje już wiersza „Pisownia” dla słów wyniku (słowa, które klikasz w swoim tekście, zachowują go), a reguły końcówek podobnych słów nie czytają już „number” jako „numb”. Gdy czegoś brakuje (model tłumaczenia, słownik niemiecki), informacja u góry karty i okna dodatku to nazywa i ma przycisk otwierający opcje dodatku. Model nieużywany przez 15 minut jest zwalniany z pamięci (**Plik > Opcje > Ogólne > Modele AI > Zwolnij model po**; „nigdy” zostawia go załadowanym). Podpowiedzi w pustych polach zawijają się przy wąskiej szerokości.
- **Wersja 1.5: jedenaście języków.** Dodatek tłumaczy między dowolnymi dwoma językami spośród angielskiego, niemieckiego, hiszpańskiego, francuskiego, portugalskiego (Brazylia), włoskiego, rosyjskiego, indonezyjskiego, tureckiego, polskiego i niderlandzkiego: przy wybranym **Tłumacz** dwie listy rozwijane nazywają język twojego tekstu i język tłumaczenia. Każdy język ma dwa małe modele OPUS-MT, do angielskiego i z angielskiego (ich licencje, Apache-2.0 lub CC BY 4.0, są pokazywane w opcjach przed pobraniem); para bez angielskiego (na przykład hiszpański na rosyjski) przechodzi przez angielski z użyciem obu modeli, a notatka pod wynikiem o tym mówi, ponieważ błąd pierwszego kroku przechodzi do drugiego. Każdy język poza angielskim ma też opcjonalny **słownik** z Wikisłownika (CC BY-SA 4.0, osobne pobranie, którego licencja jest pokazywana najpierw), używany dla pojedynczego słowa tego języka tak jak słownik niemiecki. Język bez użytecznego słownika dostaje tylko tłumaczenia modelu. Pojedyncze słowo jest nadal poprawiane jako propozycja: wiersz pisowni i słowa pod nim mówią, że są to propozycje do sprawdzenia. Własne teksty dodatku są w jedenastu językach programu (w języku wybranym dla {{APP}}; po angielsku tam, gdzie tekst nie jest przetłumaczony). Ustawienia z 1.4 nadal działają: słownik niemiecki i pobrane modele pozostają, a pierwszym wyborem jest niemiecki na angielski. Jego opcje są teraz w **Plik > Opcje > Pomocnik tłumaczenia**.
- **Wersja 1.6: jedna reguła dla pamięci.** Z {{APP}} 0.99.0 lub nowszym dodatek zgłasza swoje modele na listę modeli programu: zanim model zostanie wczytany, program sprawdza, czy mieści się w pamięci (i może najpierw zwolnić model, który był nieużywany najdłużej, albo odmówić wczytania i podać powód w wierszu komunikatów); model nieużywany przez ustawiony czas jest zwalniany przez program; gdy model jest wczytywany lub zwalniany, wiersz komunikatów mówi „Trwa zmiana modelu”. Liczba rdzeni procesora i czas, po którym model jest zwalniany, nie są już ustawiane w opcjach dodatku: to ustawienie programu dla wszystkich funkcji AI, **Plik > Opcje > Ogólne > Modele AI**, a strona opcji Pomocnika tłumaczenia o tym mówi. Ze starszym {{APP}} dodatek zachowuje własne dwa pola. Biblioteka silnika CTranslate2 ma wersję 4.9.1 (Linux).
- Modele pobrane z wersją 1.1 pozostają bez zmian. Wersja 1.1 pobierała modele z dawnego repozytorium dodatku; 1.2 zna nowe miejsce.
- Własny opis dodatku, z jego regułami korekty (pisownia amerykańska, zakrzywione cudzysłowy, bez pauz) i tym, w czym dobry jest każdy model, to plik *addons/translation-proofreading/source/README.md* w źródle {{APP}}.

## 11. Migawki

Migawka to zapisana kopia dokumentu z określonej chwili. Wykonaj ją przed dużą przeróbką:

- Dokument z co najmniej jedną migawką ma w segregatorze zagięty róg (jasny i szary) w prawym górnym rogu ikony.
- **Ctrl+5** wykonuje migawkę; **Ctrl+Shift+5** pyta o tytuł.
- Przy ustawieniu **Plik > Opcje > Ogólne > Zapisywanie** polecenie **Plik > Zapisz** (**Ctrl+S**) wykonuje migawkę każdego tekstu zmienionego od ostatniego Plik > Zapisz (z tytułem „Zapisano”); **Plik > Opcje > Zachowanie > Migawki** pokazuje krótki komunikat na pasku stanu, gdy migawki są wykonywane.
- Na karcie migawek w inspektorze możesz **porównać** migawkę z bieżącym tekstem (dodatki są pokazane kolorem, usunięcia przekreślone; wybierasz, czy porównywać słowa, zdania, akapity czy znaki) i **cofnąć** się do niej. Przed cofnięciem {{APP}} może wykonać migawkę bieżącego tekstu, aby nic nie zginęło. Migawka jest najpierw odczytywana (0.65): taka, której nie da się odczytać w całości, pozostawia bieżący tekst bez zmian, taka, która nie zawiera tekstu, zastępuje go dopiero po drugim pytaniu, a gdy migawka bieżącego tekstu została zażądana, ale nie można jej wykonać, nic nie jest cofane. Cofnięcie jest też sposobem naprawy dokumentu, którego plik jest uszkodzony lub w innym kodowaniu znaków (jest wtedy pokazany tylko do odczytu): uszkodzony plik jest najpierw zachowany jako kopia w *settings/conflicts*, a inspektor mówi gdzie (0.81.1).
- **Projekt > Usuń stare migawki…** (od 0.61.1, pod Opróżnij kosz) usuwa każdą migawkę projektu, także migawki dokumentów w Koszu, aby znów zmniejszyć projekt. Teksty pozostają takie, jakie są teraz; znikają tylko ich wcześniejsze wersje. Najpierw pada pytanie, które podaje, ile jest migawek, ilu dokumentów dotyczą i ile zajmują miejsca; nie można tego cofnąć, więc najpierw zrób kopię zapasową (**Plik > Kopia zapasowa**). Polecenie jest wyłączone, gdy projekt nie ma migawek. W projekcie w formacie Scrivener migawki opuszczają także *.scriv* (przy zapisie, który następuje zaraz po tym); od 0.61.1 dotyczy to również pojedynczej migawki usuwanej w inspektorze.

---

## 12. Tryb pisania

**F11** wypełnia ekran samym tekstem na kartce papieru. Przesuń mysz do dolnej krawędzi, aby pokazać pasek sterowania: powiększenie, szerokość papieru, wyblaknięcie tła, przewijanie jak w maszynie do pisania, liczba słów i mały inspektor ze streszczeniem i notatkami. **Alt+Shift+Up / Down** przechodzi do poprzedniego lub następnego dokumentu, **Ctrl+[** i **Ctrl+]** cofają i przesuwają w przód po dokumentach, które oglądałeś(-aś). Skróty menu Format i Wstaw (pogrubienie, style, komentarze ...) działają tu również. **Esc** lub **F11** wraca do okna głównego.

**Plik > Opcje > Zachowanie > Tryb pisania** wybiera ekran, na którym się otwiera (bieżący, główny albo wskazany nazwą), czy okno główne schodzi z drogi (jest zminimalizowane, gdy tryb pisania jest otwarty) oraz czy zamyka go **Esc** (**F11** zawsze).

Pasek sterowania jest ciemny w każdym motywie, więc jego przyciski, lista powiększenia i ich teksty pozostają czytelne także w jasnych motywach (0.94.0). Kolory tekstu, papieru i tła pochodzą z motywu. Aby je zmienić, na przykład na czerń na bieli albo zieleń na czerni, przejdź do **Plik > Opcje > Wygląd > Tryb pisania**; zmiana jest widoczna od razu, nawet gdy tryb pisania jest otwarty. Obraz tła można ustawić dla każdego projektu w **Ustawienia projektu > Foldery specjalne**.

---

## 13. Kompilacja: tworzenie manuskryptu

**Plik > Kompiluj…** (**Ctrl+Shift+E**) łączy wszystko ze Szkicu w jeden plik.

### Okno kompilacji

Okno kompilacji ma u góry **Kompiluj dla**, po lewej formaty, pośrodku układy sekcji, a po prawej zawartość, metadane, opcje i zamiany (cztery karty z ikonami; wskaż kartę myszą, aby zobaczyć jej nazwę). Kolumna Formaty jest co najmniej tak szeroka jak jej najdłuższa nazwa; przeciągnij linie między trzema kolumnami, aby któraś była szersza, a okno zapamięta szerokości na następny raz.

1. **Kompiluj dla**: format wyjściowy. Wbudowane, niewymagające innego programu: **Dokument Word (.docx)**, **PDF (.pdf)**, **E-book (.epub)**, a dla scenariuszy **Fountain** i **Final Draft**. PDF i OpenDocument przez LibreOffice, EPUB przez Pandoc oraz Markdown pojawiają się również, jeśli zainstalowano odpowiednią wtyczkę i program (zob. niżej). Obok: ile dokumentów i około ilu słów zostanie skompilowanych.
2. **Formaty** (po lewej), pod **Moje formaty** i **Formaty wbudowane**: format kompilacji decyduje o wyglądzie manuskryptu: czcionki, interlinia, rozmiar strony, marginesy, nagłówki i stopki, wygląd rozdziałów i scen, sposób, w jaki wychodzą style, notatki i cytaty. Formatów wbudowanych (*Manuscript (Times)*, *Manuscript (Courier)*, *Modern*, *Paperback (6 x 9 in)*, *E-book*, *Default (Editor Formatting)* i *Outline (Synopses)*) nie można zmieniać. Przyciski pod listą: **+** (powiel zaznaczony format, aby zrobić własną wersję i ją edytować), ołówek (edytuj własny format), **-** (usuń go), **Importuj...** i **Eksportuj...** (plik *.qbformat* do użycia na innym komputerze lub do przekazania dalej; importowany format, którego nazwa jest zajęta, dostaje numer).
3. **Układy sekcji** (pośrodku): **Czcionka** ustawia jedną czcionkę dla całego manuskryptu albo zostawia to układom („Określona przez układ sekcji”). Tabela wymienia każdy typ sekcji twojego projektu z układem, jaki dostaje w tym formacie; wybierz inny na liście. Pod tabelą mały obraz pokazuje, jak zaczyna się sekcja z wybranym układem. **Przypisz układy sekcji...** otwiera większe okno z typami sekcji po lewej i obrazami wszystkich układów formatu po prawej. Wbudowane formaty znają zwyczajowe nazwy typów sekcji: w *Manuscript (Courier)*, na przykład, *Part* dostaje **Part Title**, *Chapter* dostaje **Chapter Heading**, a *Scene* lub *Section* dostaje **Section Text**.
4. **Zawartość** (pierwsza karta): co jest kompilowane, z typem sekcji każdego dokumentu i jego układem. Wybierz folder, aby skompilować tylko część Szkicu (na przykład jeden rozdział), dodaj część wstępną (stronę tytułową) i końcową (podziękowania). **Imię i nazwisko autora** (0.94.0): strona tytułowa szablonów projektów zawiera „autor:” i imię i nazwisko autora. Pierwsza kompilacja, która pisze stronę tytułową, pyta o to imię i nazwisko (imię i nazwisko; jest zapamiętane razem z Informacjami o autorze w **Plik > Opcje > Ogólne**, więc pytanie pada raz); jeśli zostawisz puste, wiersz „autor:” jest pomijany na stronie tytułowej, w każdym formacie. Pole Autor na karcie Metadane okna kompilacji, jeśli zawiera nazwisko, jest zamiast tego używane dla skompilowanego pliku. Odznacz dokument, aby go pominąć; wyłącza to jego ustawienie **Uwzględnij w kompilacji**. **Dodaj karty obiektów** umieszcza kartę każdego obiektu z karty Obiekty (sekcja 7) po części końcowej: nagłówek dla kategorii (Postacie, Lokalizacje ...), potem każdy obiekt z jego polami; przycisk obok wybiera kategorie. Na początku jest wyłączone.
5. **Metadane** (druga karta): tytuł, podtytuł, autor, język (na przykład *en-US*), temat, słowa kluczowe, wydawca, data, opis, okładka e-booka (obraz z segregatora) i identyfikator książki. Puste pola przyjmują tytuł projektu, twoje imię i nazwisko oraz dzisiejszą datę. Dane trafiają do właściwości dokumentu PDF, e-booka i pliku Word; symbole zastępcze, takie jak `<$projecttitle>`, działają tu także.
6. **Opcje** (trzecia karta): czy tekst folderów jest kompilowany.
7. **Zamiany** (czwarta karta): zamiany tego projektu, wykonywane po zamianach formatu, na przykład imię pisane w szkicach w jeden sposób, a w książce w inny. Każdy wiersz można włączać i wyłączać, a także może być wyrażeniem regularnym, rozróżniać wielkość liter lub dotyczyć tylko całych słów.
8. **Podgląd** pokazuje strony manuskryptu jako PDF we własnym oknie, z wybranym formatem i zawartością, zanim cokolwiek zostanie zapisane.

Czcionka, zamiany i karty obiektów są zapisywane razem z projektem jak inne wybory (starsze wersje {{APP}} je ignorują).

Kliknij **Kompiluj…**, wybierz, gdzie zapisać, a gotowy plik otworzy się w edytorze tekstu lub czytniku, jeśli chcesz. Okno pokazuje postęp („12 z 148”: gotowe dokumenty i ich łączna liczba) z przyciskiem **Anuluj**; **Esc** robi to samo. Anulowanie zatrzymuje się po dokumencie, który właśnie powstaje: żaden plik nie powstaje, plik o tej nazwie sprzed kompilacji pozostaje taki, jaki był (nowy plik jest zapisywany pod tymczasową nazwą obok i przenoszony na miejsce dopiero wtedy, gdy jest kompletny), a konwerter, taki jak LibreOffice lub Pandoc, jest zatrzymywany. Wiersz komunikatów pod segregatorem mówi, że kompilacja została anulowana. **Zapisz ustawienia** zachowuje wybory bez kompilowania.

**Czcionki i formatowanie akapitów ustawione ręcznie.** W formacie *Default (Editor Formatting)* skompilowany plik zachowuje je wszystkie: czcionki, rozmiary, kolory, wersaliki, odstępy między literami, wcięcia, odstępy, interlinię i tabulatory. Każdy inny format ma własny wygląd: zastępuje czcionkę, rozmiar, wcięcia i odstępy, a zachowuje kolor tekstu, wersaliki, odstępy między literami, wyrównanie i tabulatory.

### Edycja formatu kompilacji

**Powiel i edytuj…** oraz **Edytuj…** otwierają edytor formatu. Lista po lewej zawiera jego strony:

- **Tekst**: czcionka, rozmiar, interlinia, wcięcie pierwszego wiersza, odstęp po akapitach, tekst wyjustowany, czcionka i rozmiar nagłówków, pogrubione nagłówki. **Użyj formatowania edytora projektu** zachowuje zamiast tego wygląd edytora.
- **Układy sekcji**: układy formatu (dodawanie, powielanie, usuwanie) z nagłówkiem, przedrostkiem i przyrostkiem (`Chapter <$W:chapter>`), stylem nagłówka i wyrównaniem, wielkimi literami, pustymi wierszami przed nagłówkiem, tekstem, streszczeniem i notatkami, nową stroną, separatorem przed sekcją, brakiem wcięcia w pierwszym akapicie. Podgląd pokazuje, jak zaczyna się sekcja z tym układem.
- **Przypisania**: jaki układ dostaje każdy typ sekcji w tym formacie.
- **Style**: jak styl twojego projektu wychodzi w tym formacie. **Dodaj styl…**, a potem wybierz: **Zachowaj jego wygląd**, **Zmień jego wygląd** (czcionka, rozmiar, pogrubienie, kursywa, podkreślenie, kapitaliki lub wersaliki, kolor, wyrównanie, wcięcia, odstęp przed i po, interlinia, tekst przed nim i po nim, wielkie litery, zaczęcie od nowej strony; pole zaznaczone do połowy, czyli „bez zmian”, zostawia styl takim, jaki jest), **Zrób z niego tekst podstawowy** lub **Pomiń tekst** (na przykład notatki dla siebie we własnym stylu).
- **Strona**: rozmiar papieru i marginesy, marginesy lustrzane dla książek (wewnętrzny i zewnętrzny), nagłówki i stopki (lewa, środkowa, prawa; symbole zastępcze, takie jak `<$p>` dla numeru strony i `<$title>`), strony parzyste i nieparzyste (teksty lewe na stronach lewych, prawe na prawych), nagłówek i stopka na pierwszej stronie i na stronach, na których zaczyna się rozdział, rozdziały na stronie prawej, numer pierwszej strony i jego styl (1, 2, 3 / i, ii, iii / I, II, III), rozmiar nagłówków i stopek.
- **Przypisy i komentarze**: przypisy na dole strony (w e-bookach: notatki wyskakujące), na końcu każdego rozdziału lub na końcu tekstu (PDF i e-booki; plik Word zawsze ma przypisy na dole strony), z tytułem notatek i ich rozmiarem; adnotacje śródtekstowe jako komentarze lub usunięte; połączone komentarze jako komentarze pliku Word; wyróżnienia usunięte.
- **Zamiany**: tekst zastępowany tylko w skompilowanym pliku (twój projekt pozostaje bez zmian): włączone lub wyłączone, wielkość liter, całe słowa, wyrażenia regularne (`\1` do `\9` w „Na”).
- **Przekształcenia**: cudzysłowy proste lub typograficzne, pauzy jako dywizy, kursywa jako podkreślenie (konwencja manuskryptu), kilka spacji jako jedna, brak spacji na końcu akapitów, zwykły tekst bez pogrubienia, kursywy, czcionek i kolorów.
- **Spis treści, PDF i e-book**: spis treści (jego tytuł i liczba poziomów nagłówków), zakładki PDF dla nagłówków, klikalne odnośniki internetowe w PDF, plik dla każdej sekcji, która zaczyna się na nowej stronie w e-booku, i spis treści także jako strona e-booka.

Twoje formaty są zapisywane w *~/.local/share/{{APP}}/CompileFormats* i są dostępne w każdym projekcie.

### PDF

Wbudowane wyjście PDF samo układa strony: przypisy na dole strony, na której są przywołane (długi przechodzi na następną stronę), nagłówki i stopki, rozdziały na stronach prawych, jeśli tak mówi format, nagłówki trzymane razem z tekstem, który po nich następuje, brak pojedynczych wierszy na górze lub dole strony, spis treści z klikalnymi numerami stron, zakładki dla nagłówków w panelu bocznym czytnika oraz klikalne odnośniki. Czcionki są osadzone. Obrazy zachowują rozmiar, jaki mają w tekście; obraz większy niż obszar tekstu jest zmniejszany, aby się zmieścił. Obrazy i tabele obok tekstu mają tekst opływający je, jak w edytorze. Tabela ciągnie się przez tyle stron, ile potrzebuje, wiersz po wierszu, z wierszami nagłówka na górze każdej strony.

### E-booki

Wbudowane wyjście e-book zapisuje EPUB 3 (także ze spisem treści dla starszych czytników): plik na rozdział, nagłówki w spisie treści czytnika, przypisy jako notatki wyskakujące (albo na końcu każdego rozdziału lub książki), obrazy i okładka z **Metadane…**. Tabele zachowują swój wygląd; obraz z podpisem jest rysunkiem (obok tekstu, jeśli tak jest w edytorze), a jego opis jest tekstem alternatywnym obrazu, który czytają czytniki ekranu. Książka dostaje identyfikator przy pierwszej kompilacji; pozostaje on z projektem, więc czytnik uznaje nową wersję za tę samą książkę. Czytelnicy wybierają własne czcionki i rozmiar strony, więc ustawienia strony, nagłówki i stopki nie obowiązują.

### Word

Plik Word używa nazwanych stylów (Title, Subtitle, Heading 1 do 3, Quote, Centered, Caption oraz twoje własne style akapitu i znaku), więc jego nagłówki pojawiają się w okienku nawigacji Worda. Przy opcji **Dołącz spis treści** na początku wstawiane jest pole spisu treści, a plik prosi edytor tekstu o uaktualnienie pól przy otwarciu, co uzupełnia numery stron; jeśli edytor tego nie robi, uaktualnij pole tam samodzielnie. Nagłówki parzyste i nieparzyste, pierwsza strona i numer pierwszej strony pochodzą z ustawień **Strona** formatu. Tabele są tabelami Worda z liniami, kolorami, scalonymi komórkami i wierszami nagłówka (powtarzanymi na każdej stronie w Wordzie). Obraz obok tekstu trafia do jednokomórkowej tabeli bez linii, razem ze swoim podpisem, aby Word i LibreOffice pozwalały tekstowi opływać oba; opis obrazu jest jego tekstem alternatywnym.

### Nagłówki i symbole zastępcze

W układzie sekcji nagłówek może zawierać symbole zastępcze, które Kompilacja zamienia:

| Symbol zastępczy | Wynik |
|-------------|--------|
| `<$n>` | 1, 2, 3 … |
| `<$w>` / `<$W>` / `<$WW>` | jeden, dwa … / Jeden, Dwa … / JEDEN, DWA … |
| `<$R>` / `<$r>` | I, II, III … / i, ii, iii … |
| `<$L>` / `<$l>` | A, B, C … / a, b, c … |
| `<$n:name>` | osobny licznik o nazwie *name* (działa z każdym powyższym stylem numeracji) |
| `<$title>` | tytuł dokumentu |
| `<$projecttitle>` / `<$shorttitle>` | tytuł projektu / jego pierwsze trzy słowa wersalikami |
| `<$author>`, `<$forename>`, `<$surname>` | twoje imię i nazwisko z **Plik > Opcje** lub z ustawień projektu |
| `<$wc>` / `<$wc100>` / `<$wc1000>` | liczba słów, dokładna lub zaokrąglona |
| `<$date>` / `<$year>` | dzisiejsza data (rrrr-mm-dd) / rok |
| `<$p>` | numer strony (tylko nagłówki i stopki) |
| `<$pages>` | liczba stron (nagłówki i stopki plików PDF i wydruków) |

Przykład: nagłówek `Chapter <$W:chapter>` staje się *Chapter One*, *Chapter Two* i tak dalej.

Symbole zastępcze można też wpisywać w sam tekst (**Wstaw > Autonumeracja** i **Wstaw > Symbole zastępcze**). **Pomoc > Lista wszystkich symboli zastępczych** wymienia je wszystkie; zaznacz wiersze i naciśnij **Ctrl+C**, aby je skopiować.

### Style podczas kompilacji

Nagłówki, Title, Subtitle, Block Quote, Centered i Caption dostają swój wygląd z formatu kompilacji. Każdy inny styl niesie ze sobą własne formatowanie (wcięcia, odstępy, wyrównanie, czcionkę, kolor) w każdym formacie kompilacji. W formacie **Default (Editor Formatting)** zmiany, które wprowadzono w stylach wbudowanych, są również przenoszone. Strona **Style** formatu zmienia to dla każdego stylu (zob. *Edycja formatu kompilacji*).

### Notatki podczas kompilacji

Na stronie **Przypisy i komentarze** formatu kompilacji wybierasz, gdzie trafiają przypisy w PDF lub e-booku (dół strony, koniec każdego rozdziału, koniec tekstu), czy adnotacje śródtekstowe stają się komentarzami w pliku Word, czy są usuwane, czy połączone komentarze są zachowywane oraz czy wyróżnienia są usuwane. W pliku Word przypisy są zawsze prawdziwymi przypisami na dole strony.

### Więcej formatów wyjściowych (wtyczki)

{{APP}} jest także dostarczany z tymi formatami. Każdy wymaga programu instalowanego osobno; PDF i EPUB przez LibreOffice i Pandoc są alternatywą dla wbudowanych (wychodzą z pliku Word i wyglądają jak on):

| Format | Wymaga |
|--------|-------|
| PDF (przez LibreOffice) | LibreOffice (zainstalowany w większości systemów Ubuntu i Kubuntu) |
| OpenDocument Text (.odt, przez LibreOffice) | LibreOffice |
| E-book (.epub, przez Pandoc) | Pandoc (zainstaluj „pandoc” w Discover) |
| Markdown (.md) | niczego |

**Plik > Opcje > Dodatkowe > Dodatki i wtyczki** pokazuje, które formaty są gotowe, a którym brakuje programu. Więcej formatów można zainstalować jako dodatki (sekcja 17).

### Kompilacja scenariuszy

Dla scenariuszy powstają dwa wyjścia, które nie wymagają żadnego innego programu:

- **Scenariusz Fountain (.fountain)**: zwykły plik tekstowy, który czytają Highland, Fade In, Beat, Trelby i inne programy dla scenarzystów.
- **Final Draft (.fdx)**: format pliku programu Final Draft, który większość programów dla scenarzystów także potrafi otworzyć.

W obu każdy akapit zachowuje to, czym jest (nagłówek sceny, akcja, postać, didaskalium, dialog, przejście, ujęcie); drugi program układa scenariusz po swojemu. Tytuły folderów i dokumentów, które dodaje format kompilacji, stają się sekcjami (Fountain) lub nagłówkami aktów (Final Draft); tekst, który nie jest elementem scenariusza, staje się akcją. Tytuł projektu i twoje imię i nazwisko (Plik > Opcje) tworzą stronę tytułową. Adnotacje śródtekstowe stają się notatkami w pliku Fountain i są pomijane w pliku Final Draft.

Dla Worda lub PDF wybierz format kompilacji **Default (Editor Formatting)**: zachowuje wcięcia, wersaliki i czcionkę elementów takimi, jakie widzisz w edytorze. Inne formaty kompilacji zastępują czcionkę i rozmiar, ale zachowują wcięcia.

---

## 14. Import i eksport

### Importowanie plików

**Plik > Import > Pliki…** (**Ctrl+Shift+I**) dodaje do segregatora pliki tekstowe (.txt, .md, .rtf, .docx, .odt; stare pliki Word .doc z dodatkiem opisanym niżej), scenariusze (.fountain, .fdx), obrazy, pliki PDF, audio i wideo oraz inne pliki. Możesz też przeciągać pliki z menedżera plików do segregatora. Pliki tekstowe stają się edytowalnymi dokumentami; obrazy i PDF-y są pokazywane w edytorze; audio i wideo odtwarzają się tam dzięki dodatkowi Media Playback (sekcja 6). Plik, którego nie da się skopiować do projektu (pełny dysk, plik nie do odczytania), jest wymieniony w komunikacie i nie jest dodawany, więc nie pozostaje pusta pozycja (0.92.0).

To, jak pliki są odczytywane, ustawia się w **Plik > Opcje > Udostępnianie > Import**: czy komentarze plików Word, LibreOffice i RTF stają się adnotacjami śródtekstowymi (w przeciwnym razie pozostają komentarzami obok tekstu), czy przypisy stają się przypisami śródtekstowymi (standard), czy połączonymi, czy style akapitu i znaku z plików są pomijane (tekst dostaje wygląd twojego projektu; pogrubienie, kursywa, notatki i odnośniki zostają), kodowanie plików zwykłego tekstu (automatyczne: UTF-8, w przeciwnym razie Windows Western) oraz czy pliki *.html* stają się dokumentami tekstowymi zamiast dokumentów stron internetowych.

Dokąd trafiają pliki: zaznacz folder w segregatorze, aby importować do niego (pliki trafiają na koniec), lub dokument, aby umieścić je tuż pod nim. Szkic przyjmuje tylko tekst; inne pliki upuszczone tam trafiają do Badań.

**Importuj i podziel…** czyta jeden długi plik i tnie go na wiele dokumentów, na przykład na każdym wierszu zawierającym tylko `#` albo na każdym podwójnym pustym wierszu. Od 0.94.0 potrafi także ciąć plik .docx, .odt lub .md **na jego nagłówkach**: Nagłówek 1 staje się folderem, Nagłówek 2 folderem lub dokumentem (wybierasz w oknie), tekst między nagłówkami staje się dokumentami, a niższe nagłówki pozostają w tekście. Okno pokazuje segregator, który by powstał, jako drzewo, zanim zaimportujesz. Tytuł dokumentu, którego żaden nagłówek nie nazywa, podlega jednej regule, napisanej w oknie: pierwszy niepusty wiersz jego tekstu (skrócony do 120 znaków, jeśli jest dłuższy) albo „<nazwa pliku> <numer>”, gdy **Użyj pierwszego wiersza każdej sekcji jako jej tytułu** jest wyłączone.

### Stare pliki Word (.doc)

Word 97 do 2003 zapisywał dokumenty jako *.doc* (szablony *.dot*). Stają się dokumentami tekstowymi dzięki dodatkowi **Word 97–2003 Import** (sekcja 17); bez niego plik .doc jest dodawany do segregatora w niezmienionej postaci, a ty dowiadujesz się, który dodatek go czyta. Dodatek nie wymaga żadnego innego programu. Zachowuje:

- tekst, także grecki, cyrylicę, pisma azjatyckie i tym podobne;
- nagłówki (Nagłówek 1 do 6), tytuł, podtytuł, cytaty i podpisy, które dostają style twojego projektu; styl akapitu dokumentu, który twój projekt też ma (ta sama nazwa), dostaje ten styl;
- pogrubienie, kursywę, podkreślenie, przekreślenie, kapitaliki i wersaliki, indeks górny i dolny, wyróżnienie; akapity wyśrodkowane, wyrównane do prawej i wyjustowane;
- listy punktowane i numerowane z ich poziomami (same numery tworzy {{APP}}, więc numeracja nagłówków w rodzaju „Chapter 1” nie jest przejmowana);
- tabele, jako wiersz na każdy wiersz tabeli z komórkami oddzielonymi tabulatorami;
- przypisy dolne i końcowe (jako przypisy dolne), komentarze ze skomentowanymi słowami (imię i nazwisko autora stoi na początku komentarza);
- odnośniki, wynik pól (daty, numery stron takie, jak były ostatnio pokazane), podziały stron i sekcji, łamania wierszy;
- obrazy w tekście (JPEG, PNG, BMP, TIFF);
- w projektach RTF także czcionki, rozmiary, kolory i wcięcia ustawione ręcznie (projekty Markdown zachowują wygląd projektu, jak przy każdym imporcie).

Pomijane są: nagłówki i stopki, pływające obrazy i rysunki, obrazy w starych formatach metaplików (WMF, EMF), obiekty osadzone, ukryty tekst oraz tekst usunięty ze śledzeniem zmian (tekst wstawiony pozostaje). Tekst pól tekstowych jest dodawany na końcu dokumentu. Po imporcie komunikat wymienia, co pominięto.

**Plik > Opcje > Udostępnianie > Konwersja** może ustawić LibreOffice jako pierwszy wybór dla plików *.doc* (czyta też nagłówki, rysunki i pliki Word 6/95); gdy wybrany sposób zawiedzie, próbowany jest drugi. Pliki Word 6.0 i Word 95 oraz pliki, których dodatek nie umie odczytać, są konwertowane przez LibreOffice, jeśli jest zainstalowany (w Ubuntu: `sudo apt install libreoffice-writer`); w przeciwnym razie mówi o tym komunikat. Pliki chronione hasłem trzeba najpierw otworzyć i zapisać bez hasła. Plik *.doc*, który naprawdę jest plikiem RTF, plikiem Word 2007 (.docx) lub stroną internetową, tak jak niektóre programy je zapisywały, jest czytany jako to, czym jest, także bez dodatku.

### Importowanie stron internetowych

**Plik > Import > Strona internetowa…** przenosi stronę z internetu do twojego projektu. Pracę wykonuje dodatek **Web Page Import** (sekcja 17); bez niego polecenie to mówi.

1. Zaznacz folder, który ma otrzymać stronę (w przeciwnym razie trafi do **Badań**).
2. Wybierz **Plik > Import > Strona internetowa…**. Jeśli schowek zawiera adres internetowy, jest on już wpisany; w przeciwnym razie wpisz go lub wklej.
3. Kliknij **Wczytaj** (albo naciśnij Enter). Strona i jej obrazy są pobierane, a okno pokazuje, co zostanie zaimportowane, z liczbą słów i obrazów, autorem, witryną i datą.
4. Popraw **Tytuł**, jeśli chcesz, sprawdź poniższe wybory i kliknij **Importuj**.

**Tylko artykuł** zachowuje tekst, po który przychodzisz, i pomija menu, reklamy, komunikaty o ciasteczkach, przyciski udostępniania, sekcje komentarzy i listy innych artykułów. {{APP}} znajduje artykuł tak, jak robią to narzędzia do czytania (wygrywa tekst z największą liczbą zdań i przecinków i najmniejszą liczbą odnośników; nazwy w rodzaju „article” lub „sidebar” w stronie przemawiają za częścią lub przeciw niej). Jeśli czegoś brakuje, odznacz tę opcję, aby wziąć całą stronę. **Obrazy** pobiera obrazy artykułu (bardzo duże są zmniejszane do szerokości 1600 pikseli). Dla strony z internetu obrazy są pobierane tylko z adresów publicznych: obraz leżący na twoim własnym komputerze lub w sieci prywatnej (nawet przez przekierowanie) jest pomijany, a wynik mówi, ile ich było. Strona, która sama pochodzi z twojego komputera lub sieci, jest pobierana ze wszystkimi obrazami.

**Importuj jako**:

- **Dokument tekstowy** (standard): nagłówki, akapity, cytaty, listy, tabele (wiersz na każdy wiersz), kod programu, podpisy, pogrubienie, kursywa i odnośniki stają się zwykłym dokumentem, który można edytować, opatrywać adnotacjami, dzielić i kompilować.
- **Dokument strony internetowej**: kopia artykułu tylko do odczytu, z obrazami, jako materiał pomocniczy. Nie jest liczony ani kompilowany.

**Oglądanie dokumentu strony internetowej** (0.81.3). Edytor pokazuje stronę jako prosty sformatowany tekst, z jej adresem i dwoma przyciskami nad nim. **Otwórz zapisaną stronę w przeglądarce** pokazuje stronę taką, jaka została zapisana, w twojej przeglądarce internetowej: {{APP}} umieszcza jej kopię w twoim folderze Pobrane (*Quillbinder web pages/<tytuł>*), ponieważ przeglądarka zainstalowana jako pakiet w piaskownicy (na przykład Firefox w Kubuntu) nie może czytać własnych folderów {{APP}}; wiersz pod przyciskami mówi, gdzie jest kopia, a następne kliknięcie ją zastępuje. W Windows kopia jest zamiast tego tworzona w folderze tymczasowym i usuwana po zamknięciu {{APP}}, aby nic nie było zapisywane poza folderem programu. **Otwórz oryginalny adres** otwiera adres strony, więc widzisz stronę taką, jaka jest dzisiaj; przycisk jest wyłączony, gdy strona nie ma adresu. Strony internetowe zapisane przez Scrivener (po jednym pliku *.mht*) są pokazywane tak samo; przeglądarka taka jak Firefox nie potrafi sama otwierać plików *.mht*, więc kopia jest zwykłym HTML z obrazami obok.

W obu przypadkach adres strony staje się **zakładką** dokumentu (inspektor, Zakładki), krótki opis strony staje się jego **streszczeniem**, a **notatki** zapisują adres, witrynę, autora, datę publikacji i kiedy ją pobrałeś(-aś).

**Przeciągnięcie odnośnika** z przeglądarki internetowej na folder w segregatorze, na tablicę korkową lub do konspektu importuje stronę bezpośrednio, z wyborami, których dokonałeś(-aś) ostatnio w oknie.

Warto wiedzieć: strona jest pobierana tak, jak zrobiłaby to przeglądarka, ale bez uruchamiania jej skryptów. Strony, które budują tekst skryptem, często i tak niosą go w swoich danych, i ten tekst jest wtedy używany (bez formatowania i obrazów; mówi o tym komunikat). Strony za logowaniem lub paywallem dają to, co pokazują odwiedzającemu, który nie jest zalogowany. Strony zabezpieczone (https) wymagają w twoim systemie OpenSSL 3 (jest częścią każdego Ubuntu). Adres internetowy prowadzący do pliku PDF nie jest w ten sposób importowany: pobierz plik i użyj **Plik > Import > Pliki…**.

**Scenariusze.** Pliki Fountain (*.fountain*) i Final Draft (*.fdx*) są importowane jako scenariusze: akapity dostają elementy formatu scenariusza twojego projektu, dokument jest w trybie scenariusza, a strona tytułowa pliku trafia do notatek dokumentu. Pogrubiony, kursywny i podkreślony tekst, notatki i wymuszone podziały stron są zachowywane; numery scen, znaczniki rewizji i dwukolumnowy wygląd dialogu równoległego nie są (obie wypowiedzi następują po sobie). **Importuj i podziel…** z opcją **Nagłówki scen** tnie scenariusz na jeden dokument na scenę, nazwany według jej nagłówka.

### Importowanie projektu Scrivener

**Plik > Import > Projekt Scrivener…** czyta projekt Scrivener 3 lub 2 (folder *.scriv*; wybierz znajdujący się w nim plik *.scrivx*) i tworzy z niego nowy projekt {{APP}}. Projekt Scrivener jest tylko czytany, nigdy zmieniany. Od 0.92.0 raport po imporcie wymienia także każdy plik, którego nie udało się skopiować (obraz, PDF, plik audio, obraz fiszki, mapę myśli), i taki plik nie jest liczony jako zaimportowany; wcześniej był pomijany bez słowa.

Import przenosi: strukturę segregatora, teksty, streszczenia, notatki, etykiety, statusy, słowa kluczowe, niestandardowe metadane, typy sekcji, flagi uwzględnienia w kompilacji, migawki, komentarze i przypisy, adnotacje śródtekstowe, pliki badawcze (obrazy, PDF-y i inne), kolekcje, zakładki projektu i cele. Ustawienia kompilacji nie są importowane; ustaw je raz w oknie kompilacji. Wybierz, czy nowy projekt przechowuje swój tekst jako Markdown, czy RTF.

Nagłówki i style: Scrivener oznacza sformatowany tekst w swoich plikach kodami takimi jak `<$Scr_Ps::0><$Scr_H::2>`. Nie są one tekstem; {{APP}} je usuwa. Nagłówek dostaje **Nagłówek 1** do **Nagłówek 6**, a akapit z jednym ze stylów akapitu Scrivenera dostaje styl twojego projektu o tej samej nazwie (**Tytuł**, **Cytat blokowy**; **Centered Text** Scrivenera staje się **Wyśrodkowany**). Od 0.81.3 tekst z jednym ze stylów znaku Scrivenera dostaje styl znaku twojego projektu o tej nazwie, a styl Scrivenera, dla którego twój projekt nie ma stylu o tej nazwie, staje się stylem projektu, z wyglądem, jaki nadaje mu Scrivener. Projekty zaimportowane przez wersje starsze niż 0.14 zachowały kody jako tekst; {{APP}} usuwa je, gdy taki projekt się otwiera, i mówi o tym na pasku stanu. Zakładki projektu nie były importowane przed 0.14; aby je dostać, zaimportuj projekt Scrivener jeszcze raz.

Jeśli coś nie przeniosło się poprawnie, oryginalny projekt pozostaje nietknięty i możesz skopiować tekst ze Scrivenera bezpośrednio.

### Praca w formacie Scrivener

Zamiast robić kopię, {{APP}} może też pracować na samym projekcie Scrivener 3, abyś mógł później kontynuować w Scrivenerze (na przykład na innym komputerze).

- **Otwieranie**: **Plik > Otwórz projekt…** (**Otwórz inny…** w oknie startowym) i wybierz plik *.scrivx* w folderze *.scriv*. Za pierwszym razem {{APP}} pyta: **Pracuj w formacie Scrivener** (twoje zmiany są zapisywane w *.scriv*), **Konwertuj…** (powstaje z niego własny projekt {{APP}}, jak przy imporcie powyżej) albo **Anuluj**. Potem projekt otwiera się bez pytania, także z **Plik > Otwórz ostatnie**.
- **Nowe projekty**: wybierz **Format: projekt Scrivener 3 (.scriv)** w oknie Nowy projekt.
- **Zapisywanie**: każdy zapis (także automatyczny) zapisuje do *.scriv*. Zapisywane jest tylko to, co zmieniłeś(-aś); dokumenty, których nie ruszałeś(-aś), zachowują pliki zapisane przez Scrivener. Tytuł okna zawiera *(format Scrivener)*, a **Plik > Pokaż projekt w menedżerze plików** i kopie zapasowe biorą folder *.scriv*.
- **Duże projekty** (od 0.60.5): automatyczny zapis podczas pisania umieszcza w *.scriv* tylko teksty, notatki i streszczenia, które zmieniłeś(-aś) (oraz segregator, gdy się zmienił), więc pozostaje krótki nawet przy setkach tysięcy słów. Wszystko inne, takie jak indeks wyszukiwania Scrivenera, migawki i obrazy, następuje, gdy przełączysz się do innego programu, użyjesz **Plik > Zapisz** (**Ctrl+S**) albo zamkniesz projekt; zanim więc otworzysz projekt w Scrivenerze, zamknij go tutaj albo najpierw przełącz się do innego programu.
- **Jeden program naraz**: jeśli Scrivener ma projekt otwarty (albo nie został prawidłowo zamknięty), {{APP}} mówi o tym i pyta przed otwarciem. Zamknij projekt w jednym programie, zanim otworzysz go w drugim. Gdy projekt został zmieniony w Scrivenerze (albo przez usługę synchronizacji) od chwili, gdy ostatnio miałeś(-aś) go tu otwartego, {{APP}} czyta go ponownie i mówi na pasku stanu, ile dokumentów się zmieniło; dokument zmieniony w międzyczasie w Scrivenerze nigdy nie jest nadpisywany starszym tekstem.

Co trafia do *.scriv*: segregator, teksty (jako RTF, z obrazami), streszczenia, notatki (także obrazów, PDF-ów, audio i wideo, od 0.81.3), etykiety, statusy, typy sekcji, **Uwzględnij w kompilacji**, zakładki (projektu i każdego dokumentu, także do adresów internetowych i plików), cele projektu z terminem, cele dokumentów, historia pisania, tablica korkowa (układ każdego folderu i miejsca swobodnie rozmieszczonych fiszek), tytuł, autor i język skompilowanego pliku (w ustawieniach kompilacji Scrivenera), migawki, komentarze i przypisy, adnotacje i przypisy śródtekstowe, style akapitu i znaku, słowa kluczowe, niestandardowe pola metadanych i ich wartości, kolekcje, zapisane wyszukiwania (kolekcje wyszukiwania, od 0.90.0), obrazy fiszek (od 0.90.0), obrazy, PDF-y, audio i wideo, strony internetowe, ikony (pod nazwami Scrivenera; twoje własne ikony w jego folderze *Icons*) oraz mapy myśli (jako dodatkowy plik obok plików dokumentu, którego Scrivener nie rusza). Wszystko jest zapisywane w postaci, której używa Scrivener 3 dla Windows:

- Słowa kluczowe, niestandardowe metadane i kolekcje zachowują numery i nazwy Scrivenera (od 0.81.2), więc są takie same w obu programach.
- Style (od 0.81.3): style zmienianego tekstu są w nim oznaczone tak, jak oznacza je Scrivener, więc Scrivener pokazuje te same style. Styl, który stworzyłeś(-aś) w {{APP}}, jest dodawany do stylów projektu Scrivener przy pierwszym użyciu w tekście, wraz ze swoim wyglądem.
- Adnotacje i przypisy śródtekstowe (od 0.81.3) pozostają śródtekstowe, adnotacje w swoim kolorze (wcześniej zmieniony tekst przekazywał je Scrivenerowi jako połączone komentarze i przypisy).
- Strony internetowe (od 0.81.3): strona zapisana w {{APP}} trafia jako własna zapisana strona internetowa Scrivenera (jeden plik *.mht* z obrazami) i zachowuje swój adres. Inne pliki, dla których Scrivener nie ma rodzaju, trafiają jako dokumenty tekstowe wymieniające plik, z plikiem obok; {{APP}} czyta je z powrotem takie, jakie były.
- Audio, wideo i strony internetowe, które wersje starsze niż 0.81.2 zamieniały w takie dokumenty tekstowe, stają się przy następnym zapisie znów własnymi dokumentami audio, wideo lub stron internetowych Scrivenera.
- Teksty (od 0.90.0) są zapisywane z powrotem ze wszystkim, co Scrivener w nich przechowuje: wcięciami prawymi poniżej zera, listami z ich numeracją, podkreśleniem odnośników, obrazami połączonymi z plikiem, czcionkami nazwanymi według kroju (takimi jak *Cambria-BoldItalic*) i miejscem obrazu, którego brakuje danych. Komentarze i przypisy zachowują swoje identyfikatory, to, czy ich pole jest zwinięte, i własne formatowanie, dopóki ich nie zmienisz.
- Tablica korkowa (od 0.90.0): swobodnie rozmieszczone fiszki zachowują swoje miejsca w obu programach. Scrivener zapamiętuje miejsce w szerokościach fiszki; {{APP}} zamienia je na własny rozmiar fiszki, więc układ pozostaje taki sam niezależnie od rozmiaru, w jakim każdy program pokazuje fiszki. Każdy folder zachowuje swój układ (swobodny lub w siatce), jak w Scrivenerze: wybór **Dowolny** lub siatki dla tablicy korkowej folderu jest zapamiętywany dla tego folderu.
- Etykiety i statusy (od 0.90.0): domyślna etykieta i status nowych dokumentów oraz nazwy, które Scrivener nadaje „No Label” i „No Status”, pozostają takie, jakie ma projekt.
- Zapisane wyszukiwania (od 0.90.0): zapisane wyszukiwanie Scrivenera jest w {{APP}} kolekcją wyszukiwania, a kolekcja wyszukiwania, którą tworzysz lub zmieniasz, trafia do *.scriv* jako zapisane wyszukiwanie. Lista ostatnich wyników wyszukiwania Scrivenera pozostaje jego własna. Sprawdzono z zapisanym wyszukiwaniem dowolnego ze słów; jak Scrivener nazywa pozostałe rodzaje (wszystkie słowa, dokładna fraza, wyrażenie regularne, tylko dokumenty uwzględnione lub pominięte w kompilacji), nie zostało jeszcze zobaczone w prawdziwym projekcie, więc zapisane wyszukiwanie tych rodzajów może w Scrivenerze wyjść jako wyszukiwanie wszystkich słów. To, gdzie w dokumentach szuka wyszukiwanie, nie jest częścią tego, co przechodzi: zapisane wyszukiwanie odczytane ze Scrivenera szuka wszędzie.
- Obraz fiszki (od 0.90.0): fiszka pokazująca obraz zamiast streszczenia zachowuje go w obu programach; zdjęcie go lub nałożenie w {{APP}} dociera do Scrivenera.

Co pozostaje tylko po stronie {{APP}} (w kopii roboczej w *~/.local/share/{{APP}}/ScrivenerProjects*): formaty scenariuszy, ustawienia kompilacji (ale nie tytuł, autor i język), obiekty (sekcja 7) i układ okna; to, co *.scriv* już z tego ma, pozostaje w nim bez zmian.

Przy pierwszym otwarciu lub imporcie projektu Scrivener jego segregator pokazuje foldery, które były otwarte w Scrivenerze, i zaznaczone tam dokumenty (0.90.0; {{APP}} nigdy nie zapisuje własnych ustawień okna Scrivenera). Przy pierwszym otwarciu projektu w formacie Scrivener wersją 0.81.2 lub nowszą {{APP}} czyta *.scriv* jeszcze raz i łączy słowa kluczowe, niestandardowe metadane i kolekcje swojej kopii roboczej z tymi projektu, więc nic, co zrobiłeś(-aś) starszą wersją, nie zginie; następny zapis wprowadza je do *.scriv*. Pierwsze otwarcie wersją 0.90.0 lub nowszą robi to samo dla zakładek, celów dokumentów, terminu, historii pisania i tablicy korkowej.

Jeśli zapis do *.scriv* został przerwany (awaria, pełny dysk) po tym, jak nowe dokumenty zostały zapisane w kopii roboczej, następne otwarcie zachowuje te dokumenty i mówi o tym; następny zapis wprowadza je do *.scriv*. Dokumenty usunięte w Scrivenerze pozostają usunięte.

**Plik > Zapisz jako…** (**Ctrl+Shift+S**) zapisuje otwarty projekt pod inną nazwą lub w innym miejscu i w ten sposób projekt zmienia format. W oknie dialogowym wpisz nazwę i wybierz miejsce; **typ pliku** decyduje o formacie: *Projekt Scrivener 3 (\*.scriv)* lub *Projekt {{APP}} (\*.qbx)* (nazwa kończąca się na *.scriv* lub *.qbx* decyduje sama). Projekt jest folderem: program tworzy go z nazwy (*Name.scriv* albo *Name.quillbinder* z *Name.qbx* w środku). Nazwa, która już istnieje, jest odrzucana.

- Od tej chwili okno pracuje na nowym projekcie i każdy późniejszy zapis trafia tam. Tytuł projektu to wpisana przez ciebie nazwa. Zmiany, których jeszcze nie zapisałeś(-aś), trafiają tylko do nowego projektu.
- Oryginał pozostaje na dysku takim, jaki był w tej chwili (zachowaj go jako kopię lub usuń); jest też w **Plik > Otwórz ostatnie**, pod nowym projektem. Jego plik blokady jest usuwany, gdy okno go opuszcza, a nowy projekt dostaje własny.
- Nowy projekt jest projektem samodzielnym, z własnymi **kopiami zapasowymi**: zaczyna nową serię, a dotychczasowe kopie zapasowe pozostają przy oryginale (sekcja 15, Kopie zapasowe). Projekt w formacie Scrivener dostaje w tym celu nowy identyfikator w swoim *.scrivx*.
- Zapisz jako działa tak samo z jednego formatu do tego samego formatu (nowa nazwa lub miejsce) i do drugiego (przełączenie, które do 0.92 robiło menu „Format projektu”).

**Plik > Eksport > Projekt Scrivener…** zapisuje **kopię** w formacie Scrivener, a pozycja pod nią w tym samym menu, **Projekt {{APP}}…**, kopię we własnym formacie {{APP}}; obie zostawiają okno na oryginale: nic nie zmienia się w projekcie, na którym pracujesz (przed 0.93.0 były to „Zapisz kopię jako ...” w menu „Format projektu”).

### Eksportowanie dokumentów

**Plik > Eksport > Pliki…** zapisuje zaznaczone dokumenty dla innych programów. Użyj Kompilacji dla gotowego manuskryptu; użyj Eksportu, aby przekazać komuś pojedyncze dokumenty albo zachować kopię poza {{APP}}.

1. W segregatorze zaznacz dokumenty i foldery do wyeksportowania.
2. Wybierz **Plik > Eksport > Pliki…**.
3. **Eksportuj pliki tekstowe jako**: Rich Text (.rtf), Word (.docx), Markdown (.md), zwykły tekst (.txt), Fountain lub Final Draft (dla scenariuszy Fountain jest wstępnie wybrany). Obrazy, pliki PDF, strony internetowe, audio i wideo są eksportowane w niezmienionej postaci; format dotyczy wtedy tylko ich notatek.
4. **Metadane**: **Eksportuj metadane** zapisuje obok każdego dokumentu plik tekstowy „*nazwa* (metadata).txt” z jego tytułem, rodzajem, etykietą, statusem, słowami kluczowymi, uwzględnieniem w kompilacji, typem sekcji, liczbą słów, celem, datami utworzenia i zmiany, niestandardowymi metadanymi i zakładkami. **Eksportuj streszczenia** dodaje do niego streszczenie (albo zapisuje „*nazwa* (synopsis).txt”, gdy metadane nie są eksportowane).
5. **Opcje**: **Dołącz poddokumenty** (foldery stają się folderami), **Eksportuj notatki** („*nazwa* (notes)” w formacie z kroku 3; zwykły tekst dla scenariuszy), **Eksportuj komentarze i adnotacje śródtekstowe** (odznacz, aby przekazać czysty tekst; skomentowane słowa pozostają, przypisy zawsze pozostają), **Eksportuj migawki** (folder „*nazwa* (snapshots)” z jednym plikiem na migawkę, nazwanym według daty i tytułu).
6. **Lokalizacja**: folder, w którym zapisać. Gdy zaznaczono kilka dokumentów, trafiają do nowego folderu, którego nazwę podajesz (proponowany jest tytuł projektu; jeśli folder istnieje, dodawany jest numer).
7. Kliknij **Eksportuj**. Folder otwiera się w menedżerze plików.

Wybory są zapamiętywane na następny eksport.

Szybki sposób: **przeciągnij dokumenty z segregatora** do menedżera plików lub na pulpit. Dokumenty tekstowe są zapisywane jako pliki RTF (albo Word, Markdown lub zwykły tekst: **Plik > Opcje > Zachowanie**), obrazy i inne pliki w niezmienionej postaci, a folder staje się folderem z dokumentami w środku. Jeśli menedżer plików pyta, czy przenieść, czy skopiować, obie odpowiedzi są dobre: dokumenty pozostają w twoim projekcie.

### Porównywanie i przenoszenie: plik Word lub LibreOffice a projekt

Gdy manuskrypt wraca od redaktora albo kontynuowałeś(-aś) pracę nad nim w Wordzie lub LibreOffice, **Plik > Porównaj i przenieś > Porównaj z plikiem…** znajduje, czym ten plik różni się od twojego projektu, i przejmuje stronę, którą wybierzesz.

1. Wybierz plik: plik Word (*.docx*) lub plik LibreOffice (*.odt*). Stare pliki Word (*.doc*), RTF, zwykły tekst i pliki Markdown też można porównać, ale tylko je odczytać: ich tekst może trafić do projektu; samego pliku nie można zmienić.
2. {{APP}} porównuje plik z tym, **co Kompilacja robi z projektu**, a nie z całym projektem: dokumentami grupy kompilacji (Szkic, chyba że w oknie kompilacji wybrano inną grupę), które mają **Uwzględnij w kompilacji**, w kolejności segregatora. Część wstępna i końcowa, folder Materiały i dokumenty pominięte w Kompilacji się nie liczą. Jeśli w niej znaleziono mniej niż 65 % tekstu pliku, {{APP}} mówi o tym i pyta, czy mimo to porównać (**Porównaj mimo to**); plik może należeć do innego projektu albo Kompilacja pomija to, co plik ma. Słowo uznaje się za znalezione, gdy leży w ciągu czterech słów, które ma także skompilowany tekst, więc kilka zmienionych słów nie sprawia, że plik jest niepowiązany.
3. Okno **Porównaj i przenieś** pokazuje różnice. Wybierz stronę, która jest aktualizowana:
   - **Zaktualizuj projekt z pliku**: zmiany są wprowadzane w twoich dokumentach. Wcześniej każdy zmieniony dokument dostaje migawkę „Przed porównaniem i przeniesieniem (*nazwa pliku*)” (**Dokumenty > Migawki**), więc zawsze możesz wrócić. Tekst, który jest tylko w pliku, staje się nowym dokumentem (nazwanym według jego nagłówka w pliku), umieszczonym po dokumencie poprzedzającym.
   - **Zaktualizuj plik z projektu**: wybrany plik pozostaje bez zmian. {{APP}} zapisuje obok niego kopię „*nazwa* (Quillbinder).docx” (z numerem, gdy nazwa jest zajęta). W pliku Word każda zmiana jest **śledzoną zmianą** autora „Quillbinder”: otwórz kopię w Wordzie lub LibreOffice i zaakceptuj lub odrzuć każdą zmianę tam (w Wordzie na karcie Recenzja; w LibreOffice przez Edycja, Śledzenie zmian, Zarządzaj). Kopia LibreOffice (*.odt*) dostaje zmiany bezpośrednio, bez znaczników.
4. W widoku tekst **zielony** dochodzi, **czerwony** przekreślony tekst odchodzi, a tekst **niebieski** dostaje pogrubienie, kursywę lub podkreślenie drugiej strony. Szare akapity to niezmieniony tekst wokół zmiany, pokazujący, gdzie ona jest. Każde miejsce ma odnośnik **przyjmij** lub **pomiń**; kliknięcie go przełącza.
5. Kliknij **Przenieś ... do projektu** lub **Zapisz ... w kopii pliku**. Komunikat mówi, co zrobiono; dla pliku **Otwórz plik** otwiera kopię.

**Wg rozdziałów** (**Plik > Porównaj i przenieś > Wg rozdziałów**, także pole wyboru w oknie) wymienia dokumenty po lewej, każdy z polem wyboru i tym, co się różni („2 miejsca”, „tylko w pliku”, „nie ma w pliku”). Odznacz dokument, aby zostawić go takim, jaki jest; kliknięcie pokazuje jego zmiany po prawej. Bez Wg rozdziałów wszystkie zmiany są pokazane jedna pod drugą.

**Krok po kroku** (**Plik > Porównaj i przenieś > Krok po kroku**, także w oknie) pokazuje jedną zmianę naraz: **Przyjmij** lub **Pomiń** ją albo wróć **Wstecz**. Na końcu okno mówi, ile zmian zostanie przyjętych; potem kliknij Przenieś. Oba wybory są zapamiętywane na następny raz.

Co jest porównywane: słowa oraz pogrubienie, kursywa i podkreślenie. Każdy akapit pliku jest dopasowywany do dokumentu, który ma jego słowa, więc scena jest znajdowana także tam, gdzie w pliku została przeniesiona (jest wtedy oznaczona „w innym miejscu pliku”; kolejność segregatora nie jest zmieniana). Nagłówki pliku, które nie są tekstem dokumentu, takie jak nagłówki rozdziałów tworzone przez Kompilację, nie są porównywane.

Co jest zaznaczone na początku: każda zmiana wewnątrz dokumentu. Tekst, który jest tylko po jednej stronie, jest zaznaczony, gdy ma co najmniej 100 słów (nowy rozdział pliku staje się nowym dokumentem; scena, której brakuje w pliku, jest dodawana do kopii). Cały dokument nigdy nie trafia do Kosza, a cała część nigdy nie jest usuwana z pliku, chyba że to zaznaczysz.

Ograniczenia: nowe akapity przyjmują wygląd sąsiedniego akapitu. Tekst w tabelach jest porównywany jak inne akapity (sama tabela nigdy nie jest usuwana), a pola liczą się z tekstem, który pokazują. Nagłówki i stopki, pola tekstowe oraz tekst przypisów i komentarzy pliku nie są porównywane i pozostają bez zmian; komentarze i przypisy projektu pozostają w projekcie i nie są zapisywane do pliku. Jeśli plik zmieniono po porównaniu, kopia nie jest zapisywana; porównaj ponownie.

### Drukowanie

**Plik > Drukuj bieżący dokument…** (**Ctrl+P**) drukuje to, co pokazuje bieżący edytor:

- **Tekst** (jeden dokument albo kilka w tekście połączonym): tekst z formatowaniem i obrazami, tytuł każdego dokumentu nad jego tekstem i przypisy na dole strony. Gdy klawiatura jest w podglądzie pomocniczym, drukowany jest dokument podglądu pomocniczego.
- **Tablica korkowa**: fiszki w pokazanej kolejności, tyle na arkuszu, ile się zmieści (z liniami do cięcia), albo jedna fiszka na każdym arkuszu dla papieru na fiszki.
- **Konspekt**: wiersze i kolumny tak, jak pokazuje je konspekt, z tytułami kolumn na górze każdej strony.
- **Obraz** (tak duży, jak pozwala strona), **plik PDF** (jego strony) albo **strona internetowa** (jej tekst).

Okno drukowania wybiera drukarkę, strony i liczbę kopii. **Zaznaczenie** drukuje tylko zaznaczony tekst albo tylko zaznaczone fiszki lub wiersze konspektu. Aby zapisać wydruk jako plik PDF, wybierz jako drukarkę **Drukuj do pliku (PDF)**.

**Plik > Podgląd wydruku** najpierw pokazuje strony; przycisk drukarki w podglądzie je drukuje, **Zamknij** (lub **Esc**) zamyka podgląd.

**Plik > Ustawienia wydruku…** decyduje, jak rzeczy wychodzą na papierze. Okno otwiera się na karcie dla tego, co pokazuje edytor; **Podgląd…** pokazuje wynik, zanim klikniesz **OK**. Ustawienia należą do projektu; zaznacz **Użyj tych ustawień także dla nowych projektów**, aby uczynić je standardem dla nowych projektów.

- **Strona**: rozmiar papieru i orientacja (przycisk **Papier widoku strony** przejmuje papier i marginesy z **Widok > Edycja tekstu > Rozmiar strony i marginesy…**), marginesy oraz nagłówek i stopka, każde z częścią lewą, środkową i prawą. Symbole zastępcze: `<$title>` (to, co jest drukowane), `<$projecttitle>`, `<$author>`, `<$date>`, `<$p>` (numer strony) i `<$pages>` (liczba stron). Odznacz **Nagłówek i stopka także na pierwszej stronie** dla czystej pierwszej strony.
- **Tekst**: co jest drukowane z tekstem (tytuł każdego dokumentu, pod nim streszczenie, notatki dokumentu po tekście, komentarze i adnotacje śródtekstowe jako notatki na dole strony, przypisy na końcu tekstu zamiast na dole każdej strony, wyróżnienia); czcionki (jak w edytorze albo **Jedna czcionka** do wyboru) oraz interlinia (jak w edytorze, pojedyncza, 1,5 wiersza lub podwójna, z miejscem na poprawki). Dokumenty scenariuszy zachowują swój układ scenariusza.
- **Fiszki**: rozmiar fiszki (3 x 5 in, 4 x 6 in, 5 x 8 in, A7, A6 albo własny rozmiar), tyle fiszek na arkuszu, ile się zmieści, albo jedna fiszka na arkusz, co każda fiszka pokazuje (tytuł, streszczenie, pierwsze wiersze tekstu, gdy nie ma streszczenia, obrazy, kolor etykiety jako pasek na górze, status, słowa kluczowe, linie fiszki, linię wokół każdej fiszki) oraz czcionka.
- **Konspekt**: streszczenia (jak pokazuje je konspekt, zawsze albo nigdy), także wiersze wewnątrz zamkniętych grup, tytuły kolumn na górze każdej strony, linia między wierszami, kropka w kolorze etykiety i czcionka. Przy wielu kolumnach **Poziomo** na karcie **Strona** daje im więcej miejsca.

Drukuj bieżący dokument drukuje dokument tak, jak nad nim pracujesz. Dla gotowego manuskryptu z wyglądem formatu kompilacji skompiluj go (sekcja 13) i wydrukuj plik PDF lub Word.

---

## 15. Kopie zapasowe

{{APP}} tworzy kopię zapasową (plik ZIP całego projektu) za każdym razem, gdy zamykasz projekt, i zachowuje ostatnie pięć na projekt. Zmienisz to w **Plik > Opcje > Kopia zapasowa**, gdzie możesz też wybrać folder kopii zapasowych (najlepiej na innym dysku albo w folderze kopiowanym przez twój program synchronizacji).

Od 0.91 kopie zapasowe wszystkich projektów leżą bezpośrednio w folderze kopii zapasowych, każda nazwana według swojego projektu, z datą i godziną. Od 0.92.0 nazwa zawiera także **ID** projektu, pięć znaków po znaku „#” (*My Novel #K7T2M bak 2026-11-20 15-40-00.zip*). ID należy do projektu, nie do komputera: jest takie samo na każdym komputerze, na którym projekt jest otwierany, i zostaje, gdy zmieniasz nazwę projektu albo przenosisz jego folder. Kopie zapasowe projektu to pliki, które noszą jego tytuł i jego ID, i tylko one są dla niego liczone i czyszczone, więc dwa projekty o tym samym tytule zachowują własne kopie zapasowe i własną liczbę zachowanych. Kopia lub nowy projekt, który program tworzy sam, jest projektem samodzielnym z własnym ID: **Plik > Zapisz jako…** i pozycja Projekt {{APP}} w **Plik > Eksport**, projekt przywrócony przez **Przywróć z kopii zapasowej** (dostaje też własny tytuł) oraz projekt utworzony z twojego własnego szablonu. Kopia wykonana poza programem (folder skopiowany ręcznie, ręcznie rozpakowana kopia zapasowa) ma ID swojego oryginału: nadaj takiej kopii własny tytuł. Projekt w formacie Scrivener bierze swoje ID z identyfikatora, który Scrivener przechowuje w *.scrivx*. Jeśli twój folder kopii zapasowych znajduje się w OneDrive organizacji, jej administrator mógł zakazać „#” w nazwach plików; OneDrive dla domu na to pozwala. Kopie zapasowe wykonane wersją starszą niż 0.92.0 nie mają ID w nazwie: czyszczenie nigdy ich nie usuwa, **Przywróć z kopii zapasowej** nadal je wymienia, a usuwasz je sam, gdy ich już nie potrzebujesz. Gdy zmieniasz tytuł projektu, kopie zapasowe wykonane pod starym tytułem pozostają, dopóki ich nie usuniesz. Wersje 0.70 do 0.90 przechowywały kopie zapasowe każdego projektu we własnym folderze wewnątrz folderu kopii zapasowych (*My Novel 1a2b3c4d*); taki folder pozostaje bez zmian, nic w nim nie jest już dodawane ani usuwane, a **Przywróć z kopii zapasowej** nadal wymienia jego kopie zapasowe. Kopia zapasowa działa obok okna: po chwili okno postępu pokazuje, jak daleko jest, i pozwala ją anulować. To, kto ma projekt otwarty (plik blokady z nazwą komputera i loginu), nie wchodzi do kopii zapasowej.

- Przy opcji **Twórz kopię zapasową przy każdym ręcznym zapisie** (**Plik > Opcje > Kopia zapasowa**) każde **Plik > Zapisz** (**Ctrl+S**) tworzy także kopię zapasową. Dwie kopie zapasowe w tej samej sekundzie dostają numer („... (2)”). Gdy tej kopii nie można wykonać, jest niekompletna albo musi czekać poza folderem kopii zapasowych, mówi o tym komunikat obok tekstu (0.92.0; wcześniej tylko pasek stanu, przez kilka sekund). Twój tekst jest w obu przypadkach zapisany. Komunikat pojawia się raz na przyczynę, dopóki nie zamkniesz programu; pasek stanu mówi to przy każdym zapisie.
- **Wiersz komunikatów** pod segregatorem pokazuje krótkie komunikaty po jednym, przez 5 sekund (10 z odnośnikiem) albo do kliknięcia; pozostałe czekają. Korzystają z niego także dodatki (na przykład „Model zwolniony, minimalne zużycie pamięci RAM”, gdy Pomocnik tłumaczenia albo Transkrypcja mowy zwolniły swój model, bo przez pewien czas nie był używany). Gdy model AI jest zwalniany lub wczytywany, także przy pierwszym wczytaniu, wąski wiersz na dole inspektora pokazuje „Trwa zmiana modelu”, dopóki nowy model nie będzie gotowy; gdy model nie jest wczytany, bo nie zmieściłby się w pamięci, ten wiersz podaje powód (zob. **Plik > Opcje > Ogólne > Modele AI**). Gdy jest pusty, nie zajmuje miejsca; gdy inspektor jest ukryty, wiersz pod segregatorem pokazuje te dwa teksty. **Pomoc > Rozwiązywanie problemów i programowanie > Informacje o systemie** podaje pamięć, którą program zajmuje.
- **Dodatki, które nie są zainstalowane.** **Edycja > Narzędzia pisarskie** wymienia „Pomocnik tłumaczenia” i „Speech Transcription”, gdy dodatku brakuje lub jest wyłączony; każdy pokazuje komunikat z przyciskiem otwierającym **Pomoc > Dodatki i aktualizacje > Menedżer dodatków**.
- **Plik > Kopia zapasowa > Utwórz kopię zapasową teraz** tworzy kopię od razu. Od 0.94.0 powodzenie jest zgłaszane w wierszu komunikatów pod segregatorem („Zapisano kopię zapasową.” z odnośnikiem **Pokaż folder**, który otwiera folder kopii zapasowej; podpowiedź podaje plik; komunikat z odnośnikiem pozostaje 10 sekund); żadne okno nie czeka na OK. Okno pojawia się tylko wtedy, gdy coś się nie udało albo gdy kopia zapasowa powstała, ale niektórych plików nie dało się odczytać lub starszej kopii nie dało się usunąć.
- **Kopia zapasowa, której wykonaniu przeszkodziło wyłączenie systemu lub polecenie kill**, jest odnotowywana i wykonywana przy następnym otwarciu (0.81). Od 0.93.0 notatka pozostaje, dopóki ta kopia nie zostanie wykonana (w folderze kopii zapasowych albo oczekująca obok, jak opisano wyżej): otwarcie, którego kopia zapasowa się nie udaje, zostawia notatkę na następne otwarcie. **Starsza kopia zapasowa, której czyszczenie nie może usunąć** (plik zablokowany lub chroniony), pozostaje, a komunikat raz podaje plik i powód; usuń ją ręcznie. Gdy piszesz dalej, ten komunikat jest małą informacją obok tekstu, która nie przejmuje klawiszy.
- **Utwórz kopię zapasową w…** zapisuje kopię zapasową gdziekolwiek wybierzesz, na przykład na pamięci USB.
- **Przywróć z kopii zapasowej…** (0.70) wymienia kopie zapasowe otwartego projektu z ich datą, rozmiarem i informacją, czy są kompletne; wybierz jedną i **Przywróć i otwórz**: zostaje rozpakowana do nowego folderu obok projektu, *… (przywrócono 2026-11-10 14-03)*, jej tytuł mówi „(przywrócono …)” i otwiera się tam. Projekt, który miałeś(-aś) otwarty, nie jest zmieniany. Lista zawiera kopie zapasowe z ID tego projektu, te bez ID (wykonane przed 0.92.0) oraz te innego projektu o tym samym tytule; nazwa pliku na liście pokazuje, która jest która, więc nic nie wymyka się z zasięgu. Od 0.93.0 wymienia także kopie zapasowe, które czekają we własnym folderze roboczym programu (wykonane, gdy folder kopii zapasowych był nieosiągalny; następna kopia zapasowa do folderu kopii zapasowych je tam przenosi): są oznaczone „oczekuje (jeszcze nie w folderze kopii zapasowych)” i przywraca się je w ten sam sposób. Od 0.81 przywracanie działa dla dowolnego rozmiaru (kopie zapasowe od 2 GB, pliki większe niż 1 GB, takie jak filmy, ponad 65 535 plików), każdy plik jest sprawdzany podczas zapisu, przywracanie, które nie mieści się na dysku, nie jest rozpoczynane, okno postępu pozwala je anulować, a gdy plików nie można przywrócić, komunikat wymienia każdy z nich z powodem (pełny dysk, uszkodzony wpis). Od 0.93.0 komunikat ma trzy postaci: **przywrócono** (wszystko jest na miejscu), **przywrócono niekompletnie** (tak zatytułowany, ze znakiem ostrzegawczym, brakujące pliki wymienione z powodem: obejrzyj przywrócony projekt, zanim na nim polegasz) i **nie przywrócono**; a gdy tytułu przywróconego projektu nie udało się oznaczyć „(restored …)”, komunikat o tym mówi.
- **Przenieś stare kopie zapasowe do folderu kopii zapasowych…** (0.93) przenosi kopie zapasowe, które wersje 0.70 do 0.90 trzymały we własnym folderze wewnątrz folderu kopii zapasowych (*My Novel 1a2b3c4d*), do samego folderu kopii zapasowych, pod ich nazwami, aby leżały razem z pozostałymi. Jest włączone tylko wtedy, gdy otwarty projekt ma taki folder z kopiami zapasowymi. Najpierw pyta i podaje, ile ich jest; kopia zapasowa, której nazwa jest już w folderze kopii zapasowych, nigdy nie jest nadpisywana i zostaje tam, gdzie była; raport mówi, co przeniesiono, a co pozostawiono (z powodem), a stary folder jest usuwany, gdy nic w nim nie zostanie. Te kopie zapasowe nie mają ID w nazwie, więc czyszczenie nigdy ich nie liczy ani nie usuwa, jak dotąd; **Przywróć z kopii zapasowej** wymienia je w obu miejscach.
- **Pokaż folder kopii zapasowych** otwiera folder kopii zapasowych.

**Gdzie kopia zapasowa jest składana (od 0.90.3).** Kopia zapasowa jest pakowana i sprawdzana w folderze własnym programu, *~/.local/share/{{APP}}/backup-work* (w Windows *data\backup-work* w folderze programu), a do folderu kopii zapasowych trafia tylko gotowy plik, jednym krokiem i pod ostateczną nazwą: przenoszony tam, gdy oba foldery są na tym samym dysku, w przeciwnym razie kopiowany bezpośrednio pod ostateczną nazwą i porównywany z oryginałem. Folder kopii zapasowych obserwowany przez program synchronizacji, znajdujący się na pamięci USB lub dysku sieciowym, nigdy nie zawiera więc na wpół zapisanej kopii ani pliku tymczasowego, a przerwana kopia zapasowa nigdy nie wygląda na gotową. (Przed 0.90.3 kopia zapasowa była zapisywana wewnątrz folderu kopii zapasowych jako *… .zip.part* i tam przemianowywana.) Folder roboczy jest pusty między kopiami zapasowymi, poza małą notatką o folderze kopii zapasowych, któremu służy.

**Gdy folder kopii zapasowych jest poza zasięgiem** (dysk, który nie jest podłączony, folder, którego nie można utworzyć ani zapisać, brak miejsca), kopia zapasowa jest mimo to wykonywana i czeka w folderze roboczym. Komunikat o tym mówi i podaje oba foldery, a następna kopia zapasowa projektu umieszcza oczekujące kopie na miejscu przed własną. Czeka ich tyle, ile kopii zapasowych jest zachowywanych. **Utwórz kopię zapasową w…** do folderu, którego nie można zapisać, po prostu o tym mówi, podając powód; wybierz inny folder.

**Kopie wykonane przez program synchronizacji.** Tylko pliki nazwane tak, jak {{APP}} nazywa swoje kopie zapasowe (*Title #K7T2M bak 2026-11-20 07-56-13.zip*, z „(2)” lub „(incomplete)”, gdzie to dotyczy), liczą się jako kopie zapasowe i mogą być usuwane przez czyszczenie. Inne pliki w folderze kopii zapasowych projektu są pozostawiane w spokoju i się nie liczą: na przykład kopie, które program synchronizacji robi, gdy ma kłopot z plikiem (*… -MyComputer-safeBackup-0001.zip*, *… (conflicted copy).zip*), albo kopia wykonana ręcznie. (Przed 0.90.3 takie kopie liczyły się jako kopie zapasowe swojej daty i kilka z nich mogło wypchnąć prawdziwe starsze kopie zapasowe.) **Przywróć z kopii zapasowej…** nadal je wymienia, więc możesz przywracać z kopii; kopie, których nie potrzebujesz, usuń sam.

Duże projekty (ponad 4 GB, na przykład z audio i wideo) są również prawidłowo archiwizowane (od wersji 0.20). Plik, którego nie można odczytać (na przykład z powodu uprawnień albo plik synchronizowanego folderu, którego nie ma jeszcze na tym komputerze), jest pomijany; kopia zapasowa mimo to powstaje, ale nosi nazwę *… (incomplete).zip*, komunikat wymienia brakujące pliki (także dla automatycznych kopii zapasowych przy otwieraniu lub zamykaniu projektu, 0.65), i z jej powodu żadna starsza kopia zapasowa nie jest usuwana: liczba zachowywanych kopii zapasowych liczy tylko kompletne. O tym, które kopie są najstarsze, mówi data i godzina w ich nazwach, więc zmiana nazwy projektu nie zmienia niczego w tym, które są zachowywane (0.81; wcześniej nowy tytuł sortujący się przed starym sprawiał, że każda nowa kopia zapasowa liczyła się jako najstarsza i była od razu usuwana). Po czyszczeniu program sprawdza, czy nowa kopia zapasowa nadal jest. Z niekompletnych kopii zapasowych najnowsze, do liczby do zachowania, pozostają (0.81). Folder w projekcie, który jest odnośnikiem do folderu gdzie indziej, jest archiwizowany ze swoimi plikami (0.81); odnośnik, który wskazuje donikąd lub z powrotem do projektu, czyni kopię zapasową niekompletną. Jeśli program zostanie zakończony sygnałem (wyłączenie, które nie czeka, polecenie kill), gdy włączone jest **Twórz kopię zapasową przy zamykaniu projektu**, nie ma czasu na kopię zapasową; przy następnym otwarciu projektu kopia zapasowa jest wykonywana, a pasek stanu o tym mówi (0.81). Projekt jest zapisywany przed każdą kopią zapasową; jeśli ten zapis się nie powiedzie (pełny dysk, folder, którego nie można zapisać), kopia zapasowa nie powstaje, a komunikat mówi dlaczego, więc kopii zapasowej nigdy nie brakuje twoich najnowszych zmian bez twojej wiedzy. Kopia zapasowa projektu w formacie Scrivener zawiera *.scriv* i obok niego kopię roboczą {{APP}} (*… - working copy of {{APP}}*), ze słowami kluczowymi, stylami, obiektami i innymi rzeczami, które zna tylko {{APP}}.

**Aby przywrócić kopię zapasową ręcznie** (na przykład na innym komputerze): otwórz folder kopii zapasowych, rozpakuj plik ZIP (prawy przycisk > Wypakuj) i otwórz znajdujący się w nim projekt przez **Plik > Otwórz projekt**.


**Szyfrowanie: blokowanie projektów i kopii zapasowych (0.99.0, Plik > Opcje > Szyfrowanie).** {{APP}} może zablokować pliki projektu i jego kopii zapasowych tak, aby otwierał je tylko twój klucz (OpenPGP, format używany przez Kleopatrę, Gpg4win i GnuPG). Jest wyłączone, dopóki nie utworzysz kluczy. **Utwórz klucze** to krótki kreator: proponuje hasło z sześciu losowych słów (możesz wpisać własne; słabe jest odrzucane), tworzy twój klucz i klucz odzyskiwania oraz zapisuje folder *Quillbinder Keys yyyy-mm-dd* w wybranym miejscu, najlepiej na pendrivie. Folder projektu, folder kopii zapasowych i foldery programów synchronizacji (OneDrive, Dropbox, Nextcloud, Google Drive, iCloud, Syncthing, pCloud, MEGA) są odrzucane. Folder zawiera *my-key-secret.asc* (zablokowany twoim hasłem), *my-key-public.asc*, *recovery-key-secret.asc* (zablokowany własnym hasłem), *recovery-key-public.asc* oraz *README.txt* w języku programu, bez żadnego sekretu. **Hasło odzyskiwania** jest pokazane na ekranie tylko raz: zapisz je na papierze. Nie jest zapisane nigdzie, więc sam pendrive albo sama kartka niczego nie otwierają. Następnie kreator prosi o dwa z sześciu słów, odczytuje zapisane pliki ponownie, otwiera plik testowy każdym kluczem i proponuje drugą kopię folderu w innym miejscu.

Na stronie **Importuj klucze** pobiera folder kluczy z innego urządzenia (wybierasz folder i raz wpisujesz hasło), **Importuj plik klucza** pobiera własny plik klucza (na przykład wyeksportowany z Kleopatry; tylko klucze w wersji 4 na krzywej Curve25519, inne rodzaje są odrzucane z komunikatem), **Eksportuj klucze** zapisuje folder ponownie z tego urządzenia, a **Zapomnij hasło na tym urządzeniu** usuwa zapamiętane hasło (jest przechowywane w magazynie kluczy systemu: KWallet, Secret Service, Menedżer poświadczeń Windows) i blokuje klucz, dopóki nie wpiszesz go ponownie; gdy zaszyfrowany projekt jest otwarty, jego klucz pozostaje odblokowany do czasu zamknięcia projektu, a jeśli dokument jest czytany, gdy klucz jest zablokowany, {{APP}} prosi o hasło. Dwa przełączniki należą do otwartego projektu: **Zaszyfrowane kopie zapasowe** i **Zaszyfrowany projekt**; drugi jest wyszarzony dla projektu w formacie Scrivenera, ponieważ Scrivener nie mógłby go otworzyć.

**Co robią te dwa przełączniki.** Gdy **Zaszyfrowane kopie zapasowe** są włączone, każda kopia zapasowa projektu jest plikiem *.zip.gpg* zablokowanym twoim kluczem i kluczem odzyskiwania; ten komputer potrzebuje do jej utworzenia tylko kluczy publicznych, więc skradziony komputer albo skopiowany folder synchronizacji jej nie otworzy. Działa to także dla projektów w formacie Scrivenera. Kopie zapasowe zrobione wcześniej pozostają takie, jakie były, a zwykłe kopie można nadal przywracać; liczba przechowywanych kopii liczy oba rodzaje. **Przywróć z kopii zapasowej** prosi o twoje hasło, gdy klucz jest zablokowany. Gdy **Zaszyfrowany projekt** jest włączony, każdy plik w folderze projektu jest blokowany osobno (teksty, notatki, plik projektu, migawki, zachowane wersje, obrazy i multimedia, ustawienia), więc program synchronizacji i blokada, która wskazuje komputer, działają jak dotąd; czytelne pozostają tylko blokada i mały plik *encryption.json* (odciski palców obu kluczy, nic tajnego). Włączenie lub wyłączenie zamyka projekt, konwertuje każdy plik (okno pokazuje postęp) i otwiera go ponownie; jeśli komputer zatrzyma się w środku, każdy plik jest albo stary, albo nowy, projekt nadal się otwiera, a następne otwarcie kończy pracę. Kopie zapasowe zaszyfrowanego projektu są zawsze zaszyfrowane. Projekt otwiera się z hasłem zapamiętanym na tym komputerze; w przeciwnym razie {{APP}} pyta o nie raz. Na komputerze bez twojego klucza mówi o tym i proponuje **Importuj klucze**, nie zmieniając niczego w projekcie. Uszkodzony plik jest wymieniony z nazwy, jego dokument pozostaje pusty i nigdy nie jest nadpisywany, a reszta się otwiera. Gdy projekt jest otwarty, obrazy, pliki PDF i multimedia są pokazywane z odblokowanych kopii w prywatnym folderze w pamięci komputera, usuwanym po zamknięciu projektu. Zaszyfrowanego projektu nie można zapisać w formacie Scrivenera (najpierw wyłącz szyfrowanie). Nazwy plików i folderów nie są ukryte, a wersje {{APP}} sprzed 0.99.0 nie mogą otworzyć zaszyfrowanego projektu.

**Otwieranie plików bez {{APP}}.** Zaszyfrowana kopia zapasowa to plik nazwany na przykład *My Novel #K7T2M bak 2026-11-27 09-30-00.zip.gpg*: plik ZIP zablokowany twoim kluczem. W **Kleopatrze** (Linux) lub **Gpg4win** (Windows): Plik > Importuj, wybierz *my-key-secret.asc*; potem Plik > Odszyfruj/Zweryfikuj, wybierz plik *.zip.gpg*, wpisz hasło i otwórz powstały plik ZIP. Poleceniem **gpg**: `gpg --import my-key-secret.asc`, potem `gpg --output backup.zip --decrypt backup.zip.gpg`, potem `unzip backup.zip`. W przypadku zaszyfrowanego projektu kopia zapasowa to prostsza droga wyjścia: jest jednym plikiem, a po odszyfrowaniu zwykłym plikiem ZIP z projektem w środku. Klucz odzyskiwania otwiera te same pliki: zaimportuj *recovery-key-secret.asc* i wpisz hasło odzyskiwania. Na **Androidzie** drogą wyjścia jest komputer: skopiuj tam kopię zapasową i użyj Kleopatry lub gpg (OpenKeychain nie jest już rozwijany).

**Co szyfrowanie chroni, a czego nie.** Chroni pliki na dysku, w kopiach zapasowych i w kopiach: ktoś, kto zdobędzie pliki, skradziony laptop, skopiowany pendrive albo usługa synchronizacji, zobaczy tylko zablokowane pliki. Nie chroni projektu, gdy jest otwarty (tekst jest w pamięci komputera i może być w schowku), a każdy, kto jest zalogowany jako ty, może otwierać projekty, których hasło {{APP}} pamięta. **Jeśli zgubisz oba klucze albo oba hasła, tekst przepadnie: nie ma tylnych drzwi.**

**Zasady, które utrzymują zaszyfrowany projekt zaszyfrowanym (0.99.0).** **Zapisz jako** i **Zapisz kopię** zaszyfrowanego projektu tworzą zaszyfrowany projekt (kopia ma te same dwa klucze). Projekt otwiera się tylko na komputerze, który ma tajny klucz jednego z jego dwóch kluczy **oraz część publiczną obu**: komputer tylko z *my-key-secret.asc* i bez klucza odzyskiwania mówi o tym i proponuje **Importuj klucze** (zaimportuj tam *recovery-key-public.asc*), ponieważ każdy zapis jest szyfrowany dla obu kluczy. Jeśli mały plik *encryption.json* nie może zostać odczytany albo go brak, gdy plik projektu jest zaszyfrowany, projekt nie jest otwierany i nie powstaje jego kopia zapasowa, dopóki plik nie wróci (weź jego wcześniejszą wersję z kopii zapasowej albo z historii programu synchronizacji); w tym czasie nic nie jest zapisywane. Plik zaszyfrowanego projektu z uszkodzonymi pierwszymi bajtami jest uznawany za uszkodzony (jego dokument pozostaje pusty i nigdy nie jest nadpisywany), a plik, który inny komputer zapisał po staremu po wyłączeniu szyfrowania, jest pokazywany odszyfrowany, gdy twój klucz jest odblokowany, a w przeciwnym razie pozostawiany bez zmian. **Zaszyfrowany projekt** nie może być włączony ani wyłączony, gdy inny komputer albo inne okno {{APP}} ma otwarty projekt: najpierw zamknij go tam. Gdy zaszyfrowany projekt jest otwarty, pliki tymczasowe podglądu kompilacji, kompilacji przez inny program, obrazu upuszczonego na kartę i eksportu do przeciągania trafiają do tego samego prywatnego folderu w pamięci komputera co odblokowane obrazy, a nie do systemowego folderu tymczasowego.

**Co zaszyfrowany projekt nadal pokazuje.** Nazwy plików i folderów (są to liczby), ich rozmiary i czasy. Kopie konfliktowe, kopie przerwanego zapisu i kopia awaryjna (**Zapisz teksty gdzie indziej** i to, co jest zapisywane, gdy {{APP}} nie może zapisać) są nazywane numerem dokumentu, a nie jego tytułem; folder kopii awaryjnej jest nazwany jej datą, a jej *README.txt* i notatka następnego uruchomienia nie podają żadnego tytułu ani projektu (folder projektu znajduje się po dacie). Uczenie się pisma odręcznego w programie niczego nie zachowuje, gdy zaszyfrowany projekt jest otwarty. Odszyfrowane kopie dla przeglądarek i tekst w pamięci komputera to część „niechroniona, gdy otwarta” opisana wyżej. **Włączenie szyfrowania nie cofa tego, co zostało już zachowane w postaci zwykłej**: wcześniejsze wersje, które usługa synchronizacji trzyma w swojej historii, starsze zwykłe kopie zapasowe (usuń je sam, gdy chcesz się ich pozbyć) oraz kopie w folderze Pobrane zrobione poleceniem **Otwórz zapisaną stronę w przeglądarce** pozostają, jakie były.

**Sprawdzanie zaszyfrowanej kopii zapasowej.** Po zapisaniu kopii zapasowej {{APP}} odszyfrowuje ją ponownie i porównuje z plikiem ZIP, gdy twój klucz jest odblokowany na tym komputerze. Gdy nie jest (projekt z włączonymi **Zaszyfrowanymi kopiami zapasowymi**, którego ten komputer tylko robi kopie zapasowe i którego hasło nigdy tu nie było pytane), sprawdzenie polega tylko na tym, że plik jest kompletny (jego pakiety kończą się dokładnie na końcu pliku); nie wykryje zmienionego bajtu. W przypadku projektu w formacie Scrivenera **Zaszyfrowane kopie zapasowe** to ustawienie każdego komputera: włącz je na każdym komputerze, który robi jego kopie zapasowe.

---

## 16. Opcje

**Plik > Opcje** (**Ctrl+,**) ma u góry rząd ikon dla sekcji, po lewej listę stron (lub kart), a po prawej opcje. Nic się nie zmienia, dopóki nie klikniesz **OK** lub **Zastosuj**. W lewym dolnym rogu **Zarządzaj** eksportuje używany profil do pliku i importuje inny (**Eksportuj profil...**, **Importuj profil...**, na przykład aby przenieść swoje ustawienia na inny komputer; starszy plik opcji (*.qboptions*) można zaimportować jako profil; zob. „Profile” niżej), otwiera **Zarządzaj profilami...**, zapisuje bieżący wygląd jako plik motywu (istniejący plik jest zastępowany dopiero wtedy, gdy nowy został zapisany w całości; jeśli zapis się nie uda, stary plik pozostaje, jaki był, a komunikat o tym mówi) i wymienia motywy; **Domyślne** przywraca każdej opcji wartość standardową (twoje ostatnie projekty, własne motywy, układy, paski narzędzi i skróty pozostają).

**Gdzie przechowywana jest każda opcja.** Wskaż opcję, aby zobaczyć, gdzie przechowywana jest jej wartość: jej podpowiedź mówi, czy przechowuje ją otwarty projekt, czy program dla wszystkich projektów, i czy to miejsce jest **stałe** (ustawienie, które może przechowywać tylko program, takie jak język), czy **zależy od ustawienia** **Ustawienia dla** u góry (a na stronie Wygląd także **Pokaż i zmieniaj wygląd**). (0.90.1 pokazywało zielone P lub niebieskie A przed każdą opcją; 0.90.2 zostawia to podpowiedzi.) Na małym ekranie strony się przewijają, więc OK, Anuluj i Zastosuj pozostają widoczne.

Od 0.70 okno otwiera się dla **Wszystkich projektów** (ustawień programu); wybierz u góry **Ten projekt**, aby ustawić coś tylko dla otwartego projektu. Własne ustawienia projektu mogą uczynić zapisywanie bezpieczniejszym, ale nigdy mniej bezpiecznym: liczy się krótsze opóźnienie autozapisu, kopia zapasowa, którą wykonuje program, pozostaje włączona, zachowywana jest większa liczba kopii zapasowych, a automatyczne zamykanie należy wyłącznie do programu. Ostatni zapis następuje najpóźniej 10 sekund po zmianie, bez względu na opóźnienie autozapisu.

- **Ogólne**: **Uruchamianie** (ponowne otwarcie ostatniego projektu, okna plików pulpitu, **Język** programu, od następnego uruchomienia: English, Deutsch, Español, Français, Português (Brasil), Italiano, Русский, Bahasa Indonesia, Türkçe, Polski lub Nederlands (dziewięć nowych to tłumaczenia maszynowe, oznaczone tak na liście, dopóki nie sprawdzi ich osoba mówiąca w danym języku; podręcznik, przewodnik szybkiego startu i projekt powitalny podążają za językiem, po angielsku tam, gdzie języka brak; pierwsze uruchomienie przyjmuje język systemu, gdy jest jednym z tych, a strona pierwszego uruchomienia oferuje tę samą listę); **Grupuj cyfry dużych liczb**: liczby słów, cele i statystyki z separatorem tysięcy, tak jak twój system zapisuje liczby (44,825 lub 44.825), albo 44825, gdy odznaczone); **Zapisywanie** (interwał autozapisu, liczba ostatnich projektów, migawki przy każdym Plik > Zapisz); **Modele AI** (jedno miejsce dla każdej funkcji AI: Transkrypcji mowy, Pomocnika tłumaczenia, Czytania na głos i Pisma odręcznego: **Rdzenie procesora dla AI**, domyślnie trzy czwarte rdzeni zaokrąglone w dół, na komputerze z 6 rdzeniami lub mniej liczba rdzeni minus jeden, i co najmniej jeden, z przyciskiem **Domyślnie**; **Zwolnij model po [10] min bez użycia**; oraz wiersz z użyciem pamięci: pamięć RAM komputera, każda karta graficzna zgłoszona przez silnik i wczytane modele z ich rozmiarami. Oba ustawienia należą do tego komputera, a nie do projektu. Zasada: model nieużywany przez ten czas jest zwalniany. Zanim model zostanie wczytany, gdy w użyciu (przez wszystko na komputerze) jest ponad połowa pamięci, do której trafia, najdłużej nieużywany model Quillbindera jest zwalniany jako pierwszy; gdyby nowy model nadal podniósł zajętość tej pamięci powyżej 80 %, nie zostanie wczytany, wiersz komunikatów pod inspektorem podaje powód, a funkcja zamiast niego proponuje następny mniejszy model albo procesor. Gdy sam system zgłasza brak pamięci, nieużywane modele są zwalniane od razu. Praca AI odbywa się z niskim priorytetem, aby komputer pozostał używalny); **Pismo odręczne** (własna strona, zob. *Pismo odręczne* niżej); **Czytanie na głos** (własna strona, zob. *Czytanie na głos* niżej); **Informacje o autorze** (twoje imię i nazwisko, adres, telefon i e-mail na strony tytułowe, używane przez symbole zastępcze `<$author>`, `<$street>`, `<$city>`, `<$state>`, `<$zip>`, `<$country>`, `<$phone>`, `<$email>`, `<$address>`); opcje narzędzi pisarskich (**Brudnopis**); **Wspólne szablony** (dodatkowy folder szablonów projektów, na przykład synchronizowany między twoimi komputerami; jego szablony pojawiają się w Plik > Nowy projekt); **Cytaty** (program uruchamiany przez **Format > Bibliografia/cytaty...**, na przykład Zotero lub JabRef); **Separatory** (co wchodzi między scalone dokumenty i między notatki brudnopisu dodane do dokumentu: pojedynczy Enter, pusty wiersz, podział strony, nic albo własny tekst); **Automatyczne zamykanie** (zapis i zamknięcie po określonej liczbie minut bez klawisza i myszy); **Ostrzeżenia** (**Pokaż wewnętrzną konsolę dziennika** pokazuje komunikaty programu z tej sesji, **Zresetuj wszystkie ostrzeżenia** ponownie zadaje pytania, na które odpowiedziano „Nie pytaj ponownie”).
- **Edycja**: **Opcje** (wczytywanie ostatniego powiększenia edytorów, domyślne powiększenie tekstu każdego obszaru, centymetry lub cale i krok linijki, klawisz Insert dla nadpisywania, własny format daty dla Wstaw > Bieżąca data, co pokazuje licznik na żywo, adnotacje śródtekstowe bez koloru, komentarze i przypisy, które kończą się przed interpunkcją, blokowy punkt wstawiania); **Formatowanie** (formatowanie i style nowych projektów); **Rewizje** (pięć kolorów Format > Tryb rewizji, ze wspólnej palety).
- **Zachowanie**: **Tryb pisania** (ekran, okno główne z drogi, Esc); **Odnośniki do dokumentów** (gdzie otwierają się odnośniki do dokumentów, odnośniki zwrotne); **Dwukrotne kliknięcie** (dwukrotne kliknięcie na tablicy korkowej tworzy fiszkę); **Przeciąganie i upuszczanie** (dokumenty upuszczone na dokumenty tekstowe, format tekstu przeciąganego do innych programów); **Foldery i pliki** (foldery otwierają się jako tablica korkowa; tekst z poddokumentami otwiera się jak folder); **Odtwarzanie** audio i wideo (sekcja 6); **Szybki podgląd** (panele otwierają się ponownie ze swoim projektem); **Klawisz Enter** (Enter kończy edycję streszczenia; Enter tworzy nową fiszkę lub wiersz); **Migawki** (komunikat, gdy wykonywane są migawki).
- **Wygląd**: motyw i jedenaście części okna (Ogólne ustawienia interfejsu, Segregator, Tryb pisania, Tablica korkowa, Fiszki, Inspektor i notatki, Główny edytor, Konspekt, Tekst połączony, Paski postępu celów, Znaczniki tekstowe), każda ze swoimi opcjami, czcionkami i kolorami, oraz **Kolory standardowe** (od 0.95: sześć etykiet, z którymi startują nowe projekty, sześć kolorów palety koloru tekstu i palety wyróżnienia oraz kolory, z którymi startują dwa przyciski; **Domyślne** przywraca standard programu; istniejące projekty zachowują swoje etykiety). Zob. „Motywy, kolory i czcionki” niżej. **Ogólne ustawienia interfejsu** mają także pełną ścieżkę projektu na pasku tytułu i rozmiar ikon pasków narzędzi.
- **Poprawki**: poprawki podczas pisania (sekcja 4) i **Pisownia** (słownik, słowa z cyframi, podkreślenie, osobista lista słów).
- **Klawiatura**: skrót każdego polecenia (sekcja 19).
- **Udostępnianie**: **Import** (komentarze, przypisy, style, kodowanie zwykłego tekstu, pliki HTML; sekcja 14), **Eksport** (format pliku i notatki Plik > Eksport > Pliki), **Konwersja** (jak czytane są pliki *.doc*).
- **Kopia zapasowa**: kiedy tworzyć kopię zapasową (przy otwieraniu, przy zamykaniu, przy każdym Plik > Zapisz), ile kopii zachowywać (nowa instalacja zaczyna od 25 najnowszych; pole zawiera tylko liczbę, słowa stoją wokół niej: „Zachowuj najnowsze [25] kopii zapasowych każdego projektu”), i gdzie.
- **Dodatkowe**, dla własnych dodatków {{APP}}: **Mapa myśli** (opcje tablic, kształt i wygląd nowych notatek, tło tablic bez własnego koloru w jasnych i ciemnych motywach, rozmiar ikon na notatkach i liniach), **Obiekty** (czy nowe projekty dostają twoje własne szablony obiektów, czy standardowe), **Dodatki i wtyczki** (zainstalowane formaty kompilacji i narzędzia pisarskie, foldery wtyczek), **Raporty o awariach** (po ilu sekundach bez odpowiedzi zapisywany jest raport o zawieszeniu, ile raportów jest zachowywanych).
- **Mowa** (z dodatkiem Speech Transcription): model, urządzenie, języki, mikrofon i pauzy (zob. sekcja 10, *Transkrypcja mowy*). Narzędzia pisarskie późniejszych dodatków mogą też wnieść własną sekcję.

Inne okna programu mogą otworzyć opcje na określonej stronie, na przykład stronę Odtwarzanie z odtwarzacza multimediów.

Aktualizacje i dodatki mają własne okna: **Pomoc > Dodatki i aktualizacje > Zainstaluj aktualizację lub dodatek…** i **Pomoc > Dodatki i aktualizacje > Menedżer dodatków…** (sekcja 17).

Ustawienia specyficzne dla projektu (etykiety, status, typy sekcji, wygląd tekstu, niestandardowe metadane, foldery specjalne, tytuł i autor do kompilacji) są w **Projekt > Ustawienia projektu**.

### Ustawienia dla jednego projektu

U góry **Plik > Opcje** pole **Ustawienia dla:** wybiera, gdzie przechowywane jest to, co zmieniasz:

- **Wszystkie projekty (program)**: ustawienia programu, dla każdego projektu. Tu otwiera się okno.
- **Ten projekt**: otwarty projekt zachowuje to, co tu zmieniasz, jako własne ustawienie (w *settings/options.ini* swojego folderu). Obowiązuje, dopóki ten projekt jest otwarty; to, czego projekt nie ustawia, pochodzi z ustawień programu. Tytuł okna mówi „Opcje: ten projekt”, a wiersz pod wyborem mówi, ile własnych ustawień ma projekt (kliknij liczbę, aby je zobaczyć). **Użyj ustawień programu** usuwa je wszystkie; ustawienia programu znów obowiązują dla projektu. Zmiana wykonana poza opcjami, na przykład skrót zmieniony w oknie klawiatury albo ułożony pasek narzędzi, trafia do projektu, gdy projekt ma już dla niej własną wartość, a w przeciwnym razie do programu.

Kilka ustawień należy tylko do programu i nie jest pokazywanych dla projektu: ostatnie projekty, ponowne otwarcie ostatniego projektu, okna plików pulpitu, położenia okien i układy, czasy raportów o awariach i wygląd dla nowych projektów. Wygląd projektu zawsze był jego własny: strona **Wygląd** pokazuje wygląd otwartego projektu albo wygląd dla nowych projektów (**Pokaż i zmieniaj wygląd** u jej góry); przy wybranym wyżej **Ten projekt** pokazuje tylko wygląd projektu. **Domyślne** i **Zarządzanie > Importuj profil…** działają tylko na ustawieniach programu, więc wybierz dla nich **Wszystkie projekty**.

**Które ustawienia mają pierwszeństwo** (**Ogólne > Uruchamianie**, dla wszystkich projektów): gdy zaznaczone (standard), pierwszeństwo mają własne ustawienia projektu, a program uzupełnia resztę. Gdy odznaczone, zawsze obowiązują ustawienia programu: własne ustawienia projektów nie są używane (pozostają w projektach na później), a każdy projekt pokazuje wygląd dla nowych projektów.

**Projekt > Zapisz ustawienia projektu do pliku…** obejmuje własne opcje projektu (sekcję **Opcje tego projektu**), więc można je przenieść do innego projektu.

### Motywy, kolory i czcionki

Motyw ustawia kolory, a jeśli tego chce, także czcionki każdego obszaru okna. Wybierz go w **Widok > Motywy** albo przyciskiem motywu na pasku narzędzi Motywy (włącz pasek w **Widok > Paski narzędzi**); zmiana działa od razu, bez ponownego uruchamiania. Wbudowanych jest dziewięć motywów:

| Motyw | Wygląd |
|-------|------|
| Shelin's Dark Theme | ciemne okno i segregator, ciepły papier z czarnym tekstem, Cinzel dla tytułów tekstu połączonego (od 0.95; stoi na początku listy i można go usunąć w Zarządzaj motywami; potem nie wraca po aktualizacjach) |
| Domyślny (zgodny z pulpitem) | kolory twojego pulpitu (schemat kolorów KDE) |
| Jasny | jasnoszare okno, biały papier |
| Tryb ciemny | ciemnoszary, z ciemnymi fiszkami |
| Sepia | ciepły krem i brąz |
| Łupek | szare okno, biały papier na szarym biurku |
| Północ | ciemnoniebieski z bursztynowym akcentem |
| Ocean | jasny niebieskozielony |
| Wysoki kontrast | biel i żółć na czerni |

Dodatki mogą wnosić więcej motywów (dodatek „Extra themes” ma Northern Night, Forest, Graphite, Warm Paper i Rose Dawn).

**Przycisk motywu** (0.90.2, zmieniony w 0.91 i 0.94). Jeden przycisk, **Motyw**, na pasku narzędzi Motywy, który na początku jest wyłączony (**Widok > Paski narzędzi > Motywy** go pokazuje; przed 0.94 cztery przyciski i lista zajmowały jedną trzecią głównego paska narzędzi). Jego menu wymienia:

- **Jasny** i **Ciemny**: kliknięcie pokazuje od razu ten motyw, taki jaki jest.
- **Mój wygląd 1** i **Mój wygląd 2** przechowują dwa twoje własne wyglądy. Wybranie pustego (w menu jest napisane *empty*) zapisuje w nim wygląd, który teraz widzisz (motyw ze wszystkimi zmienionymi przez ciebie kolorami i czcionkami nałożonymi na niego); wybranie wypełnionego przywraca jego wygląd. **Zapisz bieżący wygląd jako Mój wygląd 1** i **...jako Mój wygląd 2**, na końcu menu, zapisują tam wygląd, który teraz widzisz (pytają przed zastąpieniem wyglądu). Oba wyglądy są zwykłymi motywami: są na liście poniżej i w **Widok > Motywy**, gdzie można je też eksportować lub usuwać (usunięty pozostawia swój wpis pusty).
- **Wszystkie motywy**, poniżej, z zaznaczonym używanym. Wybranie jednego robi to, co **Widok > Motywy**: otwarty projekt i nowe projekty go używają, a pytanie, czy zachować twoje własne zmiany nałożone na wierzch, pada jak dotąd.
- **Twój własny wygląd nigdy nie ginie przez wybór.** Menu pokazuje motyw takim, jaki jest, więc to, co widzisz, naprawdę się zmienia. Jeśli kolory lub czcionki twojego autorstwa leżą na pokazanym motywie, zostałyby przez to utracone, więc są najpierw zapisywane: do pustego Mojego wyglądu bez pytania (pasek stanu mówi gdzie), a gdy oba zawierają wygląd, {{APP}} pyta, czy zapisać w Moim wyglądzie 1, w Moim wyglądzie 2, czy przełączyć bez zapisywania.

Jak każdy element sterujący paska narzędzi, przycisk motywu można przesunąć lub zdjąć w **Widok > Paski narzędzi > Dostosuj paski narzędzi**, a cały pasek narzędzi Motywy można włączać i wyłączać w **Widok > Paski narzędzi**.

**Strona w ciemnych motywach** (0.70). Standardowa strona {{APP}} to jasny oliwkowy papier obok szarego biurka. W ciemnym motywie strona jest zamiast tego bardzo ciemnoszara z białym tekstem (albo własna ciemna strona motywu, jeśli ją ma), więc ciemny motyw nie zostawia jasnej strony w ciemnej ramce. Kolor strony wybrany przez ciebie samego (prawy przycisk > Wygląd albo Plik > Opcje > Wygląd) pozostaje w każdym motywie.

**Zmiana kolorów i czcionek.** **Plik > Opcje > Wygląd** (albo **Widok > Motywy > Kolory i czcionki…**) wymienia po lewej części okna. Każda pokazuje swoje opcje, czcionki i kolory (kliknięcie koloru pokazuje wspólną paletę: kolory etykiet, sześć kolorów standardowych i **Inny...**):

| Część | Czcionki | Kolory |
|------|-------|--------|
| Ogólne ustawienia interfejsu | menu, przyciski i panele | okno, Tło inspektora (listy i pola tekstowe inspektora), naprzemienne wiersze, tekst, przyciski, zaznaczenie, odnośniki, podpowiedzi, obramowania, wyszarzony tekst |
| Segregator | segregator | tło, tytuły |
| Tryb pisania | (brak) | tło, papier, tekst |
| Tablica korkowa | (brak) | tablica korkowa, stempel statusu |
| Fiszki | tytuły fiszek, tekst fiszek | fiszka, tytuł, tekst, linia pod tytułem, linie w linie |
| Inspektor i notatki | streszczenie | Paski inspektora (rząd kart, nagłówki, wiersz Etykieta i Status), etykiety, karta i tekst streszczenia, notatki i ich tekst |
| Główny edytor | tytuł paska nagłówka | papier, tekst, miejsce obok kolumny tekstu, biurko wokół stron (pokazywane tylko w widoku strony), pasek nagłówka (normalny, aktywny edytor, zablokowany w miejscu) |
| Konspekt | konspekt | tło, naprzemienne wiersze, tekst |
| Tekst połączony | tytuły w tekście połączonym | (brak) |
| Paski postępu celów | (brak) | sesja, szkic, cel osiągnięty |
| Znaczniki tekstowe | (brak) | odnośniki w tekście, trafienia wyszukiwania, podkreślenie pisowni, znaczniki Pokaż znaki niedrukowane |

**Każdy projekt zachowuje własny wygląd.** Motyw segregatora, edytora, tablicy korkowej, konspektu, inspektora i trybu pisania, twoje własne kolory i czcionki dla nich, szerokość kolumny tekstu i obramowanie obok tekstu należą do projektu: zmiana ich w jednym projekcie zostawia inne bez zmian, a aktualizacja {{APP}} nigdy nie zmienia wyglądu istniejącego projektu. Lista **Pokaż i zmieniaj wygląd** u góry **Plik > Opcje > Wygląd** wybiera, czyj wygląd strona pokazuje i zmienia: **Ten projekt** (otwarty; wybrany jako pierwszy) albo **Nowe projekty** (wygląd, z jakim zaczyna się nowy projekt). Przełączanie między nimi zachowuje to, co zmieniłeś(-aś) w każdym, dopóki nie naciśniesz **Zastosuj** lub **OK**. Kolory menu i paneli (**Ogólne ustawienia interfejsu**) oraz czcionka menu, przycisków i paneli są takie same w każdym projekcie. Projekt otwarty po raz pierwszy wersją 0.40.2 lub nowszą zachowuje wygląd, jaki miał do tej pory. **Widok > Motywy** ustawia motyw otwartego projektu i nowych projektów naraz.

**Menu prawego przycisku.** Gdziekolwiek klikniesz prawym przyciskiem, menu ma podmenu **Wygląd** z kolorami i czcionkami tego, co jest pod myszą: na tekście i na papierze obok niego papier, tekst, miejsce obok kolumny, odnośniki, trafienia wyszukiwania, podkreślenie pisowni, szerokość kolumny i obramowanie; na pasku nagłówka nad edytorem jego kolory i czcionka; w segregatorze jego tło, tytuły, czcionka, wcięcie i kolory etykiet; na tablicy korkowej korek, fiszki i ich czcionki, stemple statusu, szerokość fiszki; w konspekcie jego tło, wiersze, tekst i czcionka; w inspektorze i jego notatkach paski, etykiety, karta streszczenia i notatki oraz Tło inspektora, które jest takie samo w każdym projekcie; w trybie pisania jego tło, papier i tekst. Kolor pokazuje wspólną paletę i **Kolor motywu**; **Opcje wyglądu...** otwiera tę stronę opcji. Podobnie jak opcje, te menu zmieniają wygląd otwartego projektu (mówi o tym pierwszy wiersz podmenu), a bez projektu wygląd nowych projektów.

Kliknij kolor, aby go zmienić, przycisk czcionki, aby wybrać czcionkę. Mały obraz u góry pokazuje wynik, zanim naciśniesz **Zastosuj** lub **OK**. Twoje zmiany leżą na wierzchu motywu: zmieniona pozycja jest pokazana pogrubieniem, przycisk **↺** obok niej oddaje ją motywowi, a **Przywróć kolory i czcionki motywu** robi to dla wszystkich. Kolor, którego nie zmieniłeś(-aś) i którego motyw nie ustawia, idzie za innymi; na przykład segregator używa koloru list, dopóki nie dasz mu własnego. **Tło inspektora** koloruje tylko listy i pola tekstowe inspektora; pola na paskach narzędzi, linijka i listy opcji mają kolor okna i menu.

Czcionka i rozmiar tekstu, który piszesz, nie są częścią motywu: należą do projektu (**Projekt > Ustawienia projektu > Formatowanie**, sekcja 4).

**Własne motywy.** **Zapisz jako motyw…** w opcjach (albo **Widok > Motywy > Zapisz bieżący wygląd jako motyw…**) zapisuje motyw razem z twoimi zmianami pod nową nazwą. Pojawia się on potem w **Widok > Motywy** jak inne. **Widok > Motywy > Zarządzaj motywami…** wymienia wszystkie motywy: **Użyj tego motywu**, **Powiel…** (także dla motywów wbudowanych, aby dostać własny punkt wyjścia), **Zmień nazwę…** i **Usuń** (tylko własne motywy), **Eksportuj…** (zapisuje plik *.qbtheme*, który możesz przekazać dalej) i **Importuj…** (dodaje taki plik).

Gdy wybierzesz inny motyw, mając własne zmiany, {{APP}} pyta, czy zachować je z nowym motywem, czy użyć motywu takim, jaki jest.

Odnośniki w twoim tekście są zapisywane jednym niebieskim; na ciemnym papierze są pokazywane jaśniejszym niebieskim (**Główny edytor > Odnośniki w tekście**). Tekst, który sam pokolorowałeś(-aś), zachowuje kolor w każdym motywie.

**Kontrast odnośników.** Każdy wbudowany motyw zachowuje kolory odnośników (**Odnośniki w tekście**, **Odnośniki w interfejsie**) przy kontraście co najmniej 4,5 : 1 (WCAG 2.x) na każdym papierze i tle, na którym je pokazuje (papier edytora, fiszki, streszczenie, notatki, tryb pisania oraz listy i panele interfejsu); kolor odnośnika, który spadłby poniżej, jest nieco przesuwany w stronę ciemniejszego lub jaśniejszego, a każdy obszar tekstu dostaje kolor odnośnika pasujący do własnego papieru (ciemny papier notatek obok jasnego papieru edytora ma własny, jaśniejszy odnośnik), także na standardowym oliwkowym papierze (przed 0.99.0 miał 4,05 : 1). Kolor wybrany przez ciebie nigdy nie jest zmieniany: gdy twoje własne kolory spadają poniżej 4,5 : 1 (odnośniki albo tekst na swoim papierze), **Plik > Opcje > Wygląd** pokazuje pod listą zmian notatkę, która wymienia najgorszą parę i jej współczynnik. Notatka niczego nie blokuje.

### Profile

Motyw to tylko wygląd: kolory i czcionki. **Profil** (od 0.95) zachowuje wszystko inne z ustawień programu: język, wszystkie opcje, paski narzędzi i ich miejsca, rozmiar i położenie okna, pokazane panele, dzielniki, zestaw klawiatury i twoje skróty, kopie zapasowe i folder kopii zapasowych, sprawdzanie aktualizacji, kolory standardowe, zapisane układy z **Widok > Układ**, a także nazywa swój motyw, więc przełączenie profilu przełącza też wygląd. Twoje imię i adres, nazwa tego komputera, ostatnie projekty i miejsca projektu powitalnego nie należą do profilu.

- **Przełączanie.** **Widok > Profile** wymienia profile; kliknięcie przełącza od razu, bez ponownego uruchamiania (tylko język wymaga ponownego uruchomienia; program o tym mówi). Profil, który opuszczasz, jest najpierw zapisywany. Pasek narzędzi **Profile** (**Widok > Paski narzędzi > Profile**, na początku wyłączony) pokazuje używany profil; kliknięcie jego przycisku wymienia profile. Własne ustawienia projektu (**Ustawienia dla: Ten projekt**) nadal obowiązują ponad profilem. Otwarty projekt pokazuje wygląd nowego profilu, tak jak przy wyborze motywu w **Widok > Motywy**.
- **Default.** Program, który był już używany, przy aktualizacji do 0.95 przyjmuje swoje ustawienia jako profil **Default**, więc nic nie ginie. Nowa instalacja zaczyna od standardowych ustawień programu, w jasnym lub ciemnym motywie, zależnie od ustawienia systemu, gdy system to podaje. Strona pierwszego uruchomienia wypełnia **Default**. **Default** nie można usunąć.
- **Nowy profil** z ustawień takich, jakie są teraz: **Widok > Profile > Zapisz ustawienia jako nowy profil…** (albo **Zarządzaj profilami… > Zapisz ustawienia jako nowy profil…**). Nowy profil staje się używanym. **Zarządzaj profilami…** także zmienia nazwy, kopiuje i usuwa profile (używanego profilu nie można usunąć; najpierw przełącz).
- **Shelin's** to profil dołączony do programu. Stoi na początku listy, ustawia Sitka dla nowego tekstu i własny motyw, i można go usunąć; potem nie wraca po aktualizacjach. Czcionki, które nazywa, a które nie są zainstalowane (Cinzel, Sitka), zastępują inne. Jego wybór silnika odtwarzania wymaga dodatku Extra Video and Audio Formats; wybory modeli narzędzi pisarskich obowiązują tylko tam, gdzie te dodatki i modele są zainstalowane.
- **Rozmiary na innym ekranie.** Szerokości segregatora, edytora i inspektora są zachowywane jako udziały okna; profil zrobiony na dużym ekranie pokazuje te same udziały na mniejszym ekranie lub tablecie (kolumny boczne zachowują czytelną szerokość), a okno jest sprowadzane na ekran.
- **Eksport i import.** **Zarządzaj profilami… > Eksportuj…** (albo **Plik > Opcje > Zarządzanie > Eksportuj profil…**) zapisuje profil do jednego pliku (*.qbprofile*) z jego układami i, jeśli jest twój własny, jego motywem. **Importuj…** dodaje taki plik jako nowy profil (nie przełącza się na niego). Plik może nazywać programy, foldery, pliki lub adresy; jak przy innych plikach od kogoś innego, przed ich przyjęciem jesteś pytany (folder, którego nie ma na tym komputerze, na nic by się nie przydał). Profil, który nie został zapisany przez ten program na tym komputerze (plik skopiowany lub zsynchronizowany do folderu profili), pyta w ten sam sposób o swoje programy, foldery, pliki i adresy, gdy się na niego przełączasz. Starszy plik opcji (*.qboptions*) także można zaimportować jako profil.

**Projekt > Zapisz ustawienia projektu do pliku…** pozostaje bez zmian.

---

### Czytanie na głos

**Czytanie na głos** czyta twój tekst głosem, który działa na twoim komputerze. Jest wbudowane w {{APP}} w każdym systemie i wyłączone, dopóki go nie skonfigurujesz: nic nie jest instalowane ani wczytywane, zanim to zrobisz. Dźwięk jest tylko odtwarzany, nigdy nie jest zapisywany do pliku. Czytanie na głos potrzebuje tylko **silnika AI** i modelu głosu (oba instalowane w **Plik > Opcje > Czytanie na głos**); żaden inny dodatek nie jest potrzebny, ponieważ silnik sam odtwarza dźwięk przez system dźwiękowy komputera (ALSA, PulseAudio lub PipeWire w Linuksie, WASAPI w Windows, AAudio lub OpenSL ES na Androidzie).

**Pasek narzędzi.** **Widok > Paski narzędzi > Czytanie na głos** pokazuje lub ukrywa pasek **Czytanie na głos**; w Linuksie i Windows jest pokazany od początku, a na Androidzie ukryty. Ma przycisk **Odtwarzaj/Wstrzymaj** (jeden przycisk), **Zatrzymaj** i mały przycisk **Głos** ze strzałką. Jego przyciski podążają za **Widok > Paski narzędzi > Rozmiar pasków narzędzi**. **Odtwarzaj** czyta zaznaczony tekst; gdy nic nie jest zaznaczone, czyta od kursora do końca tekstu. Czytane zdanie jest podświetlone w edytorze, a edytor przewija się do niego. Pisanie w tekście zatrzymuje czytanie, ponieważ miejsca przestałyby pasować. **Pierwsze kliknięcie Odtwarzaj i każde kliknięcie, gdy nie jest zainstalowany silnik ani model głosu, otwiera Plik > Opcje > Czytanie na głos.**

**Menu Głos** ma trzy grupy z nagłówkami: **Głos** (Kobieta 1 do 3, Mężczyzna 1 do 3), **Styl** (Spokojny, Neutralny, Akcja) i **Prędkość** (Wolna, Średnia, Szybka), a na dole **Ustawienia czytania na głos…**. Podpowiedź przycisku pokazuje wybór („Kobieta 2 · Spokojny · Średnia”). Wybór, którego używany model głosu nie potrafi wykonać, jest wyszarzony, a jego podpowiedź mówi dlaczego: Pocket TTS nie ma sterowania stylem, więc grupa Styl jest wyszarzona; głos, którego model nie ma, jest wyszarzony; głosy Qwen3-TTS są wyszarzone z napisem „Głos jeszcze niezainstalowany.”, dopóki się nie pojawią. Wybory są zapamiętywane dla tego komputera. **Prędkość** rozciąga gotowy dźwięk z zachowaniem wysokości tonu: Średnia to własne tempo modelu, Wolna i Szybka są wolniejsze i szybsze o około 15 procent.

**Głosy Qwen3-TTS.** Qwen3-TTS czyta sześcioma głosami (Kobieta 1 do 3, Mężczyzna 1 do 3), każdym w trzech stylach. Zaprojektowano je dla {{APP}} na podstawie opisów pisanych (na przykład „Kobieta, młoda dorosła, ciepły, wyraźny głos o średniej wysokości”) za pomocą modelu do projektowania głosu; żaden głos nie należy do prawdziwej osoby i nie użyto nagrania nikogo. Każdy język ma własną **paczkę głosów**, niewielkie pobranie, około 20 MB na język (18 krótkich klipów wzorcowych z ich tekstem i notatka o tym, jak powstały). **Zainstalowanie modelu Qwen3-TTS pobiera też paczkę głosów języka Czytania na głos, a późniejsze wybranie innego języka pobiera paczkę tego języka** (gdy model Qwen3-TTS jest zainstalowany); paczka jest sprawdzana z listą programu jak każdy plik modelu. Lista modeli pokazuje rozmiar modelu Qwen3-TTS razem z tą paczką. Dopóki paczka języka nie jest zainstalowana, jego głosy pozostają wyszarzone z napisem „Głos jeszcze niezainstalowany.” Usunięcie ostatniego modelu Qwen3-TTS usuwa także paczki głosów. **Niemiecki i rosyjski** czyta tylko Qwen3-TTS (Pocket TTS nie ma modelu niemieckiego ani rosyjskiego), więc wymagają modelu Qwen3-TTS i swojej paczki głosów.

**Język, od którego zaczyna Czytanie na głos**, to język twojego systemu, gdy da się dla niego mieć model i głos (Pocket TTS z gotowymi głosami albo Qwen3-TTS z paczką głosów na liście); w przeciwnym razie angielski. Inny język wybierzesz w **Plik > Opcje > Czytanie na głos**.

**Plik > Opcje > Czytanie na głos** ma: **Język** (języki programu; Bahasa Indonesia, Türkçe, Polski i Nederlands są wyszarzone, „jeszcze brak modelu dla tego języka”); **Model** (Automatyczny albo jeden model; każdy pokazuje rozmiar pobierania, swoje języki oraz, na tym komputerze, „mieści się na <karta graficzna>”, „mieści się w pamięci RAM” albo, wyszarzone, „nie mieści się (potrzeba: ...)”); **Urządzenie** (Automatycznie: karta graficzna, gdy model mieści się w jej wolnej pamięci, w przeciwnym razie pamięć komputera i procesor; karta graficzna; albo procesor); **test szybkości** (gdy wybierzesz model, krótkie zdanie testowe mierzy, jak szybko mówi na tym urządzeniu; gdy jest wolniejszy niż w czasie rzeczywistym, strona o tym mówi, a Czytanie na głos przygotowuje wtedy kilka zdań naprzód, zanim zacznie odtwarzać); część instalująca silnik i modele (taka sama jak dla innych funkcji AI); oraz odsyłacz do **Ogólne > Modele AI** w sprawie rdzeni i czasu, przez który model pozostaje wczytany. Tryb automatyczny wybiera najlepiej brzmiący model, który jest zainstalowany, mieści się i nadąża za czasem rzeczywistym: Pocket TTS tam, gdzie ma język, w przeciwnym razie Qwen3-TTS 0.6B, a potem 1.7B.

**Pamięć i szybkość.** Czytanie na głos podlega tej samej regule co inne funkcje AI (model jest zwalniany po minutach ustawionych w Ogólne > Modele AI i nie jest wczytywany, gdyby podniósł zajętość pamięci powyżej 80 procent; mówi o tym wiersz komunikatów pod inspektorem). Pracuje z niskim priorytetem i przygotowuje tylko kilka zdań naprzód względem odtwarzanego, nigdy cały rozdział.

### Pismo odręczne

**Pismo odręczne** pozwala pisać rysikiem lub palcem i zamieniać atrament na tekst. Jest wbudowane w {{APP}} w każdym systemie. Potrzebuje modelu pisma odręcznego (silnik i model są instalowane w **Plik > Opcje > Pismo odręczne**); dopóki żaden nie jest zainstalowany, pierwsze użycie przycisku otwiera tę stronę. Dla rosyjskiego nie ma jeszcze modelu.

**Przycisk.** W Linuksie i Windows przycisk jest na początku ukryty: włącz go w **Widok > Paski narzędzi > Przycisk pisma odręcznego** albo w **Plik > Opcje > Pismo odręczne** (na Androidzie jest pokazany od początku). Znajduje się na pasku Narzędzia i ma cztery wyglądy: wyszarzone pióro (wyłączone), pióro na kartce (edytor), pióro na bocznym panelu (inspektor) oraz pióro na obu (oba). **Dotknięcie** przełącza między wyłączeniem a trybem użytym ostatnio; **długie naciśnięcie** (prawy przycisk albo naciśnięcie i przytrzymanie) otwiera cztery opcje: Wyłączone, Edytor, Inspektor, Oba. Tryb jest zapamiętywany dla tego urządzenia, a nie w projekcie.

**Trzy tryby** korzystają z jednego obszaru pisania.
- **Inspektor**: nowa karta inspektora, **Pismo odręczne**, to podkładka do pisania z siatką. Inspektor otwiera się, gdy tryb go potrzebuje.
- **Edytor**: przezroczysta warstwa leży nad tekstem edytora; piszesz po tekście, a tekst pozostaje widoczny pod atramentem. Inspektor otwiera się na karcie Pismo odręczne, gdzie zbierany jest napisany przez ciebie tekst.
- **Oba**: warstwa leży nad edytorem, linią między edytorem a inspektorem i częścią podkładki inspektora. Gdy ten tryb jest włączony, linii nie można przeciągać. Siatka jest tylko nad częścią podkładki.

Na Androidzie klawiatura ekranowa pozostaje schowana, gdy tryb jest włączony; **Spacja**, **Backspace** i **Enter** są przyciskami pod podkładką.

**Pisanie i wskazywanie.** Z rysikiem pisze rysik; krótkie dotknięcie rysikiem umieszcza tam kursor tekstu; boczny przycisk rysika zaznacza tekst; palce przewijają (a dotknięcie palcem umieszcza kursor). Bez rysika pisze jeden palec; dotknięcie umieszcza kursor; długie naciśnięcie i przeciągnięcie zaznaczają; dwa palce przewijają. **Wykreślenie słów** (szybki ruch tam i z powrotem po nich) usuwa je: atrament, który nie został jeszcze odczytany, a w przeciwnym razie słowa tekstu pod kreską, albo słowo z paska. W części podkładki krótkie dotknięcie jest kropką atramentu (kropką nad „i”), tak samo jak obok atramentu napisanego chwilę wcześniej.

**Odczytywanie atramentu.** Po pauzie (na początku sekunda) napisany wiersz jest odczytywany. Każda linia siatki to jeden wiersz; bez linii atrament leżący osobno jest odczytywany jako wiersze. Atrament pozostaje przez około 3 sekundy, a potem zanika w ciągu około 2 sekund; w miarę zanikania odczytany tekst pojawia się na **pasku** na dole inspektora. Nic nie trafia do twojego tekstu samo.

**Pasek.** Dotknięcie słowa pokazuje jego inne odczyty; wybierz jeden, aby zastąpić słowo. Napisanie czegoś nad słowem rysikiem lub palcem poprawia je. **Wstaw** umieszcza tekst z paska przy kursorze tekstu ustawionym wcześniej (przesuwa się razem z tekstem, jeśli tekst się zmieni) i przesuwa miejsce za niego. **Przypnij** sprawia, że tekst przykleja się do rysika lub palca: dotknij pola tekstowego (edytora, notatek, streszczenia), aby pokazać miejsce; znacznik wyboru na szpilce upuszcza tam tekst, krzyżyk anuluje i tekst wraca na pasek. **Wyczyść** wyrzuca tekst.

**Co jest rozpoznawane.** Wiersz atramentu jest odczytywany przez model działający na twoim komputerze (PP-OCRv6 small, Apache License 2.0, 13 MB) z ONNX Runtime z **silnika pisma odręcznego**; nic nie opuszcza komputera. Odczytuje pismo w alfabecie łacińskim, drukowane i odręczne, po angielsku, niemiecku, hiszpańsku, francusku, portugalsku, włosku, indonezyjsku, turecku, polsku i niderlandzku, z literami tych języków i ich znakami diakrytycznymi; dla rosyjskiego nie ma jeszcze modelu (wybór jest wyszarzony). Każdy wiersz jest odczytywany osobno: atrament jednej linii siatki to jeden wiersz, a bez linii należą do niego pociągnięcia, których środek leży blisko ramki dotychczasowego wiersza. **Lista słów** każdego języka (najczęstsze słowa z ich częstością, wykonana z wordfreq 3.1.1 autorstwa Robyn Speer, Creative Commons Attribution-ShareAlike 4.0; pobierana razem z modelem, po jednej na język) poprawia następnie słowa, które podały znaki: spośród wymienionych słów bliskich temu, co odczytano, wygrywa to, które pasuje do kształtu atramentu i jest częste; imię albo wymyślone słowo, do którego żadne słowo z listy nie pasuje lepiej, pozostaje takie, jak odczytano; słowo, które skleja dwa słowa, jest dzielone, gdy żadne pojedyncze słowo nie pasuje. Każdy wiersz jest odczytywany z językiem głównym i zapasowym, a zachowywany jest odczyt, który pasuje lepiej. Dotknięcie słowa na pasku pokazuje następnych kandydatów i surowy odczyt. Bez listy słów dla języka używane są znaki takie, jak odczytano.

**Instalowanie.** Przycisk **Zainstaluj** w **Plik > Opcje > Pismo odręczne** otwiera ten sam panel co Czytanie na głos: najpierw **silnik pisma odręcznego** (ONNX Runtime), potem model wierszy i listy słów języków, w których piszesz, każdy z licencją pokazaną przed pobraniem. **Usuń** kasuje model i listy słów. Wiersze są odczytywane z niskim priorytetem na rdzeniach ustawionych w **Plik > Opcje > Ogólne > Modele AI**, a model jest zwalniany po czasie ustawionym tam.

**Ochrona dłoni.** Gdy rysik pisze, i przez około sekundę potem, dotknięcia palcami w strefie wokół pisanego tekstu (około 30 mm, szerszej po stronie ręki piszącej i poniżej) są ignorowane; duże obszary dotyku są ignorowane, gdy rysik jest w użyciu; bez rysika ignorowane są dotknięcia w pasie około 15 mm przy krawędzi ekranu. Krótkie, małe dotknięcie przycisku lub menu jest zawsze przyjmowane.

**Opcje.** **Plik > Opcje > Pismo odręczne** ma: **Rozpoznawanie** (część instalująca z przyciskiem Zainstaluj, język główny i język zapasowy; rosyjski jest wyszarzony, „jeszcze brak modelu dla tego języka”); **Pisanie** (pauza przed odczytaniem wiersza, jak długo atrament pozostaje, zanikanie, kolor i grubość atramentu); **Siatka** (linie, linie pisma z linią podstawową i środkową, kratka, kropki albo brak; kolor, grubość linii, krycie; rozmiar suwakiem od małego do dużego, z próbką obok); **Ochrona dłoni** (włączona lub wyłączona, praworęczny lub leworęczny, siła); **Rysik i palec**; oraz **Uczenie się twojego pisma**. Kolory, których nie ruszysz, podążają za motywem.

**Uczenie się twojego pisma.** Gdy poprawisz wiersz (inny odczyt albo napisanie czegoś nad słowem), obraz atramentu i poprawiony tekst są zachowywane na tym urządzeniu, w folderze danych programu, i nigdy nigdzie nie wysyłane. Możesz to wyłączyć i usunąć zachowane wiersze na tej samej stronie. Gdy otwarty jest zaszyfrowany projekt, nic nie jest zachowywane (pliki byłyby zwykłe); strona o tym mówi.

## 17. Aktualizacje i dodatki

**Pomoc > Rozwiązywanie problemów i programowanie > Przewodnik programisty: aktualizacje i dodatki** wyjaśnia, dla osób budujących {{APP}} lub piszących dodatki, jak działają aktualizacje i dodatki i jakie narzędzia je tworzą; zawiera też to, czego potrzebuje technicznie nastawiony znajomy, aby coś skonfigurować (silniki multimediów, wtyczki formatów kompilacji, pliki pakietów, motywów, układów i formatów scenariuszy, pliki wewnątrz folderu projektu) oraz zmienne środowiskowe, które program czyta.

{{APP}} to jeden plik programu (AppImage). Aktualizacje i rozszerzenia przychodzą jako małe pliki pakietów z końcówką **.qbpkg**. Są trzy rodzaje i wszystkie instaluje się tak samo:

- **Aktualizacja programu**: nowsza wersja samego {{APP}}.
- **Dodatek**: rozszerzenie, które można włączać i wyłączać, takie jak motywy kolorów, szablony projektów, formaty kompilacji, słowniki, narzędzia pisarskie (sekcja 10), odtwarzanie audio i wideo oraz jego dodatkowe formaty (sekcja 6), importery starych plików Word i stron internetowych (sekcja 14).

Aktualizacje programu i dodatki są na stronie **Releases** strony projektu (sekcja 1), obok AppImage.
- **Pakiet aktualizacji**: jeden plik, który zawiera aktualizację programu i kilka dodatków, instalowanych razem.

### Sprawdzanie aktualizacji

Od 0.92.0 {{APP}} może sam szukać aktualizacji. Jest to jedyna rzecz, dla której program sam łączy się z internetem, i możesz to wyłączyć.

- **Pomoc > Dodatki i aktualizacje > Sprawdzaj aktualizacje przy uruchamianiu** to przełącznik (na początku włączony). **Pomoc > Dodatki i aktualizacje > Sprawdź aktualizacje teraz** sprawdza od razu, niezależnie od przełącznika.
- **Pierwsze uruchomienie** mówi o tym na swojej stronie, zanim cokolwiek zostanie skontaktowane: pole wyboru *Look for updates when the program starts* (zaznaczone) z informacją, że nic o tobie nie jest wysyłane; jedno **OK** kończy stronę. Program uruchomiony przed 0.94.0, który nie widział tej informacji, pokazuje ją jako okno z **OK** i **Wyłącz**.
- **Co się dzieje**: kilka sekund po uruchomieniu, najwyżej raz dziennie, program pobiera jeden mały plik ze strony projektu (*shelin-boop.github.io/Quillbinder_Collection/updates-1.json*) i podpis tego pliku. Żądanie podaje nazwę programu („{{APP}}”) i nic więcej: żadnej wersji, żadnego systemu, identyfikatora ani ciasteczka. Jak przy każdym połączeniu, serwer (GitHub) widzi twój adres internetowy i godzinę. Plik wymienia najnowszą wersję programu i dodatków projektu; twój program porównuje to z tym, co jest zainstalowane, na twoim komputerze. Plik się liczy tylko wtedy, gdy ma podpis klucza wydań projektu, a plik starszy od już widzianego jest ignorowany.
- **Gdy jest aktualizacja**, okno ją nazywa: program lub dodatek, twoja wersja i nowa, co nowego i jej rodzaj. Aktualizacja **funkcjonalna** ma trzy przyciski: **Pobierz i zainstaluj**, **Ignoruj tę wersję** (żadnych więcej przypomnień dla tej wersji) i **Odłóż**. Odłożenie czeka za każdym razem dłużej: 1 dzień, 1 tydzień, 2 tygodnie, 4 tygodnie; potem jest jedno ostatnie przypomnienie i po nim przypomnienia dla tej wersji się kończą. Aktualizacja **zalecana (bezpieczeństwo)** ma tylko dwa przyciski, **Pobierz i zainstaluj** i **Odłóż**: nie można jej zignorować, a po czwartym odłożeniu wraca co 4 tygodnie. Gdy są oba rodzaje, jedno okno je wymienia i stosuje reguły bezpieczeństwa. Okno pojawia się obok twojej pracy i nie przejmuje klawiatury.
- **Pobierz i zainstaluj** pobiera pliki wymienione na liście, sprawdza każdy względem rozmiaru i sumy kontrolnej z podpisanej listy i instaluje go: dodatek lub aktualizację programu tak, jak robi to **Pomoc > Dodatki i aktualizacje > Zainstaluj aktualizację lub dodatek** (tylko pakiety z podpisem projektu). Potem proponuje ponowne uruchomienie programu.
- **Gdy nowa wersja wymaga nowego AppImage** (innych bibliotek, niż ma twój AppImage), {{APP}} pobiera go, umieszcza w miejscu AppImage, który uruchomiłeś(-aś), gdziekolwiek on jest i pod tą samą nazwą pliku, zachowuje ten, który miałeś(-aś), obok jako *…AppImage.before-update* i uruchamia się ponownie. Usuń zachowany plik, gdy nowa wersja ci się sprawdza. Jeśli do folderu twojego AppImage nie można zapisać, nowy plik trafia do twojego folderu Pobrane, a komunikat mówi, co zrobić.
- **W Windows** (0.92.0) taka wersja przychodzi jako program instalacyjny. {{APP}} pobiera go do własnego folderu (*data\update-setup*), sprawdza względem podpisanej listy projektu i pyta. Przy **Zamknij i zaktualizuj teraz** {{APP}} się zamyka, a program instalacyjny rozpakowuje nową wersję do używanego przez ciebie folderu, bez swoich stron, i ją uruchamia; twoje ustawienia, dodatki i wszystko inne w *data* pozostają. Przy **Później** nic się nie zmienia i aktualizacja jest oferowana ponownie. Jeśli Windows nie uruchomi programu instalacyjnego (nie ma on certyfikatu podpisu kodu), komunikat podaje plik, a bieżącą wersję możesz też pobrać ze strony projektu.
- **Gdy adres jest nieosiągalny**, program próbuje dwa razy w odstępie 5 sekund, a potem pokazuje mały tekst na pasku stopki pierwszego edytora, w miejscu tekstu zapisu, który dwa razy wyłania się i zanika. Żadnego okna i najwyżej raz dziennie. (**Sprawdź aktualizacje teraz** odpowiada zamiast tego komunikatem.)
- **Przy wyłączeniu** program nie łączy się z niczym, jak przed 0.92.0. Dla komputerów skonfigurowanych dla innych zmienna środowiskowa `QB_NO_UPDATE_CHECK=1` zapobiega uruchamianiu automatycznego sprawdzania bez względu na przełącznik.
- AppImage w wersji starszej niż 0.92.0, który uruchamia nowszą aktualizację programu, nie może nawiązywać bezpiecznych połączeń; sprawdzanie mówi o tym, gdy o nie poprosisz, a poza tym milczy. Pobierz raz bieżący AppImage.
- To, co program o tym zapamiętuje (przełącznik, czas ostatniego sprawdzenia, co zignorowałeś(-aś) lub odłożyłeś(-aś)), jest w *updates-state.json* w folderze danych programu.

### Instalowanie pakietu

1. Wybierz **Pomoc > Dodatki i aktualizacje > Zainstaluj aktualizację lub dodatek…** i zaznacz plik .qbpkg albo kliknij dwukrotnie plik w menedżerze plików.
2. {{APP}} pokazuje, co zawiera pakiet i co nowego. Kliknij **Zainstaluj**.
3. Jeśli pakiet zawiera aktualizację programu, {{APP}} prosi o ponowne uruchomienie. Twój otwarty projekt jest zapisywany i otwierany ponownie.

{{APP}} sprawdza każdy pakiet przed instalacją: uszkodzone pliki i pakiety zrobione dla innej wersji są odrzucane. Aktualizacja programu jest raz uruchamiana niewidocznie jako test; jeśli nie działa na twoim komputerze, nie jest instalowana. Części pakietu aktualizacji są instalowane razem: gdy jednej z nich nie można zainstalować, nie jest instalowana żadna (0.65).

**Podpisy** (0.65). Każdy pakiet na stronie wydań projektu jest podpisany kluczem projektu, a {{APP}} nosi publiczną połowę tego klucza. Okno instalacji mówi **Podpisano przez projekt Quillbinder** dla takiego pakietu: jest to pakiet opublikowany przez projekt, niezmieniony.

- Pakiet, którego podpis się nie zgadza (zmieniony po podpisaniu albo uszkodzony), nigdy nie jest instalowany.
- **Aktualizacja programu** jest instalowana tylko z podpisem projektu.
- **Dodatek** bez niego (zrobiony przez ciebie lub inną osobę) jest instalowany dopiero po wyraźnym ostrzeżeniu, z **Anuluj** jako domyślnym. W wierszu poleceń (`--install <file>`) pakiet bez podpisu projektu jest instalowany tylko wtedy, gdy dodasz `--allow-unsigned`.

**Klucze dodatków** (0.91.3). Oficjalne dodatki, także pisane przez innych autorów, mogą być podpisane własnym kluczem zamiast kluczem projektu. Projekt poświadcza taki klucz swoim własnym kluczem, w certyfikacie, który przychodzi w pakiecie i pozwala kluczowi podpisywać dodatki i nic więcej. Okno instalacji nazywa wtedy klucz: *Podpisany kluczem dodatku „Quillbinder official add-ons”, za który projekt Quillbinder ręczy*.

- **Aktualizacja programu** podpisana kluczem dodatku jest odrzucana jak niepodpisana: aktualizacje programu wymagają własnego klucza projektu.
- Klucz dodatku, którego projekt nie poświadcza, liczy się jako brak podpisu: dodatek jest instalowany dopiero po ostrzeżeniu.
- Dodatki podpisane własnym kluczem projektu, jak wszystkie do 0.91.2, pozostają zaufane. Od 0.91.3 własne dodatki projektu są podpisywane także kluczem dodatku.
- Wersje do 0.91.2 nie znają kluczy dodatków i ostrzegają przed instalacją dodatku podpisanego takim kluczem: zainstaluj najpierw aktualizację programu (jest podpisana własnym kluczem projektu), potem dodatki.

**Okno pytające przed instalacją dodatku** (0.94.0) mówi najpierw, w zdaniu lub dwóch, co dodatek robi. Linie techniczne (biblioteki, które przynosi, silnik mowy lub grafiki, środowisko uruchomieniowe, dla którego został zbudowany) są pod **Szczegóły**, które są zwinięte, dopóki ich nie otworzysz.

**Silniki AI i ich modele** (0.99.0). Funkcje AI działają na twoim komputerze: rozpoznawanie mowy, czytanie na głos i rozpoznawanie pisma odręcznego. Ich silniki to dwa osobne pakiety, **silnik AI** i **silnik pisma odręcznego**, a używane przez nie modele to osobne pobrania; nic z tego nie jest instalowane razem z {{APP}} i nic nie jest wczytywane, dopóki nie użyjesz takiej funkcji. Strona ustawień każdej funkcji (Czytanie na głos, Pismo odręczne; Transkrypcja mowy od swojej wersji 1.10) pokazuje w tym celu tę samą część:

- Stan silnika, z przyciskiem **Zainstaluj silnik**: pobiera pakiet silnika twojej wersji ze strony wydań projektu i instaluje go jak dodatek, tylko z podpisem projektu. **Zainstaluj z pliku...** przyjmuje pakiet pobrany gdzie indziej, dla komputera bez połączenia (pakiet bez podpisu projektu tylko po ostrzeżeniu). Silnik zainstalowany, gdy używany jest inny, zaczyna działać po ponownym uruchomieniu {{APP}}. Silniki są wymienione w Menedżerze dodatków jak dodatki i można je tam wyłączyć lub usunąć.
- Modele tej funkcji, z ich językami, rozmiarem pobierania, potrzebną pamięcią i licencją. **Pobierz** ściąga wybrany model ze strony wydań projektu z paskiem postępu (**Anuluj** go zatrzymuje; przerwane pobieranie jest kontynuowane od miejsca, w którym stanęło) i sprawdza każdy plik z sumą kontrolną, zanim model zostanie uznany za zainstalowany. Model, który potrzebuje więcej pamięci, niż ma twój komputer, nie jest oferowany do pobrania, a strona mówi dlaczego. **Usuń** kasuje model. Modele są przechowywane w folderze danych programu, w *ai-models*.
- {{APP}} łączy się w tym celu z internetem tylko wtedy, gdy naciśniesz jeden z tych przycisków, i nie pyta żadnego serwera, co jest dostępne: adresy to adresy wydań projektu.
- Tam, gdzie funkcja pozwala wybrać urządzenie: **Automatycznie** wybiera kartę graficzną, gdy model mieści się w jej wolnej pamięci (według sterownika), w przeciwnym razie pamięć komputera i procesor; albo wybraną kartę graficzną (karty NVIDIA, AMD i Intel przez Vulkan); albo procesor. Rdzenie i reguła dla pamięci są takie, jak w **Plik > Opcje > Ogólne > Modele AI**.
- Mowa czytana na głos jest tylko odtwarzana, nigdy nie jest zapisywana jako plik.

**Samodzielne sprawdzanie pobranych plików.** Każde wydanie zawiera także *SHA256SUMS.txt* (sumę kontrolną każdego pliku), jego podpis *SHA256SUMS.txt.sig* i klucz publiczny *quillbinder-signing-public.pem*. W terminalu, w folderze z pobranymi plikami:

    openssl pkeyutl -verify -pubin -inkey quillbinder-signing-public.pem -rawin -in SHA256SUMS.txt -sigfile SHA256SUMS.txt.sig
    sha256sum -c --ignore-missing SHA256SUMS.txt

Pierwsze polecenie odpowiada *Signature Verified Successfully*, drugie *OK* dla każdego pobranego pliku. Aby mieć pewność, że sam plik klucza jest kluczem projektu, porównaj jego 32 bajty (`openssl pkey -pubin -in quillbinder-signing-public.pem -outform DER | tail -c 32 | od -An -tx1`) z kluczem w źródle programu (*src/core/Signing.cpp* i *packaging/quillbinder-signing-public.pem*): zaczynają się od *bc 3c 58 00 08 3f 66 e6*.

### Menedżer dodatków

**Pomoc > Dodatki i aktualizacje > Menedżer dodatków…** pokazuje, co jest zainstalowane.

- **Wersja programu**: wersja wbudowana w twój AppImage i każda zainstalowana aktualizacja. Ta oznaczona *running now* jest w użyciu. Aby wrócić do wcześniejszej wersji, zaznacz ją i kliknij **Użyj tej wersji**, potem uruchom ponownie. Zainstalowane aktualizacje, których już nie potrzebujesz, można usunąć.
- **Dodatki**: odznacz dodatek, aby go wyłączyć bez usuwania; **Usuń dodatek** go kasuje. Motywy, szablony i formaty kompilacji dodatku pojawiają się w zwykłych miejscach: **Widok > Motywy**, oknie **Nowy projekt** i oknie **Kompilacja**.
- Gdy wyboru dokonanego tutaj nie można zapisać (pełny dysk, folder, do którego nie można zapisać), komunikat mówi o tym z nazwą pliku (0.92.0). Wcześniej okno pokazywało wybór jako dokonany, a przy następnym uruchomieniu był on cofany bez słowa. To samo dotyczy instalacji: dodatek lub wersja programu, której wpisu nie dało się zapisać, jest zgłaszana jako niezainstalowana, z informacją, co zrobić. Gdy instalacja zawodzi w połowie i wersji zainstalowanej wcześniej nie da się przywrócić, komunikat podaje folder, w którym się znajduje, i nazwę, jaką należy temu folderowi nadać.

### Gdy aktualizacja się nie uruchamia

Jeśli aktualizacja programu dwa razy z rzędu się nie uruchomi, {{APP}} sam ją wyłącza i uruchamia wersję wbudowaną w AppImage, z komunikatem, który o tym mówi. Nie ma to wpływu na nic w twoich projektach.

### Gdy potrzebny jest nowy AppImage

Aktualizacje programu działają na bibliotekach wewnątrz twojego AppImage. Od czasu do czasu większa aktualizacja wymaga nowego AppImage; okno instalacji mówi, gdy pakiet nie pasuje do twojego. Zastąp stary plik AppImage nowym; twoje ustawienia, dodatki i projekty pozostają bez zmian.

0.80.0 jest taką wersją: jej AppImage niesie nowe Qt (własną kompilację Qt 6.12 przygotowaną przez The Qt Company zamiast Qt 6.4.2 z Ubuntu). Dodatki z kodem programu (*Media Playback*, *Extra Video and Audio Formats*, *Speech Transcription*, narzędzia pisarskie i importery) trzeba zainstalować ponownie w ich wersjach dla 0.80.0 (pakiet aktualizacji i **media-all-formats.qbpkg** na stronie Releases przynoszą je wszystkie); do tego czasu Menedżer dodatków oznacza stare jako zbudowane dla starszego programu, a ich kod nie jest ładowany. Pełna wersja 0.80.0 przynosi każdy dodatek.

0.93.0 jest kolejną (Linux): jej AppImage niesie Qt 6.12.1, którego wbudowane biblioteki (zlib, HarfBuzz, PCRE2, libpng, libjpeg-turbo, FreeType i inne) są nowsze i mają poprawki bezpieczeństwa. Aktualizacja programu nie może ich przynieść, więc sprawdzanie aktualizacji proponuje nowy AppImage. Dodatki z kodem programu wcześniejszego AppImage (wymienione wyżej) są instalowane ponownie w swoich wersjach dla 0.93.0 w ten sam sposób, a dodatki multimedialne niosą teraz FFmpeg 9.1.

Nowy AppImage przynosi też własną nową wersję programu (na przykład AppImage 0.20.0). Uruchamia tę wersję, nawet jeśli zainstalowałeś(-aś) aktualizacje w starym AppImage: starsze aktualizacje są wyłączane i pozostają na liście w **Pomoc > Dodatki i aktualizacje > Menedżer dodatków…**, gdzie możesz je usunąć. Jeśli nowy plik ma inną nazwę lub leży w innym folderze niż stary, wybierz raz **Pomoc > Dodaj do menu aplikacji**, aby wpis w menu i dwukrotnie klikane projekty używały nowego pliku.

---

## 18. Dialekt Markdown

Nie musisz tego wiedzieć, by korzystać z {{APP}}; edytor pokazuje sformatowany tekst. Ma to znaczenie tylko wtedy, gdy otwierasz pliki tekstowe innym programem. {{APP}} zapisuje Markdown według tych reguł:

- **Każdy wiersz jest jednym akapitem**, jak w edytorze tekstu. Pusty wiersz to pusty akapit.
- Nagłówki: od `# ` do `###### ` na początku wiersza.
- Cytat blokowy: `> `. Akapit wyśrodkowany: `->text<-`. Każdy inny styl akapitu: `::Style Name:: text`.
- Style znaku (od wersji 0.4): `<span class="Style Name">text</span>`. Do tekstu zapisywane są tylko nazwy stylów. Wygląd stylu jest przechowywany raz, w pliku projektu, więc zmieniony styl nie wymaga zmian w dokumentach.
- Listy: `- ` i `1. `; dwie spacje na poziom zagnieżdżenia.
- `**bold**`, `*italic*`, `~~strikethrough~~`, `==highlight==`, `<u>underline</u>`, `<sup>superscript</sup>`, `<sub>subscript</sub>`.
- Kolory rewizji: `<rev1>…</rev1>` do `<rev5>…</rev5>`.
- Odnośniki: `[text](https://example.com)`; odnośniki do dokumentów w projekcie: `[text](qb://ID)`.
- Obrazy: `![](images/picture.png){width=300}` (obrazy są przechowywane w folderze dokumentu). Opis obrazu (0.62) to tekst w nawiasach kwadratowych: `![A brass lamp](images/lamp.png){width=300}`.
- Tabele (0.62): wiersz `:::table` z projektem w nawiasach klamrowych (JSON), po jednym wierszu na wiersz tabeli z komórkami między `|` (akapity komórki rozdzielone `<p/>`, komórka przykryta przez scaloną pozostawiona pusta, `|` w tekście zapisane jako `\|`) i wiersz `:::` na końcu.
- Obraz obok tekstu lub wyśrodkowany (0.62): `:::figure {"pos":1,"width":168}` (1 lewo, 2 prawo, 3 środek), wiersz obrazu, wiersze podpisu i `:::`.
- Złamanie wiersza wewnątrz akapitu: `<br>`.
- Czcionki ustawione ręcznie (od wersji 0.3): `<span style="font-family:'DejaVu Sans'; font-size:14pt; color:#aa0000">text</span>`. Deklaracje są zwykłym CSS, więc inne programy Markdown, które dopuszczają HTML, też je pokazują. Możliwe wpisy: `font-family`, `font-size` (pt), `font-weight` (100 do 900), `color`, `letter-spacing` (em), `font-variant:small-caps`, `text-transform:uppercase`.
- Formatowanie akapitu ustawione ręcznie (od wersji 0.3): lista w nawiasach klamrowych na początku wiersza, przed jakimkolwiek innym prefiksem: `{: align=right left=10mm right=5mm first=0mm before=6pt after=12pt line=1.5 tabs=20mm,50mmR} text`. `line` to wielokrotność (`1.5`), dokładna wysokość (`14pt`) albo minimum (`14pt+`). Tabulatory kończą się na `R` (prawy), `C` (środkowy) lub `D` (kropka dziesiętna); brak litery oznacza lewy. Zapisywane są tylko wpisy, które ustawiłeś(-aś).
- Elementy scenariusza (od wersji 0.10) są stylami akapitu nazwanymi od elementu: `::Script > Scene Heading:: INT. LIGHTHOUSE - NIGHT`. Wygląd elementu jest przechowywany raz, w pliku projektu, razem z formatem scenariusza.
- Podział strony przed akapitem (z importowanych scenariuszy i plików RTF, od wersji 0.10): `{: page=before} text`.
- Wersje przed 0.62 pokazują tabele i rysunki jako wiersze tekstu (`:::table` ...). Jeśli wrócisz do starszej wersji, nie edytuj dokumentów z tabelami ani obrazami obok tekstu.
- Wersja 0.2 pokazuje oba znaczniki jako zwykły tekst. Jeśli wrócisz do wersji 0.2 przez Menedżer dodatków, nie edytuj dokumentów, które je zawierają.
- Wersje przed 0.4 nie znają twoich własnych stylów: pokazują `<span class=…>` jako zwykły tekst, a akapity twoich własnych stylów traktują jako tekst podstawowy. Jeśli wrócisz do starszej wersji, nie edytuj dokumentów, które używają twoich własnych stylów.
- Wersje przed 0.10 traktują elementy scenariusza jak tekst podstawowy i usuwają nazwy elementów ze scenariusza, gdy tylko został pokazany w edytorze i projekt zostanie zapisany. Jeśli wrócisz do starszej wersji, nie otwieraj tam swoich scenariuszy (zob. sekcja 20).
- Ukośnik wsteczny sprawia, że następny znak jest zwykłym tekstem: `\*` to gwiazdka, nie kursywa.
- Notatki podążają za konwencją CriticMarkup:
  - `{>>inline annotation<<}`
  - `^[inline footnote]`
  - `{==text==}{>>linked comment<<}`
  - `{==text==}^[linked footnote]`
  - Komentarz może zaczynać się od koloru: `{>>@#ff8800 text<<}`.

**Plik > Konwertuj plik Markdown ⇄ RTF…** czyta zarówno ten dialekt, jak i standardowy Markdown (w którym akapity są rozdzielone pustymi wierszami), a przy zapisie Markdown pyta, który z nich chcesz.

---

## 19. Skróty klawiaturowe

### Zmiana skrótów

**Plik > Opcje > Klawiatura** (albo **Pomoc > Skróty klawiaturowe**) wymienia każde polecenie, pogrupowane według menu, z jego skrótem i skrótem standardowym, z którym przyszedł program. Wymienione są także style otwartego projektu i układy, więc styl lub układ może mieć własny skrót.

1. Znajdź polecenie: przewiń albo wpisz w polu wyszukiwania część jego nazwy lub skrót, na przykład „Ctrl+K”.
2. Kliknij polecenie, kliknij w pole skrótu pod listą i naciśnij nowe klawisze.
3. Jeśli inne polecenie ma ten skrót, jesteś pytany, czy mu go odebrać. To drugie polecenie nie ma wtedy skrótu, dopóki nie nadasz mu nowego.

**Bez skrótu** usuwa skrót zaznaczonego polecenia, **Standardowy** oddaje mu ten, z którym przyszło, a **Wszystko na standardowe** cofa wszystkie twoje zmiany. Twoje zmiany są pokazane pogrubieniem. Skrót wymaga Ctrl, Alt lub Meta albo musi być klawiszem funkcyjnym, ponieważ zwykłe litery są potrzebne do pisania. **Eksportuj** i **Importuj** zapisują twój zestaw skrótów jako plik (*.qbkeys*) i wczytują taki plik, na przykład aby przenieść go na inny komputer.

Skróty stylów należą do nazwy stylu: „Block Quote” ma swój skrót w każdym projekcie, który ma styl o tej nazwie.

### Skróty a układ klawiatury

Kilka ze standardowych skrótów to klawisze, które na niemieckiej klawiaturze wymagają **AltGr**: **Ctrl+{**, **Ctrl+|** i **Ctrl+}** (wyrównaj do lewej, wyśrodkuj, wyrównaj do prawej), **Ctrl+[** i **Ctrl+]** (wstecz i dalej), **Ctrl+\\** (lista elementów scenariusza) oraz **Ctrl+=** (indeks dolny). Od 0.94.0 program patrzy na układ klawiatury systemu **przy samym pierwszym uruchomieniu** (gdy nie ma ustawień skrótów ani wcześniejszego uruchomienia) i zachowuje odpowiedź. Przy **układzie amerykańskim** albo jakimkolwiek innym niż niemiecki pozostaje zwykły zestaw powyżej. Przy **układzie niemieckim (lub austriackim)** te polecenia dostają klawisze niewymagające AltGr (ten sam zestaw przy **układzie francuskim, belgijskim, hiszpańskim, włoskim, portugalskim, tureckim lub niderlandzkim**, w których nawiasy i klamry także wymagają AltGr; polski układ programisty, w którym litery ć i ń na AltGr kolidują z Ctrl+Alt+C i Ctrl+Alt+N, przenosi tylko cztery polecenia wymienione pod tabelą):

| Polecenie | Zestaw niemiecki |
|---------|------------|
| Wyrównaj do lewej / wyśrodkuj / wyrównaj do prawej (także w mapie myśli) | Alt+Shift+L / Alt+Shift+E / Alt+Shift+R |
| Wyjustuj (mapa myśli) | Alt+Shift+J |
| Wstecz / dalej | Alt+Shift+B / Alt+Shift+F |
| Lista elementów scenariusza | Ctrl+Shift+O |
| Indeks dolny | Ctrl+Shift+D |
| Przesuń na wierzch / na spód (mapa myśli) | Ctrl+Shift+PageUp / Ctrl+Shift+PageDown |

Układy, które używają AltGr dla liter (polski, brazylijski i pozostałe powyżej), przenoszą też cztery polecenia z Ctrl+Alt+litera, które Windows odczytuje jako AltGr: kopiuj formatowanie Ctrl+Shift+Y, wklej formatowanie Ctrl+Shift+U, przejdź do notatek dokumentu Ctrl+Shift+X oraz (nie w układzie polskim) pokaż linijkę Ctrl+Shift+W. Klawiatury rosyjska i indonezyjska zachowują zwykły zestaw.

Menu pokazują każdy klawisz tak, jak nazywa go układ. Program używany przed 0.94.0 zachowuje swoje skróty takimi, jakie były (nic nie zmienia się samo), tak samo jak każdy skrót, który sam ustawiłeś(-aś) w **Plik > Opcje > Klawiatura**: zapisany skrót nigdy nie jest zastępowany. Od 0.95 przycisk **Użyj zestawu dla mojego układu klawiatury…** na dole **Plik > Opcje > Klawiatura** stosuje zestaw pasujący do aktualnie używanego układu klawiatury (dla klawiatury niemieckiej: powyższe klawisze). Najpierw wymienia, które skróty standardowe się zmienią; skróty ustawione przez ciebie pozostają. Zestaw należy do twojego profilu. Tam, gdzie system nie podaje swojego układu, używany jest zwykły zestaw. (Linux: ustawienia klawiatury pulpitu lub serwera X; Windows: język układu klawiatury; Android: język klawiatury.)

### Skróty standardowe

| Czynność | Skrót |
|--------|----------|
| Nowy projekt / otwórz projekt | Ctrl+Shift+N / Ctrl+O |
| Zapisz / zamknij projekt / zakończ | Ctrl+S / Ctrl+W / Ctrl+Q |
| Nowy dokument / nowy folder | Ctrl+N / Alt+Shift+N |
| Zmień nazwę / duplikuj | F2 / Ctrl+D |
| Przesuń pozycję w górę / w dół / na zewnątrz / do środka | Ctrl+Alt+Up / Down / Left / Right |
| Podziel w miejscu zaznaczenia / z zaznaczeniem jako tytułem | Ctrl+K / Ctrl+Shift+K |
| Scal dokumenty | Ctrl+Shift+M |
| Tekst połączony / tablica korkowa / konspekt | Ctrl+1 / Ctrl+2 / Ctrl+3 |
| Pokaż lub ukryj segregator / inspektor | Ctrl+Alt+B / Ctrl+Alt+I |
| Tryb pisania / pełny ekran | F11 / Ctrl+Shift+F11 |
| Aktywuj segregator / aktywuj edytor / drugi edytor | Alt+1 / Alt+2 / F6 |
| Wstecz / dalej | Ctrl+[ / Ctrl+] (układ niemiecki: Alt+Shift+B / Alt+Shift+F) |
| Poprzedni / następny dokument | Alt+Shift+Up / Alt+Shift+Down |
| Pokaż w segregatorze | Ctrl+Shift+R |
| Znajdź / zamień / następny / poprzedni | Ctrl+F / Ctrl+H / F3 / Shift+F3 |
| Wyszukiwanie w projekcie | Ctrl+Shift+F |
| Użyj zaznaczenia do wyszukiwania | Ctrl+E |
| Znajdź według formatowania / następne / poprzednie | Ctrl+F3 / Ctrl+G / Ctrl+Shift+G |
| Brudnopis (dodatek) | Alt+Shift+Enter; z każdego programu: zob. sekcja 10 |
| Audio i wideo (dodatek): odtwarzaj / wstrzymaj | Ctrl+Enter |
| Audio i wideo: przeskocz w tył / przeskocz w przód | Ctrl+Alt+, / Ctrl+Alt+. |
| Audio i wideo: wstaw znacznik czasu | Ctrl+Shift+Enter |
| Filtruj tablicę korkową lub konspekt | Ctrl+F (gdy jedno z nich ma klawiaturę) |
| Odnośnik do dokumentu | Ctrl+L |
| Komentarz / przypis | Shift+F2 / Ctrl+Shift+F2 |
| Adnotacja śródtekstowa / przypis śródtekstowy | Ctrl+Shift+A / Ctrl+Alt+F |
| Wykonaj migawkę / z tytułem | Ctrl+5 / Ctrl+Shift+5 |
| Cele projektu / słowa kluczowe projektu | Ctrl+Shift+T / Ctrl+Alt+K |
| Importuj pliki / kompiluj | Ctrl+Shift+I / Ctrl+Shift+E |
| Drukuj bieżący dokument | Ctrl+P |
| Okno czcionek / okno akapitu | Ctrl+T / Ctrl+Alt+P |
| Czcionka większa / mniejsza | Ctrl+> / Ctrl+< |
| Kopiuj formatowanie / wklej formatowanie | Ctrl+Alt+C / Ctrl+Alt+V |
| Wklej z formatowaniem / wklej i dopasuj styl | Ctrl+Alt+Shift+V / Ctrl+Shift+V |
| Bez stylu / Nagłówek 1 / Nagłówek 2 / Nagłówek 3 | Ctrl+Alt+0 / Ctrl+Alt+1 / Ctrl+Alt+2 / Ctrl+Alt+3 |
| Następnych sześć stylów z listy stylów (Format > Styl > Dostosuj listę stylów) | Ctrl+Alt+4 ... Ctrl+Alt+9 |
| Przejdź do streszczenia / przejdź do notatek dokumentu (Nawigacja > Inspekcja) | Ctrl+Alt+Y / Ctrl+Alt+N |
| Włączenie lub wyłączenie małego interfejsu (Widok > Mały interfejs) | Ctrl+Alt+S |
| Utwórz kopię zapasową teraz | Ctrl+Shift+B |
| Tryb scenariusza / lista elementów scenariusza | Ctrl+8 / Ctrl+\\ (układ niemiecki: Ctrl+8 / Ctrl+Shift+O) |
| W scenariuszu: następny element / zmień element | Enter / Tab (zob. sekcja 4, Pisanie scenariuszy) |
| Powiększ / pomniejsz / przywróć powiększenie | Ctrl++ / Ctrl+- / Ctrl+0 |
| Powiększ obszar pod myszą | Ctrl+kółko myszy |
| Pokaż lub ukryj linijkę | Ctrl+Alt+R |
| Przewijanie jak w maszynie do pisania | Meta+Ctrl+T |
| Opcje | Ctrl+, |
| Ten podręcznik | F1 |

---

## 20. Rozwiązywanie problemów

### Raporty o awariach i informacje o systemie

Jeśli {{APP}} kończy działanie z powodu błędu, zapisuje na twoim komputerze **raport o awarii**, zanim się zamknie. Jeśli jego okno nie odpowiada przez 30 sekund (zawiesza się; czas ustawia się w **Plik > Opcje > Dodatkowe > Raporty o awariach**), zapisuje **raport o zawieszeniu**, który pokazuje, gdzie utknął; jeśli się odblokuje, raport mówi, po jakim czasie. Przy następnym uruchomieniu {{APP}} po awarii albo po tym, gdy musiałeś(-aś) zakończyć zawieszony {{APP}}, komunikat mówi, co się stało. Jeśli program zakończył działanie w ciągu minuty od uruchomienia, komunikat proponuje **Uruchom bez ostatniego projektu**, na wypadek gdyby przyczyną był projekt.

- **Pomoc > Rozwiązywanie problemów i programowanie > Raporty o awariach…** wymienia raporty o awariach i zawieszeniach z ich datą i rodzajem błędu („zawieszenie (odzyskane)”, gdy {{APP}} pracował dalej). **Kopiuj do schowka** kopiuje zaznaczony, abyś mógł go wkleić do wiadomości do osoby, która opiekuje się {{APP}} dla ciebie; **Otwórz folder** pokazuje pliki; **Usuń** i **Usuń wszystkie…** je kasują. Zachowywanych jest 20 najnowszych raportów (**Plik > Opcje > Dodatkowe > Raporty o awariach** zmienia liczbę).
- **Plik > Opcje > Ogólne > Ostrzeżenia > Pokaż wewnętrzną konsolę dziennika** pokazuje komunikaty programu i Qt od jego uruchomienia (ostatnie 1000), z nowymi w miarę ich pojawiania się; przydatne, gdy coś nie działa, a żaden komunikat nie mówi dlaczego.
- **Pomoc > Rozwiązywanie problemów i programowanie > Informacje o systemie** pokazuje ten sam obraz programu i komputera, który niesie raport. Zaczyna się od **Rzeczy do sprawdzenia**: tego, co może wkrótce sprawiać kłopoty, jak prawie pełny lub tylko do odczytu dysk, projekt w folderze sieciowym, dysk kopii zapasowych z mniejszą ilością wolnego miejsca, niż potrzebuje projekt, mało wolnej pamięci, długi czas autozapisu, wyłączone automatyczne kopie zapasowe albo niedawne raporty o awariach i zawieszeniach. Potem następują wersja programu, środowisko uruchomieniowe (zestaw bibliotek wewnątrz AppImage, z dniem wydania jego Qt i tym, jak jest stare: stare środowisko oznacza, że nowszy AppImage może przynieść poprawki bezpieczeństwa) oraz zainstalowane aktualizacje i dodatki, system operacyjny, pulpit, ekrany (od 0.93.1 z obszarem, którego może użyć okno na każdym), urządzenia wejściowe (klawiatura, mysz, ekran dotykowy, rysik) i to, czy pokazana jest klawiatura ekranowa (0.93.1), procesor i pamięć, foldery z ich wolnym miejscem, ustawienia zapisywania i kopii zapasowych oraz otwarty projekt. Użyj **Kopiuj do schowka** lub **Zapisz jako…**, gdy zgłaszasz problem, który nie jest awarią.

Raport wymienia pliki, foldery i dokumenty oraz polecenia i klawisze użyte krótko przed błędem (wpisane litery są tylko liczone, nigdy nagrywane). Nie zawiera tekstu twoich dokumentów. Twój folder domowy jest zapisany jako **~**.

Jeśli {{APP}} został zatrzymany z zewnątrz albo komputer wyłączono bez zamykania, raportu nie ma, ale następne uruchomienie też o tym mówi. W obu przypadkach twoja praca jest zapisana do ostatniego automatycznego zapisu, na kilka sekund przed końcem; jeśli czegoś brakuje, zajrzyj do migawek dokumentu albo do najnowszej kopii zapasowej (sekcje 11 i 15).

**„The project file cannot be read.”** Jeśli plik projektu (*YourTitle*{{FILE}}) jest uszkodzony, na przykład przez awarię komputera podczas jego zapisu, {{APP}} proponuje kopię, którą przechowuje (z ostatniego otwarcia projektu): **Użyj kopii**. Uszkodzony plik jest zachowywany obok (*… .damaged-<date>*). Zmian w segregatorze od tego czasu (tytuły, kolejność, etykiety) brakuje; wszystkie teksty są na miejscu, a dokumenty utworzone od tego czasu trafiają do folderu **Odzyskane dokumenty** w Materiałach, nazwane według pierwszego wiersza.

**„Plik projektu zawierał niedozwolone w nim znaki.”** Wersje przed 0.20 mogły zapisać niewidoczne znaki sterujące (wklejone z innych programów do tytułu lub streszczenia), po czym projekt przestawał się otwierać. {{APP}} usuwa je przy otwieraniu projektu i mówi o tym; nic więcej nie jest zmieniane. Od 0.20 nigdy nie są zapisywane.

**Dokument jest pokazany jako pusty, z notatką „Nie można odczytać pliku tego dokumentu”.** Jego plik tekstowy jest, ale nie można go odczytać (zmieniono jego uprawnienia albo dysk, na którym jest, zniknął). {{APP}} pokazuje go jako pusty i nie pozwala go edytować, aby plik nigdy nie został nadpisany. Sprawdź uprawnienia pliku albo dysk, potem zamknij i otwórz projekt ponownie.

**Naciśnięcie Page Down lub Page Up zamknęło program.** Był to błąd w wersjach przed 0.19; zaktualizuj do 0.19 lub nowszej. Do tego czasu przewijaj myszą lub paskiem przewijania.

**Program nie uruchamia się po dwukrotnym kliknięciu.** Kliknij plik prawym przyciskiem > Właściwości > Uprawnienia i zaznacz „Jest wykonywalny” (w Dolphin: „Zezwól na wykonywanie pliku jako programu”). Potem kliknij go dwukrotnie jeszcze raz.

**Okna plików wyglądają inaczej niż w innych programach KDE.** Włącz **Używaj okien plików pulpitu** w **Plik > Opcje > Ogólne** i uruchom {{APP}} ponownie.

**Ikony pasków narzędzi są słabo widoczne w ciemnym lub jasnym motywie.** {{APP}} wybiera jasny lub ciemny zestaw ikon twojego pulpitu, pasujący do motywu. Jeśli pulpit nie ma pasującego zestawu, używa własnych ikon. Jeśli pojedyncze ikony nadal nie pasują, wybierz **Domyślny (zgodny z pulpitem)** w **Widok > Motywy** i ustaw kolory w własnych ustawieniach pulpitu.

**Zmieniłem(-am) kolory i chcę z powrotem kolory motywu.** **Plik > Opcje > Wygląd > Przywróć kolory i czcionki motywu** albo przycisk **↺** obok pojedynczego koloru lub czcionki.

**Aktualizacja programu się nie uruchomiła.** {{APP}} sam wyłącza aktualizację, która zawodzi (sekcja 17). Możesz spróbować zainstalować ją ponownie albo dalej używać wersji wbudowanej.

**Brakuje formatu kompilacji.** Sprawdź **Plik > Opcje > Dodatkowe > Dodatki i wtyczki**: wymienia, jakiego programu format wymaga. Zainstaluj go w Discover i uruchom {{APP}} ponownie.

**Sprawdzanie pisowni nie zna mojego języka.** Dla hiszpańskiego, francuskiego, portugalskiego brazylijskiego, włoskiego, rosyjskiego, indonezyjskiego, tureckiego, polskiego i niderlandzkiego zainstaluj dodatek „<język> spelling dictionary” (**Pomoc > Dodatki i aktualizacje > Zainstaluj aktualizację lub dodatek**; strona pobierania je wymienia), uruchom {{APP}} ponownie i wybierz język w **Plik > Opcje > Poprawki** (strona Pisownia). Dla innego języka zainstaluj odpowiedni pakiet słownika *hunspell* w Discover.

**Projekt „seems to be open in another window or on another computer.”** Dzieje się tak, gdy projekt jest w synchronizowanym folderze otwartym gdzie indziej albo po awarii na innym komputerze. Jeśli masz pewność, że nie jest otwarty nigdzie indziej, wybierz **Tak**.

**Alt+przeciągnięcie nie otwiera podglądu pomocniczego.** Niektóre pulpity używają Alt+przeciągnięcia do czegoś własnego, na przykład przesuwania okien. Użyj zamiast tego **Nawigacja > Otwórz > W podglądzie pomocniczym** albo menu prawego przycisku dokumentu, albo zmień klawisz w ustawieniach okien pulpitu.

**Po powrocie do starszej wersji programu scenariusz jest zwykłym tekstem bez układu.** Wersje przed 0.10 nie znają elementów scenariusza i zapisują pokazany scenariusz jako zwykły tekst. Przełącz się znów na bieżącą wersję (**Pomoc > Dodatki i aktualizacje > Menedżer dodatków**). Tekst jest cały; aby odzyskać układ, przywróć dokument z migawki lub kopii zapasowej wykonanej wcześniej (sekcje 11 i 15) albo nadaj akapitom ich elementy ponownie za pomocą menu podręcznego w stopce.

**Skrót brudnopisu nic nie robi w innych programach.** Skróty ustawione w **Plik > Opcje > Klawiatura** działają tylko wewnątrz {{APP}}. Dla innych programów polecenie musi uruchomić pulpit (sekcja 10, *Z każdego programu*). Jeśli przeniesiesz AppImage, wpisz polecenie ponownie (podaje ono miejsce AppImage).

**Aparat brudnopisu mówi, że nie może odczytać ekranu.** W Waylandzie może to robić tylko własny program pulpitu do zrzutów ekranu. Zainstaluj *Spectacle* (`sudo apt install kde-spectacle`) i spróbuj ponownie.

**Edycja > Narzędzia pisarskie mówi „Nie zainstalowano narzędzi pisarskich”.** Narzędzia są dodatkami: zainstaluj pakiet aktualizacji 0.11 albo włącz je w **Pomoc > Dodatki i aktualizacje > Menedżer dodatków**.

**Plik audio lub wideo oferuje tylko „Otwórz w aplikacji domyślnej”.** Odtwarzanie wewnątrz {{APP}} wymaga dodatku *Media Playback* (w pakiecie aktualizacji 0.12); zainstaluj go albo włącz w **Pomoc > Dodatki i aktualizacje > Menedżer dodatków**. Wiersz pod przyciskiem mówi, czego brakuje.

**Film się odtwarza, ale nie ma dźwięku, albo plik audio mówi „Nie znaleziono wyjścia dźwięku”.** {{APP}} odtwarza dźwięk przez serwer dźwięku pulpitu (PipeWire lub PulseAudio). Sprawdź, czy dźwięk działa w innych programach i czy w ustawieniach dźwięku pulpitu wybrano właściwe wyjście; {{APP}} od razu podąża za zmianą wyjścia.

**„Tego pliku nie można tu odtworzyć: używa H.264 i AAC ...”** Plik jest w formacie objętym patentami, który *Media Playback* pomija. Zainstaluj dodatek **Extra Video and Audio Formats** albo pakiet **media-all-formats.qbpkg**, który przynosi go razem z *Media Playback* (oba na stronie Releases strony projektu, sekcja 1), i uruchom {{APP}} ponownie.

**Plik się nie odtwarza albo odtwarza się bez obrazu.** Niektóre rodzaje nie wchodzą w skład wbudowanego dekodowania (wideo AV1, WMA Pro, WMV 9). Otwórz plik innym programem (prawy przycisk na odtwarzaczu: **Open with Default Application**) albo przekonwertuj go na inny format.

**Plik .doc jest dodawany jako plik, nie jako tekst.** Dodatek **Word 97–2003 Import** nie jest zainstalowany albo jest wyłączony (**Pomoc > Dodatki i aktualizacje > Menedżer dodatków**); po jego zainstalowaniu uruchom {{APP}} ponownie. Komunikat po imporcie mówi, co dodatek pominął. Pliki Word 6.0 i 95 wymagają LibreOffice (`sudo apt install libreoffice-writer`).

**Strona internetowa wchodzi pusta albo z niewłaściwą częścią.** Odznacz **Tylko artykuł** w oknie Strona internetowa, aby wziąć całą stronę, i usuń to, czego nie potrzebujesz. Stron, które wymagają logowania albo są w całości budowane skryptami, nie da się odczytać bez przeglądarki: otwórz stronę w przeglądarce, zaznacz tekst i przeciągnij go do segregatora (stanie się nowym dokumentem). „Secure web pages cannot be fetched” oznacza, że brakuje OpenSSL 3 (`sudo apt install libssl3`, część każdego Ubuntu) albo dodatek jest uszkodzony (zainstaluj go ponownie).

**Pierwsza sekunda pliku jest cicha albo czas na początku stoi w miejscu.** Wyjście dźwięku potrzebuje chwili na uruchomienie; przy niektórych serwerach dźwięku dłużej niż sekundy. Przewiń trochę wstecz (Ctrl+Alt+,), jeśli ominął cię początek.

**Strony w widoku strony nie pasują do mojego wydrukowanego manuskryptu.** Nie mają pasować: widok strony pokazuje twój tekst na arkuszach podczas pisania. Strony gotowego pliku wyznacza format kompilacji (sekcja 13).

**„Scrivener ma ten projekt otwarty.”** Scrivener zostawia w *.scriv* plik blokady, gdy ma projekt otwarty, i po awarii. Zamknij projekt w Scrivenerze (na każdym komputerze) i otwórz go tu ponownie; jeśli Scrivener nigdzie nie działa, wybierz **Tak**.

**Projekt w formacie Scrivener nie pokazuje moich najnowszych zmian ze Scrivenera.** {{APP}} czyta *.scriv* ponownie tylko przy otwarciu projektu. Zamknij go tutaj (**Plik > Zamknij projekt**), poczekaj, aż Scrivener lub usługa synchronizacji skończy, i otwórz go ponownie.

**Menu Wstaw, Nawigacja, Projekt, Dokumenty i Format są szare.** Żaden projekt nie jest otwarty. Otwórz lub utwórz jeden (menu Plik).

**Gdzie zgłosić problem.** **Pomoc > Zgłoś problem** otwiera w przeglądarce stronę zgłoszeń projektu (https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose), z formularzem dla problemu i dla życzenia; potrzebne jest konto GitHub i możesz pisać po angielsku lub niemiecku. Zgłoszenia są publiczne: nie dołączaj swojego manuskryptu ani projektu; **Pomoc > Rozwiązywanie problemów i programowanie > Informacje o systemie** (**Kopiuj do schowka**) zawiera to, czego zgłoszenie potrzebuje o programie i komputerze. **Problem bezpieczeństwa** nie jest zgłaszany tam, lecz prywatnie, jak opisano w *SECURITY.md* projektu (karta Security repozytorium, **Report a vulnerability**).

**Spis treści pliku Word nie ma numerów stron.** Numery stron uzupełnia edytor tekstu, gdy uaktualnia pole; uaktualnij je tam.

**Coś poszło nie tak z dokumentem.** Zajrzyj na kartę migawek w inspektorze albo przywróć kopię zapasową (sekcja 15).

## 21. Live Sync

**Czym jest.** Live Sync (0.99.0) utrzymuje jeden projekt jednakowy na kilku twoich urządzeniach w tej samej sieci (na przykład na komputerze stacjonarnym i laptopie albo na tablecie), dopóki są połączone, a kilka osób może pracować w jednym projekcie jednocześnie. Jest **wyłączony, dopóki go nie włączysz**: przy wyłączonym nic nie działa, nic nie nasłuchuje w sieci, a program nie pokazuje żadnego przycisku, karty ani znaku Live Sync. Działa w twojej własnej sieci (Wi-Fi albo kablowej); nie ma serwera ani konta. Pierwsza wersja działa w obrębie jednej sieci lokalnej.

**Włączanie: Plik > Opcje > Live Sync.** Strona ma **Włącz Live Sync**, **Nazwa tego urządzenia** (nazwa, którą widzą twoje pozostałe urządzenia), **Hostuj dalej po zamknięciu ostatniego okna** (na początku wyłączone: przy wyłączonym zamknięcie ostatniego okna projektu, nad którym pracują inni, najpierw pyta, a potem kończy hostowanie; przy włączonym hostowanie trwa dalej w małym programie działającym w tle, należącym do {{APP}}, który pokazuje okno „Hostowanie n projektów dla innych urządzeń” z przyciskami **Otwórz projekt** i **Zakończ hostowanie i zamknij program**, a przy następnym uruchomieniu {{APP}} hostowanie wraca do niego) oraz **Sparowane urządzenia** z przyciskami **Sparuj urządzenie...** i **Usuń**. Dalej są dwie zwijane części: **Więcej sposobów połączenia** (adresy i port tego urządzenia oraz **Połącz przez adres...** dla sieci, która nie ogłasza urządzeń) i **Szczegóły** (krótki odcisk, protokół i czasy odpowiedzi; nigdy żadna treść). Wiersz na tej stronie i na stronie Szyfrowanie mówi, gdzie to urządzenie przechowuje swoje klucze: w magazynie kluczy systemu (KWallet albo Secret Service, Menedżer poświadczeń Windows, Android Keystore) albo, gdy system go nie ma, w portfelu Quillbinder. Przełączniki działają od razu; nic nie trzeba potwierdzać przyciskiem OK.

**Portfel Quillbinder.** Komputer bez magazynu kluczy przechowuje swoje klucze we własnym portfelu: jednym pliku w folderze danych programu, zablokowanym głównym hasłem. Gdy po raz pierwszy trzeba zachować klucz, okno prosi o wybranie hasła i wpisanie go dwa razy; słabe hasło jest odrzucane z podaniem powodu, jak w kreatorze kluczy (co najmniej dwanaście znaków i nie do odgadnięcia). Potem program pyta o hasło raz przy każdym uruchomieniu, gdy potrzebny jest klucz. Nigdzie nie jest zapisane. Jeśli je zgubisz, klucze w portfelu przepadną: klucz urządzenia Live Sync zostanie wtedy utworzony od nowa, a urządzenia trzeba będzie sparować ponownie, i trzeba będzie ponownie wpisać hasło szyfrowania przechowywane w portfelu.

**Parowanie.** Urządzenia ufają sobie dopiero wtedy, gdy je sparujesz. Na jednym urządzeniu otwórz **Sparuj urządzenie...** i wybierz **Otwórz okno parowania na tym urządzeniu**: pozostaje otwarte przez dwie minuty i dopiero wtedy urządzenie jest znajdowane przez pozostałe i pokazuje swoją nazwę. Na drugim urządzeniu wybierz je na liście (albo **Połącz przez adres...**). Oba urządzenia pokazują teraz nazwę drugiego i te same sześć cyfr dużą czcionką. Porównaj je i naciśnij **Kody są zgodne** na obu urządzeniach, albo **Różnią się**, jeśli nie są takie same: ktoś inny może wtedy być między urządzeniami i nic nie zostaje sparowane. Zamknięcie okna niczego nie paruje. **Usuń** (na stronie Live Sync) odbiera urządzenie od razu: traci dostęp do każdego projektu tego urządzenia i trzeba je sparować ponownie.

**Udostępnianie i dołączanie.** Otwórz projekt i wybierz **Udostępnij ten projekt...** w menu przycisku **Live Sync** (pasek narzędzi; przycisk pokazuje się tylko przy włączonym Live Sync). To urządzenie jest wtedy **hostem** projektu: jego folder jest projektem. W **Projekty hostowane...** nadajesz każdemu sparowanemu urządzeniu dostęp: **Edytor**, **Czytelnik** (może tylko czytać) albo **Brak**. Na drugim urządzeniu wybierz **Dołącz do projektu...**: pokazuje projekty sparowanych urządzeń, które są włączone; urządzenie, które nie jest jeszcze sparowane, jest pokazane jako „Najpierw sparuj”. Dołączenie tworzy na tym urządzeniu pełną kopię (pokazuje to pasek postępu) i ją otwiera; kopia jest przechowywana w folderze danych programu, a nie tam, gdzie trzymasz własne projekty. Projektu w formacie Scrivenera nie można udostępnić. Dopóki projekt jest udostępniony, konwersje, które przepisują każdy plik (Markdown lub RTF, szyfrowanie, odszyfrowanie), są niedostępne: najpierw zakończ udostępnianie.

**Przycisk.** Jego ikona ma kształt i kolor, a tekst podaje stan: **Nieudostępniony**, **Aktualny**, **Wysyłanie zmian...**, **Brak połączenia (wersja robocza zachowana)** (hosta nie można osiągnąć: projekt jest tylko do odczytu, a to, co napisano, zostaje na tym urządzeniu), **Problem** (podpowiedź mówi jaki). Gdy nie jest otwarty żaden udostępniony projekt, pokazuje najgorszy stan ze wszystkich twoich udostępnionych projektów. Menu: **Udostępnij ten projekt...**, **Dołącz do projektu...**, **Poproś o kopię zapasową**, **Przekaż hostowanie...**, **Projekty hostowane...**, **Wersje robocze do odzyskania...** i **Zakończ udostępnianie** (przy dołączonym projekcie **Opuść**); polecenie jest włączone tylko tam, gdzie może zadziałać.

**Kto co edytuje.** Dokument jest edytowany na jednym urządzeniu naraz. Pierwszy wpisany znak albo wklejenie przejmuje blokadę dokumentu; pozostałe urządzenia widzą go tylko do odczytu. W segregatorze dokument, który edytuje inne urządzenie, ma kłódkę („Edytowany na urządzeniu ...”), dokument, który edytujesz sam, ma małe pióro, a dokument, którego posiadacz jest nieosiągalny, ma przyciemnioną kłódkę („Nieosiągalne od ...”). Przesunięcia i inne zmiany struktury, ustawienia projektu, obiekt albo mapa myśli są blokowane tak samo, ale tylko w czasie ich wykonywania. Blokada nie jest oddawana sama: jeśli zostawisz dokument otwarty, inni proszą o niego.

**Prośba o edycję.** Przy dokumencie zajętym gdzie indziej pasek mówi „Tylko do odczytu: dokument jest edytowany na urządzeniu ...” i ma przycisk **Poproś o edycję**. Drugie urządzenie widzi „Urządzenie ... prosi o możliwość edycji „...”.” z przyciskami **Zapisz i przekaż** (zapisuje, puszcza dokument i możesz pisać) oraz **Odrzuć**. Odrzucenie mówi ci „Nad „...” pracuje się na urządzeniu ...” i zaczyna od nowa czas oczekiwania 20 sekund; możesz anulować albo poprosić ponownie. Gdy czas minął bez zgody (także gdy drugiego urządzenia nie można osiągnąć), pasek proponuje **Przejmij**: najpierw powstaje migawka ostatniego zapisanego tekstu dokumentu, a to, co drugie urządzenie napisało i nie zapisało, zostaje tam jako wersja robocza do odzyskania. Karta **Live Sync** inspektora pokazuje to samo dla zaznaczonego dokumentu (**Poproś o edycję**, **Przejmij**, **Zakończ edycję**), a także kto bierze udział i co każdy edytuje, ostatnią kopię zapasową i liczbę wersji roboczych do odzyskania.

**Kopie zapasowe.** Kopie zapasowe tworzy tylko host, według zwykłych zasad (sekcja 15). Na innym urządzeniu **Poproś o kopię zapasową** (menu albo karta inspektora) prosi o nią hosta; karta pokazuje, czy kopia zapasowa jest tworzona, gotowa, oczekuje, jest niekompletna albo się nie powiodła, oraz kiedy ostatnia się udała. Kopia zapasowa nigdy nie zawiera własnych plików roboczych Live Sync, a przywrócona kopia zapasowa jest nowym projektem, a nie projektem udostępnionym. Gdy host tworzy kopię zapasową, możesz pracować dalej: kopia jest brana z kopii projektu, która jest kompletna w jednej chwili, więc nigdy nie zawiera połowy zmiany. Kopia zapasowa, której nie udało się utworzyć, mówi dlaczego i nigdy nie jest pokazana jako gotowa; wcześniejsze kopie zostają. **Migawki** projektu udostępnionego także tworzy host: tworzenie, zmiana nazwy, usuwanie i cofanie działają jak zwykle, ale są wykonywane na hoście i potrzebują jego odpowiedzi (cofnięcie wymaga blokady edycji dokumentu, a migawka jest odrzucana, gdy dokument zajmuje ktoś inny). Migawki pojawiają się na każdym urządzeniu.

**Gdy host zostanie utracony.** Urządzenia, które mogą hostować, przejmują hostowanie w kolejności, którą ustawisz w **Projekty hostowane...** (**W górę**, **W dół**; domyślnie komputery stacjonarne, tablet lub czytelnik tylko wtedy, gdy jest to dozwolone). **Przekaż hostowanie...** celowo przekazuje hostowanie innemu urządzeniu, które jest włączone. Urządzenia, które nie są członkami projektu, nigdy nie przejmują hostowania: jeśli któreś spróbuje, dowiesz się, że nieznane urządzenie próbowało i zostało odrzucone. Jeśli dwaj hosty pracowali osobno, zostaje ten z późniejszym uprawnieniem, a drugi przekazuje swoje dodatkowe zmiany jako wersje robocze do odzyskania.

**Wersje robocze do odzyskania.** Zmiany, których nie udało się wysłać (host utracony na stałe, przejęcie, dwaj hosty, którzy pracowali osobno), są przechowywane na urządzeniu, które je napisało, i nigdy nie są usuwane samoczynnie. **Wersje robocze do odzyskania...** wymienia je z czasem, dokumentem i powodem (na przykład „niewysłana przed przejęciem”). **Porównaj** pokazuje wersję roboczą obok dokumentu w widoku porównania, **Zachowaj jako kopię** robi z niej dokument, **Przejmij** wstawia ją do dokumentu (po jego zablokowaniu i wykonaniu migawki obecnego tekstu), a **Usuń** usuwa ją po zapytaniu.

**Co nie jest szyfrowane.** Połączenia Live Sync są szyfrowane (TLS 1.3), a każde urządzenie jest znane po swoim kluczu, a nie po nazwie. Same pliki to inna sprawa: projekt, który **nie** jest zaszyfrowany (sekcja 15), jest przechowywany jako zwykłe pliki na każdym urządzeniu, które ma kopię, czytelne dla każdego, kto może czytać dysk tego urządzenia, a **kopie zapasowe ZIP takiego projektu to także zwykłe pliki ZIP**. Live Sync tego nie zmienia. **Użyj własnego szyfrowania swoich urządzeń** (szyfrowanie całego dysku w twoim systemie; na Androidzie szyfrowanie pamięci urządzenia) i trzymaj kopie zapasowe na **zaszyfrowanym dysku**. Jeśli chcesz zablokować same pliki, zaszyfruj projekt przed udostępnieniem: udostępniony zaszyfrowany projekt pozostaje zaszyfrowany na każdym urządzeniu, a urządzenie potrzebuje jednego ze swoich kluczy, aby dołączyć.

**Wyłączanie.** Wyłączenie **Włącz Live Sync** zatrzymuje nasłuchiwanie i połączenia; udostępnione projekty zostają na każdym urządzeniu takie, jakie są, i są kopiami tylko do odczytu, dopóki nie włączysz go ponownie albo nie wybierzesz **Zapisz jako projekt niezależny...**.

## Autorzy i licencja

- Główny programista: Claude.AI w. Opus 5.5 Extra

{{APP}} jest wolnym oprogramowaniem na licencji Apache License 2.0: każdy może go używać, kopiować, zmieniać i przekazywać dalej, odpłatnie lub nie, zachowując licencję i plik NOTICE, który nazywa projekt i jego źródło; bez gwarancji. Wersje do 0.70.0 były publikowane na licencji Zero-Clause BSD (0BSD). Okno O programie (w menu Pomoc) pokazuje wersję i licencję.

{{APP}} korzysta z części innych projektów (Qt, Hunspell, ICU, ikony Breeze i inne biblioteki wewnątrz AppImage), każda na własnej licencji. **Pomoc > Licencje stron trzecich** (także przycisk okna O programie) wymienia je z pełnymi tekstami licencji; te same teksty są wewnątrz AppImage, w *usr/share/doc/quillbinder/licenses*. Kod źródłowy każdej z nich, dokładnie w użytej wersji, jest w wydaniu **Third-party source code** na stronie projektu (https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources); okno podaje plik dla każdej biblioteki, a noty licencyjne dodatków robią to samo dla swoich.

Scrivener i Scapple to nazwy produktów firmy Literature & Latte. {{APP}} jest niezależnym programem, niewykonanym przez Literature & Latte ani z nią niezwiązanym; czyta i zapisuje ich formaty plików, aby twoje projekty mogły przechodzić między programami.
