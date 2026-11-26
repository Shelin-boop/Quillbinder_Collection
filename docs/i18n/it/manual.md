# Manuale utente di {{APP}}

Questo manuale è stato tradotto automaticamente e non è ancora stato controllato da una persona di madrelingua italiana. Dove qualcosa non è chiaro, fa fede il manuale in inglese.

{{APP}} è uno studio di scrittura per testi lunghi: romanzi, sceneggiature, drammi teatrali, fumetti, tesi e saggi. Scrivi in piccoli pezzi (scene, sezioni, note), li disponi come vuoi e alla fine li *compili* in un unico manoscritto.

Questo manuale descrive la versione {{VERSION}}. Premi **F1** in qualsiasi momento per aprirlo: si apre nel punto che corrisponde a ciò che stai guardando, cioè il documento, la bacheca, la struttura o il Testo combinato, il raccoglitore, la scheda dell'ispettore che ha la tastiera oppure la finestra di dialogo aperta (una finestra di dialogo senza una parte propria apre l'inizio). A sinistra la finestra ha un **indice** (un clic su un titolo ti porta lì) e sopra il testo un **campo di ricerca**: premi **Ctrl+F** per andarci, scrivi una parola e **Return** mostra il risultato successivo, **Shift+Return** quello precedente (i pulsanti **Successivo** e **Precedente** fanno lo stesso); la riga accanto al campo dice quale risultato di quanti viene mostrato, e ogni risultato è evidenziato nel testo. Ciò che è destinato a chi sviluppa {{APP}} (plugin, pacchetti, la cartella del progetto nei dettagli, variabili d'ambiente) si trova nella guida per sviluppatori, **Aiuto > Risoluzione problemi e sviluppo > Guida per sviluppatori: aggiornamenti e componenti aggiuntivi**.

---

## 1. Primi passi

### Come ottenere il programma

{{APP}} si scarica dalla pagina **Releases** della sua pagina di progetto, https://github.com/Shelin-boop/Quillbinder_Collection/releases. Ogni release contiene il programma come un unico file (*{{APP}}-&lt;version&gt;-x86_64.AppImage*), un aggiornamento del programma per chi ha già un AppImage di {{APP}} (sezione 17) e i componenti aggiuntivi (file *.qbpkg*, sezione 17). Rendi eseguibile l'AppImage (clic destro > **Proprietà** > **Permessi** > *È eseguibile*, oppure `chmod +x` in un terminale) e fai doppio clic su di esso; **Aiuto > Aggiungi al menu delle applicazioni** lo inserisce nel menu del tuo desktop. Il file *SHA256SUMS.txt* di ogni release elenca le somme di controllo dei suoi file.

**{{APP}} è software {{LABEL}}.** Fai dei backup dei tuoi progetti (sezione 15) e tieni una copia dei tuoi manoscritti anche fuori dal programma.

### Iniziare un progetto

Quando {{APP}} si avvia senza un progetto, si apre la **finestra iniziale**:

- **Nuovo progetto**: scegli una categoria e un modello (per esempio Vuoto, Romanzo, Romanzo con parti o Racconto), scrivi un nome, scegli dove salvare e fai clic su **Crea**. **Formato** decide come viene memorizzato il progetto: come progetto {{APP}} (lo standard) oppure come progetto Scrivener 3 (*.scriv*), così da poterlo aprire anche in Scrivener (sezione 14, *Lavorare nel formato Scrivener*).
- **Progetti recenti**: fai doppio clic su un progetto su cui hai lavorato prima.
- **Apri altro…**: trova un progetto sul disco.
- **Importa progetto Scrivener…**: crea un progetto {{APP}} da un progetto Scrivener (vedi sezione 14). Per continuare a lavorare sul progetto Scrivener stesso, apri invece il suo file *.scrivx* con **Apri altro…**.

{{APP}} riapre l'ultimo progetto all'avvio (puoi disattivarlo in **File > Opzioni > Generale**).

**Il primissimo avvio** mostra una sola pagina con tutto già compilato (**OK** lo conferma): i backup (se un progetto viene salvato in un backup alla chiusura, quanti backup conservare, all'inizio 25, e dove), da un AppImage Linux la domanda se aggiungere {{APP}} al menu delle applicazioni (e se spostare l'AppImage in *~/Applications*) e l'avviso sul controllo degli aggiornamenti con il suo interruttore. Il progetto di benvenuto si apre quando premi **OK**; prima non compare alcuna finestra vuota del programma. Tutto può essere cambiato più tardi in **File > Opzioni**; lì, in **Generale > Avvio**, c'è anche la scelta di quali impostazioni valgano per prime, quelle del progetto o sempre quelle del programma (sezione 16, *Impostazioni per un progetto*).

### Il progetto di benvenuto e la Guida rapida

La prima volta che {{APP}} si avvia, apre il **progetto di benvenuto**: un breve progetto che mostra, un documento per volta, ciò che {{APP}} offre oltre l'ordinario (la barra delle icone, uno zoom per ogni parte della finestra, le mappe mentali, le schede degli oggetti, i commenti di un'intera cartella, Confronta e trasferisci, i progetti Scrivener, i temi, gli aggiornamenti e i componenti aggiuntivi, e ciò che protegge il tuo testo). Comincia dal documento **Benvenuto** in cima al raccoglitore; i suoi collegamenti portano a ogni tappa del percorso. Nulla di ciò che fai nel progetto di benvenuto può danneggiare i tuoi progetti. (Un progetto indicato sulla riga di comando, o un file di progetto aperto con doppio clic, si apre al suo posto; il progetto di benvenuto arriva allora al prossimo avvio.)

Il progetto di benvenuto viene salvato come "Benvenuto in {{APP}}" nella tua cartella *Documenti*. **Aiuto > Progetto di benvenuto…** lo riapre come l'avevi lasciato, oppure crea una copia nuova per ricominciare da capo (**Crea una copia nuova**; la vecchia copia va nel cestino del desktop). Eliminalo quando vuoi; il comando ne crea uno nuovo.

**Aiuto > Guida rapida** mostra in una sola pagina dove si trovano le impostazioni importanti (temi e modalità scura, colori e caratteri, formattazione, zoom, barre degli strumenti, scorciatoie, backup ...) e riassume ciò che {{APP}} sa fare.

### Salvataggio

Non hai mai bisogno di salvare a mano. {{APP}} salva automaticamente alcuni secondi dopo che smetti di scrivere, quando passi a un altro programma e quando chiudi il progetto. Se continui a scrivere, salva alla prima breve pausa (un respiro tra due parole) una volta trascorsi 5 secondi, e al più tardi dopo 10 secondi anche senza alcuna pausa. **Ctrl+S** salva subito, se vuoi esserne sicuro.

Ogni file viene scritto prima con un nome temporaneo e sostituisce il vecchio solo quando è stato scritto per intero e il computer lo ha messo sul disco. Se il disco è pieno o l'unità si guasta durante il salvataggio, il vecchio file resta com'era e le tue modifiche restano nella finestra finché il salvataggio non funziona di nuovo.

**Lo stato di salvataggio è sempre sullo schermo** (0.70). La barra a piè di pagina del primo editor (il tratto vuoto tra il totale della cartella e il conteggio delle parole, oppure a sinistra nelle altre barre) dice **Salvato**, **Non ancora salvato** (alcuni secondi dopo che hai scritto) oppure, su fondo rosso, **Non salvato: l'ultimo salvataggio non è riuscito alle 14:03**; finché i salvataggi falliscono, anche il titolo della finestra porta un segno. Un salvataggio non riuscito mostra anche un messaggio che nomina i documenti non salvati e offre **Salva i testi altrove…**; non prende mai la tastiera, quindi il prossimo tasto che premi va ancora nel tuo testo. Il programma riprova da solo (all'inizio ogni 15 secondi, poi ogni minuto), il messaggio ricompare dopo cinque minuti di insuccessi e il segno rosso resta finché un salvataggio non è riuscito. Le mappe mentali vengono salvate e controllate allo stesso modo: una lavagna che non può essere salvata resta nella finestra, viene contata come non salvata quando chiudi il progetto e finisce nella copia di emergenza.

**Messaggi brevi** (0.93.0). In fondo alla finestra non c'è alcuna riga di stato. I messaggi che prima comparivano lì ("Salvato di nuovo: tutte le modifiche sono sul disco.", l'esito di un backup e simili) compaiono per qualche secondo nello stesso posto del testo di salvataggio, nella barra a piè di pagina del primo editor (quello di sinistra o superiore quando ce ne sono due), e dopo di essi torna il testo di salvataggio. Un messaggio troppo lungo per lo spazio viene accorciato; fermaci sopra il puntatore per leggerlo per intero. In un editor molto stretto il testo è sostituito da un piccolo segno colorato (verde salvato, giallo non ancora salvato, rosso non riuscito) con il testo come suggerimento. In modalità composizione, e finché nessun progetto è aperto, non viene mostrato nulla.

**File su cui {{APP}} non scrive** (0.70). Un file di testo danneggiato (un file RTF il cui inizio è rotto o che è troncato) oppure salvato da un altro programma con un'altra codifica dei caratteri (per esempio un file Markdown salvato come Windows-1252 o UTF-16, magari dopo averlo modificato fuori dal programma) viene mostrato per quanto può essere letto e non può essere modificato; un messaggio lo dice e il file non viene mai cambiato. Copia il suo testo in un nuovo documento per continuare, oppure salva il file in UTF-8 nel programma che lo ha scritto, poi chiudi e riapri il progetto; oppure riportalo a una delle sue istantanee (dalla 0.81.1): il file danneggiato viene prima conservato come copia in *settings/conflicts* nella cartella del progetto, e il testo dell'istantanea diventa il testo del documento, che può essere modificato e viene salvato di nuovo. Unisci documenti e Dividi rifiutano un documento simile indicandone il motivo, mentre Sostituisci nel progetto, Converti virgolette e le modifiche di stile lo lasciano com'è e lo nominano. Se un comando lo cambia ugualmente, il testo modificato non va perso: viene conservato come file RTF in *settings/conflicts* e un messaggio dice dove (0.81.1). Un file di mappa mentale che non può essere letto viene trattato allo stesso modo. I caratteri che non possono essere memorizzati (caratteri di controllo provenienti da dati incollati) vengono omessi come prima; ora un breve messaggio nella barra a piè di pagina lo dice una volta per documento.

**Un salvataggio arriva per intero** (0.65). Quando un salvataggio scrive più file (il testo di un nuovo documento e il file di progetto, un testo e le sue immagini, la scheda Oggetti), tutti vengono prima scritti accanto ai loro posti, poi elencati in un piccolo giornale (*settings/journal.json*) e solo allora messi al loro posto insieme. Se il programma o il computer si ferma a metà, la prossima apertura completa quel salvataggio dal giornale oppure, se il giornale non era ancora completo, lascia il progetto com'era al salvataggio precedente; un avviso dice quale dei due casi. Un giornale che non può essere letto (un guasto del disco) non viene mai scambiato per uno vuoto: esso e i file del suo salvataggio vengono conservati in una cartella in *settings/conflicts*, nulla viene messo al suo posto né eliminato, e l'avviso lo dice (0.81.1). In una cartella sincronizzata, il giornale di un salvataggio dell'altro computer viene lasciato stare per 15 minuti (quel salvataggio potrebbe essere ancora in corso lì, o i suoi file potrebbero essere ancora in arrivo): nel frattempo questo computer non salva il progetto e riprova da solo; se l'altro computer si è fermato durante quel salvataggio, questo lo completa dopo quei 15 minuti (0.81.1). Se un file di un salvataggio non può essere scritto, nessuno di essi sostituisce qualcosa e le tue modifiche restano nella finestra per il tentativo successivo. Dalla 0.93.0 un'unità che segnala un errore quando una cartella viene sincronizzata dopo che i file sono stati messi al loro posto (un vero errore, non un file system che non può sincronizzare) fa contare il salvataggio come non riuscito: il giornale resta, il messaggio dice che l'unità ha segnalato un errore e il salvataggio o l'apertura successivi completano il salvataggio dal giornale. Finché un giornale che non può essere letto non ha potuto essere messo da parte (la cartella *settings/conflicts* non è scrivibile), nessun salvataggio scrive un nuovo giornale sopra di esso: il salvataggio aspetta, il suo messaggio nomina il file e il programma salva da solo non appena il file può essere messo da parte. Un file temporaneo di un salvataggio precedente che è di intralcio viene spostato da parte, mai sovrascritto. Una sincronizzazione della cartella che fallisce prima che qualcosa venga sostituito è un fallimento pulito: nulla è stato cambiato. Quando l'errore arriva dopo che i file sono al loro posto, il programma annota ciò che c'è sul disco (così i nuovi tentativi non segnalano falsi conflitti "modificato altrove" e non scrivono due volte le immagini) e sincronizza di nuovo la cartella a ogni salvataggio: "Salvato" segue sempre una sincronizzazione della cartella che è stata controllata. Un file system che risponde con un errore a una sincronizzazione della cartella già all'apertura del progetto viene ricordato come uno che non può sincronizzare le cartelle, e i suoi salvataggi non vengono segnalati come falliti.

Se esci dalla sessione o spegni il computer mentre {{APP}} è aperto, il programma salva tutto prima di terminare (in quel caso non viene fatto alcun backup). Se quel salvataggio fallisce (un disco pieno, un'unità scomparsa), i testi e le note non salvati (e, dalla 0.93.0, il file di progetto così com'è in memoria) vengono scritti come file RTF in una cartella di emergenza, *~/.local/share/{{APP}}/Emergency*, e il prossimo avvio dice dove si trovano e quanti file sono stati scritti (l'avviso dice "incompleto" e nomina ciò che manca quando un file non ha potuto essere scritto). Se termina in modo imprevisto, il prossimo avvio te lo dice (sezione 20, *Rapporti di arresto anomalo*).

**Chiudere un progetto che non può essere salvato** chiede cosa fare; **Mantieni aperto** è l'impostazione predefinita. **Salva i testi altrove…** scrive i testi e le note non ancora salvati come file RTF in una cartella a tua scelta (un'altra unità, una chiavetta USB), e il progetto resta aperto. Dalla 0.93.0 questa copia di emergenza contiene anche il file di progetto così com'è nella finestra (il raccoglitore, i titoli e le etichette), le lavagne delle mappe mentali e gli oggetti modificati, e viene creata anche quando sono cambiati solo il raccoglitore o gli Oggetti. Il messaggio dice quanti file sono stati scritti, nella forma "n di m", e nomina i file mancanti e il perché; una copia a cui mancano file reca l'intestazione **incompleta** (con un segnale di avvertimento) e non sembra mai una copia finita. Una cartella già esistente non viene mai riutilizzata: una seconda copia nello stesso secondo riceve un numero. Il file di progetto nella copia si chiama come il file di progetto con ".copy" aggiunto (così la cartella non viene scambiata per un progetto per errore); il README nella copia spiega come rimetterla a posto: chiudi il progetto, rinomina il file di progetto nella cartella del progetto (resta com'era), copia lì il file della copia di emergenza, rinominalo con il nome del file di progetto e riapri il progetto. Per un progetto in formato Scrivener il README dice che il file appartiene alla copia di lavoro e non indica alcun modo di tornare indietro. Quando alla fine non c'era nulla di non salvato (nel frattempo un nuovo tentativo è riuscito), il messaggio dice che tutto è salvato e che non serviva alcuna copia. **Chiudi senza salvare** lo chiude e perde quelle modifiche.

### Dove vive il tuo lavoro

Un progetto è una cartella chiamata *TuoTitolo*{{SUFFIX}}. All'interno, ogni documento è un normale file di testo, così la tua scrittura resta leggibile anche senza {{APP}}. Fai doppio clic sul file *TuoTitolo*{{FILE}} dentro la cartella per aprire il progetto. Non rinominare né spostare a mano i file all'interno della cartella del progetto.

Per trovare il file di un documento fuori da {{APP}}, apri *contents-by-title.txt* nella cartella del progetto (0.65): elenca ogni documento del raccoglitore con il suo titolo, rientrato in base alla posizione, e i file che contengono il suo testo, le sue note o i suoi file multimediali (per esempio *content/<id>/text.md*). {{APP}} lo scrive a ogni salvataggio che cambia il raccoglitore e non lo legge mai da solo. (Un progetto in formato Scrivener non ha un elenco simile; il file *.scrivx* di Scrivener nomina i suoi documenti.)

Un progetto dovrebbe essere aperto in una sola finestra alla volta. Se è già aperto in un'altra finestra o su un altro computer (per esempio tramite una cartella sincronizzata), {{APP}} ti avvisa prima di aprirlo. Se avvii di nuovo {{APP}} con un progetto che è aperto in una finestra del programma (dal gestore file o da un collegamento sul desktop), porta in primo piano quella finestra invece di aprire il progetto due volte; un altro progetto si apre in una finestra propria.

**Un progetto in una cartella sincronizzata** (OneDrive, Dropbox, Nextcloud, Syncthing o qualsiasi altro programma di sincronizzazione) può essere usato su più computer, uno alla volta. Chiudilo su un computer e lascia che il programma di sincronizzazione finisca prima di aprirlo sull'altro. {{APP}} aiuta in questo a livello di file, qualunque sia il programma di sincronizzazione:

- Mentre un progetto è aperto, un piccolo file di blocco al suo interno nomina il computer e viene rinnovato ogni pochi minuti. Contiene il nome del computer, il tuo nome utente su di esso, la versione e il numero di processo del programma, quando il progetto è stato aperto e quando il blocco è stato rinnovato l'ultima volta, e un codice che distingue i computer (un hash, non leggibile come nome). Chiunque abbia accesso alla cartella condivisa può vederli. In **File > Opzioni > Generale > Salvataggio**, **Nome di questo computer nelle cartelle condivise** imposta un nome tuo ("Portatile"); viene usato al posto del nome del computer nei file di blocco, nei nomi delle versioni conservate e in *settings/ui-<computer>.json*, e il tuo nome utente viene allora omesso. Su un altro computer, aprendo il progetto compare quindi il nome di quel computer, da quando ha il progetto aperto e il suo ultimo segno di vita; **Apri comunque** o **Annulla**. Un blocco che non è stato rinnovato da più di 15 minuti (il progetto non è stato chiuso lì, o quel computer è spento) viene mostrato come residuo e **Apri** è l'impostazione predefinita.
- Se il progetto viene aperto su un altro computer mentre è aperto qui, un avviso lo dice e offre di chiuderlo qui.
- Se un file del progetto è stato modificato altrove (su un altro computer tramite il programma di sincronizzazione, o da un altro programma) dopo che {{APP}} lo ha letto, ed è stato modificato anche qui, l'altra versione viene conservata prima che venga salvata la tua: come **istantanea** del documento (confrontala nella scheda Istantanee dell'ispettore) e come copia nella cartella *settings/conflicts* del progetto. Un avviso dice quali documenti. Se l'altra versione non può essere conservata (per esempio perché il disco è pieno o *settings/conflicts* non è scrivibile), la tua non viene salvata sopra di essa: il salvataggio fallisce con un messaggio che nomina il file e le tue modifiche restano non salvate finché l'altra versione non può essere conservata. Un documento che hai solo guardato qui non viene mai salvato sopra una versione più recente proveniente da altrove.
- Aprire e chiudere un progetto senza cambiare nulla non modifica nessuno dei suoi file; la disposizione della finestra e i punti in cui eri in ogni documento vengono conservati per computer (*settings/ui-<computer>.json*). Nel formato Scrivener nemmeno il *.scriv* viene toccato, e mentre è aperto riceve un file di blocco di {{APP}} (*quillbinder.lock*); Scrivener non lo legge. Prima di ogni salvataggio nel *.scriv*, {{APP}} cerca il blocco di Scrivener stesso: se Scrivener ha aperto il progetto dopo {{APP}}, nel *.scriv* non viene scritto altro (un avviso lo dice una volta, poi un breve messaggio nella barra a piè di pagina); le tue modifiche restano nella copia di lavoro di {{APP}}. Chiudi il progetto in Scrivener e salva di nuovo (**File > Salva**). Se chiudi prima il progetto qui, le modifiche vanno nel *.scriv* alla prossima apertura qui; dove nel frattempo Scrivener ha cambiato qualcosa, viene letta la sua versione.
- Se il progetto elenca il testo di un documento il cui file non è ancora arrivato su questo computer (il programma di sincronizzazione non ha finito), dopo l'apertura un avviso lo nomina; quel documento viene mostrato vuoto, non può essere modificato e al suo posto non viene salvato nulla. Chiudi il progetto, aspetta che il programma di sincronizzazione abbia finito e riaprilo. Lo stesso vale per un testo che ha potuto essere letto solo in parte (0.65).
- Un file modificato altrove viene notato anche quando ha mantenuto dimensione e data (alcuni programmi di sincronizzazione impostano la data da soli): {{APP}} confronta il suo contenuto con ciò che ha letto o scritto (0.65).
- Il contenuto decide anche nell'altra direzione (0.92.0): un file la cui **sola data** è cambiata non è un'altra versione. Alcuni programmi di sincronizzazione reimpostano la data di un file qualche secondo dopo che {{APP}} lo ha scritto, quando il loro caricamento è finito; per questo non viene conservato nulla, non arriva alcun avviso e il file non viene riscritto. Anche un file di progetto che contiene già ciò che un salvataggio scriverebbe viene lasciato stare, così un salvataggio che non cambia nulla non dà al programma di sincronizzazione niente da trasportare.
- L'avviso **Altre versioni conservate** ha il pulsante **Non mostrare più finché {{APP}} non viene chiuso**. Le altre versioni vengono comunque conservate come istantanee e come copie in *settings/conflicts*; solo l'avviso non compare più finché non esci dal programma. Il prossimo avvio lo mostra di nuovo.
- **Un lucchetto su ciò a cui lavora un altro dispositivo (0.92.0).** Mentre un progetto è aperto, ogni dispositivo annota anche, accanto al file di blocco, quali documenti ha aperti in un editor o ha modificato da quando ha aperto il progetto. Quando un secondo dispositivo apre il progetto, il suo raccoglitore mostra un piccolo lucchetto all'estremità destra delle righe di quei documenti, e sulle cartelle che li contengono. Fermaci sopra il puntatore: il suggerimento nomina il dispositivo e quando ha aperto il progetto. È un'indicazione, non una protezione: nulla viene bloccato e puoi comunque modificare il documento. Il segno viaggia attraverso il tuo programma di sincronizzazione, quindi può avere alcuni minuti; richiede la 0.92.0 o più recente su entrambi i dispositivi (Scrivener non annuncia nulla). Un elenco il cui dispositivo non ha dato segni di vita da più di 15 minuti non viene mostrato. La nota è un piccolo file per dispositivo (*settings/lock-work-…json*; per un progetto in formato Scrivener *quillbinder.lock-work-…json* nel *.scriv*); contiene il nome del dispositivo come nel file di blocco, l'ora e i numeri interni dei documenti, nessun titolo e nessun testo. Viene scritta solo quando l'elenco cambia, mai a ogni battitura, viene rimossa quando il progetto è chiuso e non è inclusa nei backup.
- **Formato Scrivener, due computer** (0.65): i documenti aggiunti al *.scriv* altrove (Scrivener su un altro computer) mentre il progetto è aperto qui vengono conservati, e il raccoglitore che li nomina non viene sostituito; un avviso ti chiede di chiudere il progetto e riaprirlo, il che mostra sia i loro documenti sia i tuoi. Immagini, PDF, file audio e video e mappe mentali modificati altrove nel *.scriv* vengono conservati così come sono (prima la versione di qui veniva copiata sopra di essi); quando uno è stato modificato su entrambi i lati, l'altra versione viene prima conservata in *settings/conflicts*.

Un progetto in **formato Scrivener** è invece una cartella *TuoTitolo.scriv*; aprilo con il suo file *.scrivx*. {{APP}} salva le tue modifiche al suo interno, così il progetto resta un progetto Scrivener. **File > Salva con nome…** salva un progetto con un altro nome o in un altro posto, in entrambi i formati (sezione 14).

### Privacy: cosa lascia il tuo computer

{{APP}} contatta internet per un solo motivo: cercare aggiornamenti. Scarica un piccolo file, non invia nulla su di te e puoi disattivarlo nel menu Aiuto. (La sezione 17, *Controllo degli aggiornamenti*, dice esattamente cosa viene scaricato e quando. Prima della 0.92.0 il programma non cercava affatto aggiornamenti.) Altre tre cose vanno su internet, e solo quando le avvii tu:

- **File > Importa > Pagina web…** scarica la pagina che indichi e le sue immagini da quel sito. Come un browser, invia il nome di un browser comune (Firefox su Linux), le lingue del tuo sistema (per la lingua della pagina) e, come ogni connessione, l'indirizzo internet del tuo computer.
- **Scaricare un modello linguistico** per il componente aggiuntivo Assistente di traduzione (sezione 10) lo preleva dalle release del repository di {{APP}} su GitHub. Invia il nome del componente ("Quillbinder-Translation-Addon") e, come ogni connessione, il tuo indirizzo internet.
- **Scaricare un modello vocale** (il componente aggiuntivo Trascrizione vocale, sezione 10) preleva il modello da Hugging Face (huggingface.co, un'azienda negli USA) o dall'indirizzo che imposti nelle sue opzioni. Invia "{{APP}}/<version>" come nome del programma e, come ogni connessione, il tuo indirizzo internet. Un modello viene installato solo se la sua somma di controllo è quella nota; un file da un altro indirizzo o uno che hai scaricato tu stesso viene usato solo dopo il tuo consenso.

Ciò che gli altri possono vedere del tuo lavoro:

- I **progetti nelle cartelle condivise** portano un file di blocco mentre sono aperti (vedi sopra): nome del computer, nome utente e orari. Imposta un nome tuo in **File > Opzioni > Generale > Salvataggio** per ometterli.
- Le **informazioni sull'autore** (nome, indirizzo, telefono, e-mail in **File > Opzioni**) restano nelle impostazioni del programma su questo computer; non finiscono mai nelle impostazioni del progetto. Arrivano in un manoscritto solo dove il formato di compilazione usa i segnaposto (<$author>, <$address> ...).
- **Il testo eliminato non è sparito ovunque**: le istantanee conservano le versioni precedenti di un documento (**Progetto > Elimina istantanee vecchie** le rimuove) e i backup conservano l'intero progetto com'era (sezione 15). I backup sono normali file ZIP senza password; una cartella dei backup dentro una cartella sincronizzata viene caricata dal programma di sincronizzazione.
- **I tuoi file non sono cifrati da {{APP}}** (0.65): testi, note, impostazioni del progetto e backup sono normali file, leggibili da chiunque possa leggere la cartella in cui si trovano. Per mantenere privato un manoscritto, tieni il progetto e i suoi backup su un disco cifrato (il programma di installazione di Kubuntu offre di cifrare l'intero disco; per una chiavetta USB o un'unità esterna, i Plasma Vault di KDE o un'unità cifrata con LUKS) e tieni la cartella dei backup fuori dalle cartelle condivise che non controlli.
- I **rapporti di arresto anomalo** restano su questo computer finché non ne copi o invii uno tu stesso (sezione 20). Descrivono lo stato del programma e nominano il progetto aperto e la sua cartella. Su Windows accanto a ogni rapporto viene conservato un dump della memoria; può contenere testo che stavi scrivendo, e il rapporto lo dice.

### Aggiungere {{APP}} al menu delle applicazioni

La prima volta che avvii il file del programma ({{APP}}.AppImage), la pagina del primo avvio offre di aggiungere {{APP}} al menu delle applicazioni. Dopo di che puoi avviarlo come qualsiasi altro programma, e un doppio clic su un progetto lo apre. Nessuna finestra conferma la voce; un errore viene segnalato. La stessa pagina (e **Aiuto > Aggiungi al menu delle applicazioni**) offre di **spostare l'AppImage in ~/Applications** (inizialmente spuntato quando il file non è ancora lì), così che la voce di menu lo avvii da un posto fisso in cui il file resta (la casella lo dice con queste parole, senza parentesi); se lo spostamento fallisce (un disco pieno, un file con quel nome già presente), nulla va perso: il file resta dov'era, la voce punta lì e un messaggio dice perché. Se in seguito sposti tu stesso l'AppImage, la voce nomina ancora il vecchio posto: usa di nuovo **Aiuto > Aggiungi al menu delle applicazioni**, che scrive il posto attuale del file (una voce più vecchia viene sostituita, non raddoppiata). Dopo uno spostamento, gli aggiornamenti del programma, la copia *.before-update* e **Aiuto > Risoluzione problemi e sviluppo > Informazioni di sistema** usano subito il nuovo posto.

L'unico comando del menu Aiuto, **Aiuto > Aggiungi al menu delle applicazioni**, segue lo stato della voce: aggiunge la voce finché non ce n'è alcuna e, una volta che c'è, si chiama *Rimuovi dal menu delle applicazioni* e toglie ciò che l'aggiunta ha creato (il file della voce, l'icona e i tipi di file) lasciando l'AppImage dov'è. Se sposti tu il file del programma, rimuovi la voce e aggiungila di nuovo.

### Su Windows: tutto in una cartella

La versione Windows di {{APP}} (sperimentale, pubblicata dalla 0.91.1) è una cartella che decomprimi dove vuoi, per esempio *C:\Users\<il tuo nome>\Quillbinder*; non richiede installazione né diritti di amministratore. Conserva le sue impostazioni, i componenti aggiuntivi, gli aggiornamenti del programma, i caratteri installati e i rapporti di arresto anomalo nella cartella *data* all'interno di quella cartella, e nulla nelle cartelle di Windows o nel registro. L'intera cartella può essere spostata o copiata (su una chiavetta USB o un altro computer); eliminarla rimuove il programma. Una cartella in cui il programma non può scrivere (come *Program Files*) viene rifiutata con un messaggio che dice dove metterla invece. Gli aggiornamenti del programma si installano con **Aiuto > Componenti aggiuntivi e aggiornamenti > Installa aggiornamento o componente aggiuntivo**; per passare a una versione scaricata di nuovo, decomprimila e copia la cartella *data* nella nuova cartella. I tuoi progetti restano dove li tieni.

Ci sono due modi per ottenere quella cartella (0.90.4). Il **programma di installazione** (*...-setup.exe*) chiede una cartella e vi decomprime {{APP}}; non richiede diritti di amministratore e non cambia nient'altro sul computer: nessuna voce in App, nulla nel registro, nessuna voce nel menu Start (la guida descritta più avanti le offre). Decomprimere una versione più recente nella stessa cartella mantiene *data*, cioè le tue impostazioni e i componenti aggiuntivi. Il **file zip** contiene la stessa cartella: decomprimilo completamente. Windows può avvisare per entrambi i file perché non sono firmati (SmartScreen: *Ulteriori informazioni*, poi *Esegui comunque*).

Due cose differiscono da Linux a causa di Windows stesso (0.90.4). Il tasto **AltGr** scrive il suo carattere, come ti aspetti: su una tastiera tedesca AltGr+7, 8, 9, 0 e < danno { [ ] } e |, anche se {{APP}} ha comandi su Ctrl+Alt con questi tasti; per i comandi premi i tasti Ctrl e Alt di sinistra. E i file necessari solo per un momento (una pagina web salvata decompressa per la visualizzazione, o la sua copia per il browser) vengono conservati nella cartella temporanea di Windows e rimossi quando {{APP}} si chiude. La Trascrizione vocale su Windows usa il processore, non la scheda grafica.

Al primo avvio una guida dice dove il programma conserva i suoi file e avvisa quando la sua cartella si trova dentro la cartella di un servizio di sincronizzazione (OneDrive, Dropbox, Google Drive ...) o ha un percorso lungo. Chiede inoltre quanto segue, che **Aiuto > Risoluzione problemi e sviluppo > Questo computer** e **File > Opzioni > Generale > Questo computer** mostrano di nuovo:

- **Backup**: non esiste una cartella di backup predefinita. Finché non ne scegli una, non viene fatto alcun backup automatico; la barra a piè di pagina del primo editor mostra **Nessun backup** accanto al testo di salvataggio (fai clic per scegliere la cartella) e il primo backup che dovrebbe partire chiede una cartella. La cosa migliore è un'altra unità.
- **Note del Blocco appunti e trascrizioni**: in Documenti (l'impostazione predefinita), nella cartella del programma o in una cartella tua. Una modifica dà a entrambi gli strumenti il nuovo posto; ciò che hanno scritto prima resta dov'è.
- **Windows**: una voce nel menu Start, un collegamento sul desktop e i tipi di file del progetto (i file *.qbx* si aprono con {{APP}}, che viene offerto sotto **Apri con** per i file *.scrivx* di Scrivener). Sono le uniche cose che il programma mette fuori dalla sua cartella, ciascuna solo quando la aggiungi e ciascuna con **Rimuovi**. Rimuovile prima di eliminare o spostare la cartella.

I caratteri installati con **Formato > Carattere > Installa file di caratteri** restano nella cartella del programma e sono solo di {{APP}}; gli altri programmi non li vedono.

I componenti aggiuntivi con codice di programma hanno le proprie versioni per Windows (i loro nomi di file terminano con *-windows*): **Media Playback** riproduce ciò che Windows stesso riproduce, **Extra Video and Audio Formats** aggiunge FFmpeg e **Trascrizione vocale** trascrive il parlato con il processore (le schede grafiche non sono ancora usate su Windows). **Assistente di traduzione** fa parte della versione completa anche su Windows (0.92.0). I componenti aggiuntivi senza codice di programma (temi, modelli, formati di compilazione) sono gli stessi file su ogni sistema.

### Sui tablet Android

La versione Android di {{APP}} (sperimentale, pubblicata dalla 0.91.3) è un'app per tablet con Android 9 o successivo (ARM a 64 bit). Ha le stesse finestre che su un computer, fatte per le dita: un dito scorre e tocca, due dita fanno lo zoom, un dito che resta mezzo secondo su qualcosa apre il suo menu del clic destro (in un testo basta un tocco sul testo selezionato) e le barre degli strumenti si ripiegano in tutte le righe che servono allo schermo. Scarica il file APK sul tablet e aprilo; Android chiede se il browser o l'app File possono installare app, cosa che consenti per questa installazione. Una nuova versione arriva come nuovo APK e si installa sopra la vecchia, mantenendo le tue impostazioni e i componenti aggiuntivi; **Aiuto > Componenti aggiuntivi e aggiornamenti > Cerca aggiornamenti ora** ti dice quando ce n'è una e apre il suo download nel browser (i componenti aggiuntivi più recenti si installano dentro {{APP}}).

Con una tastiera e un mouse esterni (dalla 0.92.0): Ctrl + rotellina del mouse fa lo zoom, e Ctrl + clic o Shift + clic scelgono più documenti, come su un computer. La linea tra due pannelli mostra una maniglia; un mouse premuto a pochi pixel di distanza la prende con sé. Il tasto **Indietro** di Android chiude il menu, il suggerimento o la finestra in primo piano; ogni finestra ha un pulsante che la chiude e una finestra più grande dello schermo scorre. Su un tablet la puntina della mappa mentale fissata compare sulla riga selezionata del raccoglitore (fai clic su di essa, oppure usa il menu del clic destro della riga: *Fissa la mappa mentale di questo elemento*, *Rilascia la mappa mentale fissata*) e un tocco sul lucchetto di una riga mostra chi ci lavora. Un dito sulla panoramica della mappa mentale funziona come il mouse: un tocco sposta la vista lì, un dito trascina il rettangolo della parte mostrata e la maniglia nel suo angolo ne cambia la dimensione. **File > Salva con nome…** su un tablet chiede prima il formato, poi apre la finestra dei file.

Al primo avvio {{APP}} chiede l'accesso ai file del tablet. Con esso, i progetti vanno in *Documents/Quillbinder*, i backup in *Documents/Quillbinder/Backups*, le esportazioni in *Documents/Quillbinder/Exports* e le note del Blocco appunti in *Documents/Quillbinder/Scratch Pad*, dove altre app, un'app di sincronizzazione e un computer collegato con un cavo possono raggiungerli; il pulsante apre la pagina *Accesso a tutti i file* di Android, dove lo attivi per {{APP}}. Senza di esso, tutto resta nella cartella propria dell'app, che nessun'altra app vede e che Android elimina insieme all'app; *File > File su questo tablet* lo chiede di nuovo e dice dove sono le cose. Quando Android mette da parte l'app (il tasto Home, un'altra app), {{APP}} salva subito.

I dizionari ortografici (inglese e tedesco; le altre lingue sono componenti aggiuntivi, vedi sezione 4), gli strumenti di scrittura, gli importatori di pagine web e di vecchi file Word e (dalla 0.92.0) Assistente di traduzione fanno parte dell'app; Assistente di traduzione viene installato al primo avvio di una versione e scarica un modello linguistico solo quando ne scegli uno. I componenti aggiuntivi per Android (i loro nomi di file terminano con *-android-arm64*) si installano con **Aiuto > Componenti aggiuntivi e aggiornamenti > Installa aggiornamento o componente aggiuntivo**; la finestra dei file si apre nella cartella Download e {{APP}} si riavvia quando il componente aggiuntivo lo richiede. I pacchetti fatti per Linux o Windows e gli aggiornamenti del programma vengono rifiutati con il motivo: l'app stessa si aggiorna solo con un nuovo APK. Non ancora nella versione Android: Trascrizione vocale, audio e video nell'editor e rapporti di arresto anomalo. Il testo copiato dentro {{APP}} mantiene la formattazione quando lo incolli; gli appunti di Android trasportano solo testo semplice, quindi il testo copiato in altre app arriva semplice.

---

## 2. La finestra principale

La finestra ha tre colonne:

| Area | A cosa serve |
|------|--------------|
| **Raccoglitore** (a sinistra) | L'indice del tuo intero progetto: ogni documento e ogni cartella. |
| **Editor** (al centro) | Mostra ciò che hai selezionato nel raccoglitore: testo, una bacheca di schede o una struttura. |
| **Ispettore** (a destra) | Informazioni sul documento corrente: sinossi, note, segnalibri, metadati, istantanee e commenti. |

La prima volta che {{APP}} si avvia su uno schermo più stretto di 1600 px, la finestra si apre ingrandita. Dimensione, posizione e stato della finestra (ingrandita o no) vengono salvati un attimo dopo ogni modifica, così un arresto anomalo non li fa perdere. Nascondi il raccoglitore con **Ctrl+Alt+B** e l'ispettore con **Ctrl+Alt+I** per avere più spazio. Questo vale per una finestra più larga che alta; una finestra più alta che larga dispone le stesse tre aree in modo diverso (sezione successiva). I colori e i caratteri di tutte e tre, e della bacheca, della struttura e della modalità composizione, derivano dal tema (**Vista > Temi**) e possono essere cambiati area per area (sezione 16), anche con un clic destro sull'area (**Aspetto**). Ogni progetto conserva il proprio aspetto.

La **barra degli strumenti** in alto contiene, da sinistra a destra: il pulsante **Vista** (raccoglitore, raccolte, ispettore, layout), nuovo documento (con un menu per cartelle, modelli e importazione), nuova cartella, cestino, **annulla** e **ripeti**, il pulsante **cerca** (ricerca nel progetto; la freccia accanto elenca gli altri strumenti di ricerca: Cerca nel documento, Sostituisci nel progetto, Trova per formattazione), il campo di **ricerca rapida**, i tre pulsanti di vista (testo, bacheca, struttura), i segnalibri del progetto, il riferimento rapido, commenti e note a piè di pagina, compila, la modalità composizione, il **microfono** della Trascrizione vocale (quando quel componente aggiuntivo è installato) e il pulsante dell'ispettore. Decidi tu che cosa contiene (vedi "Barre degli strumenti personalizzate" più sotto).

La **barra di formattazione** sotto di essa contiene lo stile, il carattere, la dimensione, l'interlinea, la formattazione predefinita del testo, la finestra dei caratteri, grassetto, corsivo, sottolineato, barrato, evidenziazione, colore del testo, allineamento, elenchi, lo spazio prima e dopo il paragrafo, il rientro della prima riga e dell'intero paragrafo, commenti, note a piè di pagina, annotazioni in linea e immagini. Mostra o nascondi le due barre con **Vista > Barre degli strumenti**.

**Finestre di dialogo.** Una finestra che il programma apre (Opzioni, Compila, Obiettivi del progetto ...) non è mai più grande dello schermo senza i pannelli del desktop: su un piccolo schermo di portatile il suo contenuto scorre o si rimpicciolisce, e i suoi pulsanti restano in vista. Quando si chiude, la tastiera torna dov'era, di solito nel testo, così puoi continuare a scrivere subito.

**Dove sono?** Fai doppio clic nello spazio accanto al testo (la carta accanto alla colonna o l'area intorno): il raccoglitore seleziona il documento accanto al clic, apre le cartelle che lo contengono e fa lampeggiare due volte la sua riga in arancione. L'editor mantiene ciò che mostra. **Naviga > Mostra nel raccoglitore** (**Ctrl+Shift+R**) fa lo stesso per il documento al cursore.

Il **piè di pagina** di ogni editor mostra il conteggio di parole e caratteri (seleziona del testo per contare solo la selezione) e l'obiettivo del documento. Fai clic sul conteggio per i dettagli. Quando l'editor mostra una cartella (o un documento con sottodocumenti), il piè di pagina mostra anche il **totale** della cartella: le parole del suo testo e di tutto ciò che contiene, per esempio "Cartella: 12.345 parole". La bacheca e la struttura mostrano lo stesso totale al centro del loro piè di pagina; con più elementi mostrati, è il totale di tutti. Un clic su un totale ne mostra i dettagli (parole, caratteri, caratteri senza spazi, paragrafi e pagine), come fa il conteggio di un testo. L'ispettore lo mostra sotto **Totale** nei metadati di una cartella, e anche fermare il mouse su una cartella nel raccoglitore lo mostra (con le parole proprie della cartella quando ha un testo suo).

### I menu

| Menu | Che cosa contiene |
|------|-------------------|
| **File** | Progetti, salvataggio, il formato del progetto ({{APP}} o Scrivener), backup, importazione ed esportazione, confronto e trasferimento con un file Word o LibreOffice, compilazione, stampa, conversione di file, opzioni |
| **Modifica** | Annulla, appunti, selezione, ricerca e sostituzione (nel testo, nel progetto, per formattazione), spostamento dei documenti nel raccoglitore, strumenti di scrittura |
| **Inserisci** | Immagini, tabelle, collegamenti a documenti e oggetti, collegamenti web, commenti, note a piè di pagina, marche temporali multimediali, numerazioni automatiche, segnaposto, caratteri speciali, data e ora |
| **Vista** | Le tre modalità di vista, l'editor diviso (**Dividi editor**), le posizioni di raccoglitore, editor e ispettore e i layout salvati (**Layout**), i temi di colore (**Temi**), raccoglitore, raccolte e ispettore (e **Tasti di aggiunta rapida nel raccoglitore**), barre degli strumenti, opzioni di modifica del testo (vista pagina, righello, scorrimento macchina da scrivere), opzioni di bacheca e struttura, colori delle etichette, zoom, modalità composizione, schermo intero |
| **Naviga** | Indietro e avanti, documento precedente e successivo, apertura dei documenti nell'altro editor, nel copyholder o come riferimento rapido, il blocco dell'editor, in quale editor carica il raccoglitore, le schede dell'ispettore (tutte, con i nomi dei loro suggerimenti: **Sinossi e note**, **Segnalibri**, **Metadati**, **Istantanee**, **Commenti e note a piè di pagina**, la mappa mentale, gli oggetti e le schede degli strumenti di scrittura) |
| **Progetto** | Nuovi documenti, impostazioni del progetto (anche salvate su file e caricate da file), obiettivi, parole chiave, statistiche, cronologia di scrittura, cestino, eliminazione delle istantanee vecchie |
| **Documenti** | Istantanee, rinomina, duplicazione, divisione, unione, conversione, icona, etichetta, stato, segnalibri |
| **Formato** | Carattere (con l'installazione di caratteri), paragrafo, stile, elenchi, evidenziazione, copia della formattazione, modalità revisione |
| **Finestra** | Blocco appunti, finestre del riferimento rapido |
| **Aiuto** | **Trova comando**, questo manuale, la Guida rapida, il progetto di benvenuto, l'elenco di tutti i segnaposto, l'elenco delle scorciatoie da tastiera, l'aiuto sulle mappe mentali, il sottomenu **Componenti aggiuntivi e aggiornamenti** (Gestione componenti aggiuntivi, Installa aggiornamento o componente aggiuntivo, il controllo degli aggiornamenti), l'aggiunta di {{APP}} al menu delle applicazioni (e la sua rimozione), il sottomenu **Risoluzione problemi e sviluppo** (informazioni di sistema, Questo computer, rapporti di arresto anomalo, Apri cartella dei plugin, la guida per sviluppatori), Segnala un problema, le licenze di terze parti, Informazioni su |

**Trova comando** (**Aiuto > Trova comando…**) apre una finestra con un campo di ricerca e l'elenco di tutti i comandi dei menu, ciascuno con il suo menu e la sua scorciatoia. Scrivi una parte di un nome (più parole restringono la ricerca, in qualsiasi ordine); **Return** o un doppio clic esegue il comando scelto. Un comando che al momento è disattivato è grigio e non viene eseguito. Non ha una scorciatoia propria.

Un sottomenu mostra nomi brevi: **File > Importa > File**, **Documenti > Dividi > alla selezione**, **Naviga > Apri > come riferimento rapido**. La scorciatoia di un comando è mostrata accanto ad esso e può essere cambiata (sezione 19). Ogni voce di un menu ha una lettera propria (sottolineata quando tieni premuto **Alt**), quindi **Alt** e le lettere raggiungono ogni comando.

Senza un progetto aperto, i menu **Inserisci**, **Naviga**, **Progetto**, **Documenti** e **Formato** sono grigi, e lo sono anche i comandi degli altri menu che richiedono un testo o un raccoglitore (Trova, Zoom, il layout dell'editor, Focus sul raccoglitore ...). Funzionano senza progetto: creare, aprire e importare progetti, convertire file, installare aggiornamenti e componenti aggiuntivi, le opzioni, i temi e i layout, gli strumenti di scrittura che non richiedono un progetto e Aiuto.

### I pannelli in base alla forma della finestra

La finestra dispone il raccoglitore, l'editor e l'ispettore in base alla propria forma, e cambia da sola quando la forma cambia (trascini la finestra rendendola più alta che larga, oppure giri un tablet). Una finestra quadrata conta come larga.

- **Finestra larga** (più larga che alta, o quadrata): tre colonne. Scegli il loro ordine in **Vista > Layout > Finestra larga**: uno qualsiasi dei sei ordini di raccoglitore, editor e ispettore. Le larghezze sono stabilite dalle linee tra le colonne.
- **Finestra alta** (più alta che larga): due righe sotto le barre degli strumenti. Una riga è l'editor, a tutta larghezza della finestra; l'altra contiene il raccoglitore e l'ispettore affiancati. In **Vista > Layout > Finestra alta**, **Editor al centro** mette la riga dell'editor sopra l'altra, **Editor in basso** sotto di essa; **Raccoglitore a sinistra** e **Raccoglitore a destra** scelgono il lato del raccoglitore (l'ispettore prende l'altro). Le altezze delle due righe e le larghezze del raccoglitore e dell'ispettore sono stabilite dalle linee tra di essi. Con la tastiera su schermo visibile (Android), la riga del raccoglitore e dell'ispettore mantiene la sua altezza; sposta la linea se la vuoi diversa.
- Un raccoglitore o un ispettore nascosto lascia il suo spazio all'altro nella riga alta, a tutta larghezza; con entrambi nascosti, l'editor ha l'intera finestra. Due editor (**Vista > Dividi editor**) restano dentro il posto dell'editor.
- **Vista > Layout > Ripristina layout** riporta la forma che la finestra ha ora ai suoi valori iniziali: raccoglitore, editor, ispettore in una finestra larga; l'editor al centro e il raccoglitore a sinistra in una finestra alta.

Entrambi i gruppi sono sempre nel menu; ciascuno ha effetto quando la finestra ha quella forma, e ogni forma torna come l'avevi lasciata (posizioni e dimensioni). Le scelte appartengono a questo computer o tablet e sono conservate nelle impostazioni del programma, mai in un progetto: un progetto che condividi tra computer conserva la disposizione di ogni dispositivo e nulla di essa viene scritto nei file del progetto.

### Layout

Un layout ricorda come è disposta la finestra: se raccoglitore, raccolte, ispettore, barra degli strumenti e barra di formattazione sono mostrati, quanto sono larghe le colonne, come è diviso l'editor, quale vista usa ogni editor e in quale editor carica il raccoglitore. Non dice nulla sui documenti, quindi ogni layout funziona in ogni progetto.

Scegli un layout in **Vista > Layout** o dal pulsante **Vista** della barra degli strumenti. Sono incorporati sette layout:

| Layout | Disposizione |
|--------|--------------|
| **Predefinito** | Raccoglitore, un editor, ispettore |
| **Raccoglitore ed editor** | Raccoglitore e testo, senza ispettore |
| **Solo editor** | Solo il testo con le due barre |
| **Struttura ed editor** | Struttura a sinistra, testo a destra. Il raccoglitore carica i gruppi nella struttura; un clic su una riga apre quel documento a destra |
| **Bacheca ed editor** | Bacheca sopra, testo sotto, collegati nello stesso modo |
| **Due editor** | Due testi affiancati |
| **Pianificazione** | Raccoglitore, bacheca e l'ispettore con sinossi e note |

**Vista > Layout > Salva layout corrente** memorizza la finestra com'è ora con un nome a tua scelta; spunta "Ricorda anche la dimensione della finestra" se il layout deve ridimensionare la finestra. **Gestisci layout** mostra tutti i layout con una piccola immagine e ti permette di usare, aggiornare (sostituire con la finestra corrente), rinominare, eliminare, esportare e importare i tuoi layout. I layout incorporati non possono essere cambiati; salva invece una finestra modificata con un nuovo nome. Ogni layout può avere una scorciatoia da tastiera (sezione 19, gruppo "Layout").

### Le barre degli strumenti e le loro posizioni

Dalla 0.91 la finestra ha otto barre degli strumenti in due righe (dieci dalla 0.95, con due inizialmente disattivate). La prima riga contiene **Principale** (Vista, nuovo testo, nuova cartella, cestino, Annulla, Ripeti), **Temi** (il pulsante dei temi, inizialmente disattivato), **Profili** (il profilo in uso e un menu per cambiarlo, inizialmente disattivata; dalla 0.95), **Cerca** (il pulsante di ricerca, il campo di ricerca rapida, le modalità di vista) e **Strumenti** (segnalibri, riferimento rapido, commenti, Compila, Mostra caratteri invisibili, Modalità composizione, i pulsanti degli strumenti di scrittura, l'ispettore). La seconda riga contiene **Stile e carattere**, **Carattere e allineamento**, **Paragrafo e inserimento**, la **barra delle icone** ed **Etichette** (un pulsante per ogni colore di etichetta del progetto aperto e **Nessuna etichetta**; inizialmente disattivata; dalla 0.95).

- **Spostare una barra degli strumenti.** Ogni barra ha una maniglia all'inizio (due colonne di punti; il puntatore diventa una croce con le frecce quando ci passa sopra). Trascina la maniglia e la barra va dove la porti:
  - **Lungo la sua riga.** Finché il puntatore resta tra i punti centrali delle due barre vicine, la barra lo segue subito. Lo spazio libero davanti a una barra è largo quanto lo rendi tu. Spostandosi a destra, la barra prende prima lo spazio libero dopo di sé; quando raggiunge la barra successiva la spinge avanti, fin dove le barre che seguono possono stare insieme alla fine della riga. Spostandosi a sinistra, si ferma dove finisce la barra precedente, e le barre che seguono restano dove sono (trascinale fino a essa se vuoi che stiano di nuovo insieme). La prima barra di una riga resta al bordo della finestra.
  - **In un altro punto dell'ordine.** Porta il puntatore oltre il centro di una barra vicina, o su una qualsiasi altra barra della riga. La barra per il momento resta dov'è e una linea nel colore di evidenziazione mostra dove si metterà; rilascia il pulsante del mouse per metterla lì. Quindi è possibile qualsiasi ordine, anche per la prima e l'ultima barra di una riga.
  - **In un'altra riga.** Porta il puntatore nell'altra riga: la linea mostra lì il posto. Qualsiasi barra può stare in qualsiasi riga, una barra di formattazione accanto alla barra principale o la barra di ricerca tra le barre di formattazione.
  - **In una riga propria, o sul bordo inferiore.** Porta il puntatore appena sotto le righe delle barre (appare una linea che attraversa la finestra) per aprire una nuova riga, oppure sul bordo inferiore della finestra per metterci la barra. Il bordo inferiore accoglie barre affiancate e in righe, come quello superiore.
  - **Esc**, o un rilascio dove non si vede alcuna linea (per esempio in mezzo alla finestra), lascia tutto com'era.

  Le barre dopo il vecchio posto si accostano, e lo spazio libero che c'era davanti alla barra resta nella sua riga; al nuovo posto la barra sta subito dopo la barra precedente e da lì puoi tirarla ancora a destra. In una finestra troppo stretta per lo spazio libero che hai lasciato, lo spazio si riduce prima che una barra perda dei pulsanti; torna quando la finestra è di nuovo larga. Una barra disattivata mantiene il suo posto. Una barra non può stare ai lati né fluttuare come finestra a sé. {{APP}} ricorda le righe, l'ordine e gli spazi.
- **Disattivare una barra degli strumenti.** **Vista > Barre degli strumenti** ha un interruttore per ogni barra; la stessa lista compare con un clic destro su una barra o sullo spazio libero accanto alle barre. **Mostra barra degli strumenti** e **Mostra barra di formattazione** attivano o disattivano un'intera riga come prima (le quattro barre della prima riga, le tre barre di formattazione) e i layout usano questi due; l'interruttore di una singola barra non è utilizzabile mentre la sua riga è disattivata.
- **Vista > Barre degli strumenti > Ripristina posizioni delle barre** rimette tutte le barre nelle loro posizioni standard (le due righe nel loro ordine standard, nulla sul bordo inferiore) e chiude gli spazi tra di esse.

### Barre degli strumenti personalizzate

**Vista > Barre degli strumenti > Personalizza barre degli strumenti** (o un clic destro su una barra) apre una finestra con due liste: i comandi disponibili a sinistra, il contenuto della barra a destra. Scegli la barra in alto.

- Aggiungi un comando con il pulsante a freccia o con un doppio clic. Ogni comando dei menu è disponibile, anche quelli degli strumenti di scrittura (Modifica > Strumenti di scrittura); il campo di ricerca sopra la lista lo trova.
- Rimuovi un pulsante con l'altra freccia o con un doppio clic, e spostalo con i pulsanti su e giù o trascinandolo.
- **Separatore** e **Spazio flessibile** si possono usare quante volte vuoi. Uno spazio flessibile spinge a destra ciò che segue; due di essi centrano ciò che sta tra di loro.
- Alcune voci sono più di un comando: **Vista**, **Aggiungi**, **Cerca**, il **campo di ricerca rapida**, **Modalità di vista**, **Segnalibri del progetto**, **Commenti e note a piè di pagina**, **Layout** e **Temi** (il pulsante dei temi con il suo menu) per le quattro barre della prima riga; le liste di stile, carattere, dimensione e interlinea e i quattro pulsanti del paragrafo per le tre barre di formattazione. Un controllo simile esiste una volta sola: aggiunto a una barra, si sposta lì dalla barra che lo aveva. Un controllo della prima riga non può andare su una barra di formattazione, né il contrario.
- Sotto le liste scegli icone piccole, medie o grandi e se le barre della prima riga mostrano il testo accanto o sotto le icone.
- **Set standard** rimette nella barra i pulsanti con cui il programma è arrivato.

Se una barra contiene più di quanto sta nella finestra, una freccia alla sua estremità apre il resto. Una barra rimasta senza alcun pulsante viene disattivata; riattivata in **Vista > Barre degli strumenti**, ha il suo set standard. Una barra che avevi disposto in una versione precedente alla 0.91 è stata divisa sulle nuove barre: ogni pulsante è andato alla barra che lo contiene nei set standard, e ciò che avevi tolto resta tolto.

### La barra delle icone

La **barra delle icone** contiene fino a dieci icone di documento che usi spesso. Un clic su una di esse dà quell'icona ai documenti e alle cartelle selezionati (nel raccoglitore, o nella bacheca o struttura in cui stai lavorando); il pulsante **↺** dopo di esse restituisce loro l'icona standard e l'ultimo pulsante sceglie le icone della barra. Mostra o nascondi la barra con **Vista > Barre degli strumenti > Barra delle icone**.

**Vista > Barre degli strumenti > Scegli le icone per la barra delle icone…** (anche nel menu del clic destro della barra) apre una finestra con tutte le icone a sinistra, comprese le icone del tuo progetto, e le icone della barra a destra. Aggiungi con **Aggiungi →** o con un doppio clic, rimuovi con **← Rimuovi** o con un doppio clic, e cambia l'ordine con **Su** e **Giù**. **Set standard** rimette le dieci icone con cui il programma è arrivato (Libro verde, Correzione bozze, Spuntato, Non spuntato, Punto interrogativo, Bandiera rossa, Blocco note giallo, Scheda personaggio, Scheda luogo, Avvertimento).

---

## 3. Il raccoglitore

Ogni progetto ha tre cartelle speciali:

- **Bozza** (in alcuni modelli *Manoscritto*): tutto ciò che diventerà il tuo libro. Solo ciò che si trova nella Bozza viene compilato.
- **Ricerca**: note, schede dei personaggi, immagini, PDF e tutto ciò che raccogli.
- **Cestino**: gli elementi eliminati aspettano qui finché non scegli **Progetto > Svuota cestino**. La sua domanda, come ogni domanda prima di qualcosa che non si può annullare, nomina l'azione sul suo pulsante e ha **Annulla** come impostazione predefinita, così il tasto Enter non elimina mai nulla (0.70).

### Lavorare con gli elementi

- **Nuovo documento**: **Ctrl+N** (o il pulsante ➕). Viene aggiunto sotto l'elemento selezionato. Un documento chiamato "Senza titolo" (o privo di titolo) prende la prima riga del suo testo come titolo non appena scrivi: al massimo 60 caratteri, tagliati a fine parola, senza annotazioni in linea, note a piè di pagina in linea e immagini. Il titolo segue la prima riga finché non rinomini tu stesso il documento; un documento di cui elimini il testo torna "Senza titolo". I documenti "Senza titolo" che hanno già del testo ricevono il nome all'apertura del progetto. Il titolo è un normale titolo: viene salvato, compilato (quando i titoli vengono compilati, una prima riga che è un titolo di sezione compare due volte) e scritto nei progetti Scrivener.
- **Nuova cartella**: **Alt+Shift+N**.
- **Rinomina**: **F2**, oppure doppio clic lento sul titolo.
- **Sposta**: trascina gli elementi con il mouse, oppure usa **Ctrl+Alt+tasti freccia**: Su e Giù spostano dentro la cartella; Sinistra sposta fuori dalla cartella; Destra sposta dentro l'elemento sopra. Un documento rilasciato *tra* due elementi finisce lì; rilasciato *su* un elemento, ne diventa un sottodocumento. Se annidi i documenti per errore, disattiva il rilascio sui documenti di testo in **File > Opzioni > Comportamento** (le cartelle accettano sempre i rilasci).
- **Aprire o chiudere un intero ramo**: tieni premuto **Alt** e fai clic sulla freccia di una cartella o di un gruppo.
- **Elimina**: premi **Delete**. L'elemento va nel Cestino; una bacheca o una struttura che lo mostrava come uno di più documenti scelti lo lascia andare subito (0.94.0).
- **Duplica**: **Ctrl+D**.
- **Dividi e unisci**: **Documenti > Dividi > alla selezione** (**Ctrl+K**) taglia il documento al cursore in due; non funziona dentro una tabella o la didascalia di un'immagine (metti il cursore prima o dopo). **Documenti > Unisci** (**Ctrl+Shift+M**) unisce i documenti selezionati nel primo, con le loro sinossi, parole chiave e note; gli altri vanno nel Cestino.
- **Cambia l'icona**: clic destro > **Cambia icona** (o **Documenti > Cambia icona**), oppure la barra delle icone (sezione 2). Vedi "Icone" più sotto.

**Clic destro.** Su un documento, il menu del clic destro contiene i comandi per i documenti selezionati (rinomina, duplica, sposta nel cestino, istantanee, etichetta, stato e altro). Sullo spazio vuoto sotto i documenti offre solo ciò che crea qualcosa di nuovo: **Nuovo testo**, **Nuova cartella**, **Nuovo da modello** e l'**Aspetto** del raccoglitore.

Un documento può avere dei sottodocumenti, proprio come una cartella. Cartelle e documenti si comportano allo stesso modo; le cartelle semplicemente hanno un aspetto diverso e per impostazione predefinita si aprono come una bacheca.

**I tasti del raccoglitore (0.94.0).** Con la tastiera nel raccoglitore, **Return** apre il documento selezionato nell'editor e vi porta la tastiera (come fa un gestore file), e i tasti freccia si muovono nella lista. **Space** fa parte della digitazione di un nome: scrivi le prime lettere di un titolo, spazi compresi ("Capitolo t"), e la selezione salta al primo titolo che inizia così. La voce **Vista > Tasti di aggiunta rapida nel raccoglitore** è inizialmente disattivata (anche dopo un aggiornamento). Una volta attivata, il raccoglitore ha i tasti di aggiunta rapida: **Return** crea un nuovo documento vuoto sotto quello selezionato e **Space** apre i documenti selezionati come pannelli di riferimento rapido (e nella lista di una raccolta o di una ricerca).

Un clic nel raccoglitore carica il documento nell'editor. Dopo **Indietro**, un collegamento a un documento, o con un editor bloccato, l'editor può mostrare un documento diverso da quello selezionato nel raccoglitore: una **barra grigia** segna allora nel raccoglitore il documento dell'editor. **Naviga > Mostra nel raccoglitore** (**Ctrl+Shift+R**) lo seleziona.

**Raccoglitore stretto, cartelle profonde.** Ogni livello di cartelle è rientrato di 12 px (la metà di prima; cambialo in **File > Opzioni > Aspetto > Raccoglitore > Rientro per livello**). Quando i titoli sono più lunghi della larghezza del raccoglitore, in fondo al raccoglitore compare una barra di scorrimento, così puoi leggere la fine dei titoli lunghi senza allargare il raccoglitore.

### Icone

**Documenti > Cambia icona** (anche nel menu del clic destro del raccoglitore, della bacheca e della struttura) è costruito come il menu delle icone di Scrivener 3 per Windows:

- **Ripristina icona predefinita**: torna l'icona standard di un documento, di una cartella o di un file.
- **Icona da testo…**: crea un'icona di al massimo tre caratteri (per esempio *PR*, *Cap1* o *A*). Scegli il colore del riquadro, delle lettere e del bordo; la piccola immagine mostra il risultato. L'icona viene conservata nel progetto e data subito ai documenti selezionati.
- **Gestisci icone…**: le icone di questo progetto. **Aggiungi immagine…** trasforma immagini (PNG, SVG, JPEG e altre) in icone, **Da testo…** è lo stesso di sopra, **Rinomina…** ed **Elimina** cambiano la lista (i documenti con un'icona eliminata riavranno la loro icona standard), **Usa per i documenti selezionati** dà l'icona scelta ai documenti selezionati. Le icone sono memorizzate nel progetto (cartella *icons*), così viaggiano con esso.
- **Recenti**: le ultime cinque icone scelte.
- **Libro**, **Personaggi**, **Bandiera**, **Luoghi**, **Note**, **Da fare**, **TV**, **Stazione di sosta**: sottomenu con le icone di ciascun gruppo in diversi colori (per esempio Libro blu, verde, rosso e giallo; sette bandiere; cinque blocchi note; Spuntato e Non spuntato, Superato, Non superato e Non chiaro in Da fare).
- Poi le singole icone: Retro del libro, Lavagna, Calendario, Ciak, Orologio, Ingranaggio, Occhio, Figure e grafici, Schedario, Frontespizio, Informazioni, Leggio, Lampadina, Lente d'ingrandimento, Musica, Note musicali, Struttura, Matita, Correzione bozze, Punto interrogativo, Commenti del revisore, Forbici, Fumetto, Virgolette, Statistiche, Cartella sincronizzata, Tabelle, Provetta, Nuvoletta di pensiero e Avvertimento.
- **Icone del progetto**: le tue icone (quando il progetto ne ha) e **Altre icone (desktop)**: le icone del tema del desktop che le versioni precedenti offrivano.

**Colore dell'etichetta nelle icone.** Le parti bianche di molte icone (le pagine di un blocco note, la stoffa di una bandiera bianca, la carta di un foglio, il riquadro di un'icona creata da testo) assumono il colore dell'etichetta del documento. Attivalo o disattivalo con **Vista > Usa colore etichetta in > Icone** (o **File > Opzioni > Aspetto > Raccoglitore**).

**Icone e Scrivener.** Le icone di {{APP}} sono disegnate da zero, ma ognuna porta il nome dell'icona di Scrivener che rappresenta. Un progetto in formato Scrivener (sezione 14) conserva questi nomi, così Scrivener mostra la propria icona per essi; le icone create in Scrivener tornano qui come l'icona corrispondente. Le tue icone vanno nella cartella *Icons* del *.scriv*. Un'icona per cui questo programma non ha un'immagine mantiene il suo nome e viene mostrata qui come l'icona più vicina.

### Etichette, stato e "Includi nella compilazione"

Fai clic destro su un elemento per impostare la sua **etichetta** (un colore, per esempio per il personaggio del punto di vista) e il suo **stato** (per esempio *Prima stesura* o *Fatto*). Cambia le liste in **Progetto > Impostazioni del progetto**.

Un documento o una cartella con un'etichetta mostra il suo colore anche nell'ispettore: la scheda della sinossi e le note del documento assumono una tinta chiara del colore dell'etichetta (il 15 % della sua saturazione), così vedi a colpo d'occhio quale etichetta ha l'elemento. Gli elementi senza etichetta, o con una grigia, mantengono il normale colore della scheda.

La barra degli strumenti **Etichette** (**Vista > Barre degli strumenti > Etichette**, inizialmente disattivata) mostra uno accanto all'altro tutti i colori di etichetta del progetto aperto, con un pulsante per nessuna etichetta. Un clic imposta subito l'etichetta dei documenti selezionati, come fa **Documenti > Etichetta**; il suggerimento di ogni colore nomina l'etichetta. Le etichette con cui parte un nuovo progetto si impostano in **File > Opzioni > Aspetto > Colori standard**.

Gli elementi con **Includi nella compilazione** disattivato restano nella Bozza ma vengono lasciati fuori quando compili.

### Raccolte

Le raccolte sono liste aggiuntive di documenti, per esempio "Da documentare" o "Scene con Anna". Mostrale con il pulsante **Raccolte**. Usa il pulsante ➕ sopra il raccoglitore per creare una raccolta dai documenti selezionati, oppure fai clic destro sui documenti e scegli **Aggiungi alla raccolta**. Rimuovere un documento da una raccolta non lo elimina. Anche le ricerche salvate (sezione 9) sono raccolte; si aggiornano da sole.

- **Aggiungi documenti** trascinandoli dal raccoglitore (o dall'icona nell'intestazione di un editor, da una bacheca, da una struttura) sulla scheda della raccolta.
- **Dentro la lista** di una raccolta, trascina i documenti nell'ordine che preferisci (l'ordine del raccoglitore non cambia), rilascia documenti in un punto e premi **Delete** per togliere i documenti selezionati dalla raccolta. Le raccolte di ricerca non possono essere cambiate in questo modo.
- **Trascina le schede** per mettere le raccolte in un altro ordine.
- I documenti nella lista possono essere trascinati come quelli nel raccoglitore, e **Space** apre quelli selezionati come pannelli di riferimento rapido.

---

## 4. Scrivere

### Formattazione del testo

| Azione | Scorciatoia |
|--------|-------------|
| Grassetto / corsivo / sottolineato | Ctrl+B / Ctrl+I / Ctrl+U |
| Evidenzia | Ctrl+Shift+H |
| Apice / pedice | Ctrl+Shift++ / Ctrl+= (layout tedesco: Ctrl+Shift+D per il pedice, vedi "Scorciatoie e layout di tastiera" più sotto) |
| Allinea a sinistra / centra / allinea a destra | Ctrl+{ / Ctrl+\| / Ctrl+} (layout tedesco: Alt+Shift+L / Alt+Shift+E / Alt+Shift+R) |
| Finestra dei caratteri | Ctrl+T |
| Dimensione del carattere più grande / più piccola | Ctrl+> / Ctrl+< |
| Finestra del paragrafo | Ctrl+Alt+P |
| Copia formattazione / incolla formattazione | Ctrl+Alt+C / Ctrl+Alt+V |
| Ingrandisci / riduci / ripristina zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom dell'area sotto il mouse | Ctrl+rotellina del mouse |
| Mostra o nascondi il righello | Ctrl+Alt+R |
| Incolla senza formattazione | Ctrl+Shift+V |
| Incolla con formattazione (da altri programmi) | Ctrl+Alt+Shift+V |
| Annulla / ripeti | Ctrl+Z / Ctrl+Shift+Z (vedi "Annulla e ripeti" più sotto) |

Grassetto, corsivo, apice e simili agiscono sulla selezione. Senza una selezione, dentro una parola cambiano l'intera parola; alla fine di una parola (o senza nulla intorno al cursore) cambiano ciò che scrivi dopo: scrivi "H", premi Ctrl+=, scrivi "2", premi di nuovo Ctrl+= e scrivi "O" per ottenere H₂O.

Gli **stili** (Heading 1–6, Title, Subtitle, Block Quote e i tuoi) si scelgono dalla lista a sinistra della barra di formattazione. Gli stili dicono a Compila che cos'è un paragrafo, quindi usali invece di rendere il testo grande e in grassetto a mano. Vedi "Stili" più sotto.

**Che cosa mostra la barra di formattazione.** La barra mostra il testo come lo vedi, per tutta la selezione. **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, apice e pedice sono attivi quando tutto il testo selezionato appare così, anche quando è il suo stile a renderlo tale (il grassetto di un titolo di sezione). Un clic cambia ciò che vedi: **Grassetto** su un titolo di sezione in grassetto lo rende non grassetto, e resta non grassetto quando il progetto viene salvato e riaperto. Anche il carattere, la dimensione, l'interlinea, i pulsanti di allineamento e la lista degli stili mostrano la selezione; quando la selezione contiene due caratteri, due dimensioni, due allineamenti o paragrafi di due stili, quel campo resta vuoto (nessun pulsante di allineamento è attivo). Una dimensione scritta nel campo vuoto dà quella dimensione a tutta la selezione.

**Elenchi**: usa i pulsanti degli elenchi, oppure scrivi "- " o "1. " all'inizio di un paragrafo.

**Tipografia intelligente**: le virgolette dritte diventano virgolette curve e due trattini diventano un trattino lungo mentre scrivi. Disattivala in **File > Opzioni > Modifica**.

**Modalità revisione** (**Formato > Modalità revisione**): tutto ciò che scrivi viene colorato con uno di cinque colori di revisione, così puoi vedere che cosa è cambiato in un giro di correzioni. Scegli **Disattivata** per fermarla.

**Spaziatura e rientri nella barra di formattazione.** Quattro pulsanti dopo i pulsanti degli elenchi impostano i paragrafi che contengono il cursore o la selezione: lo **spazio prima** e lo **spazio dopo** il paragrafo (0, 3, 6, 9, 12 o 18 pt), il **rientro della prima riga** e il **rientro dell'intero paragrafo** (in centimetri o pollici, come li mostra il righello). Il valore del paragrafo al cursore è spuntato; **Altro...** chiede un valore qualsiasi. **Modifica > Annulla** annulla una modifica. Una barra di formattazione che hai disposto tu riceve i quattro pulsanti una volta, dopo i pulsanti degli elenchi; **Vista > Barre degli strumenti > Personalizza barre degli strumenti** li sposta o li rimuove.

**Annulla e ripeti.** **Modifica > Annulla** e **Modifica > Ripeti** funzionano nel campo di testo che ha la tastiera (l'editor, la sinossi, le note, un titolo) e dicono che cosa annulleranno: "Annulla digitazione", "Annulla incolla", "Annulla grassetto", "Ripeti elimina". Con la tastiera altrove, per esempio nel raccoglitore, sono grigi: spostare documenti nel Cestino, svuotare il Cestino ed eliminare istantanee non si può annullare in questo modo (un documento nel Cestino torna trascinandolo fuori; per le istantanee vedi la sezione 11). La freccia accanto ai pulsanti **Annulla** e **Ripeti** della barra degli strumenti elenca i passi, il prossimo per primo; sceglierne uno annulla (o ripristina) quel passo e ogni passo che lo precede nella lista. Ogni comando del menu Formato è un passo.

**Colore del testo ed evidenziazione nella barra di formattazione.** I due pulsanti con una barra colorata (una penna per evidenziare, una lettera *T* per il colore del testo) funzionano come quelli di Scrivener e di Word: un clic applica il colore mostrato nella barra al testo selezionato oppure, senza selezione, a ciò che scrivi dopo. Ferma il mouse su un pulsante per un momento e sotto di esso si apre una piccola finestra con **rosso, blu, giallo, verde, bianco e nero**, più **Nessuna evidenziazione** (oppure **Automatico** per il colore del testo) e **…** per qualsiasi altro colore. Un clic su un colore lo applica e lo rende il colore del pulsante, che lo conserva per il clic successivo (anche dopo un riavvio). Un'evidenziazione viene disegnata esattamente nel colore che hai scelto, su una pagina chiara e su una scura. Dove il testo sopra di essa sarebbe difficile da leggere (sotto un contrasto di 4,5 : 1), il testo appare scuro o chiaro solo sullo schermo; il suo colore memorizzato, e l'evidenziazione nei file salvati e compilati, restano come scelti.

### Caratteri

Tutto il testo di un progetto ha un solo aspetto: la **formattazione predefinita** (vedi sotto). Su di essa puoi dare a qualsiasi pezzo di testo un carattere proprio.

- **Barra di formattazione**: la lista dei caratteri e il campo della dimensione cambiano subito il testo selezionato. Senza selezione cambiano l'**intero paragrafo** che contiene il cursore (e ciò che scrivi dopo lì). Le frecce del campo della dimensione cambiano il testo a ogni clic; una dimensione che scrivi ha effetto con **Enter**. La barra di formattazione, il menu Formato e le loro scorciatoie agiscono sul testo in cui si trovava per ultimo il cursore: un editor, le note nell'ispettore, il testo dei segnalibri nell'ispettore o un lungo campo di testo di un oggetto (lì: grassetto, corsivo, sottolineato, barrato, colore del testo ed evidenziazione). Un clic nel campo della dimensione non cambia questo.
- Dopo una modifica nella barra di formattazione, una barra sopra l'editor offre la stessa cosa per l'intero progetto: in un paragrafo con uno stile (per esempio un titolo di sezione) **Cambia lo stile “Heading 1”** dà allo stile il nuovo carattere o la nuova dimensione, e tutto il testo di quello stile in ogni documento lo segue; in un paragrafo senza stile **Usa per tutto il testo** ne fa il carattere o la dimensione del progetto (vedi "Formattazione predefinita" più sotto). Fai clic su **×**, o semplicemente continua a scrivere altrove, per lasciare la modifica dov'è. Ogni documento che cambia riceve prima un'istantanea.
- **Formato > Carattere > Mostra caratteri…** (**Ctrl+T**) apre la finestra dei caratteri: carattere, variante (leggero, normale, grassetto, corsivo e qualsiasi altra offra il carattere), dimensione, sottolineato, barrato, maiuscole (maiuscoletto o tutto maiuscolo), spaziatura delle lettere in percentuale e colore del testo. Viene applicato solo ciò che cambi nella finestra, quindi una selezione con parole in grassetto e normali le mantiene. **Carattere del progetto** riporta il testo al carattere, alla dimensione e al colore del progetto.
- **Grassetto, Corsivo, Sottolineato, Barrato**: un pulsante è attivo quando *tutto* il testo selezionato ha quel formato, comunque tu abbia selezionato (anche da destra a sinistra). Una selezione solo in parte in grassetto mostra Grassetto disattivato, e **Grassetto** allora rende tutto grassetto; un secondo clic rende tutto normale. Nella finestra dei caratteri, anche una variante che scegli viene impostata per l'intera selezione.
- **Formato > Carattere > Più grande / Più piccolo** (**Ctrl+>** / **Ctrl+<**) cambiano la dimensione di un punto; ogni parte della selezione mantiene la propria dimensione e cresce o si riduce.
- **Formato > Carattere > Colore del testo…** e **Colore del testo automatico**.

La scelta del carattere o della dimensione propri del progetto viene memorizzata come "nessun carattere proprio": un testo simile segue il progetto quando in seguito cambi la formattazione predefinita.

**Caratteri con più design.** Alcuni caratteri riuniscono sotto un solo nome più design di una stessa famiglia: Sitka di Windows 11, per esempio, contiene le dimensioni ottiche Small, Text, Subheading, Heading, Display e Banner (ciascuna disegnata per il proprio intervallo di dimensioni, le più grandi più strette e con minuscole più piccole), e alcuni caratteri contengono nello stesso modo versioni condensate o larghe. {{APP}} elenca ogni design come carattere a sé (per esempio **Sitka Small**, **Sitka Text**, **Sitka Banner**), solo con le varianti di quel design, così che grassetto e corsivo restano dentro il design che hai scelto; prima della 0.40.1, il testo in grassetto poteva uscire in un altro design e apparire più piccolo e più stretto. I file dei caratteri non vengono cambiati. Il carattere con il nome originale mantiene il proprio design, così il testo composto con esso in precedenza mantiene il suo aspetto.

**Installare caratteri.** **Formato > Carattere** ha i comandi per i caratteri che non sono ancora sul tuo computer:

- **Installa file di caratteri…**: scegli file di caratteri (TTF, OTF, TTC, PFB, WOFF o un file ZIP che li contiene). Vengono copiati nella tua cartella dei caratteri (*~/.local/share/fonts*) e le liste dei caratteri di {{APP}} li conoscono subito; gli altri programmi li vedono dopo il loro prossimo avvio. Non serve alcuna password e i caratteri sono solo per il tuo utente.
- **Apri cartella dei caratteri**: apre quella cartella nel gestore file; i caratteri che metti lì tu stesso vengono trovati dopo **Ricarica elenco dei caratteri**.
- **Apri il gestore dei caratteri del desktop**: apre il gestore dei caratteri di KDE (Impostazioni di sistema > Testo e caratteri > Gestione dei caratteri) o il programma dei caratteri di un altro desktop, dove puoi anche installare caratteri per tutti gli utenti e rimuoverli.
- **Ricarica elenco dei caratteri**: cerca nuovi caratteri e li rende noti.

### Formattazione del paragrafo

**Formato > Paragrafo > Paragrafo…** (**Ctrl+Alt+P**) imposta, per i paragrafi che contengono il cursore o la selezione: allineamento, rientro sinistro e destro, rientro della prima riga (un valore negativo dà un rientro sporgente), spazio prima e dopo, interlinea (un multiplo dell'altezza di riga, un'altezza esatta o un'altezza minima) e tabulazioni (a sinistra, centrata, a destra, con punto decimale). Viene applicato solo ciò che cambi. **Ripristina predefinito** rimuove la formattazione che hai impostato a mano; il paragrafo torna a seguire il suo stile e il progetto.

Modi rapidi: i pulsanti di allineamento, la lista dell'interlinea nella barra di formattazione, **Formato > Paragrafo > Interlinea** e **Aumenta rientro / Riduci rientro**.

Applicare uno stile di paragrafo sostituisce la formattazione impostata a mano nei paragrafi che raggiunge: rientri, spaziatura, allineamento e tabulazioni, e i caratteri, le dimensioni, i colori, la spaziatura delle lettere e le maiuscole del testo al loro interno. Grassetto, corsivo, sottolineato, barrato, evidenziazioni e stili di carattere restano. Seleziona del testo su più paragrafi per dare a tutti lo stile, per l'intero paragrafo; una selezione che termina esattamente all'inizio di un paragrafo (per esempio dopo **Shift+Down**) lascia stare quel paragrafo.

**Formato > Copia formattazione** (**Ctrl+Alt+C**) ricorda la formattazione del carattere e del paragrafo al cursore; **Incolla formattazione** (**Ctrl+Alt+V**) la applica alla selezione, oppure all'intero paragrafo quando nulla è selezionato.

### Caratteri invisibili

**Vista > Modifica del testo > Mostra caratteri invisibili** (anche il pulsante ¶ della barra degli strumenti) mostra i caratteri che non vedi: un punto per ogni spazio, una freccia per una tabulazione, un piccolo cerchio per uno spazio non separabile, una freccia piegata per un'interruzione di riga (**Shift+Enter**) e ¶ alla fine di ogni paragrafo. Sono disegnati solo sullo schermo, nel testo principale di ogni editor; non fanno mai parte del testo, di una stampa o della compilazione. La scelta viene conservata. Il loro colore è quello del testo, sfumato nella carta; un colore proprio si imposta in **File > Opzioni > Aspetto > Segni testuali** (oppure clic destro sul testo > **Aspetto**), e appartiene all'aspetto del progetto come la carta e il testo (la modalità composizione mantiene il colore sfumato del testo).

### Il righello

**Vista > Modifica del testo > Mostra righello** (**Ctrl+Alt+R**) mostra un righello sopra il testo. Appartiene al paragrafo che contiene il cursore.

- Il marcatore in alto è il **rientro della prima riga**, i marcatori in basso sono il **rientro sinistro** e il **rientro destro**. Trascinali. Il rientro sinistro porta con sé la prima riga.
- **Fai clic** nella metà inferiore del righello per aggiungere una tabulazione. Trascina una tabulazione per spostarla; trascinala in su o in giù fuori dal righello per rimuoverla. Fai **clic destro** su una tabulazione per renderla a sinistra, centrata, a destra o decimale.
- I marcatori si agganciano ai quarti di centimetro (sedicesimi di pollice); tieni premuto **Shift** per posizionarli liberamente.
- Fai **doppio clic** sul righello per la finestra del paragrafo. Il menu del clic destro passa tra centimetri e pollici (anche in **File > Opzioni > Modifica**).

Le tabulazioni sono misurate dall'inizio della prima riga del paragrafo. Senza tabulazioni proprie, un paragrafo ha una posizione di tabulazione ogni 2,1 cm.

Il righello mostra lunghezze reali. Lo zoom ingrandisce le lettere sullo schermo ma non i rientri e le tabulazioni, quindi confronta le distanze al 100 %.

### Formattazione predefinita

- **Questo progetto**: **Progetto > Impostazioni del progetto > Formattazione** (anche il pulsante con i cursori nella barra di formattazione) imposta carattere, dimensione, interlinea, rientro della prima riga, spazio dopo i paragrafi e l'allineamento del testo dell'intero progetto (quattro pulsanti: a sinistra, centrato, a destra, giustificato). Nel Pannello stili, **Nessuno stile** > **Ridefinisci dalla selezione** prende questi valori dal paragrafo al cursore (vedi "Stili" più sotto). **Applica** cambia tutti i documenti in una volta e lascia la finestra aperta, così puoi vedere il risultato dietro di essa; **OK** fa lo stesso e chiude la finestra. **Usa corrente** prende i valori dal paragrafo che contiene il cursore.
  - Con **Cambia anche il testo con un carattere, una dimensione o una spaziatura propri** (attivo in modo predefinito), anche il testo formattato a mano o arrivato con un carattere proprio da un'importazione lo segue, finché ha il carattere, la dimensione e la spaziatura principali del suo documento. Ciò che differisce dal testo principale resta com'è: una riga di titolo più grande, un brano in un altro carattere, un paragrafo senza rientro. Disattiva la casella per lasciare stare tutto il testo formattato a mano.
  - Ogni documento che cambia riceve prima un'istantanea (ispettore, scheda Istantanee, "Prima che cambiasse la formattazione del progetto"), così puoi tornare indietro. I documenti script mantengono il proprio formato.
- **Nuovi progetti**: **File > Opzioni > Modifica > Formattazione dei nuovi progetti**, con gli stessi quattro pulsanti di allineamento e **Usa la formattazione nell'editor corrente**. I progetti esistenti mantengono la loro formattazione. Lo standard è 11,0 pt, interlinea 1,09, nessun rientro della prima riga e 6 pt dopo i paragrafi. Anche i progetti importati da Scrivener partono con questi valori (con il carattere più usato nel progetto Scrivener).
- **Tornare al predefinito**: seleziona dei documenti nel raccoglitore e scegli **Documenti > Converti > Testo alla formattazione predefinita…**. Decidi tu se vengono rimossi caratteri e colori, rientri e spaziatura, allineamento e tabulazioni. Grassetto, corsivo, stili, note e collegamenti restano sempre. Prima viene creata un'istantanea di ogni documento (ispettore, scheda Istantanee), così puoi tornare indietro.

### Stili

Uno stile salva una formattazione con un nome. Il testo ricorda il nome, quindi quando cambi lo stile, tutto il testo che lo usa cambia con esso, in tutto il progetto.

Ce ne sono di due tipi:

- Gli **stili di paragrafo** si applicano a interi paragrafi. Contengono la formattazione del paragrafo (allineamento, rientri, spaziatura, interlinea, tabulazioni) e, se vuoi, la formattazione dei caratteri per il paragrafo (carattere, dimensione, grassetto, corsivo, colore). Negli elenchi sono contrassegnati con "¶", oppure "¶a" quando includono la formattazione dei caratteri.
- Gli **stili di carattere** si applicano al testo selezionato, o alla parola al cursore. Contengono solo la formattazione dei caratteri. Sono contrassegnati con "a".

Lascia il testo del corpo su **Nessuno stile**. Segue la formattazione predefinita del progetto e Compila può dargli qualsiasi aspetto. Usa gli stili solo per il testo che deve apparire diverso dal testo del corpo.

**Applicare uno stile**: sceglilo nella lista a sinistra della barra di formattazione, in **Formato > Stile**, nel menu del clic destro sul testo, oppure nel **Pannello stili** (**Formato > Stile > Pannello stili**), una piccola finestra che resta aperta accanto al testo; un doppio clic su una riga applica lo stile. **Nessuno stile** rimuove uno stile di carattere dalla selezione; dove non ce n'è alcuno, rimuove lo stile di paragrafo. Le finestre e le domande che il Pannello stili apre compaiono sopra il pannello.

**L'elenco degli stili** della barra di formattazione e del menu del clic destro (**Stile**) mostra gli stili che scegli, nell'ordine che scegli: **Personalizza elenco degli stili…** (in fondo alla lista e in **Formato > Stile**) elenca tutte le voci; togli la spunta a quelle che non ti servono, trascinale in un altro ordine o usa le frecce, e **Tutti, nell'ordine degli stili** riporta l'elenco completo. Con più di dieci voci l'elenco ne mostra dieci e scorre. Sotto di esso, sempre in vista: **Nuovo stile dalla selezione…**, **Ridefinisci stile dalla selezione…**, **Pannello stili** e **Personalizza elenco degli stili…**. Uno stile che non è nell'elenco viene comunque nominato nella barra di formattazione quando il testo lo usa; **Formato > Stile** elenca sempre tutti gli stili. La scelta appartiene al progetto (un nuovo stile compare in fondo all'elenco) e fa parte degli stili nei file delle impostazioni del progetto.

**Nessuno stile è lo stile del testo principale.** Nel Pannello stili, fai clic destro su **Nessuno stile** (oppure selezionalo e usa il pulsante Ridefinisci) e scegli **Ridefinisci dalla selezione…**: carattere, dimensione, interlinea, rientro della prima riga, spazio dopo i paragrafi e allineamento del paragrafo al cursore diventano lo stile del testo principale del progetto (i valori di **Progetto > Impostazioni del progetto > Formattazione**). Una domanda mostra prima i valori. Tutto il testo senza stile nell'intero progetto lo segue, anche il testo con un carattere, una dimensione o una spaziatura propri; il testo con stile mantiene ciò che il suo stile imposta. Ogni documento che cambia riceve prima un'istantanea.

**Creare uno stile**:

1. Formatta del testo come deve apparire lo stile e fai clic al suo interno o selezionane una parte.
2. Scegli **Formato > Stile > Nuovo stile dalla selezione…** (oppure il pulsante più del Pannello stili).
3. Inserisci un nome e scegli che cosa salvare: tutta la formattazione (uno stile di paragrafo con formattazione dei caratteri), solo la formattazione del paragrafo oppure gli attributi dei caratteri (uno stile di carattere).
4. Decidi se il carattere e la sua dimensione fanno parte dello stile. Senza di essi lo stile mantiene il carattere del testo circostante.
5. Per gli stili di paragrafo, scegli quale stile riceve il paragrafo successivo quando premi Enter alla fine.

Uno stile di paragrafo memorizza solo ciò che differisce dal testo del corpo. Tutto il resto continua a seguire la formattazione del progetto, così un nuovo carattere predefinito raggiunge anche il testo con stile.

**Cambiare uno stile**:

- **Formato > Stile > Ridefinisci stile dalla selezione…** prende la formattazione del testo al cursore, come sopra. Uno stile di paragrafo resta uno stile di paragrafo, uno stile di carattere uno stile di carattere.
- **Formato > Stile > Modifica stile** (o il pulsante a matita del Pannello stili) apre una finestra con il nome, **Carattere…**, **Paragrafo…** e lo stile del paragrafo seguente. Un'anteprima mostra il risultato.

Dopodiché {{APP}} passa in rassegna l'intero progetto e aggiorna il testo, nei documenti aperti e nei file degli altri. Ciò che la modifica tocca (carattere, dimensione, grassetto, corsivo, colore, spaziatura delle lettere, maiuscole, allineamento, rientri, spaziatura, interlinea, tabulazioni) raggiunge **tutto** il testo dello stile, anche dove quel testo aveva un valore proprio: impostato a mano, o portato da un'importazione (i titoli importati di solito nominano il proprio carattere, dimensione e spaziatura in ogni paragrafo). La formattazione che la modifica non tocca resta com'è, per esempio una parola colorata a mano in un titolo quando è cambiata solo la dimensione del titolo. Ogni documento che cambia riceve prima un'istantanea ("Prima che lo stile ... cambiasse"). Questo non si può annullare con **Modifica > Annulla**; ripristina un'istantanea, ridefinisci di nuovo lo stile oppure usa **Ripristina la definizione incorporata** nel Pannello stili per uno stile incorporato.

**Rinominare ed eliminare**: rinomina uno stile in **Modifica stile**. I titoli di sezione, Title, Subtitle, Block Quote, Centered e Caption mantengono i loro nomi, perché Compila e i file Markdown li conoscono per nome. Quando elimini uno stile (**Formato > Stile > Elimina stile**), il testo che lo usava mantiene il suo aspetto come formattazione impostata a mano.

**Altri progetti**: **Formato > Stile > Importa stili…** legge gli stili di un altro progetto. **Usa questi stili nei nuovi progetti** rende gli stili del progetto aperto l'insieme di partenza di ogni nuovo progetto; **File > Opzioni > Modifica** torna agli stili incorporati.

Nel Pannello stili, i pulsanti a freccia cambiano l'ordine degli stili in tutte le liste.

### Markdown o RTF: come vengono memorizzati i documenti

Ogni progetto memorizza i suoi documenti in uno di due formati. Scegli quando crei o importi un progetto, e puoi cambiare più tardi in **Progetto > Impostazioni del progetto > Formattazione**.

- **Markdown** (consigliato): file di testo semplice che qualsiasi programma può aprire, oggi e fra cinquant'anni. L'aspetto del testo deriva dalla formattazione predefinita. Caratteri e formattazione del paragrafo che imposti a mano vengono scritti nel testo come piccoli marcatori, solo dove li hai usati (sezione 18); tutto il resto resta semplice.
- **RTF**: file di testo formattato come un elaboratore di testi. Ogni paragrafo porta la sua formattazione completa, quindi un cambiamento della formattazione predefinita non raggiunge il testo esistente.

Il cambio converte ogni documento. Da RTF a Markdown vengono mantenuti caratteri, colori, allineamento, rientri sinistro e destro e tabulazioni. Interlinea, rientro della prima riga e spaziatura dei paragrafi del testo arrivato da altri programmi seguono poi il progetto. Fai prima un backup (**File > Backup > Esegui backup ora**).

Il testo incollato da altri programmi in un progetto Markdown assume l'aspetto del testo intorno (grassetto, corsivo e collegamenti restano). Il testo copiato dentro {{APP}} mantiene i suoi caratteri. **Modifica > Incolla con formattazione** (**Ctrl+Alt+Shift+V**) mantiene anche l'aspetto del testo di un elaboratore di testi o di una pagina web: caratteri, dimensioni, colori e l'allineamento, i rientri e la spaziatura dei suoi paragrafi (il primo paragrafo incollato porta la sua formattazione del paragrafo solo in un paragrafo vuoto). **Modifica > Incolla e adatta lo stile** (**Ctrl+Shift+V**) fa l'opposto e incolla testo semplice. Il menu del clic destro del testo li offre tutti e tre, uno sotto l'altro: **Incolla**, **Incolla e adatta lo stile** e **Incolla con formattazione**.

Per convertire un singolo file fuori da un progetto, usa **File > Converti file Markdown ⇄ RTF…**.

### Commenti e note a piè di pagina

- **Commento** (**Shift+F2**): seleziona del testo e aggiungi un commento. Il testo viene evidenziato e il commento compare nell'ispettore.
- **Nota a piè di pagina** (**Ctrl+Shift+F2**): seleziona la parola a cui appartiene la nota; scrivi la nota nell'ispettore.
- **Annotazione in linea** (**Ctrl+Shift+A**): una nota scritta direttamente nel testo, mostrata a colori dentro un fumetto. Le annotazioni vengono rimosse quando compili (a meno che tu non scelga di esportarle come commenti).
- **Nota a piè di pagina in linea** (**Ctrl+Alt+F**): una nota a piè di pagina scritta direttamente nel testo.

Per cambiare il colore di un'annotazione in linea, selezionala (**Modifica > Seleziona > Seleziona annotazione o nota** seleziona l'intera nota al cursore) e scegli un colore del testo; l'annotazione mantiene quel colore. **Formato > Carattere > Colore del testo automatico** le ridà il solito rosso.

Fai clic su un commento nell'ispettore per saltarci. Fai clic destro su di esso per cambiarne il colore, convertirlo in un altro tipo di nota o eliminarlo.

Un clic su un testo commentato apre l'ispettore con quel commento. **Alt+clic** apre invece il commento o la nota a piè di pagina in una piccola finestra al mouse, dove puoi leggerlo e modificarlo senza l'ispettore; fai clic altrove o premi **Esc** per chiuderla.

### Immagini e collegamenti

- **Inserisci > Immagine da file…** mette un'immagine nel testo; puoi anche trascinare un'immagine nel testo o incollarne una. Fai clic destro sull'immagine e scegli **Dimensione immagine…** per cambiare quanto appare grande.
- **Inserisci > Collega a documento…** (**Ctrl+L**) collega il testo selezionato a un altro documento del progetto. Un clic sul collegamento apre quel documento (nell'altro editor, nello stesso editor, nel suo copyholder o in un pannello di riferimento rapido; vedi **File > Opzioni > Comportamento**).
- **Inserisci > Inserisci collegamento web…** collega a una pagina web.
- I collegamenti web e di posta (*http*, *https*, *mailto*) si aprono subito. Qualsiasi altra destinazione di un collegamento, in un testo, una mappa mentale, un segnalibro o una pagina web importata (un file, una cartella, un indirizzo di rete, il tipo di collegamento di un altro programma), viene prima mostrata per intero e aperta solo quando fai clic su **Apri**; collegamenti simili possono arrivare con un progetto, una pagina web o un file Word di qualcun altro. Per un file o una cartella sul tuo computer la domanda nomina il suo tipo (per esempio "documento di testo semplice (.txt)"), il suo nome completo e la sua cartella; un nome lungo va a capo, così se ne può leggere la fine, e un file che non c'è viene indicato come mancante. I collegamenti a programmi, script, scorciatoie e programmi di installazione non vengono mai aperti; aprili tu stesso dal tuo gestore file se ti fidi.

### Tabelle

**Inserisci > Tabella…** chiede il numero di righe e colonne e il design, e mette la tabella al cursore. Dentro una tabella, **Tab** va alla cella successiva (nell'ultima cella aggiunge una riga) e **Shift+Tab** torna indietro. **Formato > Tabella**, anche nel menu del clic destro dentro una tabella, ha il resto:

- **Inserisci riga sopra / sotto**, **Inserisci colonna a sinistra / a destra**.
- **Elimina righe** ed **Elimina colonne** (quelle delle celle selezionate, o della cella con il cursore), **Elimina tabella**.
- **Unisci celle** unisce le celle selezionate in una; **Dividi cella** divide di nuovo una cella unita.
- **Colore della cella…** dà alle celle selezionate uno sfondo proprio; **Colore della cella del design** lo toglie di nuovo.
- **Proprietà della tabella…** cambia il design.

**Il design.** Sono pronti cinque design: **Griglia**, **Griglia con riga di intestazione**, **Righe a strisce**, **Linee tra le righe** e **Nessuna linea**. Scegline uno e cambia ciò che ti pare; il design diventa allora **Personalizzato**:

- **Linee**: spessore (0: nessuna linea), colore e **Solo tra le righe** (nessuna linea ai lati).
- **Celle**: lo spazio dentro una cella e lo spazio tra le celle (0: le celle vicine condividono una linea).
- **Righe di intestazione**: quante righe in alto sono titoli, con il loro colore. Stampa e compilazione le ripetono in cima a ogni pagina su cui continua una tabella lunga. **Una riga su due** riceve un colore proprio se ne scegli uno.
- **Posizione nel testo**: la larghezza (una quota della larghezza del testo, o larga quanto il suo contenuto), le larghezze delle colonne (tutte uguali, o quote come "30, 70"), l'allineamento e dove sta: **Su righe proprie**, oppure **A sinistra** o **A destra** con il testo che scorre accanto.

Le righe e le colonne che aggiungi dopo mantengono il design (le strisce continuano). Il testo nelle celle di una tabella non è giustificato, anche quando lo è il testo del corpo (una cella stretta allargherebbe le sue parole); un allineamento che dai a un paragrafo in una cella resta. Le tabelle vengono conservate in entrambi i formati di memorizzazione e nel formato Scrivener; anche le tabelle nei file RTF di altri programmi (per esempio un file Word salvato come RTF) diventano tabelle.

### Immagini accanto al testo, didascalie e descrizioni

Fai clic destro su un'immagine e scegli **Immagine…** (oppure metti il cursore accanto ad essa e scegli **Formato > Immagine…**):

- **Larghezza sullo schermo**: l'altezza segue; **Dimensione originale** torna alla dimensione propria dell'immagine.
- **Posizione**: **In linea, come una lettera** (come prima), **A sinistra** o **A destra** con il testo che scorre intorno, oppure **Centrata su righe proprie**.
- **Didascalia**: mostrata sotto l'immagine e compilata con essa. Un'immagine in linea non ha una didascalia; dalle prima una posizione.
- **Descrizione**: che cosa mostra l'immagine, in una o due frasi. Non viene mostrata nel testo. Gli e-book, i file Word e Markdown la conservano con l'immagine (come suo "testo alternativo"), e i lettori di schermo la leggono alle persone che non possono vedere l'immagine.

L'immagine con la sua didascalia si sposta come un unico pezzo; scrivi la didascalia sotto l'immagine come qualsiasi altro testo (ha lo stile **Caption**).

### Trova e sostituisci

- **Ctrl+F** (**Modifica > Trova > Trova**) apre la barra di ricerca sopra il testo. Mostra quante corrispondenze ci sono e le evidenzia; **F3** e **Shift+F3** vanno alla corrispondenza successiva e precedente. **Maiuscole/minuscole** e **Parole intere** restringono la ricerca.
- **Ctrl+H** aggiunge la riga di sostituzione: **Sostituisci** cambia la corrispondenza selezionata e va alla successiva, **Sostituisci tutto** cambia ogni corrispondenza nel testo che vedi (un solo passo per **Annulla**). Il nuovo testo assume la formattazione del testo che sostituisce e i commenti restano collegati.
- **Ctrl+E** (**Usa la selezione per la ricerca**) mette il testo selezionato nella barra di ricerca.
- Nel testo combinato, trova e sostituisci agiscono su tutti i documenti mostrati.
- Mentre la tastiera è in un **copyholder**, Ctrl+F cerca nel documento del copyholder.
- **Ctrl+Shift+F** cerca nell'intero progetto, **Modifica > Trova > Sostituisci nel progetto** sostituisce nell'intero progetto e **Modifica > Trova > Trova per formattazione** cerca evidenziazioni, commenti, colori e stili (tutto nella sezione 9).
- Nella bacheca e nella struttura, Ctrl+F apre invece la **barra del filtro** (sezione 5).

### Ortografia

Le parole sbagliate ricevono una sottolineatura rossa ondulata; fai clic destro in un punto qualsiasi della parola sottolineata per avere dei suggerimenti o per aggiungere la parola al tuo dizionario (anche sulle parole con un apostrofo, come "Kayel's" o "D'Nersun": il menu offre sempre l'intera parola sottolineata). Su una parola sbagliata il menu è breve, così si apre proprio al mouse: i suggerimenti, **Aggiungi "..." al dizionario**, **Ignora "..."** e **Altri comandi** con tutto ciò che il menu contiene altrove (Taglia, Copia, Incolla, Stile, commenti, Aspetto ...). Il nome del programma e i suoi termini ({{APP}}, outliner, subdocument, copyholder) contano sempre come corretti. Scegli la lingua in **File > Opzioni > Correzioni** (pagina Ortografia); i dizionari che hai installato come componenti aggiuntivi (uno per lingua del programma, ciascuno con la sua licenza) compaiono in quell'elenco; lì scegli anche la sottolineatura (onde, punti o una linea retta), se le parole con numeri ("Route66") vengono controllate, e **Visualizza elenco delle parole personali...** mostra le parole che hai aggiunto e rimuove quelle che hai aggiunto per errore. I dizionari provengono dal tuo sistema e dai componenti aggiuntivi. Su Ubuntu e Kubuntu è installato l'inglese; per altre lingue installa in Discover il pacchetto *hunspell* corrispondente (per esempio "hunspell-it" per l'italiano).

### Inserimenti speciali

Il menu **Inserisci** offre anche la data e l'ora correnti, i caratteri speciali (trattino lungo, trattino medio, puntini di sospensione, spazio non separabile, simbolo di paragrafo, punto elenco), le **numerazioni automatiche** e i **segnaposto**, che Compila sostituisce (sezione 13).

### Correzioni durante la digitazione

**File > Opzioni > Correzioni** imposta che cosa viene corretto mentre scrivi. Una parola viene corretta quando la finisci, cioè quando dopo di essa scrivi uno spazio, un Return o un segno di punteggiatura; **Ctrl+Z** subito dopo annulla la sola correzione.

- **Correggi le maiuscole delle frasi**: la prima parola di una frase riceve la lettera maiuscola (non dopo abbreviazioni come "Dr." o "e.g."). **Maiuscola per "i"**: "i" e "i'm" diventano "I" e "I'm". Entrambe sono inizialmente disattivate.
- **Virgolette intelligenti** nello stile che scegli (“inglese”, „tedesco“, »tedesco«, «francese», ”svedese”); un apostrofo dentro una parola è sempre ’. Dove il segno singolo di chiusura non è l'apostrofo (tedesco ‚…‘, francese ‹…›), un ' scritto subito dopo una parola chiude un segno singolo aperto nel paragrafo; se poi continui a scrivere lettere ("geht's"), torna a essere l'apostrofo. Due trattini diventano un trattino lungo (—); tre punti possono diventare un unico carattere di puntini di sospensione (inizialmente disattivato). In modalità script queste correzioni si possono disattivare.
- **Virgolette già presenti nel progetto** (dalla 0.61.1): sotto **Virgolette intelligenti**, **Converti in virgolette tipografiche…** trasforma le " e gli ' dritti del progetto nei segni dello stile scelto accanto a **Usa virgolette intelligenti**, e **Converti in virgolette dritte…** riporta ogni virgoletta tipografica e ogni apostrofo a " e '. Si apre una finestra come Sostituisci nel progetto (sezione 9): la direzione e di nuovo lo stile, dove convertire (titoli, testo, note, sinossi, metadati personalizzati, commenti e note a piè di pagina), l'ambito (l'intero progetto o i documenti selezionati nel raccoglitore, il Cestino solo se lo spunti) e un'istantanea di ogni testo prima che cambi (inizialmente spuntata). **Conta** mostra quanti segni cambierebbero; **Converti** chiede ancora una volta e lo fa. La formattazione resta (una virgoletta in grassetto resta in grassetto). Un segno si apre dopo nulla, uno spazio, una parentesi, un trattino o un segno di apertura, e si chiude negli altri casi; un apostrofo dentro una parola o prima di un numero ('90s) diventa ’. Un segno dritto con uno spazio da entrambi i lati resta com'è, e un apostrofo all'inizio di una parola ('tis) diventa un segno di apertura: controllali poi. Trova e sostituisci non può farlo, perché un segno deve diventare di apertura o di chiusura a seconda di ciò che gli sta accanto.
- **Rilevamento dei dati**: un indirizzo web scritto diventa un collegamento; **[[Titolo]]** diventa un collegamento al documento con quel titolo e, se non ce n'è nessuno, accanto al testo viene creato un nuovo documento vuoto con quel nome (i "collegamenti ai documenti" di Scrivener).
- **Sostituzioni aggiuntive**: una lista di ciò che scrivi e di ciò che diventa, per esempio "(c)" e ©, "->" e →. **Modifica sostituzioni...** cambia la lista; **Sostituzioni standard** riporta la lista con cui il programma è arrivato.
- **Completamento automatico**: in modalità script, la lista di nomi di personaggi e intestazioni di scena offerta mentre scrivi.

### Zoom, scorrimento macchina da scrivere e larghezza fissa

- **Ctrl+rotellina del mouse** fa lo zoom dell'area sotto il mouse, ovunque si trovi: gli editor, il raccoglitore, l'ispettore, la struttura, la bacheca (schede e relativo testo insieme), una mappa mentale, la modalità composizione e le finestre a sé (riferimento rapido, strumenti di scrittura); la tastiera può essere altrove. Ogni area mantiene il suo zoom finché non lo cambi di nuovo.
- **Ctrl++ / Ctrl+-** o la casella dello zoom nel piè di pagina ingrandiscono il testo sullo schermo senza cambiare la dimensione del carattere. **Vista > Ripristina zoom** (**Ctrl+0**) torna allo zoom predefinito del testo.
- **File > Opzioni > Modifica** imposta lo zoom con cui parte ogni area ("Zoom predefinito del testo", per ogni area della finestra e per ogni parte con uno zoom proprio: la sinossi, le note, le liste dei segnalibri, il testo dei segnalibri, la lista e il testo delle istantanee, la lista e la scheda degli oggetti nell'ispettore e nella finestra Oggetti, le nuove mappe mentali) e se gli editor si riaprono con lo zoom che avevano per ultimo ("Ricarica l'ultimo zoom scelto degli editor principali").
- **Dove eri.** {{APP}} ricorda, con il progetto, la posizione del cursore e la parte di testo sullo schermo per ogni documento: mostrato di nuovo (dal raccoglitore, da un collegamento, con Indietro e Avanti, nel copyholder, in un pannello di riferimento rapido o dopo aver riaperto il progetto), un documento si apre dove l'avevi lasciato. Una cartella nel testo combinato ricorda quale dei suoi documenti aveva il cursore. Un risultato di ricerca o un collegamento che punta a un certo punto va invece lì. Le immagini mantengono il loro zoom e i file PDF il loro zoom e la loro pagina.
- Lo **scorrimento macchina da scrivere** (**Vista > Modifica del testo**) mantiene la riga che stai scrivendo al centro dello schermo.
- L'**editor a larghezza fissa** mostra il testo come una colonna centrata di larghezza comoda (500 px al 100 % a meno che tu non la cambi), per quanto sia larga la finestra. Disattivalo, o cambia la larghezza, in **File > Opzioni > Aspetto > Editor principale** ("Usa editor a larghezza fissa", "Larghezza della colonna di testo"), oppure con un clic destro sulla carta (**Aspetto > Larghezza della colonna di testo**). La carta della colonna è #d1d495 e l'area accanto #999999 come standard ("Carta" e "Accanto alla colonna di testo" tra i colori dell'Editor principale); restano uguali quando scegli un altro tema, finché non li cambi o ripristini.
- **Bordo accanto al testo.** A sinistra e a destra del testo la carta ha un bordo del proprio colore, così il testo non tocca il margine della carta: 10 px al 100 % per i nuovi progetti, che cresce con lo zoom. Sta fuori dalla colonna di testo, quindi le righe vanno a capo dove andavano. Cambialo in **File > Opzioni > Aspetto > Editor principale** ("Bordo accanto al testo", da 0 a 100 px) oppure con un clic destro sulla carta (**Aspetto > Bordo accanto al testo**). I progetti creati prima della 0.40.2 mantengono l'aspetto che avevano, senza bordo, finché non ne dai loro uno. Un clic sul bordo mette il cursore all'inizio o alla fine della riga accanto.
- **Fare clic e clic destro** non spostano mai la vista: il cursore va dove fai clic e il testo resta dov'è sullo schermo (prima della 0.40.2, un clic o un clic destro vicino al bordo superiore o inferiore, o un clic destro accanto al testo, poteva far saltare il testo).

### Vista pagina

**Vista > Modifica del testo > Vista pagina** (anche nel menu del pulsante **Vista** della barra degli strumenti) mostra il testo su fogli di carta invece che in un'unica colonna infinita. Il piè di pagina mostra allora la pagina del cursore e il numero di pagine, per esempio "Pagina 3 di 12". Scegli di nuovo il comando per tornare alla colonna. Il colore della scrivania intorno alle pagine si imposta in **File > Opzioni > Aspetto > Editor principale** (oppure clic destro > **Aspetto** mentre le pagine sono mostrate).

Le pagine sono un modo di guardare il testo, nulla di più. Non mostrano che aspetto avrà un manoscritto compilato o stampato: scrivi a pezzi, e titoli, caratteri e dimensione della pagina vengono decisi quando compili (sezione 13). Nulla delle pagine viene memorizzato nei tuoi documenti.

- **Vista > Modifica del testo > Dimensione e margini della pagina…** imposta la carta (A4, A5, US Letter e altre, o una dimensione tua), verticale o orizzontale, e i quattro margini. L'impostazione appartiene al progetto; spunta **Usa questa pagina anche per i nuovi progetti** per renderla il tuo standard.
- Lo zoom ingrandisce insieme fogli, margini e lettere. Un foglio più largo dell'editor può essere fatto scorrere lateralmente. Come nella vista a colonna, rientri e tabulazioni mantengono la loro lunghezza reale, quindi le interruzioni di riga possono differire un poco tra i livelli di zoom.
- Nel testo combinato, ogni documento inizia su un foglio a sé e il suo ultimo foglio termina subito sotto il testo, così i documenti brevi non riempiono lo schermo di carta vuota.
- La vista pagina vale per tutto ciò che mostra i tuoi testi nella finestra principale e nelle finestre del riferimento rapido: entrambi gli editor e i copyholder. Le note mantengono il loro aspetto semplice e la modalità composizione ha una propria carta.
- Il colore intorno ai fogli è **Intorno alle pagine (vista pagina)** in **File > Opzioni > Aspetto > Editor principale**.

### Scrittura di script

Sceneggiature, testi teatrali e script a fumetti si scrivono in **modalità script**. Uno script è fatto di *elementi*: intestazione di scena, azione, personaggio, indicazione tra parentesi, dialogo, transizione e così via. Ogni elemento ha il suo posto sulla pagina e in modalità script i tasti **Tab** ed **Enter** ti portano da un elemento al successivo, così non devi mai impostare un rientro a mano.

**Attivarla.** Fai clic in un documento e scegli **Formato > Scrittura di script > Modalità script** (**Ctrl+8**). La voce di menu nomina il formato script del progetto, per esempio *Modalità script - Sceneggiatura*. La modalità script appartiene al documento: script e testi normali vivono fianco a fianco in uno stesso progetto, e gli script hanno un'**icona gialla** nel raccoglitore. Scegli di nuovo il comando per disattivarla; il testo mantiene il suo aspetto. Anche un nuovo documento creato tra gli script (in una cartella i cui testi sono tutti script) è uno script.

**Il piè di pagina.** In uno script, il piè di pagina dell'editor mostra, al posto del conteggio delle parole, che cosa faranno **Tab** ed **Enter** al cursore e, a destra, un menu a comparsa con l'elemento del paragrafo corrente. Scegli un altro elemento da quel menu e il paragrafo viene riformattato. Il testo che non è un elemento dello script viene mostrato come **Testo generale**.

**Tab ed Enter.**

- **Enter** alla fine di un paragrafo inizia l'elemento che di solito segue: l'azione dopo un'intestazione di scena, il dialogo dopo il nome di un personaggio e così via.
- **Enter** su una riga vuota apre la lista degli elementi al cursore; premi la lettera sottolineata nella lista (S per intestazione di scena, C per personaggio ...). **Formato > Scrittura di script > Mostra l'elenco degli elementi** (**Ctrl+\\**) apre la stessa lista in qualsiasi momento.
- **Tab** su una riga vuota cambia la riga in un altro elemento; premilo di nuovo per il successivo, **Shift+Tab** per tornare indietro.
- **Tab** alla fine di un paragrafo fa ciò che si addice all'elemento: dopo il nome di un personaggio o una riga di dialogo inizia un'indicazione tra parentesi, e in un'intestazione di scena scrive " - ", pronto per l'ora del giorno.
- **Enter** in mezzo a un paragrafo lo divide; entrambe le metà mantengono l'elemento. **Shift+Enter** è un'interruzione di riga dentro il paragrafo.
- Gli elementi scritti in maiuscolo (intestazioni di scena, nomi di personaggi, transizioni) trasformano in maiuscolo ciò che scrivi. Parentesi e prefissi come "SFX: " vengono scritti al posto tuo.

**Completamento automatico.** Mentre scrivi, una piccola lista offre ciò che si adatta all'elemento: INT. ed EXT. all'inizio di un'intestazione di scena, le ore del giorno dopo il suo " - ", i luoghi e i nomi di personaggi già usati nel documento e le transizioni consuete come CUT TO:. **Enter** o **Tab** prende la voce evidenziata, i tasti freccia ne scelgono un'altra, **Esc** chiude la lista.

**Cambiare gli elementi.** **Formato > Scrittura di script > Cambia elemento in** cambia il paragrafo con il cursore, o tutti i paragrafi selezionati. Scegliere lì un elemento in un documento normale rende il documento uno script. Ogni elemento è un comando, quindi puoi dare una scorciatoia ai tuoi preferiti in **File > Opzioni > Tastiera**.

**Formati script.** Un progetto usa un solo formato script per tutti i suoi script. **Formato > Scrittura di script > Formato script** elenca *Sceneggiatura*, *Testo teatrale* e *Sceneggiatura a fumetti*, i formati dei componenti aggiuntivi e i tuoi. Quando scegli un altro formato, gli script del progetto ricevono il nuovo layout: ogni elemento diventa l'elemento dello stesso tipo nel nuovo formato (l'azione diventa una didascalia di regia, per esempio). Un elemento senza corrispondente diventa testo generale che mantiene il suo aspetto; la domanda prima del cambiamento nomina questi elementi.

**Impostazioni script.** **Formato > Scrittura di script > Impostazioni script…** cambia il formato del progetto o ne costruisce uno nuovo:

- il nome del formato, il suo carattere (per impostazione predefinita il carattere da macchina da scrivere del tuo sistema: Courier Prime, Courier New o un carattere simile) e la dimensione, e se le lunghezze sono in pollici o centimetri;
- la lista degli elementi: aggiungili, rimuovili e riordinali (l'ordine è l'ordine della lista degli elementi e di **Tab** su una riga vuota);
- per ogni elemento: nome, tipo (che cos'è per gli altri programmi: intestazione di scena, azione, personaggio ...) e la sua lettera nella lista; grassetto, corsivo, sottolineato, maiuscole e colore; allineamento, rientri da entrambi i margini, righe vuote prima di esso e interlinea; che cosa fanno **Enter** e **Tab**; un testo che viene scritto prima e dopo il paragrafo; e le liste per il completamento automatico;
- un'anteprima di tutti gli elementi.

Un elemento rinominato mantiene i suoi paragrafi in ogni script del progetto. Il pulsante **Gestione** parte da un altro formato, carica o salva un file di formato (*.qbscript*) e salva il formato per usarlo in altri progetti.

**Da sapere.**

- I tasti e le liste funzionano allo stesso modo nel testo combinato, nel secondo editor, nei copyholder, nei pannelli di riferimento rapido e in modalità composizione. Solo gli editor della finestra principale hanno il piè di pagina con i suggerimenti; altrove **Ctrl+\\** mostra la lista degli elementi.
- La vista pagina (sopra) mostra uno script su fogli; con il solito carattere da macchina da scrivere a 12 punti, una pagina di sceneggiatura corrisponde grosso modo a un minuto di film, ma le pagine sullo schermo non sono le pagine del file compilato.
- Il carattere e i rientri degli elementi appartengono al formato script, non al documento. I file dei tuoi script contengono solo i nomi degli elementi (sezione 18).
- Importare ed esportare script, nei formati di altri programmi di sceneggiatura: sezione 14. Compilare uno script intero: sezione 13.
- Gli script richiedono la versione 0.10 o successiva. Un progetto che contiene script lo dice nel proprio file di progetto, e da questa versione in poi {{APP}} chiede prima di aprire un progetto che richiede una versione più recente di quella in esecuzione. Le versioni 0.9 e precedenti non chiedono: mostrano gli script come testo normale e perdono gli elementi di ogni script che vi guardi.

---

## 5. Tre modi di guardare il tuo progetto

Scegli la vista con i tre pulsanti della barra degli strumenti o con **Ctrl+1**, **Ctrl+2** e **Ctrl+3**.

### Testo combinato (Ctrl+1): molti documenti come un unico testo

Seleziona più documenti nel raccoglitore (Ctrl+clic o Shift+clic) o una cartella e premi **Ctrl+1**. Tutti i testi compaiono uno dopo l'altro, separati da una sottile linea tratteggiata, e puoi scrivere in tutti come se fossero un unico documento. Le modifiche vanno automaticamente nei documenti giusti. Mostra i titoli dei documenti tra di essi con **Vista > Modifica del testo > Mostra i titoli nel Testo combinato**.

### Bacheca (Ctrl+2): schede

Ogni documento è una scheda che mostra il suo titolo e la sua sinossi; un documento senza sinossi mostra invece le prime righe del suo testo (sbiadite, in corsivo). La scheda di un'immagine mostra l'immagine, e così la scheda di un documento la cui sinossi è un'immagine (sezione 7). Trascina le schede per riordinare i documenti. Fai doppio clic su una scheda per modificarne il titolo o la sinossi. Premi **Enter** per aprire le schede selezionate come testo.

Nel piè di pagina della bacheca:

- **Griglia** o **libera**: in modalità libera puoi mettere le schede dove vuoi, come su una vera bacheca di sughero. **Applica ordine libero** riordina poi i documenti nel raccoglitore in base alle schede da sinistra a destra, dall'alto in basso. La disposizione che scegli per una cartella viene ricordata con quella cartella (dalla 0.90.0): la volta successiva che la mostri, la sua bacheca è disposta allo stesso modo. Una cartella per cui non hai mai scelto una disposizione mantiene quella che mostra l'editor.
- **Fili per etichetta**: dispone le schede in righe per etichetta.
- **Dimensione delle schede**, con il cursore, i tasti freccia o la rotellina del mouse sopra di esso; viene conservata subito (**Ctrl+rotellina del mouse** sulla bacheca fa lo zoom delle schede e del loro testo insieme, come le altre aree, vedi sezione 4).

Un doppio clic su un punto vuoto della bacheca crea una nuova scheda (**File > Opzioni > Comportamento > Doppio clic**). Quando modifichi una sinossi su una scheda, **Return** termina la modifica e **Shift+Return** va a capo (**File > Opzioni > Comportamento > Tasto Return**, dove Return può anche essere impostato per creare una nuova scheda o riga della struttura dopo quella selezionata).

I timbri di stato, la modalità libera e la disposizione per etichetta si trovano anche in **Vista > Opzioni della bacheca**; le tinte delle etichette per schede e raccoglitore in **Vista > Usa colore etichetta in**. Anche le colonne della struttura sono elencate in **Vista > Opzioni della struttura**.

### Struttura (Ctrl+3): una tabella

Mostra i documenti come una tabella con titolo, sinossi, etichetta, stato, conteggi di parole, obiettivi, date e i tuoi metadati personalizzati. Fai clic sul pulsante delle colonne nel piè di pagina per scegliere le colonne. Fai clic sull'intestazione di una colonna per ordinare (solo per la visualizzazione; l'ordine del raccoglitore non cambia). Trascina le righe per riordinare i documenti.

### Filtrare la bacheca e la struttura

Mentre la bacheca o la struttura ha la tastiera, **Ctrl+F** (**Modifica > Trova > Trova**) apre una **barra del filtro** in cima all'editor. Mentre scrivi, restano solo le schede o le righe che corrispondono; la barra dice quante ("2 di 9"). Nella struttura restano i gruppi dei documenti corrispondenti, così vedi a che cosa appartengono.

- Fai clic sul **triangolo** a sinistra del campo per le opzioni: dove cercare (**Titoli**, **Sinossi**, **Parole chiave**, **Testo**), un'**etichetta**, uno **stato**, una **parola chiave**, se il documento è **incluso nella compilazione** e **Mostra ciò che non corrisponde**. Più scelte insieme restringono ulteriormente la lista.
- Il filtro resta mentre apri altre cartelle, così puoi percorrere il progetto guardando solo, per esempio, le scene di un personaggio.
- **Fatto** rimuove il filtro e mostra di nuovo tutto.

Trascinare le schede riordina ancora i documenti: una scheda rilasciata tra due schede che vedi finisce accanto ad esse nel raccoglitore. **Applica ordine libero** della bacheca libera è disattivato mentre un filtro nasconde delle schede.

---

## 6. Due editor, segnalibri e riferimento rapido

### Editor diviso

Fai clic sul pulsante di divisione a destra dell'intestazione dell'editor per mostrare un secondo editor. Alt+clic divide nell'altra direzione. **Vista > Dividi editor** offre le stesse scelte. **F6** salta tra gli editor. Un clic nel raccoglitore mostra il documento nell'editor che hai usato per ultimo; **Naviga > Effetto della selezione del raccoglitore** lo cambia in editor di sinistra o superiore, editor di destra o inferiore, oppure l'altro editor, qualunque sia quello in cui stai scrivendo.

In una bacheca o in una struttura, il pulsante del piè di pagina **La selezione agisce sull'altro editor** (anche in **Vista > Opzioni della bacheca** e **Opzioni della struttura**) fa mostrare all'altro editor la scheda su cui fai clic. È un modo rapido per sfogliare le scene mantenendo la panoramica dei capitoli.

**Naviga > Editor > Blocca in posizione** mantiene un editor sul suo documento, anche quando fai clic altrove nel raccoglitore.

### Copyholder

Ogni editor ha un **copyholder**: una seconda vista semplice dentro l'editor che contiene un altro documento come riferimento, per esempio delle note o la scheda di un personaggio. Con due editor e i loro copyholder puoi vedere quattro documenti contemporaneamente.

- **Per aprirlo**, tieni premuto **Alt** e trascina un documento dal raccoglitore, da una bacheca o da una struttura sulla barra dell'intestazione dell'editor. Senza Alt, il rilascio carica il documento nell'editor stesso. **Naviga > Apri > nel copyholder** e il menu del clic destro di un documento fanno lo stesso per il documento selezionato.
- Il copyholder ha una stretta barra di intestazione con il titolo e nessun piè di pagina. Mostra il testo di un documento (o un'immagine o un PDF); non ha bacheca, struttura né testo combinato. Puoi scrivere e formattare al suo interno come in qualsiasi editor.
- **Per spostarlo**, fai clic destro sulla sua barra di intestazione e scegli sinistra, destra, sopra o sotto. La prima volta si apre accanto a un editor largo e sotto uno stretto; poi, dove l'hai messo per ultimo. Trascina la linea tra editor e copyholder per cambiarne le dimensioni.
- **Per mostrare un altro documento**, rilascialo sulla barra di intestazione del copyholder, oppure trascinalo di nuovo con Alt sulla barra di intestazione dell'editor.
- **Per chiuderlo**, fai clic sulla **X** a destra della sua barra di intestazione, oppure scegli **Naviga > Editor > Chiudi copyholder**. Il pulsante a sinistra della X lo chiude e apre invece il documento come finestra di riferimento rapido.

**Navigare con un copyholder.** Mentre un editor mostra una bacheca o una struttura e ha il suo copyholder aperto, il pulsante **La selezione agisce su** nel piè di pagina ha tre posizioni. Il primo clic fa aprire la tua selezione nell'altro editor, come al solito. Il secondo clic gira la freccia in modo che punti dentro il riquadro: ora la scheda o la riga su cui fai clic si apre nel copyholder. Un terzo clic lo disattiva. Una disposizione utile: il manoscritto nell'editor di sinistra, una struttura della tua ricerca nell'editor di destra con il suo copyholder sotto, e **Naviga > Effetto della selezione del raccoglitore > Editor di sinistra o superiore**. Il raccoglitore comanda allora il manoscritto e la struttura comanda il copyholder.

Il copyholder, la sua posizione e il suo documento vengono ricordati con il progetto.

### L'intestazione dell'editor

L'intestazione sopra ogni editor mostra il titolo del documento e contiene i pulsanti **indietro** e **avanti** (**Ctrl+[** e **Ctrl+]**), un menu delle sezioni del documento e i pulsanti per il documento precedente e successivo (**Alt+Shift+Up / Down**).

- **Cronologia**: ogni editor ricorda i documenti che ha mostrato. Tieni premuto il pulsante indietro o avanti (oppure fai clic destro su di esso) per una lista e scegli un documento per andarci direttamente; **Naviga > Indietro a** e **Avanti ad** mostrano le stesse liste. La cronologia viene conservata con il progetto.
- **L'icona** accanto al titolo rappresenta il documento. Fai clic su di essa per un menu (Blocca in posizione, Mostra nel raccoglitore ...). Trascinala come un documento nel raccoglitore: sulla barra di intestazione dell'altro editor, nel raccoglitore, su una scheda di raccolta, sul pulsante Riferimento rapido o Segnalibri della barra degli strumenti, oppure sul testo per creare un collegamento.
- **Documenti > Raggruppa** (**Ctrl+Shift+J**, anche nel menu contestuale del raccoglitore) sposta gli elementi selezionati in una nuova cartella al posto del primo; il suo titolo è pronto per essere scritto. Con la tastiera nel raccoglitore, **Modifica > Annulla** annulla l'intero passo.
- **Documenti > Converti > Scheda in oggetto** (anche nel menu contestuale del raccoglitore) crea un Oggetto da ogni documento scheda selezionato, per esempio una scheda di personaggio o di luogo: il titolo è il nome, il testo va in **Altro**, un'immagine nel testo diventa l'immagine. Il nome della cartella (Personaggi, Luoghi) sceglie il tipo; per un'altra cartella ti viene chiesto. L'elemento del raccoglitore resta: selezionarlo mostra l'Oggetto nell'ispettore e Return lo apre nella finestra Oggetti. Il documento mantiene il suo testo (un progetto Scrivener vede un normale documento); il collegamento viene conservato nei dati Oggetti propri del progetto. **Modifica > Annulla** con la tastiera nel raccoglitore lo annulla.
- **Naviga > Vai a** elenca l'intero raccoglitore come un menu e carica il documento scelto nell'editor attivo. **Naviga > Gruppo contenitore** mostra la cartella o il gruppo a cui appartiene il documento, con il documento selezionato lì.
- Un **doppio clic sul divisore** tra due editor li rende della stessa dimensione.

### Trascinare e rilasciare in sintesi

| Trascina ... | su ... | e ... |
|---|---|---|
| un documento (raccoglitore, raccolta, bacheca, struttura, icona dell'intestazione, segnalibro) | la barra di intestazione di un editor | viene mostrato in quell'editor; con **Alt**: nel suo copyholder |
| | il testo in un editor | un collegamento al documento; sul testo selezionato, quel testo diventa il collegamento |
| | il testo, tenendo premuto **Alt** | il testo del documento viene inserito |
| un'immagine dal raccoglitore | il testo in un editor | l'immagine viene inserita |
| un documento | il pulsante **Riferimento rapido** della barra degli strumenti | ogni documento si apre in un pannello a sé |
| | il pulsante **Segnalibri** della barra degli strumenti | viene aggiunto ai segnalibri del progetto |
| | una scheda di raccolta, una lista di segnalibri | viene aggiunto |
| | la barra di intestazione di un pannello di riferimento rapido | viene mostrato in quel pannello |
| | il gestore file o il desktop | viene salvato lì come file (vedi "Esportare i documenti") |
| il testo selezionato | il raccoglitore | un nuovo documento con quel testo; il testo resta dov'era |
| i file dal gestore file | il raccoglitore o una bacheca | vengono importati |
| un collegamento web da un browser | una cartella nel raccoglitore, la bacheca, la struttura | la pagina viene importata (componente aggiuntivo Web Page Import, sezione 14) |
| un file immagine, o un'immagine dal raccoglitore | la scheda nell'ispettore (modalità immagine) | diventa l'immagine della scheda |
| le parole chiave da **Progetto > Parole chiave del progetto** | un documento nel raccoglitore, una raccolta, la bacheca, la struttura; la lista delle parole chiave dell'ispettore | vengono assegnate; con **Alt**: insieme al gruppo a cui appartengono |

Alcuni desktop usano Alt+trascinamento per spostare le finestre. Se Alt+trascinamento non arriva al programma, cambia il tasto modificatore per spostare le finestre nelle impostazioni del tuo desktop (KDE: Impostazioni di sistema, comportamento delle finestre; il punto esatto differisce tra le versioni), oppure usa invece i comandi di menu (**Naviga > Apri > nel copyholder**, **Progetto > Parole chiave del progetto > Assegna ai documenti selezionati**).

### Pannelli di riferimento rapido

Un pannello di riferimento rapido è un documento in una finestra a sé, per esempio per tenere in vista la scheda di un personaggio mentre scrivi. Il testo in un pannello può essere modificato e formattato come nell'editor (lì funzionano le scorciatoie dei menu Formato e Inserisci).

**Per aprire i pannelli:**

- **Naviga > Apri > come riferimento rapido**, oppure il pulsante Riferimento rapido della barra degli strumenti, per i documenti selezionati.
- Premi **Space** nella bacheca o nella struttura, oppure nel raccoglitore e in una lista di raccolta o di risultati di ricerca quando **Vista > Tasti di aggiunta rapida nel raccoglitore** è attivo (vedi sezione 2, i tasti del raccoglitore). Senza quell'opzione, Space nel raccoglitore appartiene alla digitazione di un nome per saltarci.
- Trascina i documenti sul **pulsante Riferimento rapido** della barra degli strumenti: ognuno si apre in un pannello a sé e l'editor mantiene il suo documento.
- **Naviga > Apri riferimento rapido** mostra l'intero raccoglitore come un menu, così puoi aprire un documento senza selezionarlo prima.

**In un pannello:**

- Il **titolo** nella barra di intestazione può essere modificato (rinomina il documento). L'**icona** accanto ad esso apre un menu (Mostra nel raccoglitore, Apri nell'editor, aggiungi ai segnalibri del progetto o rimuovi da essi) e può essere trascinata come il documento stesso. Rilascia un documento sulla barra di intestazione per mostrarlo in questo pannello.
- Il pulsante in **alto a destra** mostra la **sinossi**, le **note**, le **parole chiave** o i **metadati** del documento sotto il suo testo. Ciò che cambi lì cambia il documento.
- La **barra del piè di pagina** mostra il conteggio delle parole e ti permette di impostare **etichetta** e **stato**.
- Il **pulsante dei segnalibri** a sinistra della barra del piè di pagina apre i **segnalibri del progetto** accanto al documento. Un clic su un documento con segnalibro lo mostra nel pannello, così un solo pannello basta per spostarsi tra le tue note. Trascina i segnalibri in un altro ordine, rilascia dei documenti e rimuovi un segnalibro con il pulsante ➖ (il documento resta nel raccoglitore). File e collegamenti web si aprono con un doppio clic.
- Il pulsante ➕ sotto i segnalibri crea un **nuovo documento** e gli mette un segnalibro. La prima volta ti viene chiesta la cartella (per esempio una cartella "Note" in Ricerca); spunta **Non chiedere più** per usare da allora quella cartella. Cambiala in **Progetto > Impostazioni del progetto > Cartelle speciali**.
- Il pulsante **Fluttuante** mantiene il pannello sopra la finestra principale; **Finestra > Pannelli di riferimento rapido mobili** lo fa per tutti.

Il menu Finestra elenca i pannelli aperti e li chiude tutti insieme. I pannelli aperti quando chiudi un progetto tornano quando lo riapri, con la loro posizione, dimensione, riquadro inferiore e barra laterale (disattivalo in **File > Opzioni > Comportamento**).

### Audio e video

I file audio e video nel raccoglitore (per esempio interviste nella cartella Ricerca) vengono riprodotti dentro {{APP}} quando è installato il componente aggiuntivo **Media Playback** (sezione 17; arriva con l'aggiornamento 0.12). Senza di esso, un file simile mostra **Apri con l'applicazione predefinita**, come prima.

**Riprodurre.** Fai clic sul file nel raccoglitore. Un video mostra la sua immagine; un file audio mostra il suo titolo, l'artista e la durata. La **barra del piè di pagina** dell'editor ha i controlli di riproduzione:

- il pulsante **pausa e riavvolgi** a sinistra (vedi sotto),
- **vai all'inizio**, **salta indietro**, **riproduci / pausa**, **salta avanti**,
- la **posizione**: trascina il cursore, oppure fai clic accanto alla sua maniglia,
- il tempo, la **velocità** (da 0,5× a 2×; un parlato più lento è più facile da trascrivere; la voce diventa più bassa o più alta con la velocità, come un nastro) e il **volume** (clic: muto; la freccia: un cursore).

Un clic sull'immagine di un video lo riproduce o lo mette in pausa; un clic destro offre anche **Apri con l'applicazione predefinita**. Con la tastiera nel lettore: **Space** riproduce o mette in pausa, i **tasti freccia sinistra** e **destra** saltano, **Home** va all'inizio.

**Trascrivere.** Dividi l'editor, mostra il file audio in un editor e la tua trascrizione nell'altro e continua a scrivere: i comandi di **Naviga > Multimedia** funzionano ovunque sia la tastiera, anche nei pannelli di riferimento rapido e in modalità composizione.

| Comando | Scorciatoia |
|---------|-------------|
| Riproduci / Pausa | Ctrl+Return |
| Salta indietro / Salta avanti (5 secondi) | Ctrl+Alt+, / Ctrl+Alt+. |
| Inserisci marca temporale multimediale (anche **Inserisci > Inserisci marca temporale multimediale**: lo stesso comando, mostrato in entrambi i menu) | Ctrl+Shift+Return |
| Stop, Vai all'inizio, Pausa e riavvolgi, Più veloce, Più lento, Velocità normale, Volume su, Volume giù, Muto, Vai alla marca temporale al cursore | nessuna scorciatoia (dagliene una in **File > Opzioni > Tastiera**) |

I comandi agiscono sul file che hai aperto o usato per ultimo (in uno qualsiasi degli editor, in un copyholder o in un pannello di riferimento rapido).

**Pausa e riavvolgi.** Quando è attivo (il pulsante appare premuto), ogni pausa torna indietro un poco, di 2 secondi a meno che tu non imposti un altro valore in **File > Opzioni > Comportamento > Riproduzione**. Quando premi Ctrl+Return per scrivere ciò che hai sentito, risenti le ultime parole quando prosegui.

**Marche temporali.** **Inserisci marca temporale multimediale** scrive il tempo del lettore al cursore, come `[03:07]` (con le ore da un'ora in poi: `[1:02:03]`). **Vai alla marca temporale al cursore** fa il contrario: prende la marca temporale nella riga del cursore (la più vicina prima di esso o intorno ad esso) e vi sposta il lettore. Le marche temporali sono testo normale; possono anche essere scritte a mano (`3:07`, `[03:07]`, `1:02:03.5`).

**In un copyholder o in un pannello di riferimento rapido** il lettore ha i suoi controlli di riproduzione in basso. Un file audio nel copyholder dell'editor della trascrizione è una buona disposizione per un solo editor.

**Dove era.** Quando riapri un file audio o video, riprende da dove l'avevi messo in pausa (disattivalo in **File > Opzioni > Comportamento > Riproduzione**). Mostrare un altro documento nell'editor ferma il file.

**Che cosa viene riprodotto.** Il componente aggiuntivo *Media Playback* (2.0) riproduce formati di uso libero: audio MP3, Ogg Vorbis, Opus, FLAC, ALAC, WAV, AIFF e WavPack; video WebM, MKV e Ogg con immagini VP8, VP9 o Theora (anche MJPEG e altri formati liberi). Il componente porta con sé tutto ciò che gli serve.

**Più formati.** H.264, H.265 (HEVC), AAC (M4A), AC-3, E-AC-3, DTS, video MPEG-1/2/4, VC-1 e WMV, WMA e AMR sono coperti da brevetti in alcuni paesi; arrivano con il componente aggiuntivo separato **Extra Video and Audio Formats**. Il modo più semplice per ottenere entrambi è l'unico pacchetto **media-all-formats.qbpkg** nella pagina Releases: installato come ogni componente aggiuntivo (sezione 17), installa *Media Playback* ed *Extra Video and Audio Formats* insieme. Entrambi compaiono poi in **Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi**, dove i formati extra possono essere disattivati o rimossi da soli. Se serva una licenza di brevetto per usare questi formati dipende dal paese. Finché è installato e attivo, *Media Playback* lo usa, e anche i file MP4/M4V/MOV, AVI, MPEG, WMV, FLV e 3GP vengono riprodotti (non WMA Pro e WMV 9). Senza di esso, un file simile mostra sotto il lettore un messaggio che nomina i suoi formati e questo componente aggiuntivo; tutto il resto funziona come prima. Il video AV1 non viene riprodotto con la decodifica incorporata.

Dalla 0.80.0 il lettore decodifica sempre con il proprio FFmpeg: il Qt che {{APP}} usa da allora (la build di Qt 6.12 di The Qt Company) non ha modo di riprodurre tramite **GStreamer** del sistema, quindi **File > Opzioni > Comportamento > Riproduzione > Decodifica** mostra quella scelta come non disponibile.

### Segnalibri

I **segnalibri del documento** (ispettore, scheda Segnalibri) collegano un documento ad altri documenti, a file sul tuo disco o a pagine web. I **segnalibri del progetto** sono disponibili da ogni documento, dal pulsante dei segnalibri nella barra degli strumenti e dalla barra laterale dei pannelli di riferimento rapido. La scheda Segnalibri dell'ispettore mostra entrambe le liste, una sopra l'altra: **Segnalibri del documento** e **Segnalibri del progetto**, ciascuna con i propri pulsanti ➕ e ➖. Un clic su un segnalibro mostra il documento sotto le liste; un documento che l'editor mostra già compare lì come copia di sola lettura (scrivi nell'editor), così l'editor mantiene la sua larghezza di riga. Un doppio clic apre il segnalibro. Trascina documenti, file o collegamenti web nella lista dei segnalibri, oppure usa il pulsante ➕; un collegamento trascinato da un browser web mantiene il titolo della pagina. Trascina un segnalibro in su o in giù per cambiare l'ordine. I divisori tra le due liste e il testo sotto di esse si possono spostare; le dimensioni che dai loro vengono conservate con il progetto e ritornano dopo un riavvio o un aggiornamento del programma, anche quando in quel momento non è scelto alcun segnalibro. I documenti rilasciati sul **pulsante Segnalibri** della barra degli strumenti diventano segnalibri del progetto; lo stesso vale per i documenti selezionati con **Documenti > Aggiungi ai segnalibri del progetto**; l'ispettore mostra allora la sua scheda Segnalibri con la nuova voce selezionata.

---

## 7. L'ispettore

I pulsanti in alto nell'ispettore permettono di passare tra:

- **Sinossi e note**: la sinossi è un breve riassunto che compare sulla scheda. Puoi usare un'immagine invece del testo: fai clic sul pulsante dell'immagine della scheda e rilascia un file immagine o un'immagine dal raccoglitore sull'area grigia (oppure fai doppio clic sull'area per scegliere un file; clic destro per rimuovere l'immagine). Sotto ci sono le **note del documento** (oppure, cambiando con il piccolo menu, le **note del progetto**, condivise da tutto il progetto). **Naviga > Ispeziona > Vai alla sinossi** (**Ctrl+Alt+Y**) e **Vai alle note del documento** (**Ctrl+Alt+N**) portano lì la tastiera dal testo; **Alt+2** torna all'editor. Dalla 0.93.0 le note (note del documento e del progetto) iniziano come testo semplice: nessun carattere, dimensione, rientro della prima riga, spaziatura dei paragrafi o stile dalla formattazione del testo del progetto, nel carattere dell'interfaccia del programma allo zoom delle note. La formattazione compare solo quando la applichi tu (la barra di formattazione, i menu, le scorciatoie) e poi resta. Le note che hanno già del testo mantengono l'aspetto che avevano. Il testo incollato nelle note è semplice; **Modifica > Incolla con formattazione** (**Ctrl+Alt+Shift+V**) mantiene la formattazione che porta con sé.
- **Segnalibri**: i segnalibri del documento e quelli del progetto. Un clic su un segnalibro di un documento del progetto mostra il suo testo sotto le liste. Lì puoi scrivere e formattare come in un editor (la barra di formattazione, il menu Formato e le loro scorciatoie agiscono su di esso mentre il cursore vi si trova): tutto va subito nel documento e nel suo file, e un editor che mostra lo stesso documento mostra subito la modifica. Il testo lì ha uno zoom proprio.
- **Mappa mentale**: una lavagna di note per il documento o la cartella (vedi "Mappe mentali" più sotto).
- **Oggetti**: i personaggi, i luoghi, le ambientazioni, i punti di interesse, il meteo e le altre cose della tua storia, ciascuno con una scheda propria e collegamenti tra di loro (vedi "Oggetti" più sotto).
- **Metadati**: etichetta, stato, tipo di sezione, includi nella compilazione, date di creazione e modifica, **metadati personalizzati** e **parole chiave**.
- **Istantanee** (sezione 11).
- **Commenti e note a piè di pagina**.

Gli strumenti di scrittura dei componenti aggiuntivi possono aggiungere schede proprie; i loro pulsanti vengono per primi nella riga, per esempio **Trascrizioni** di Trascrizione vocale (sezione 10).

**Lo zoom proprio dell'ispettore.** **Ctrl+rotellina del mouse** su una parte dell'ispettore rende quella parte più grande o più piccola, senza cambiare gli editor. Le schede con due parti hanno uno zoom per ciascuna: **Sinossi e note** (la scheda, le note), **Segnalibri** (la lista dei segnalibri del documento, la lista dei segnalibri del progetto, il testo del segnalibro sotto di esse: tre zoom), **Istantanee** (la lista, il testo dell'istantanea), **Oggetti** (i pulsanti delle categorie con la lista, la scheda). La lista **Zoom** in basso mostra e imposta lo zoom della parte con la tastiera, o della parte su cui hai fatto zoom per ultima; il suo titolo nomina la parte ("Zoom: Note") e nella scheda Mappa mentale è lo zoom della lavagna. Le altre schede (Metadati, Commenti) e le righe di pulsanti in alto e in basso condividono un unico zoom; con **Vista > Zoom separato per le schede dell'ispettore** (anche nel menu del clic destro della lista Zoom) Metadati e Commenti ne mantengono uno ciascuna.

**L'ispettore ricorda**, con il progetto, quale scheda era aperta, dove hai messo i divisori dentro le schede, il segnalibro mostrato e l'oggetto mostrato nella scheda Oggetti.

**Commenti di una cartella.** Con una cartella nell'editor (come bacheca o struttura), la scheda dei commenti e delle note a piè di pagina elenca i commenti e le note a piè di pagina di ogni documento all'interno della cartella e delle sue sottocartelle, raggruppati per documento. Un clic sul nome di un documento lo apre; un clic su **Commento ▸** (o **Nota a piè di pagina ▸**) apre il documento e mostra quel punto; il testo di un commento o di una nota a piè di pagina può essere modificato proprio lì. Nel testo combinato (più documenti come un unico testo), la scheda elenca le note di tutti i documenti mostrati.

### Mappe mentali

Ogni documento e ogni cartella può avere una **mappa mentale**: una lavagna su cui metti note ovunque, le colleghi con linee e frecce, le colori e le raggruppi su forme di sfondo. Le lavagne possono essere salvate come file Scapple e i file Scapple possono essere letti (vedi *File e Scapple* più sotto). Fai clic sul pulsante **Mappa mentale** dell'ispettore (tra Segnalibri e Metadati). Il pulsante è sottolineato quando il documento ha una mappa mentale.

**Una mappa mentale fissata (0.92.0).** Normalmente la scheda mostra la mappa mentale del documento o della cartella che hai selezionato. Puoi invece fissarla su un elemento: la scheda continua allora a mostrare la mappa mentale di quell'elemento, qualunque cosa selezioni nel raccoglitore o apra nell'editor. È pensato per una lavagna che vuoi accanto mentre scrivi, come una mappa della trama tenuta su una cartella. Tre posti fanno la stessa cosa:

- **Nel raccoglitore**, passa il mouse su una riga: vicino alla sua estremità destra compare una piccola puntina. Fai clic su di essa e quell'elemento è fissato; la sua puntina resta ed è piena. Un clic sulla puntina piena lo rilascia. Fare clic su una puntina non seleziona nulla.
- **Nella scheda Mappa mentale**, il pulsante della puntina a destra della piccola barra fissa la lavagna che la scheda mostra e la rilascia di nuovo. Mentre una lavagna è fissata, la barra nomina il suo elemento (*Fissata: Trama*, subito dopo il pulsante della panoramica; un titolo lungo viene accorciato, e il titolo intero si mostra quando fermi il mouse su di esso), così è chiaro che la lavagna non appartiene al documento selezionato.
- **Nei menu**, **Naviga > Ispeziona > Fissa la mappa mentale di questo elemento** e **Naviga > Ispeziona > Rilascia la mappa mentale fissata** (all'inizio non hanno scorciatoie; dagliene nelle opzioni, se vuoi).

Finché un elemento è fissato, una sottile barra in fondo al raccoglitore dice *Mappa mentale fissata:* con il suo titolo e ha il pulsante **Rilascia**. Al massimo un elemento di un progetto è fissato; fissarne un altro sposta la fissazione. Anche un elemento senza mappa mentale può essere fissato: la scheda mostra una lavagna vuota su cui iniziare. Solo la scheda Mappa mentale è fissata; le altre schede dell'ispettore seguono come sempre il documento selezionato, e una lavagna aperta in una finestra grande resta quella che è. La fissazione viene conservata con il progetto su questo computer quando lo chiudi, e viene rilasciata quando l'elemento va nel Cestino.

**Nell'ispettore** la lavagna riempie la scheda. La piccola barra sopra di essa ha **Nuova nota**, **Nuova forma di sfondo**, **Adatta alla vista**, **Menu** (tutti i comandi: File, Modifica, Vista, Note, Formato), il pulsante **panoramica** (vedi *Guardarsi intorno*) e **Apri in finestra**. La barra sotto di essa conta le note e ha il cursore dello zoom. **Apri in finestra** mostra la stessa lavagna in una finestra grande con gli stessi menu nella sua barra dei menu e un ispettore a destra (**Formato nota**, **Stili delle note**, **Impostazioni documento**); le modifiche in un posto compaiono subito nell'altro. Un clic destro sulla lavagna apre i comandi per ciò che si trova sotto il mouse. Una nuova nota (pulsante, menu o doppio clic) prende subito la tastiera, anche quando prima era nel raccoglitore: ciò che scrivi sostituisce il suo testo selezionato (0.94.0).

I comandi di una mappa mentale si trovano in questi menu propri, non nei menu della finestra principale. Più sotto, **Menu > Note > Collega** significa: il pulsante **Menu** della lavagna dell'ispettore, poi **Note**, poi **Collega** (nella finestra grande: **Note > Collega** della sua barra dei menu). Mentre la lavagna ha la tastiera, i suoi tasti vengono per primi: **Ctrl+Z** annulla una modifica della lavagna, **Ctrl+F** cerca nelle sue note, **Ctrl+P** la stampa.

**Aiuto sulle mappe mentali.** **Aiuto > Aiuto sulle mappe mentali** (anche il pulsante **?** all'estremità destra della barra della lavagna e **Menu > Aiuto** della lavagna) mostra una breve guida con immagini: come creare note, come tracciare una linea (trascina una nota su un'altra), come tracciare una freccia (tieni premuto **Alt** quando rilasci; **Ctrl+Alt** per una freccia all'indietro, **Ctrl+Alt+Shift** per entrambe le estremità), le parole su una linea e le forme di sfondo. Il suo pulsante **Apri lavagna di esempio** (anche **Menu > Aiuto > Apri lavagna di esempio**) apre una lavagna di prova in una finestra a sé, con note da trascinare l'una sull'altra ed esempi finiti di linee e frecce. Nulla sulla lavagna di esempio appartiene al tuo progetto e non viene salvato.

**Note.**

- **Doppio clic** su un punto vuoto: una nuova nota, pronta per essere scritta. **Ctrl+Enter** crea una nuova nota sotto quella selezionata (impilata). Con **Ctrl** al doppio clic, la nuova nota viene collegata alle note selezionate; con **Alt**, da una freccia verso di essa; con **Ctrl+Alt**, da una freccia all'indietro.
- **Doppio clic** su una nota per cambiarne il testo; **Esc** o un clic all'esterno termina la modifica. Una nota breve diventa stretta quanto il suo testo; una lunga va a capo alla larghezza delle nuove note (entrambe in **Menu > File > Opzioni mappa mentale…**).
- **Trascina** le note per spostarle; con **Ctrl** al rilascio vengono copiate. I **tasti freccia** spostano le note selezionate quando **Menu > Modifica > I tasti freccia spostano le note** (**M**) è attivo (con **Shift** di 10 px); altrimenti selezionano la nota successiva in quella direzione. **Enter** o **F2** modifica la nota selezionata, **Delete** rimuove le note selezionate.
- **Trascina una nota su un'altra** per collegare le due con una linea; di nuovo per rimuovere la linea. Con **Alt** la linea ha una freccia verso la nota su cui hai rilasciato; con **Ctrl+Alt** una freccia all'indietro; con **Ctrl+Alt+Shift** frecce a entrambe le estremità.
- **Doppio clic su una linea** per metterci dentro una nuova nota, oppure sull'etichetta di una linea per cambiarla (**Menu > Note > Etichetta del collegamento…**).
- **Menu > Note > Collega** (**Ctrl+.**), **Collega con freccia** (**Ctrl+Alt+.**), **Rimuovi frecce** (**Ctrl+,**) e **Scollega** agiscono sulle note selezionate: la prima che hai selezionato è il centro.
- **Menu > Note > Nuova nota collegata** crea una nota a sinistra, a destra, sopra o sotto quella selezionata, collegata ad essa (con o senza freccia).
- **Menu > Note > Dividi** (**Ctrl+Shift+K**) divide una nota al cursore; **Unisci** unisce le note selezionate.
- **Menu > Note > Impila** (**Ctrl+'**), **Allinea**, **Distribuisci**, **Stessa larghezza / altezza**, **Adatta automaticamente**, **Porta in primo piano**, **Porta in secondo piano**.
- **Menu > Modifica > Aggiungi collegamento…** (**Ctrl+K**) collega il testo di una nota a una pagina web o a un file; i documenti trascinati dal raccoglitore sulla lavagna diventano note con collegamenti ad essi. Un clic su un collegamento lo apre: un documento si apre come si aprono i collegamenti ai documenti nel testo (**File > Opzioni > Comportamento**).
- **Menu > Note > Aggiungi le note al raccoglitore** trasforma le note selezionate in documenti dentro il documento o la cartella a cui appartiene la lavagna, nell'ordine di lettura della lavagna.

Le **forme di sfondo** (**Menu > Note > Nuova forma di sfondo**, **Ctrl+Shift+G**) sono aree colorate dietro le note. Create con delle note selezionate, la forma le circonda. Una forma **magnetica** (**Menu > Note > Magnetica**, **Ctrl+Shift+M**) porta con sé le note che ha sopra quando viene spostata. Una nuova forma parte grigio medio (#808080) e non magnetica; **File > Opzioni > Extra > Mappa mentale** (e **Menu > File > Opzioni mappa mentale…**) imposta il riempimento, il bordo e **Magnetica** con cui partono le nuove forme. (Prima della 0.93.0 una nuova forma era grigio chiaro e magnetica; un colore che avevi scelto lì resta, e un'opzione **Magnetica** memorizzata è stata disattivata una volta.)

**Il menu del clic destro di una nota** (o di una forma di sfondo) cambia le note selezionate: **Colore di riempimento**, **Colore del testo** e **Colore del bordo** offrono i colori delle etichette del tuo progetto, i sei colori della barra di formattazione (rosso, blu, giallo, verde, bianco, nero) e **Altro...** per qualsiasi colore, la stessa scelta ovunque in {{APP}}; **Forma** è **Rettangolo**, **Rettangolo arrotondato**, **Nuvola** o **Frastagliato** (una nota senza bordo ne riceve uno sottile, così la forma si vede); **Bordo** imposta lo spessore; **Icone** mette fino a otto icone su una nota, una a ogni angolo e una al centro di ogni lato, scelte tra le icone del raccoglitore (**Documenti > Cambia icona**), comprese le tue icone. Fai clic destro vicino al posto di un'icona per rimuovere solo quella; **Rimuovi tutte le icone** le cancella.

**Il menu del clic destro di una linea** appartiene solo a quella linea, così le altre linee delle stesse note non vengono cambiate: le sue frecce (**Nessuna freccia**, una freccia verso l'una o l'altra nota, oppure **Frecce a entrambe le estremità**), **Colore della linea**, **Icona al centro** (un'icona disegnata al centro della linea), l'etichetta, **Inserisci una nota sulla linea** e **Scollega**. Ogni modifica può essere annullata da sola.

Le icone, i colori delle linee e le icone delle linee sono aggiunte proprie di {{APP}}; vengono conservate nel file della lavagna in elementi che Scapple non legge (una lavagna esportata come file Scapple ha le note, le linee e le frecce senza di essi). **File > Opzioni > Extra > Mappa mentale** imposta la dimensione delle icone e la forma delle nuove note.

**Aspetto.** **Menu > Formato** ha grassetto, corsivo, sottolineato, barrato, più grande e più piccolo (**Ctrl+Shift+>** e **Ctrl+Shift+<**), i colori del testo, del riempimento e del bordo, lo spessore del bordo (nessuno, da 1 a 5 px) e lo stile (arrotondato, squadrato, frastagliato, nuvola), l'ombra e **Sfuma** (**Ctrl+Shift+F**, la nota viene mostrata sbiadita). Gli **Stili delle note** memorizzano un aspetto con un nome; **Menu > Formato > Stile nota** ne applica uno (da **Ctrl+Alt+1** a **9**, **Ctrl+Alt+0** per Predefinito), ne crea uno dalla nota selezionata, oppure copia e incolla l'aspetto di una nota (**Alt+Shift+C** / **Alt+Shift+V**). Le **Impostazioni documento** nell'ispettore della finestra impostano lo sfondo, il colore del testo, il carattere e la direzione di scrittura della lavagna (e **Imposta come predefiniti** per le nuove lavagne).

**Sfondo.** Un clic destro su un punto vuoto della lavagna (nell'ispettore o nella finestra), **Menu > Formato > Colore dello sfondo** o il pulsante dello sfondo delle **Impostazioni documento** offrono i colori delle etichette del tuo progetto, i sei colori della barra di formattazione, il predefinito (**Predefinito per temi chiari** o **Predefinito per temi scuri** nei menu, **Usa predefinito** nelle Impostazioni documento) e **Altro...**. Una lavagna ha uno sfondo proprio per i temi chiari e uno per i temi scuri: il colore che scegli viene conservato per il tipo di tema in uso e l'altro tipo mantiene il proprio. Una lavagna senza un colore proprio mostra il predefinito di **File > Opzioni > Extra > Mappa mentale** ("Sfondo delle lavagne", uno per i temi chiari e uno per quelli scuri) e lo segue quando lo cambi. **Imposta come predefiniti** nelle Impostazioni documento rende lo sfondo della lavagna il predefinito del tipo di tema in uso. Una lavagna mantiene il suo colore di sfondo anche quando non c'è ancora nulla.

**Zoom e posizione.** Ogni lavagna ricorda, con il progetto, il suo zoom e la parte di essa sullo schermo, uno per l'ispettore e uno per la finestra grande. Una lavagna su cui non hai mai fatto zoom si apre allo zoom predefinito delle mappe mentali (**File > Opzioni > Modifica**, "Zoom predefinito del testo").

**Selezionare.** Clic, **Shift+clic** o **Ctrl+clic** per aggiungere, trascina una cornice intorno alle note, **Ctrl+A**. **Menu > Modifica > Seleziona** ha le note impilate sotto, le note sulle forme selezionate (**Alt+A**), le note collegate (**Ctrl+Shift+A**) e interi gruppi collegati; sotto di esso ci sono **Inverti selezione** (**Alt+Shift+I**) e **Deseleziona** (**Ctrl+D**).

**Guardarsi intorno.** Zoom con **Ctrl+rotellina del mouse**, **Ctrl++** e **Ctrl+-**, **Dimensione reale** (**Ctrl+0**), **Adatta alla vista** (**Ctrl+9**), **Zoom sulla selezione** (**Ctrl+Shift+8**) e **Zoom casuale** (**Ctrl+Shift+R**). Tieni premuto **Space** e trascina per spostare la lavagna; tieni premuto **Z** per vedere un momento l'intera lavagna. **Menu > Modifica > Trova** (**Ctrl+F**) trova e sostituisce il testo nelle note. **Menu > Vista > Guide di pagina** mostra dove finiscono le pagine nella stampa.

**La panoramica (0.93.0).** Una piccola mappa nell'angolo in alto a sinistra della lavagna mostra tutta la lavagna in piccolo, e una cornice per la parte che vedi ora; la cornice si sposta e cambia dimensione mentre scorri e fai zoom. Note, forme di sfondo e linee sono disegnate nei loro colori; il testo di una nota viene mostrato solo dove può ancora essere letto a quella dimensione, altrimenti la nota è la sua cornice colorata. Fai clic nella panoramica per spostare la vista in quel punto; trascina la cornice per spostare la vista con essa. Lo zoom resta, e anche una lavagna bloccata (**Impedisci modifica**) può essere spostata. La rotellina del mouse sulla panoramica funziona come sulla lavagna (**Ctrl+rotellina** fa lo zoom). Trascina la maniglia nel suo angolo in basso a destra per cambiarne la larghezza (parte da un quinto della larghezza della vista, almeno 80 px, e non è mai più larga della metà della vista); l'altezza segue la forma di ciò che mostra, mai più della metà della vista, e la larghezza si riduce con essa quando l'altezza supererebbe quel limite. La panoramica appartiene solo alla vista: non è nelle esportazioni, nelle stampe né nell'immagine della lavagna. All'inizio è **automatica**: compare quando una parte della lavagna sta fuori dalla vista e si nasconde quando tutta ci sta (con un piccolo margine, così non sfarfalla al bordo). Il pulsante **panoramica** nella barra della lavagna dell'ispettore (subito dopo **Menu**) passa da **automatica** a **sempre visibile** (il pulsante appare premuto); lo stesso interruttore è la voce **Panoramica sempre visibile** nel menu Vista della lavagna (nell'ispettore: il pulsante **Menu**, poi **Vista**; nella finestra grande: la sua barra dei menu). La scelta e le dimensioni vengono conservate su questo computer, nelle impostazioni del programma (una scelta per tutte le lavagne; la dimensione della panoramica dell'ispettore e quella della finestra grande a parte); nulla va nel progetto.

**Testo, immagini e file.** Incolla o rilascia del testo per creare una nota (un testo con righe vuote può diventare più note), rilascia un'immagine per una nota con immagine e rilascia un file di testo per creare note dai suoi paragrafi.

**File e Scapple.** **Menu > File > Esporta** scrive la lavagna come PDF (**Ctrl+Shift+E**), immagine PNG, testo semplice, elenco di testo, RTF, struttura OPML, le sue immagini o un file Scapple (*.scap*); **Menu > File > Importa mappa mentale o file Scapple…** legge un file *.scap* nella lavagna. **Stampa** e **Anteprima di stampa** la stampano. **Menu > File > Impedisci modifica** blocca la lavagna.

**Opzioni mappa mentale** (**Menu > File > Opzioni mappa mentale…**, e lo stesso in **File > Opzioni > Extra > Mappa mentale** della finestra principale): se **Enter** termina la modifica di una nota (allora **Shift+Enter** va a capo), se **Enter** su una nota selezionata ne crea una nuova, se le nuove note assumono l'aspetto della nota selezionata, linee tratteggiate senza frecce, note strette per testi brevi, quanto sono sbiadite le note sfumate, e la dimensione, la larghezza, il colore e il bordo delle nuove note e forme.

**Dove viene conservata la lavagna.** Una lavagna viene salvata da sola, come file nel formato dei file Scapple (*content/<ID>/mindmap.scap* nella cartella del progetto; *Files/Data/<UUID>/quillbinder-mindmap.scap* in un progetto Scrivener, accanto ai file propri del documento). Un documento riceve il file solo quando qualcosa viene messo sulla sua lavagna. Annulla (**Ctrl+Z**) funziona per ogni lavagna per conto suo.

{{APP}} legge i file Scapple come li scrive Scapple 1.x, per quanto è noto il loro formato; non è stato possibile provarli con lo stesso Scapple (vedi le note della versione 0.17.0 nel file di stato).

### Oggetti: personaggi, luoghi e altro

Gli **Oggetti** sono le persone, i luoghi e le cose della tua storia, ciascuno con una scheda di campi: un personaggio con razza, altezza, colore degli occhi, tratti e storia di fondo; un luogo con il suo tipo, la sua ambientazione e il suo meteo; e così via. Appartengono all'intero progetto, non a un documento, e sono tenuti separati dal raccoglitore. I collegamenti li uniscono, così puoi muoverti con un clic attraverso uno sfondo: la scheda del personaggio, il villaggio in cui vive, il paesaggio intorno, il suo meteo.

Fai clic sul pulsante **Oggetti** dell'ispettore (a destra del pulsante **Mappa mentale**), oppure scegli **Naviga > Ispeziona > Oggetti del progetto: personaggi, luoghi, ambientazioni, ...**. **Finestra > Oggetti…** (o il pulsante della finestra nella barra della scheda) mostra gli stessi oggetti in una grande finestra a sé, con la lista a sinistra e la scheda a destra. Le modifiche in un posto compaiono subito nell'altro.

**Categorie.** I pulsanti colorati in alto sono le categorie: all'inizio **Personaggi**, **Luoghi**, **Ambientazioni**, **Punti di interesse** e **Meteo**. Fai clic su uno per vedere i suoi oggetti. Puoi rinominarle, dar loro altri colori, cambiarne l'ordine, rimuoverle e aggiungere categorie tue (fazioni, incantesimi, navi ...): vedi *Modelli e colori* più sotto.

**Creare oggetti.** **+** nella barra della scheda crea un nuovo oggetto nella categoria scelta (nella cartella dell'oggetto selezionato); scrivi il suo nome in cima alla scheda. Il pulsante con la cartella crea una **cartella** per ordinare, per esempio "Villaggi" dentro Luoghi; le cartelle possono contenere cartelle. Trascina oggetti e cartelle nella lista per spostarli. **F2** o un clic destro rinomina, e il menu del clic destro ha anche **Duplica** ed **Elimina…** (anche nel menu **≡** della barra). Il campo **Filtro** mostra solo gli oggetti il cui nome o testo contiene ciò che scrivi.

**La scheda.** Sotto la striscia colorata con la categoria e il nome ci sono i campi del modello della categoria. Tutto ciò che scrivi viene salvato da solo, come i tuoi documenti. I tipi di campo sono:

- **Testo breve** (una riga) e **Data** (in qualsiasi forma tu voglia, come "primavera 1203" o "il terzo inverno").
- **Numero** con un'unità (cm, anni, °C ...).
- **Testo lungo**, che può contenere collegamenti (vedi sotto) e parole in **grassetto**, *corsivo*, sottolineate, barrate, colorate ed evidenziate: con il cursore nel campo, la barra di formattazione, il menu Formato e le loro scorciatoie (**Ctrl+B**, **Ctrl+I**, **Ctrl+U**; anche nella finestra Oggetti) agiscono su di esso. Anche le versioni 0.20 e 0.21 di {{APP}} li mostrano.
- **Scelta**: una lista da cui scegliere; puoi anche scrivere qualcos'altro.
- **Sì / no**.
- **Immagine**: fai clic sull'area (o rilascia un'immagine su di essa) per sceglierne una; viene copiata nel progetto, rimpicciolita se è più grande di 1600 pixel. Il clic destro la rimuove.
- **Collegamento a oggetti**: gli oggetti collegati sono mostrati come pulsanti colorati. **+** ne aggiunge uno: scrivi per cercare, poi scegli un oggetto, oppure scegli **Nuovo in Luoghi: "Ashvale"** per creare un nuovo oggetto con quel nome e collegarlo subito. Un clic su un oggetto collegato mostra la sua scheda; un clic destro rimuove il collegamento.
- **Collegamento a documenti**: documenti del raccoglitore che appartengono all'oggetto, come "Compare in" di un personaggio o "Scene ambientate qui" di un luogo. Un clic apre il documento nell'editor.

**Pulsante del colore.** Il pulsante **Colore** della striscia dà a un oggetto un colore proprio (o gli restituisce il colore della categoria), per esempio per contrassegnare i personaggi principali.

**Collegamenti dentro i testi lunghi.** In un campo di testo lungo, seleziona una parola e scegli **Collega a oggetto…** (o **Collega a documento…**) nel suo menu del clic destro; senza selezione il nome dell'oggetto viene inserito come collegamento. Un clic sul collegamento lo segue; **Rimuovi collegamento** nel menu del clic destro lo toglie di nuovo. Nel testo di un documento, **Inserisci > Collega a oggetto…** mette nel testo un collegamento a un oggetto; un clic su di esso mostra l'oggetto nell'ispettore. Quando il manoscritto viene compilato, questi collegamenti diventano parole normali.

**Avanti e indietro.** **‹** e **›** nella barra della scheda vanno indietro e avanti tra gli oggetti che hai guardato, come in un browser web.

**Collegato da.** In fondo a ogni scheda, **Collegato da** elenca gli oggetti che puntano a questo (in un campo di collegamento o in un testo), così il villaggio mostra chi ci abita. Un clic porta lì.

**In questo documento.** Quando il documento nell'editor è collegato a degli oggetti (tramite un campo di collegamento a documenti, per esempio "Compare in"), la scheda mostra **In questo documento:** con quegli oggetti, e il pulsante **Oggetti** è sottolineato come le altre schede con contenuto.

**Eliminare.** Eliminare un oggetto chiede prima conferma. I collegamenti ad esso vengono rimossi dagli altri oggetti (nei testi la parola collegata resta, come testo semplice). Un collegamento ad esso in un documento dice, quando ci fai clic, che l'oggetto non esiste più. Eliminare una cartella mantiene i suoi oggetti; salgono di un livello.

**Modelli e colori.** **Progetto > Modelli di oggetto e colori…** (anche nel menu **≡** della scheda e nel menu del clic destro di un pulsante di categoria) modifica le categorie e i loro campi:

- A sinistra, le categorie: **Aggiungi**, **Rimuovi** e le frecce per l'ordine. Rimuovere una categoria che ha oggetti chiede se anch'essi devono essere eliminati; **No** non cambia nulla.
- A destra, il nome e il **Colore** della categoria e i suoi campi: **Aggiungi campo**, **Rimuovi**, **Su**, **Giù**. Per ogni campo, il suo nome e il suo tipo; un numero può avere un'unità, una scelta la sua lista di scelte (una per riga) e un campo di collegamento le categorie a cui può collegarsi (nessuna spuntata: tutte).
- **Usa per i nuovi progetti** conserva queste categorie e questi modelli per i progetti che crei da ora in poi. **Modelli standard** ripristina una categoria al modello con cui è arrivata, oppure aggiunge le categorie standard che avevi rimosso.
- Rimuovere un campo da un modello nasconde ciò che gli oggetti hanno al suo interno, ma non lo elimina dai loro file. Per un campo standard, **Modelli standard > Ripristina questa categoria allo standard** lo riporta con il suo contenuto; un campo tuo non torna, quindi rinomina un campo invece di rimuoverlo se potresti volerne di nuovo il contenuto.

**Dove vengono conservati gli oggetti.** Nella cartella del progetto, in una cartella *objects*: un piccolo file per oggetto (*objects/characters/<ID>.json* e così via), i modelli, i colori e le cartelle in *objects/schema.json* e le immagini in *objects/images*. Il file di progetto stesso non viene cambiato, quindi le versioni più vecchie di {{APP}} aprono il progetto come prima e lasciano stare gli oggetti. Un progetto in formato Scrivener conserva i suoi oggetti nella sua copia di lavoro (sezione 14); Scrivener non li conosce. I backup li includono.

---

## 8. Organizzare il tuo progetto

### Parole chiave

Le parole chiave sono etichette, per esempio i personaggi o i luoghi di una scena. Aggiungile nell'ispettore o gestiscile tutte con **Progetto > Parole chiave del progetto** (**Ctrl+Alt+K**), dove puoi assegnare loro dei colori, assegnarle ai documenti selezionati e trovare tutti i documenti con una parola chiave. Puoi anche **trascinare le parole chiave** da quella finestra sui documenti nel raccoglitore, in una raccolta, nella bacheca o nella struttura (se il documento è uno di più selezionati, tutti ricevono le parole chiave), oppure nella lista delle parole chiave dell'ispettore o di un pannello di riferimento rapido. Tieni premuto **Alt** mentre rilasci per assegnare anche il gruppo di una parola chiave. Dentro la finestra, trascina una parola chiave su un'altra per metterla in quel gruppo. I colori delle parole chiave compaiono sulle schede.

### Metadati personalizzati

In **Progetto > Impostazioni del progetto > Metadati personalizzati** puoi aggiungere campi tuoi: testo, casella di controllo, lista (un insieme fisso di scelte) o data, per esempio *POV*, *Luogo* o *Giorno della storia*. Compaiono nell'ispettore e come colonne della struttura.

### Salvare le impostazioni del progetto in un file

**Progetto > Salva impostazioni del progetto su file…** scrive le impostazioni del progetto aperto in un file a sé (*.qbsettings*): etichette, stato, tipi di sezione, metadati personalizzati, parole chiave, la formattazione predefinita, gli stili, il formato script, gli obiettivi, le impostazioni di compilazione, la vista pagina e la dimensione della pagina, le impostazioni di stampa, l'aspetto (tema, colori, caratteri, larghezza della colonna, bordo), le opzioni proprie del progetto (File > Opzioni con "Questo progetto") e i modelli e i colori della scheda Oggetti. Il testo dei tuoi documenti non ne fa parte. Etichette, tipi di sezione e le cartelle che le impostazioni di compilazione nominano vengono memorizzati per nome, così il file può essere letto in un altro progetto.

**Progetto > Carica impostazioni del progetto da file…** legge un file simile nel progetto aperto. Quando le impostazioni del progetto sono uguali a quelle del file, lo dice e non cambia nulla. Quando differiscono, una finestra elenca ogni sezione con ciò che differisce (per esempio "solo nel file: Idea; solo nel progetto: Rivedere") e ti lascia scegliere per ogni sezione:

- **Prendi dal file**: le impostazioni del file per questa sezione.
- **Mantieni quello del progetto**: la sezione resta com'è.
- **Predefinito di Quillbinder**: ciò che riceve un nuovo progetto (le impostazioni per i nuovi progetti di **File > Opzioni**, le etichette e gli stati standard ...).

**Prendi tutto dal file** e **Mantieni tutto del progetto** impostano ogni sezione in una volta. Qualunque cosa tu scelga, nulla di ciò che i tuoi documenti usano va perso: un'etichetta, uno stato, un tipo di sezione, una parola chiave, un campo di metadati personalizzati, una categoria di oggetti o uno stile che i documenti usano ancora resta nel progetto anche quando il file non lo ha, e il messaggio finale nomina ciò che è stato mantenuto. Quando la formattazione o uno stile cambiano, il testo di tutti i documenti li segue, come quando li cambi tu stesso. Prima del caricamento, {{APP}} conserva le impostazioni com'erano in un file nella cartella *settings* del progetto ("settings-before-loading-*data*.qbsettings"); carica quel file per tornare indietro.

### Tipi di sezione

I tipi di sezione dicono a Compila che cos'è ogni documento: *Parte*, *Capitolo*, *Scena*, *Sezione* e così via. Impostali per documento nell'ispettore, oppure lascia che {{APP}} decida in base alla posizione (**Impostazioni del progetto > Tipi di sezione**, "Tipi predefiniti per struttura"), per esempio: le cartelle del primo livello sono capitoli, i documenti di testo al loro interno sono scene.

### Modelli di documento

Metti dei documenti modello (per esempio una scheda di personaggio) in una cartella, poi scegli quella cartella in **Impostazioni del progetto > Cartelle speciali**. I modelli compaiono in **Progetto > Nuovo da modello**. In **Documenti > Modello predefinito per i sottodocumenti** puoi far usare a una cartella un modello per ogni nuovo documento che crei al suo interno.

### Modelli di progetto

**File > Salva come modello…** salva il progetto corrente (struttura, impostazioni ed eventuali documenti di esempio) come modello per i nuovi progetti.

---

## 9. Cercare

### Ricerca rapida

Scrivi alcune lettere nel campo della barra degli strumenti. Una lista mostra i documenti il cui **titolo**, la cui **sinossi** o il cui **testo** le contiene, fino a dieci per tipo sotto un titolo proprio. Sceglierne uno lo apre: i punti del testo che contengono ciò che hai scritto sono **segnati**, e il primo è selezionato e portato in vista. I segni scompaiono quando scegli un altro documento nel raccoglitore. È la via più rapida verso un documento di cui conosci il nome.

### Ricerca nel progetto

**Ctrl+Shift+F**, o la lente d'ingrandimento nella barra degli strumenti, mostra un campo di ricerca sopra il raccoglitore. Scrivi, e il raccoglitore viene sostituito dalla lista viola dei risultati.

- Fai clic su un risultato per aprirlo. **Le corrispondenze sono evidenziate nel testo**, la prima è selezionata e portata in vista, e **F3** / **Shift+F3** saltano dall'una all'altra. **Ctrl+F** apre la barra di ricerca già compilata con il tuo termine di ricerca.
- La **X** nell'intestazione dei risultati torna al raccoglitore; l'editor mantiene il documento che mostra. Fai di nuovo clic sulla lente d'ingrandimento nella barra degli strumenti per nascondere il campo di ricerca.
- Con le raccolte mostrate, la scheda **Risultati della ricerca** esegue di nuovo la tua ultima ricerca. L'ultima ricerca viene conservata con il progetto.

La lente d'ingrandimento a sinistra del campo di ricerca apre le opzioni:

| Gruppo | Voci |
|--------|------|
| **Cerca in** | **Tutto**, oppure uno tra: Titolo, Testo, Sinossi, Note, Parole chiave, Etichetta, Stato, Metadati personalizzati. Un clic sceglie quel solo posto. Tieni premuto **Alt** mentre fai clic (o mentre apri il menu) per scegliere più posti. |
| **Operatore** | **Tutte le parole** (ogni parola deve comparire, in qualsiasi ordine), **Una parola qualsiasi**, **Frase esatta**, **Espressione regolare** |
| **Opzioni** | Parole intere, Maiuscole/minuscole, Inverti risultati (i documenti che *non* corrispondono), Cerca solo nella Bozza, Cerca solo nella selezione del raccoglitore (i documenti selezionati nel raccoglitore e tutto ciò che contengono), Cerca documenti "inclusi" / Cerca documenti "esclusi" (secondo **Includi nella compilazione**), Includi il cestino |

Esempio: per vedere tutto ciò che ha ancora lo stato "Da fare", cerca `Da fare` e scegli **Stato** sotto Cerca in.

**Salva ricerca come raccolta…** in fondo al menu conserva la ricerca, con le sue opzioni, come una raccolta che viene eseguita di nuovo ogni volta che la apri.

### Sostituisci nel progetto

**Modifica > Trova > Sostituisci nel progetto** sostituisce un testo nell'intero progetto in una volta.

1. Inserisci il testo da **Sostituire** e il testo da mettere al suo posto (**Con**). Lascia vuoto "Con" per rimuovere il testo.
2. Scegli **Maiuscole/minuscole**, **Solo parole intere** o **Espressione regolare** (allora "Con" può usare da `\1` a `\9` per i gruppi dell'espressione).
3. Spunta dove sostituire: **Titoli**, **Testo**, **Note**, **Sinossi**, **Metadati personalizzati** (campi di testo), **Commenti e note a piè di pagina** (i testi mostrati nell'ispettore).
4. Scegli **Intero progetto** oppure **Solo i documenti selezionati** (i documenti selezionati nel raccoglitore, con ciò che contengono). Il Cestino viene lasciato stare a meno che tu non lo spunti.
5. **Conta** ti dice quante occorrenze cambierebbero e dove. **Sostituisci tutto** chiede ancora una volta e poi lo fa.

Sostituisci nel progetto non può essere annullato nel suo insieme. Lascia spuntato **Crea un'istantanea di ogni testo prima che venga modificato**: ogni testo modificato ha allora un'istantanea chiamata "Prima della sostituzione …" a cui tornare (sezione 11). Titoli e sinossi non hanno istantanee; per una modifica grande, fai prima un backup con **File > Backup > Esegui backup ora**. Un testo aperto in un editor può essere ripristinato anche lì con **Modifica > Annulla**.

La formattazione resta com'è: il nuovo testo assume la formattazione del testo che sostituisce. Un testo che non è aperto in un editor viene modificato nel suo file, quindi **Modifica > Annulla** non annulla quella modifica (lo fa l'istantanea creata prima); lo stesso vale per Converti virgolette, le schede dei commenti e Confronta e trasferisci.

### Trova per formattazione

**Modifica > Trova > Trova per formattazione** (**Ctrl+F3**) apre una piccola finestra che resta aperta mentre lavori. Scegli che cosa cercare:

| Trova | Trova | Scelta aggiuntiva |
|------|-------|--------------|
| Testo evidenziato | testo segnato con l'evidenziatore | limita a un colore |
| Commenti e note a piè di pagina | testo con un commento o una nota a piè di pagina collegati (ispettore) | limita a un colore di commento |
| Annotazioni in linea, Note a piè di pagina in linea | note scritte nel testo | colore dell'annotazione |
| Colore di revisione | testo scritto in modalità revisione | quale revisione |
| Testo colorato | testo il cui colore hai impostato a mano | limita a un colore |
| Collegamenti | collegamenti web e collegamenti a documenti | |
| Formato dei caratteri | testo in grassetto, corsivo, sottolineato, barrato (impostato a mano, non da uno stile) | quale di essi |
| Stile | testo con uno stile di paragrafo o di carattere | quale stile |
| Immagini | immagini nel testo | |

**Contenente il testo** restringe la ricerca ai punti che contengono una parola (per commenti e note a piè di pagina conta anche il testo proprio della nota). **Successivo** e **Precedente** selezionano il punto successivo. Con **Cerca in: Tutti i documenti**, la ricerca prosegue nei documenti seguenti del raccoglitore e li apre; con **Solo l'editor corrente** resta dove sei. **Ctrl+G** e **Ctrl+Shift+G** ripetono la ricerca senza la finestra. **Chiudi** (o **Esc**) chiude la finestra.

---

## 10. Obiettivi, statistiche e strumenti di scrittura

- **Progetto > Obiettivi del progetto** (**Ctrl+Shift+T**): imposta un obiettivo di parole o di caratteri per la Bozza e per ogni sessione di scrittura, e se vuoi anche una scadenza e i tuoi giorni di scrittura. {{APP}} può così calcolare quanto devi scrivere in ogni sessione. Il piè di pagina mostra i tuoi progressi accanto al conteggio delle parole (dalla 0.94.0): una piccola barra e, per esempio, "83 / 1,000 oggi" per la sessione e "12,400 / 80,000 bozza" per la Bozza, ciascuna solo finché il relativo obiettivo è impostato; un clic sull'indicatore apre questa finestra. Le due barre sottili sotto la casella di ricerca rapida (blu: Bozza; arancione: sessione) restano, e si possono disattivare in **File > Opzioni > Aspetto > Interfaccia generale**. Le parole che entrano nel progetto in un solo passaggio non vengono contate come scritte nella sessione (0.94.0): il testo importato (**File > Importa**, **Importa e dividi**, file trascinati, pagine web), un incolla di più di 200 parole e le parole che un progetto contiene già quando lo apri per la prima volta. **Azzera sessione ora** riporta il conteggio a 0 e il suggerimento della casella di ricerca rapida lo mostra subito. La finestra degli obiettivi riserva lo spazio delle sue barre, quindi i campi non si spostano quando imposti un obiettivo.
- **Obiettivi del documento**: fai clic sull'obiettivo nel piè di pagina dell'editor.
- **Obiettivi di taglio** (0.95.0): un obiettivo può essere un numero negativo, in ogni campo di obiettivo (la Bozza, la sessione e un documento). È un obiettivo per tagliare: "-2000" per una sessione significa tagliare 2000 parole oggi, "-4000" per un documento significa portarlo a 4000 parole. La sua barra si riempie man mano che il conteggio scende (una barra di sessione in base alle parole rimosse, una barra di documento o di Bozza finché il conteggio non scende al numero), e un numero negativo è mostrato in rosso. Con un obiettivo di sessione negativo, il conteggio di oggi è netto: conta le parole rimosse come un numero sotto 0, in rosso; con un obiettivo positivo resta come prima e non scende mai sotto 0. Scrivener non accetta obiettivi negativi: un obiettivo negativo viene conservato solo nella copia propria di {{APP}} di un progetto in formato Scrivener; il *.scriv* riceve 0 ("nessun obiettivo") al suo posto.
- **Progetto > Statistiche del progetto**: parole e caratteri nella Bozza, nel testo compilato e nell'intero progetto, più una stima del numero di pagine. In un progetto grande il conteggio richiede un momento; il puntatore di attesa lo segnala.
- **Progetto > Cronologia di scrittura**: quante parole hai scritto in ogni giorno.

Gli strumenti di scrittura qui sotto sono componenti aggiuntivi (sezione 17): **Generatore di nomi**, **Blocco appunti** e **Statistiche del testo** (arrivati con l'aggiornamento 0.11 in un unico pacchetto) e **Trascrizione vocale** (0.61); ciascuno si può disattivare o rimuovere in **Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi**. Senza di essi, **Modifica > Strumenti di scrittura** lo dice; la Gestione componenti aggiuntivi si trova nel menu Aiuto.

### Statistiche del testo

**Progetto > Statistiche del testo…** conta una parte del tuo progetto. In alto scegli quale testo:

- **Testo selezionato** (scelto quando apri la finestra con del testo selezionato),
- **Testo nell'editor** (nel testo combinato: tutti i documenti mostrati),
- **Documenti selezionati nel raccoglitore**, con i documenti che contengono,
- **Bozza (inclusa nella compilazione)**, oppure l'**Intero progetto** (cestino escluso).

La finestra ha quattro schede:

- **Panoramica**: parole (contate esattamente come nella barra del piè di pagina e negli obiettivi), caratteri con e senza spazi, paragrafi, frasi, parole diverse, pagine da 250 parole, tempo di lettura a 230 parole al minuto e medie (lettere per parola, parole per frase e per paragrafo, la frase più lunga; passaci sopra con il puntatore per leggerla).
- **Leggibilità (stime)**, anch'essa nella prima scheda. Per l'inglese: il *Flesch Reading Ease* (da 0 a 100, più alto è più facile; da 60 a 70 è inglese semplice) e il *Flesch-Kincaid Grade Level* (la classe scolastica americana necessaria per seguire il testo). Per il tedesco: l'*indice di Flesch di Amstad* (la stessa scala) e la *Wiener Sachtextformel* (più o meno la classe scolastica, da 4 facile a 15 molto difficile). Per ogni lingua: l'indice *LIX* (parole per frase più la quota di parole con più di sei lettere; sotto 30 molto facile, sopra 60 molto difficile). La lingua viene riconosciuta automaticamente o scelta in alto. Questi valori sono indicazioni, non verdetti: sillabe e frasi vengono individuate con regole empiriche, e le formule sono state create per la prosa informativa, non per dialoghi o poesia.
- **Frequenza delle parole**: ogni parola con il suo conteggio e la sua quota, dalla più frequente. **Nascondi parole comuni** esclude articoli, pronomi, preposizioni e simili (inglese e tedesco). Imposta una lunghezza minima, cerca una parola o conta le maiuscole separatamente. Fai doppio clic su una parola per cercarla nel progetto.
- **Frasi ripetute**: gruppi da due a sei parole che hai usato più di una volta all'interno delle frasi, per esempio una descrizione data due volte. Le frasi composte solo da parole comuni sono escluse. Fai doppio clic per cercare nel progetto.
- **Documenti**: una tabella dei documenti con parole, caratteri, frasi, parole per frase e la loro quota sul totale. Fai clic sull'intestazione di una colonna per ordinare; fai doppio clic su un documento per aprirlo.

**Esporta tabella…** salva la tabella della scheda corrente come file CSV per un foglio di calcolo.

### Generatore di nomi

**Modifica > Strumenti di scrittura > Generatore di nomi…** crea nomi per i personaggi.

1. Spunta gli elenchi dei **nomi** (62 origini da Europa, Americhe, Asia e Africa, per esempio inglese, americano, irlandese e gaelico irlandese, tedesco, austriaco, svizzero, olandese, francese, franco-canadese, italiano, spagnolo, messicano, portoghese, brasiliano, danese, svedese, norvegese, finlandese, islandese, polacco, ceco, ungherese, russo, greco, turco, ebraico, arabo, persiano, indiano, giapponese, cinese, coreano, yoruba, swahili e zulu; per ora non ci sono elenchi scozzesi o gallesi). I **cognomi** provengono dallo stesso elenco del nome, oppure togli la spunta a **L'elenco del nome** e scegli altri elenchi (un nome italiano con un cognome irlandese).
2. Scegli **Genere**, **Numero di nomi** e cosa **Mostrare** (nome completo, solo nome, solo cognome, iniziali e cognome). **Con secondo nome** aggiunge un secondo nome di battesimo.
3. **Regolazione fine**: **comune ↔ raro** sceglie tra i nomi più comuni (a sinistra) o quelli rari (a destra); funziona con gli elenchi contrassegnati *per frequenza* (per esempio americano, svedese, ungherese, ebraico), che riportano la frequenza di ogni nome. **Allitterazione** dà a quella quota di nomi un cognome con la stessa iniziale del nome (Peter Parker); **Cognomi doppi** unisce due cognomi con un trattino. **Il nome inizia con** / **Il cognome inizia con** limitano i nomi a quelli che cominciano con queste lettere.
4. Fai clic su **Genera nomi**. Seleziona dei nomi e fai clic su **Copia** (tutti i nomi se nessuno è selezionato) o su **Inserisci nel testo** (anche con doppio clic) per metterli al cursore dell'editor.

Alle donne viene dato il femminile del cognome dove una lingua lo prevede (polacco *Kowalska*, ceco *Nováková*, russo *Иванова*). I nomi giapponesi, cinesi, coreani, ungheresi e vietnamiti mostrano prima il cognome. Alcuni elenchi sono nella loro scrittura originale (russo, greco, ebraico, arabo e altri); i loro nomi nell'elenco lo indicano.

**Aggiungi elenco personale…** trasforma un file di testo con un nome per riga in un elenco tutto tuo (nomi, nomi femminili o maschili, oppure cognomi). Scegli l'elenco e fai clic su **Rimuovi elenco personale** per eliminarlo. Gli elenchi di nomi provengono dal progetto open source *Faker* (licenza MIT); sono elenchi di nomi in uso, non elenchi storici verificati.

### Blocco appunti

Il blocco appunti contiene note al di fuori di ogni progetto: idee mentre è aperto un altro progetto, citazioni copiate da una pagina web, l'immagine di una parte dello schermo. Aprilo con **Finestra > Blocco appunti** (**Alt+Shift+Return**).

- **+** crea una nota (la freccia accanto: testo formattato o semplice). Il titolo sopra il testo è il nome della nota. **−** elimina la nota selezionata.
- Le note formattate accettano **grassetto**, *corsivo* e sottolineato (pulsanti o **Ctrl+B / I / U**) e immagini: incolla un'immagine oppure fai clic sulla **fotocamera** per catturare una parte dello schermo. La fotocamera è disponibile solo per le note formattate.
- Tutto viene salvato automaticamente mentre scrivi. Se una nota non può essere salvata (disco pieno, cartella delle note scomparsa), la riga di stato lo segnala e la nota resta aperta con il suo testo: un'altra nota, una nuova nota o un'altra cartella delle note si apre solo dopo che è stata salvata, e una finestra offre **Copia testo** e **Riprova**. Se il programma termina mentre una nota non può essere salvata, il suo testo finisce nella cartella di emergenza *~/.local/share/{{APP}}/Emergency* (0.65).
- **Invia al progetto** elenca ogni progetto aperto, anche quelli aperti in altre finestre di {{APP}}. Per ciascuno: **Nuovo documento nella cartella Ricerca**, **Nuovo documento in** una cartella a tua scelta, oppure **Aggiungi alla fine di** un documento di testo. Formattazione e immagini vengono portate con sé. La nota resta nel blocco appunti finché non la elimini.

Le note sono normali file in una cartella, di solito *Documents/{{APP}} Scratch Pad* (testo semplice come .txt, note formattate come .html con le loro immagini nella sottocartella nascosta *.images*). In **File > Opzioni > Generale > Blocco appunti** scegli un'altra cartella, il tipo delle nuove note e se il blocco appunti resta sopra le altre finestre. Le note modificate altrove (un'altra finestra di {{APP}}, una cartella sincronizzata) compaiono subito.

**Da ogni programma.** La scorciatoia nel programma funziona mentre {{APP}} ha la tastiera. Per aprire il blocco appunti da qualsiasi programma, fai avviare {{APP}} dal desktop con `--scratch-pad`: in KDE, **Impostazioni di sistema > Tastiera > Scorciatoie > Aggiungi nuovo > Comando o script**, inserisci il comando mostrato in **File > Opzioni > Tastiera** (pulsante **Copia comando**) e premi i tasti che vuoi. Il blocco appunti si apre allora da solo, senza una finestra di progetto, e invia le note a qualunque progetto sia aperto. Se {{APP}} è stato aggiunto al menu delle applicazioni (**Aiuto > Aggiungi al menu delle applicazioni**), la voce di menu ha anche l'azione **Blocco appunti**.

**La fotocamera** cattura l'immagine con il programma di schermate di KDE *Spectacle*, se è installato (necessario con Wayland, dove i programmi non possono leggere lo schermo da soli): trascina sulla parte che vuoi. Senza Spectacle, su X11, {{APP}} mostra lo schermo e tu trascini sulla parte desiderata; **Esc** annulla, **Enter** cattura l'intero schermo.

### Trascrizione vocale

Parla invece di scrivere: il componente aggiuntivo **Trascrizione vocale** (versione 0.61 o successiva) scrive ciò che dici. Tutto avviene su questo computer; mentre parli non viene inviato nulla da nessuna parte.

**Prima della prima registrazione: scegli un modello.** Il riconoscimento vocale richiede un modello, che si scarica una sola volta. Apri **File > Opzioni > Voce** (oppure la freccia accanto al pulsante del microfono, **Opzioni della trascrizione vocale…**), scegli un modello e premi **Scarica e usa**:

- **Small**: download da 190 MB, veloce; adatto a un parlato chiaro, traduzione solo approssimativa.
- **Medium** (consigliato): 539 MB; sta al passo con il parlato sulla maggior parte dei processori degli ultimi anni; buon riconoscimento e buona traduzione.
- **Large v3**: circa 1,1 GB; il migliore, ma su un processore spesso più lento del parlato, per cui nei passaggi lunghi il testo resta indietro. Una scheda grafica aiuta.
- **Large v3 Turbo**: 574 MB; vicino a Large v3 e più veloce, ma non sa tradurre.

I modelli provengono dai file dei modelli di whisper.cpp su Hugging Face. Sotto **Fonte** puoi indicare un altro indirizzo, oppure usare **Usa un file scaricato da me…** per un file di modello che hai scaricato tu. Prima di essere usato, un modello viene controllato: prima il suo checksum (SHA-256), poi la descrizione del modello contenuta nel file e una breve trascrizione di prova. Solo se tutto è a posto sostituisce il modello in uso, che viene poi eliminato. Un modello della fonte ufficiale deve avere il checksum noto; altrimenti non viene installato (il download potrebbe essere danneggiato). Un file da un altro indirizzo, o uno scaricato da te, il cui checksum non è quello noto, viene letto solo dopo il tuo consenso, perché il motore vocale deve leggerlo. Gli altri indirizzi devono iniziare con *https://*. Un download interrotto riprende da dove si era fermato la volta successiva. **Test di velocità** mostra quanto tempo impiega questo computer per una breve registrazione con il modello e il dispositivo scelti.

**Registrazione.** Premi il pulsante del microfono nella barra Strumenti (oppure **Modifica > Strumenti di scrittura > Trascrizione vocale**). L'ispettore si apre sulla sua scheda **Trascrizioni** (il microfono, il primo nella fila di pulsanti dell'ispettore) e inizia una nuova trascrizione, con un nome composto da data e ora. Parla; una breve pausa chiude un pezzo e il suo testo compare qualche secondo dopo. Una pausa più lunga (3 s, se non la cambi) inizia un nuovo paragrafo. Premi di nuovo il pulsante per fermare: i pezzi ancora in attesa vengono scritti, poi il testo si può modificare. La freccia accanto al pulsante sceglie il microfono.

**Le trascrizioni** sono file di testo semplice in una cartella tutta loro, fuori da ogni progetto (di solito *Documents/{{APP}} Transcriptions*), così i tuoi progetti restano come sono. Nella scheda puoi modificare una trascrizione (salvata automaticamente), rinominarla, copiarne tutto il testo per incollarlo dove ti serve, eliminarla (finisce nel cestino del desktop) oppure aprire la cartella. Mentre una registrazione viene scritta, il suo testo è di sola lettura. Se una trascrizione modificata non può essere salvata, resta aperta (la riga di stato ne spiega il motivo); se nel frattempo inizia una registrazione o il programma termina, il suo testo finisce nella cartella di emergenza *~/.local/share/{{APP}}/Emergency* (0.65).

**Registra ora, scrivi il testo dopo** (0.64). Scrivere il testo richiede molta potenza di calcolo; su un portatile a batteria (o su un telefono) puoi solo registrare e far scrivere il testo più tardi. Scegli **Solo registrazione** in **File > Opzioni > Voce** (*All'avvio del microfono*; su Android è l'impostazione standard). Il pulsante del microfono allora si limita a registrare: non gira nient'altro e non viene caricato alcun modello. La registrazione viene conservata come file audio (WAV, circa 1,9 MB al minuto) accanto alle trascrizioni, con lo stesso tipo di nome, e compare nell'elenco della scheda con un pulsante **Trascrivi** nella sua riga; **Trascrivi tutto** scrive il testo di ogni registrazione che ancora non ne ha, una dopo l'altra. Il testo compare poi come con una registrazione dal vivo e si può modificare. Una registrazione viene conservata dopo la scrittura del suo testo, a meno che tu non spunti **Elimina una registrazione dopo la scrittura del suo testo**; rinominare o eliminare una trascrizione si porta dietro la sua registrazione. Per scrivere il testo su un altro computer, scegli una cartella delle trascrizioni che il tuo programma di sincronizzazione mantiene uguale su entrambi. Se una registrazione viene interrotta mentre se ne scrive il testo (o il programma termina), mantiene il suo pulsante **Trascrivi** e può essere rieseguita.

**Lingue.** La **lingua principale** è la lingua del testo; la **lingua di riserva** è un'altra lingua che parli (entrambe si scelgono tra le undici lingue del programma: inglese, tedesco, spagnolo, francese, portoghese, italiano, russo, indonesiano, turco, polacco e olandese). Con l'inglese come lingua principale, ciò che dici nella lingua di riserva viene scritto in inglese (whisper traduce solo verso l'inglese). Le parole della lingua di riserva dentro frasi inglesi possono essere capite male (in un test, "Bahnhof" è diventato "Bon Huff"); mettile sotto **Parole di aiuto**, e verranno scritte come le pronunci ("Bahnhof", "Brötchen"). Con un'altra lingua principale, o con Large v3 Turbo, il parlato resta nella lingua in cui è stato pronunciato. **Conserva l'originale accanto alla traduzione** aggiunge l'originale tra parentesi dopo un pezzo tradotto (richiede più tempo). **Parole di aiuto**: nomi e parole che usi, così vengono scritti a modo tuo.

**Altre opzioni.** **Calcola con**: il processore, oppure una scheda grafica che whisper.cpp può usare tramite Vulkan (mostrata quando ne viene trovata una; le schede con troppa poca memoria per il modello sono elencate ma non si possono scegliere). Su Windows il componente aggiuntivo per ora usa solo il processore: lì Small o Medium tengono meglio il passo. **Thread del processore**: uno in meno di quelli del processore mantiene fluida la digitazione. **Precisione**: *Accurata* valuta diversi testi possibili e individua prima la lingua di ogni pezzo: migliore, ma più lenta. **Microfono**, **Sensibilità** (bassa per una stanza rumorosa, alta per una voce sommessa), **Una pausa che chiude un pezzo**, **Nuovo paragrafo dopo una pausa di** (*mai* lo disattiva), la **cartella** e, sotto **Avanzate**: omettere il testo nel silenzio e nel rumore (whisper a volte inventa una riga come "Thank you for watching" in quei momenti) e usare il testo precedente come contesto per il pezzo successivo.

---

### Assistente di traduzione

Dalla 0.92.0 il componente aggiuntivo **Assistente di traduzione** (versione 1.5; fino alla 1.4 si chiamava Translation and Proofreading) è uno dei componenti aggiuntivi propri di {{APP}} e viene fornito con la versione completa; per un {{APP}} che hai già, è un pacchetto a sé nella pagina Releases (sezione 17). Traduce tra due qualsiasi di undici lingue e controlla l'ortografia e la grammatica di testi in inglese, sul tuo computer: un testo non lo lascia mai. {{APP}} stesso non ha funzioni di IA; questo componente aggiuntivo lavora con modelli linguistici e, come ogni altro, si può disattivare o rimuovere nella Gestione componenti aggiuntivi.

- **Questo strumento dà solo suggerimenti. Le sue traduzioni non sono precise: controllale una per una, il che richiede una buona conoscenza di entrambe le lingue.** La stessa frase compare nella finestra del componente aggiuntivo e nella sua descrizione.

- Ha una scheda propria nell'ispettore e una finestra propria (il suo comando si trova nel sottomenu Strumenti di scrittura del menu Modifica, e ha un pulsante nella barra degli strumenti). Il testo entra, il risultato esce con ogni modifica evidenziata, in tempo reale mentre scrivi; un clic su una parola mantiene l'originale oppure propone altre parole.
- Richiede un modello, che scegli e scarichi una volta nelle opzioni del componente aggiuntivo (da alcune centinaia di megabyte a diversi gigabyte; un modello troppo grande per il computer non viene proposto). I modelli vengono scaricati dalle release *models-1*, *models-2*, *models-3* e *models-4* del repository di {{APP}}, ciascuno viene installato solo se il suo checksum è quello noto e, dopo, il componente aggiuntivo funziona senza alcuna connessione. Un modello, CoEdIT Large, può essere usato solo per scopi non commerciali; il componente aggiuntivo lo dice prima del download.
- **Una parola da sola** (1.3). Una parola inglese: il risultato è la parola come dovrebbe essere e, sotto, che cosa significa (i significati provengono dal dizionario WordNet 3.0, integrato nel componente aggiuntivo; una parola che non ha riceve un significato scritto dal modello Qwen3 che hai scaricato, con l'avvertenza che può essere impreciso, e senza un tale modello il componente aggiuntivo dice che cosa puoi scaricare), i suoi suggerimenti ortografici e parole di significato simile. Una parola tedesca: il componente aggiuntivo la cerca prima nel **dizionario tedesco**, un estratto di Wiktionary che scarichi separatamente nelle opzioni del componente aggiuntivo (la sua licenza, CC BY-SA 4.0, viene mostrata prima del download): la forma base, le categorie grammaticali, i significati e le parole inglesi di ogni significato. Dei significati di una categoria ne vengono mostrati i primi cinque, il resto è dietro "altro"; un clic su una parola inglese ne fa il risultato, e "Simili" segue poi quella parola e la sua categoria grammaticale. Per ciò che il dizionario non dice, e per una parola che non ha, il piccolo modello traduce la parola e l'output avverte che può essere impreciso. Dall'inglese al tedesco funziona allo stesso modo per una parola inglese, secondo le categorie grammaticali che WordNet conosce.
- **Inverti e Traduci di nuovo** (1.3). **Inverti** trasforma tedesco-inglese in inglese-tedesco e viceversa: il risultato passa nel testo e viene tradotto nell'altro senso. **Traduci di nuovo** traduce il risultato nella lingua del tuo testo e lo mostra sotto il risultato, finché non cambi il testo; richiede il modello per il percorso inverso (OPUS-MT inglese-tedesco (piccolo) per un testo tedesco) e propone di scaricarlo se manca. Mentre traduci, la seconda riga dei controlli mostra il modello di traduzione (sceglierne un altro cambia l'impostazione e la traduzione) e l'elenco degli stili è nascosto.
- **Testi brevi** (1.3). Una singola parola e un testo fino a tre parole non vengono mai affidati al grande modello di traduzione (è migliore per frasi intere e inaffidabile per parole singole): prima il dizionario, altrimenti il piccolo modello, anche quando è scelto quello grande. Per un testo breve di più parole, "Altre traduzioni:" elenca le migliori traduzioni complete del modello, senza nomi inventati.
- **Versione 1.4.** Una singola parola in entrambe le direzioni ora mostra, sotto il risultato, le traduzioni di ciascuna categoria grammaticale che ha (una parola tedesca che è un verbo in minuscolo e un sostantivo con la maiuscola ottiene entrambe; una parola che il dizionario non ha viene tradotta in entrambe le forme), poi "Traduzioni simili" (parole di significato simile al risultato inglese) o, dall'inglese al tedesco, "Parole tedesche simili": le parole tedesche i cui significati nominano la parola inglese o uno dei suoi sinonimi (richiede il dizionario tedesco; senza, una riga lo dice), poi la breve descrizione. Con "Automatico" una singola parola tedesca che non è una parola inglese viene riconosciuta come tedesca quando il dizionario tedesco è installato. Prima che il tedesco venga tradotto, le maiuscole dei sostantivi tedeschi vengono corrette nella copia che va al modello (il tuo testo resta com'è): una parola che il dizionario conosce solo come sostantivo riceve la maiuscola, una parola nota in entrambi i modi ("essen") mantiene la forma che hai digitato, tranne subito dopo un articolo o una preposizione articolata ("das Essen", "im Essen"); senza il dizionario una nota sotto la traduzione dice che i sostantivi tedeschi richiedono la maiuscola. Le alternative che differiscono tra loro o dalla traduzione solo per le maiuscole vengono mostrate una volta sola. L'area della parola non mostra più una riga "Ortografia" per le parole del risultato (le parole su cui fai clic nel tuo testo la mantengono), e le regole sui suffissi delle parole simili non leggono più "number" come "numb". Dove manca qualcosa (un modello di traduzione, il dizionario tedesco), un avviso in cima alla scheda e alla finestra del componente aggiuntivo lo nomina e ha un pulsante che apre le opzioni del componente aggiuntivo. Un modello che non è stato usato per 15 minuti viene scaricato dalla memoria per liberarla (**File > Opzioni > Correzione > Velocità > Scarica i modelli dopo**; "mai" lo mantiene caricato). I suggerimenti nei campi vuoti vanno a capo quando la larghezza è ridotta.
- **Versione 1.5: undici lingue.** Il componente aggiuntivo traduce tra due qualsiasi tra inglese, tedesco, spagnolo, francese, portoghese (Brasile), italiano, russo, indonesiano, turco, polacco e olandese: con **Traduci** scelto, due elenchi a discesa nominano la lingua del tuo testo e la lingua della traduzione. Ogni lingua ha due piccoli modelli OPUS-MT, da e verso l'inglese (le loro licenze, Apache-2.0 o CC BY 4.0, vengono mostrate nelle opzioni prima di un download); una coppia senza inglese (spagnolo-russo, per esempio) passa per l'inglese con entrambi i modelli, e una nota sotto il risultato lo dice, perché un errore del primo passaggio si trascina nel secondo. Ogni lingua tranne l'inglese ha anche un **dizionario** facoltativo da Wiktionary (CC BY-SA 4.0, un download separato di cui viene prima mostrata la licenza), usato per una parola singola di quella lingua come lo era il dizionario tedesco. Una lingua senza un dizionario utilizzabile riceve solo le traduzioni del modello. Una parola singola viene comunque corretta come suggerimento: la riga dell'ortografia e le parole sotto di essa dicono che sono suggerimenti da controllare. I testi propri del componente aggiuntivo sono disponibili nelle undici lingue del programma (la lingua che hai scelto per {{APP}}; inglese dove un testo non è tradotto). Le impostazioni della 1.4 continuano a funzionare: il dizionario tedesco e i modelli che hai scaricato restano, e tedesco-inglese è la prima scelta. Le sue opzioni si trovano ora in **File > Opzioni > Assistente di traduzione**.
- I modelli scaricati con la versione 1.1 restano come sono. La versione 1.1 scaricava i modelli dal precedente repository del componente aggiuntivo; la 1.2 conosce il nuovo indirizzo.
- La descrizione del componente aggiuntivo, con le sue regole per la correzione (ortografia americana, virgolette ricurve, niente trattini lunghi) e i punti di forza di ogni modello, si trova in *addons/translation-proofreading/source/README.md* nel sorgente di {{APP}}.

## 11. Istantanee

Un'istantanea è una copia salvata di un documento in un certo momento. Creane una prima di una grande riscrittura:

- Un documento con almeno un'istantanea mostra un angolo piegato (chiaro e grigio) in alto a destra della sua icona nel raccoglitore.
- **Ctrl+5** crea un'istantanea; **Ctrl+Shift+5** chiede un titolo.
- Con **File > Opzioni > Generale > Salvataggio**, **File > Salva** (**Ctrl+S**) crea un'istantanea di ogni testo modificato dall'ultimo File > Salva (con il titolo "Salvato"); **File > Opzioni > Comportamento > Istantanee** mostra un breve messaggio nella barra di stato quando vengono create istantanee.
- Nella scheda delle istantanee dell'ispettore puoi **confrontare** un'istantanea con il testo corrente (le aggiunte sono mostrate a colori, le eliminazioni barrate; scegli se confrontare parole, frasi, paragrafi o caratteri) e **ripristinarla**. Prima del ripristino, {{APP}} può creare un'istantanea del testo corrente, così non si perde nulla. L'istantanea viene letta per prima (0.65): una che non si può leggere per intero lascia il testo corrente com'è, una che non contiene testo lo sostituisce solo dopo una seconda domanda e, quando l'istantanea del testo corrente è stata richiesta ma non si può creare, non viene ripristinato nulla. Il ripristino è anche il modo per riparare un documento il cui file è danneggiato o in un'altra codifica dei caratteri (viene mostrato in sola lettura): il file danneggiato viene prima conservato come copia in *settings/conflicts*, e l'ispettore dice dove (0.81.1).
- **Progetto > Elimina istantanee vecchie…** (dalla 0.61.1, sotto Svuota cestino) elimina ogni istantanea del progetto, anche quelle dei documenti nel Cestino, per rimpicciolire di nuovo il progetto. I testi restano come sono ora; vanno via solo le loro versioni precedenti. Una domanda iniziale indica quante istantanee ci sono, di quanti documenti e quanto spazio occupano; non si può annullare, quindi fai prima un backup (**File > Backup**). Il comando è disattivato finché il progetto non ha istantanee. In un progetto in formato Scrivener le istantanee escono anche dal *.scriv* (con il salvataggio che segue subito); dalla 0.61.1 vale lo stesso anche per una singola istantanea eliminata nell'ispettore.

---

## 12. Modalità composizione

**F11** riempie lo schermo con nient'altro che il tuo testo su un foglio di carta. Porta il mouse sul bordo inferiore per mostrare la barra di controllo: zoom, larghezza della carta, dissolvenza dello sfondo, scorrimento macchina da scrivere, conteggio delle parole e un piccolo ispettore con sinossi e note. **Alt+Shift+Up / Down** passa al documento precedente o successivo, **Ctrl+[** e **Ctrl+]** vanno indietro e avanti tra i documenti che hai guardato. Le scorciatoie dei menu Formato e Inserisci (grassetto, stili, commenti ...) funzionano anche qui. **Esc** o **F11** riporta alla finestra principale.

**File > Opzioni > Comportamento > Modalità composizione** sceglie lo schermo su cui si apre (quello corrente, quello principale o uno indicato per nome), se la finestra principale si toglie di mezzo (ridotta a icona mentre la modalità composizione è aperta) e se **Esc** la chiude (**F11** lo fa sempre).

La barra di controllo è scura in ogni tema, così i suoi pulsanti, l'elenco dello zoom e i loro testi restano leggibili anche nei temi chiari (0.94.0). I colori di testo, carta e sfondo provengono dal tema. Per cambiarli, per esempio in nero su bianco o verde su nero, vai in **File > Opzioni > Aspetto > Modalità composizione**; la modifica si vede subito, anche mentre la modalità composizione è aperta. Un'immagine di sfondo si può impostare per progetto in **Impostazioni del progetto > Cartelle speciali**.

---

## 13. Compilazione: creare il manoscritto

**File > Compila…** (**Ctrl+Shift+E**) riunisce tutto ciò che c'è nella Bozza in un unico file.

### La finestra di compilazione

La finestra di compilazione ha **Compila per** in alto, i formati a sinistra, i layout delle sezioni al centro e, a destra, contenuti, metadati, opzioni e sostituzioni (quattro schede con icone; tieni il mouse su una scheda per vederne il nome). La colonna dei formati è larga almeno quanto il suo nome più lungo; trascina le linee tra le tre colonne per allargarne una, e la finestra ricorda le larghezze per la volta successiva.

1. **Compila per**: il formato di output. Integrati, senza bisogno di altri programmi: **Documento Word (.docx)**, **PDF (.pdf)**, **E-book (.epub)** e, per gli script, **Fountain** e **Final Draft**. Compaiono anche PDF e OpenDocument tramite LibreOffice, EPUB tramite Pandoc e Markdown, se sono installati il plugin e il programma corrispondenti (vedi sotto). Accanto: quanti documenti e circa quante parole verranno compilati.
2. **Formati** (a sinistra), sotto **I miei formati** e **Formati integrati**: il formato di compilazione decide l'aspetto del manoscritto: caratteri, interlinea, dimensione della pagina, margini, intestazioni e piè di pagina, l'aspetto di capitoli e scene, come risultano stili, note e citazioni. I formati integrati (*Manuscript (Times)*, *Manuscript (Courier)*, *Modern*, *Paperback (6 x 9 in)*, *E-book*, *Default (Editor Formatting)* e *Outline (Synopses)*) non si possono modificare. I pulsanti sotto l'elenco: **+** (duplica il formato selezionato per creare una tua versione e modificarla), la matita (modifica il tuo formato), **-** (eliminalo), **Importa...** ed **Esporta...** (un file *.qbformat* da usare su un altro computer o da regalare; un formato importato il cui nome è già preso riceve un numero).
3. **Layout delle sezioni** (al centro): **Carattere** imposta un unico carattere per l'intero manoscritto, oppure lo lascia ai layout ("Determinato dal layout della sezione"). La tabella elenca ogni tipo di sezione del tuo progetto con il layout che riceve in questo formato; scegline un altro nell'elenco. Sotto la tabella, una piccola immagine mostra come inizia una sezione con il layout selezionato. **Assegna layout delle sezioni...** apre una finestra più grande con i tipi di sezione a sinistra e le immagini di tutti i layout del formato a destra. I formati integrati conoscono i nomi consueti dei tipi di sezione: in *Manuscript (Courier)*, per esempio, *Part* riceve **Part Title**, *Chapter* riceve **Chapter Heading**, e *Scene* o *Section* riceve **Section Text**.
4. **Contenuti** (prima scheda): ciò che viene compilato, con il tipo di sezione di ogni documento e il suo layout. Scegli una cartella per compilare solo una parte della Bozza (per esempio un capitolo), aggiungi la parte iniziale (frontespizio) e la parte finale (ringraziamenti). **Il nome dell'autore** (0.94.0): il frontespizio dei modelli di progetto riporta "di" e il nome dell'autore. La prima compilazione che scrive un frontespizio chiede quel nome (nome e cognome; viene conservato con le Informazioni sull'autore in **File > Opzioni > Generale**, quindi viene chiesto una volta sola); se lo lasci vuoto, la riga "di" viene omessa dal frontespizio, in ogni formato. Il campo Autore della scheda Metadati della finestra di compilazione, se contiene un nome, viene invece usato per il file compilato. Togli la spunta a un documento per escluderlo; questo disattiva la sua impostazione **Includi nella compilazione**. **Aggiungi schede degli oggetti** inserisce una scheda per ogni oggetto della scheda Oggetti (sezione 7) dopo la parte finale: un titolo per categoria (Personaggi, Luoghi ...), poi ogni oggetto con i suoi campi; il pulsante accanto sceglie le categorie. All'inizio è disattivato.
5. **Metadati** (seconda scheda): titolo, sottotitolo, autore, lingua (per esempio *en-US*), argomento, parole chiave, editore, data, descrizione, la copertina di un e-book (un'immagine del raccoglitore) e l'identificatore del libro. I campi vuoti prendono il titolo del progetto, il tuo nome e la data di oggi. I dettagli finiscono nelle proprietà del documento del PDF, nell'e-book e nel file Word; anche qui funzionano segnaposto come `<$projecttitle>`.
6. **Opzioni** (terza scheda): se il testo delle cartelle viene compilato.
7. **Sostituzioni** (quarta scheda): sostituzioni di questo progetto, eseguite dopo quelle del formato, per esempio un nome scritto in un modo nelle tue bozze e in un altro nel libro. Ogni riga si può attivare e disattivare e può essere un'espressione regolare, distinguere maiuscole e minuscole o riguardare solo parole intere.
8. **Anteprima** mostra le pagine del manoscritto come PDF in una finestra a parte, con il formato e i contenuti che hai scelto, prima che venga salvato qualcosa.

Il carattere, le sostituzioni e le schede degli oggetti vengono conservati con il progetto come le altre scelte (le versioni più vecchie di {{APP}} le ignorano).

Fai clic su **Compila…**, scegli dove salvare e il file finito si apre nel tuo elaboratore di testi o lettore, se vuoi. **Salva impostazioni** conserva le scelte senza compilare.

**Caratteri e formattazione dei paragrafi impostati a mano.** Con il formato *Default (Editor Formatting)*, il file compilato li mantiene tutti: caratteri, dimensioni, colori, maiuscole, spaziatura tra le lettere, rientri, spaziatura, interlinea e tabulazioni. Ogni altro formato ha un aspetto proprio: sostituisce carattere, dimensione, rientri e spaziatura, e mantiene colore del testo, maiuscole, spaziatura tra le lettere, allineamento e tabulazioni.

### Modificare un formato di compilazione

**Duplica e modifica…** e **Modifica…** aprono l'editor dei formati. L'elenco a sinistra contiene le sue pagine:

- **Testo**: carattere, dimensione, interlinea, rientro della prima riga, spazio dopo i paragrafi, testo giustificato, carattere e dimensione dei titoli, titoli in grassetto. **Usa la formattazione dell'editor del progetto** mantiene invece l'aspetto dell'editor.
- **Layout delle sezioni**: i layout del formato (aggiungi, duplica, rimuovi) con titolo, prefisso e suffisso (`Chapter <$W:chapter>`), stile e allineamento del titolo, maiuscolo, righe vuote prima del titolo, testo, sinossi e note, una nuova pagina, il separatore prima della sezione, nessun rientro sul primo paragrafo. L'anteprima mostra come inizia una sezione con questo layout.
- **Assegnazioni**: quale layout riceve ogni tipo di sezione in questo formato.
- **Stili**: come risulta in questo formato uno stile del tuo progetto. **Aggiungi uno stile…**, poi scegli: **Mantieni il suo aspetto**, **Cambia il suo aspetto** (carattere, dimensione, grassetto, corsivo, sottolineato, maiuscoletto o tutto maiuscolo, colore, allineamento, rientri, spazio prima e dopo, interlinea, testo prima e dopo, maiuscolo, inizia su una nuova pagina; una casella spuntata a metà, o "invariato", lascia lo stile com'è), **Rendilo testo normale** oppure **Ometti il testo** (per esempio note per te stesso in uno stile a parte).
- **Pagina**: formato della carta e margini, margini speculari per i libri (interno ed esterno), intestazioni e piè di pagina (sinistra, centro, destra; segnaposto come `<$p>` per il numero di pagina e `<$title>`), pagine affiancate (testi di sinistra sulle pagine di sinistra, testi di destra sulle pagine di destra), intestazione e piè di pagina sulla prima pagina e sulle pagine in cui inizia un capitolo, capitoli su una pagina di destra, il numero della prima pagina e il suo stile (1, 2, 3 / i, ii, iii / I, II, III), la dimensione di intestazioni e piè di pagina.
- **Note a piè di pagina e commenti**: note a piè di pagina in fondo alla pagina (negli e-book: note a comparsa), alla fine di ogni capitolo o alla fine del testo (PDF ed e-book; un file Word ha sempre le note a piè di pagina in fondo alla pagina), con il titolo delle note e la loro dimensione; annotazioni in linea come commenti o rimosse; commenti collegati come commenti del file Word; evidenziazioni rimosse.
- **Sostituzioni**: testo sostituito solo nel file compilato (il tuo progetto resta com'è): attivo o disattivo, maiuscole/minuscole, parole intere, espressioni regolari (`\1` fino a `\9` in "Con").
- **Trasformazioni**: virgolette dritte o tipografiche, trattini come trattini corti, corsivo come sottolineato (la convenzione del manoscritto), più spazi come uno solo, nessuno spazio alla fine dei paragrafi, testo semplice senza grassetto, corsivo, caratteri o colori.
- **Indice, PDF ed e-book**: un indice (il suo titolo e quanti livelli di titolo), segnalibri PDF per i titoli, collegamenti web cliccabili nel PDF, un file per sezione che inizia su una nuova pagina in un e-book, e l'indice anche come pagina dell'e-book.

I tuoi formati sono conservati in *~/.local/share/{{APP}}/CompileFormats* e sono disponibili in ogni progetto.

### PDF

L'output PDF integrato impagina le pagine da sé: note a piè di pagina in fondo alla pagina in cui sono richiamate (una lunga prosegue nella pagina successiva), intestazioni e piè di pagina, capitoli su pagine di destra se il formato lo prevede, titoli tenuti insieme al testo che segue, nessuna riga isolata in cima o in fondo a una pagina, un indice con numeri di pagina su cui si può fare clic, segnalibri per i titoli nel pannello laterale del lettore e collegamenti cliccabili. I caratteri sono incorporati. Le immagini mantengono la dimensione che hanno nel testo; un'immagine più grande dell'area di testo viene ridotta per adattarsi. Immagini e tabelle accanto al testo hanno il testo che scorre attorno, come nell'editor. Una tabella prosegue su tutte le pagine che servono, riga per riga, con le righe di intestazione in cima a ogni pagina.

### E-book

L'output e-book integrato scrive EPUB 3 (con un indice anche per i lettori più vecchi): un file per capitolo, i titoli nell'indice del lettore, note a piè di pagina come note a comparsa (oppure alla fine di ogni capitolo o del libro), immagini e la copertina da **Metadati…**. Le tabelle mantengono il loro design; un'immagine con didascalia è una figura (accanto al testo, se lo è nell'editor) e la sua descrizione è il testo alternativo dell'immagine, che i lettori di schermo leggono. Il libro riceve un identificatore la prima volta che viene compilato; resta con il progetto, quindi un lettore considera una nuova versione come lo stesso libro. I lettori scelgono i propri caratteri e la propria dimensione di pagina, quindi le impostazioni di pagina, le intestazioni e i piè di pagina non si applicano.

### Word

Il file Word usa stili con nome (Title, Subtitle, Heading 1 to 3, Quote, Centered, Caption, e i tuoi stili di paragrafo e di carattere), così i suoi titoli compaiono nel riquadro di navigazione di Word. Con **Includi un indice**, all'inizio viene inserito un campo indice, e il file chiede all'elaboratore di testi di aggiornare i suoi campi all'apertura, cosa che riempie i numeri di pagina; se un elaboratore di testi non lo fa, aggiorna tu il campo lì. Intestazioni su pagine affiancate, prima pagina e numero della prima pagina provengono dalle impostazioni **Pagina** del formato. Le tabelle sono tabelle Word con le loro linee, colori, celle unite e righe di intestazione (ripetute su ogni pagina in Word). Un'immagine accanto al testo va in una tabella di una sola cella senza linee, insieme alla sua didascalia, in modo che Word e LibreOffice facciano scorrere il testo attorno a entrambe; la descrizione di un'immagine è il suo testo alternativo.

### Titoli e segnaposto

In un layout di sezione, il titolo può contenere segnaposto che la compilazione sostituisce:

| Segnaposto | Risultato |
|-------------|--------|
| `<$n>` | 1, 2, 3 … |
| `<$w>` / `<$W>` / `<$WW>` | one, two … / One, Two … / ONE, TWO … |
| `<$R>` / `<$r>` | I, II, III … / i, ii, iii … |
| `<$L>` / `<$l>` | A, B, C … / a, b, c … |
| `<$n:name>` | un contatore separato chiamato *name* (funziona con ogni stile di numerazione qui sopra) |
| `<$title>` | il titolo del documento |
| `<$projecttitle>` / `<$shorttitle>` | il titolo del progetto / le sue prime tre parole in maiuscolo |
| `<$author>`, `<$forename>`, `<$surname>` | il tuo nome da **File > Opzioni** o dalle impostazioni del progetto |
| `<$wc>` / `<$wc100>` / `<$wc1000>` | il conteggio delle parole, esatto o arrotondato |
| `<$date>` / `<$year>` | la data di oggi (aaaa-mm-gg) / l'anno |
| `<$p>` | il numero di pagina (solo intestazioni e piè di pagina) |
| `<$pages>` | il numero di pagine (intestazioni e piè di pagina di file PDF e stampe) |

Esempio: il titolo `Chapter <$W:chapter>` diventa *Chapter One*, *Chapter Two*, e così via.

I segnaposto si possono anche digitare nel testo stesso (**Inserisci > Numerazione automatica** e **Inserisci > Segnaposto**). **Aiuto > Elenco di tutti i segnaposto…** li elenca tutti; seleziona delle righe e premi **Ctrl+C** per copiarle.

### Stili durante la compilazione

I titoli, Title, Subtitle, Block Quote, Centered e Caption ricevono il loro aspetto dal formato di compilazione. Ogni altro stile porta con sé la propria formattazione (rientri, spaziatura, allineamento, carattere, colore), in ogni formato di compilazione. Con **Default (Editor Formatting)**, vengono riportate anche le modifiche che hai fatto agli stili integrati. La pagina **Stili** di un formato cambia questo comportamento per ogni stile (vedi *Modificare un formato di compilazione*).

### Note durante la compilazione

Nella pagina **Note a piè di pagina e commenti** di un formato di compilazione scegli dove vanno le note a piè di pagina in un PDF o in un e-book (in fondo alla pagina, alla fine di ogni capitolo, alla fine del testo), se le annotazioni in linea diventano commenti nel file Word o vengono rimosse, se i commenti collegati vengono mantenuti e se le evidenziazioni vengono rimosse. In un file Word, le note a piè di pagina sono sempre vere note a piè di pagina in fondo alla pagina.

### Altri formati di output (plugin)

{{APP}} include anche questi formati. Ciascuno richiede un programma da installare separatamente; PDF ed EPUB tramite LibreOffice e Pandoc sono un'alternativa a quelli integrati (partono dal file Word e gli somigliano):

| Formato | Richiede |
|--------|-------|
| PDF (tramite LibreOffice) | LibreOffice (installato sulla maggior parte dei sistemi Ubuntu e Kubuntu) |
| OpenDocument Text (.odt, tramite LibreOffice) | LibreOffice |
| E-book (.epub, tramite Pandoc) | Pandoc (installa "pandoc" in Discover) |
| Markdown (.md) | niente |

**File > Opzioni > Plugin** mostra quali formati sono pronti e a quali manca un programma. Altri formati si possono installare come componenti aggiuntivi (sezione 17).

### Compilare script

Per gli script si ottengono due output, che non richiedono altri programmi:

- **Fountain Screenplay (.fountain)**: un file di testo semplice che Highland, Fade In, Beat, Trelby e altri programmi di sceneggiatura leggono.
- **Final Draft (.fdx)**: il formato di file di Final Draft, che anche la maggior parte dei programmi di sceneggiatura sa aprire.

In entrambi, ogni paragrafo mantiene ciò che è (intestazione di scena, azione, personaggio, indicazione tra parentesi, dialogo, transizione, inquadratura); l'altro programma impagina lo script a modo suo. I titoli di cartelle e documenti che il formato di compilazione aggiunge diventano sezioni (Fountain) o titoli di atto (Final Draft); il testo che non è un elemento di script diventa azione. Il titolo del progetto e il tuo nome (File > Opzioni) formano il frontespizio. Le annotazioni in linea diventano note in un file Fountain e vengono omesse da un file Final Draft.

Per Word o PDF, scegli il formato di compilazione **Default (Editor Formatting)**: mantiene i rientri, le maiuscole e il carattere degli elementi come li vedi nell'editor. Gli altri formati di compilazione sostituiscono carattere e dimensione ma mantengono i rientri.

---

## 14. Importazione ed esportazione

### Importare file

**File > Importa > File…** (**Ctrl+Shift+I**) aggiunge al raccoglitore file di testo (.txt, .md, .rtf, .docx, .odt; i vecchi file Word .doc con il componente aggiuntivo qui sotto), script (.fountain, .fdx), immagini, PDF, audio e video e altri file. Puoi anche trascinare file dal gestore file nel raccoglitore. I file di testo diventano documenti modificabili; immagini e PDF vengono mostrati nell'editor; audio e video vengono riprodotti lì con il componente aggiuntivo Media Playback (sezione 6). Un file che non si può copiare nel progetto (disco pieno, file illeggibile) viene nominato in un messaggio e non viene aggiunto, così non resta alcun elemento vuoto (0.92.0).

Il modo in cui i file vengono letti si imposta in **File > Opzioni > Condivisione > Importa**: se i commenti dei file Word, LibreOffice e RTF diventano annotazioni in linea (altrimenti restano commenti accanto al testo), se le note a piè di pagina diventano note in linea (l'impostazione standard) o note collegate, se gli stili di paragrafo e di carattere dei file vengono omessi (il testo riceve l'aspetto del tuo progetto; grassetto, corsivo, note e collegamenti restano), la codifica dei file di testo semplice (automatica: UTF-8, altrimenti Windows occidentale) e se i file *.html* diventano documenti di testo invece di documenti pagina web.

Dove vanno i file: seleziona una cartella nel raccoglitore per importare al suo interno (i file vanno in fondo), oppure un documento per metterli subito sotto di esso. La Bozza accetta solo testo; gli altri file trascinati lì vanno in Ricerca.

**Importa e dividi…** legge un file lungo e lo taglia in molti documenti, per esempio a ogni riga che contiene solo `#`, o a ogni doppia riga vuota. Dalla 0.94.0 può anche tagliare un file .docx, .odt o .md **in corrispondenza dei suoi titoli**: Titolo 1 diventa una cartella, Titolo 2 una cartella o un documento (scegli nella finestra), il testo tra i titoli diventa documenti e i titoli inferiori restano nel testo. La finestra mostra come albero il raccoglitore che ne risulterebbe prima dell'importazione. Il titolo di un documento che nessun titolo nomina segue una regola, scritta nella finestra: la prima riga non vuota del suo testo (accorciata a 120 caratteri se più lunga), oppure "<nome del file> <numero>" quando **Usa la prima riga di ogni sezione come titolo** è disattivato.

### Vecchi file Word (.doc)

Word 97-2003 salvava i documenti come *.doc* (modelli *.dot*). Diventano documenti di testo con il componente aggiuntivo **Word 97–2003 Import** (sezione 17); senza di esso, un file .doc viene aggiunto al raccoglitore così com'è e ti viene detto quale componente aggiuntivo lo legge. Il componente aggiuntivo non richiede altri programmi. Mantiene:

- il testo, compresi greco, cirillico, scritture asiatiche e simili;
- titoli (Titolo da 1 a 6), titolo, sottotitolo, citazioni e didascalie, che ricevono gli stili del tuo progetto; uno stile di paragrafo del documento che anche il tuo progetto ha (stesso nome) riceve quello stile;
- grassetto, corsivo, sottolineato, barrato, maiuscoletto e tutto maiuscolo, apice e pedice, evidenziazione; paragrafi centrati, allineati a destra e giustificati;
- elenchi puntati e numerati con i loro livelli (i numeri sono creati da {{APP}}, quindi la numerazione "Capitolo 1" dei titoli non viene ripresa);
- tabelle, come una riga per ogni riga con le celle separate da tabulazioni;
- note a piè di pagina e note di chiusura (come note a piè di pagina), commenti con le parole commentate (il nome dell'autore viene per primo nel commento);
- collegamenti, il risultato dei campi (date, numeri di pagina come erano stati mostrati l'ultima volta), interruzioni di pagina e di sezione, interruzioni di riga;
- immagini nel testo (JPEG, PNG, BMP, TIFF);
- nei progetti RTF anche caratteri, dimensioni, colori e rientri impostati a mano (i progetti Markdown mantengono l'aspetto del progetto, come con ogni importazione).
Esclusi: intestazioni e piè di pagina, immagini e disegni mobili, immagini nei vecchi formati metafile (WMF, EMF), oggetti incorporati, testo nascosto e testo eliminato con le modifiche registrate (il testo inserito resta). Il testo delle caselle di testo viene aggiunto alla fine del documento. Dopo l'importazione un messaggio indica che cosa è stato escluso.

**File > Opzioni > Condivisione > Conversione** può fare di LibreOffice la prima scelta per i file *.doc* (legge anche intestazioni, disegni e file Word 6/95); quando la strada scelta fallisce, si prova l'altra. I file Word 6.0 e Word 95, e i file che il componente aggiuntivo non riesce a leggere, vengono convertiti con LibreOffice, se è installato (su Ubuntu: `sudo apt install libreoffice-writer`); altrimenti il messaggio lo dice. I file protetti da password vanno prima aperti e salvati senza password. Un file *.doc* che in realtà è un file RTF, un file Word 2007 (.docx) o una pagina web, come alcuni programmi li hanno salvati, viene letto per quello che è, anche senza il componente aggiuntivo.

### Importare pagine web

**File > Importa > Pagina web…** porta una pagina del web nel tuo progetto. Il lavoro lo fa il componente aggiuntivo **Web Page Import** (sezione 17); senza di esso, il comando te lo dice.

1. Seleziona la cartella che deve ricevere la pagina (altrimenti finisce in **Ricerca**).
2. Scegli **File > Importa > Pagina web…**. Se negli appunti c'è un indirizzo web, è già inserito; altrimenti digitane o incollane uno.
3. Fai clic su **Carica** (o premi Enter). La pagina e le sue immagini vengono scaricate, e la finestra mostra che cosa verrà importato, con il numero di parole e di immagini, l'autore, il sito e la data.
4. Correggi il **Titolo** se vuoi, controlla le scelte qui sotto e fai clic su **Importa**.

**Solo l'articolo** mantiene il testo per cui sei venuto e lascia fuori menu, pubblicità, avvisi sui cookie, pulsanti di condivisione, sezioni dei commenti ed elenchi di altri articoli. {{APP}} individua l'articolo come fanno gli strumenti di lettura (vince il testo con più frasi e virgole e meno collegamenti; nomi come "article" o "sidebar" nella pagina contano a favore o contro una parte). Se manca qualcosa, togli la spunta per prendere l'intera pagina. **Immagini** scarica le immagini dell'articolo (quelle molto grandi vengono portate a una larghezza di 1600 pixel). Per una pagina da internet, le immagini vengono scaricate solo da indirizzi pubblici: una che si trova sul tuo computer o su una rete privata (anche tramite un reindirizzamento) viene esclusa, e il risultato indica quante. Una pagina che proviene essa stessa dal tuo computer o dalla tua rete viene scaricata con tutte le sue immagini.

**Importa come**:

- **Documento di testo** (l'impostazione standard): titoli, paragrafi, citazioni, elenchi, tabelle (una riga per riga), codice di programma, didascalie, grassetto, corsivo e collegamenti diventano un normale documento che puoi modificare, annotare, dividere e compilare.
- **Documento pagina web**: una copia di sola lettura dell'articolo con le sue immagini, per riferimento. Non viene contata né compilata.

**Visualizzare un documento pagina web** (0.81.3). L'editor mostra la pagina come semplice testo formattato, con il suo indirizzo e due pulsanti sopra. **Apri pagina salvata nel browser** mostra la pagina com'era quando è stata salvata, nel tuo browser web: {{APP}} ne mette una copia nella tua cartella Download (*Quillbinder web pages/<titolo>*), perché un browser installato come pacchetto in sandbox (Firefox su Kubuntu, per esempio) non può leggere le cartelle di {{APP}}; la riga sotto i pulsanti dice dove si trova la copia, e il clic successivo la sostituisce. Su Windows la copia viene invece creata in una cartella temporanea e rimossa alla chiusura di {{APP}}, così non viene scritto nulla fuori dalla cartella del programma. **Apri indirizzo originale** apre l'indirizzo della pagina, così vedi la pagina com'è oggi; il pulsante è disattivato quando la pagina non ha un indirizzo. Le pagine web salvate da Scrivener (un file *.mht* ciascuna) vengono mostrate allo stesso modo; un browser come Firefox non può aprire da solo i file *.mht*, quindi la copia è HTML semplice con le immagini accanto.

In ogni caso, l'indirizzo della pagina diventa un **segnalibro** del documento (ispettore, Segnalibri), la breve descrizione della pagina diventa la sua **sinossi** e le **note** registrano l'indirizzo, il sito, l'autore, la data di pubblicazione e il momento in cui l'hai scaricata.

**Trascinare un collegamento** da un browser web su una cartella nel raccoglitore, sulla bacheca o nella struttura importa direttamente la pagina, con le scelte fatte l'ultima volta nella finestra.

Da sapere: la pagina viene scaricata come farebbe un browser, ma senza eseguirne gli script. Le pagine che costruiscono il loro testo con uno script spesso lo hanno comunque nei propri dati, che vengono allora usati (senza formattazione e immagini; un messaggio lo dice). Le pagine dietro un login o un paywall restituiscono ciò che mostrano a un visitatore non autenticato. Le pagine sicure (https) richiedono OpenSSL 3 sul tuo sistema (presente in ogni Ubuntu). Un indirizzo web che porta a un file PDF non viene importato in questo modo: scarica il file e usa **File > Importa > File…**.

**Script.** I file Fountain (*.fountain*) e Final Draft (*.fdx*) vengono importati come script: i paragrafi ricevono gli elementi del formato script del tuo progetto, il documento è in modalità script e il frontespizio del file finisce nelle note del documento. Testo in grassetto, corsivo e sottolineato, note e interruzioni di pagina forzate vengono mantenuti; numeri di scena, segni di revisione e l'aspetto a due colonne del dialogo simultaneo no (i due interventi si susseguono). **Importa e dividi…** con **Intestazioni di scena** taglia uno script in un documento per scena, chiamato come la sua intestazione.

### Importare un progetto Scrivener

**File > Importa > Progetto Scrivener…** legge un progetto Scrivener 3 o 2 (la cartella *.scriv*; scegli il file *.scrivx* al suo interno) e ne crea un nuovo progetto {{APP}}. Il progetto di Scrivener viene solo letto, mai modificato. Dalla 0.92.0 il resoconto dopo l'importazione nomina anche ogni file che non è stato possibile copiare (un'immagine, un PDF, un file audio, l'immagine di una scheda, una mappa mentale), e un tale file non viene contato come importato; prima veniva escluso senza una parola.

L'importazione porta con sé: la struttura del raccoglitore, testi, sinossi, note, etichette, stati, parole chiave, metadati personalizzati, tipi di sezione, indicatori di inclusione nella compilazione, istantanee, commenti e note a piè di pagina, annotazioni in linea, file di ricerca (immagini, PDF e altri), raccolte, segnalibri del progetto e obiettivi. Le impostazioni di compilazione non vengono importate; configurale una volta nella finestra di compilazione. Scegli se il nuovo progetto conserva il suo testo come Markdown o RTF.

Titoli e stili: Scrivener contrassegna il testo con stile all'interno dei suoi file con codici come `<$Scr_Ps::0><$Scr_H::2>`. Non sono testo; {{APP}} li toglie. Un titolo riceve da **Titolo 1** a **Titolo 6**, e un paragrafo con uno degli stili di paragrafo di Scrivener riceve lo stile del tuo progetto con lo stesso nome (**Title**, **Block Quote**; il **Centered Text** di Scrivener diventa **Centered**). Dalla 0.81.3 il testo con uno degli stili di carattere di Scrivener riceve lo stile di carattere con quel nome del tuo progetto, e uno stile di Scrivener per cui il tuo progetto non ha uno stile con quel nome diventa uno stile del progetto, con l'aspetto che Scrivener gli dà. I progetti importati da versioni precedenti alla 0.14 mantenevano i codici come testo; {{APP}} li toglie quando un tale progetto viene aperto e lo dice nella barra di stato. I segnalibri del progetto non venivano importati prima della 0.14; per averli, importa di nuovo il progetto Scrivener.

Se qualcosa non è arrivato correttamente, il progetto originale è intatto e puoi copiare il testo direttamente da Scrivener.

### Lavorare in formato Scrivener

Invece di fare una copia, {{APP}} può anche lavorare direttamente su un progetto Scrivener 3, così potrai proseguire più tardi in Scrivener (per esempio su un altro computer).

- **Apertura**: **File > Apri progetto…** (**Apri altro…** nella finestra iniziale) e scegli il file *.scrivx* all'interno della cartella *.scriv*. La prima volta {{APP}} chiede: **Lavora in formato Scrivener** (le tue modifiche vengono salvate nel *.scriv*), **Converti…** (ne viene creato un progetto {{APP}} a sé, come con l'importazione descritta sopra) o **Annulla**. Dopo, il progetto si apre senza la domanda, anche da **File > Apri recenti**.
- **Nuovi progetti**: scegli **Formato: Progetto Scrivener 3 (.scriv)** nella finestra Nuovo progetto.
- **Salvataggio**: ogni salvataggio (anche quello automatico) scrive nel *.scriv*. Viene scritto solo ciò che hai modificato; i documenti che non hai toccato mantengono i file scritti da Scrivener. Il titolo della finestra dice *(formato Scrivener)*, e **File > Mostra il progetto nel gestore file** e i backup usano la cartella *.scriv*.
- **Progetti grandi** (dalla 0.60.5): il salvataggio automatico mentre scrivi mette nel *.scriv* solo i testi, le note e le sinossi che hai modificato (e il raccoglitore, quando è cambiato), così resta breve anche con centinaia di migliaia di parole. Tutto il resto, come l'indice di ricerca di Scrivener, le istantanee e le immagini, segue quando passi a un altro programma, usi **File > Salva** (**Ctrl+S**) o chiudi il progetto; quindi, prima di aprire il progetto in Scrivener, chiudilo qui o passa prima a un altro programma.
- **Un programma alla volta**: se Scrivener ha il progetto aperto (o non è stato chiuso correttamente), {{APP}} lo dice e chiede prima di aprirlo. Chiudi il progetto in un programma prima di aprirlo nell'altro. Quando il progetto è stato modificato in Scrivener (o da un servizio di sincronizzazione) dall'ultima volta che lo avevi aperto qui, {{APP}} lo rilegge e ti dice nella barra di stato quanti documenti sono cambiati; un documento modificato nel frattempo in Scrivener non viene mai sovrascritto con un testo più vecchio.

Che cosa va nel *.scriv*: il raccoglitore, i testi (come RTF, con le loro immagini), sinossi, note (anche di immagini, PDF, audio e video, dalla 0.81.3), etichette, stati, tipi di sezione, **Includi nella compilazione**, segnalibri (del progetto e di ogni documento, anche verso indirizzi web e file), obiettivi del progetto con la scadenza, obiettivi dei documenti, la cronologia di scrittura, la bacheca (la disposizione di ogni cartella e le posizioni delle schede collocate liberamente), titolo, autore e lingua del file compilato (nelle impostazioni di compilazione di Scrivener), istantanee, commenti e note a piè di pagina, annotazioni e note a piè di pagina in linea, stili di paragrafo e di carattere, parole chiave, campi di metadati personalizzati con i loro valori, raccolte, ricerche salvate (raccolte di ricerca, dalla 0.90.0), le immagini delle schede (dalla 0.90.0), immagini, PDF, audio e video, pagine web, icone (con i nomi di Scrivener; le tue icone nella sua cartella *Icons*) e mappe mentali (come file aggiuntivo accanto ai file del documento, che Scrivener lascia stare). Tutto viene scritto nella forma usata da Scrivener 3 per Windows:

- Parole chiave, metadati personalizzati e raccolte mantengono i numeri e i nomi di Scrivener (dalla 0.81.2), quindi sono uguali in entrambi i programmi.
- Stili (dalla 0.81.3): gli stili di un testo che modifichi vi vengono contrassegnati come li contrassegna Scrivener, così Scrivener mostra gli stessi stili. Uno stile creato in {{APP}} viene aggiunto agli stili del progetto Scrivener la prima volta che un testo lo usa, con il suo aspetto.
- Annotazioni e note a piè di pagina in linea (dalla 0.81.3) restano in linea, le annotazioni nel loro colore (prima, un testo che modificavi le passava a Scrivener come commenti e note a piè di pagina collegati).
- Pagine web (dalla 0.81.3): una pagina salvata in {{APP}} entra come pagina web salvata propria di Scrivener (un file *.mht* con le sue immagini) e mantiene il suo indirizzo. Altri file per cui Scrivener non ha un tipo entrano come documenti di testo che nominano il file, con il file accanto; {{APP}} li rilegge com'erano.
- Audio, video e pagine web che le versioni precedenti alla 0.81.2 trasformavano in tali documenti di testo tornano a essere documenti audio, video o pagina web propri di Scrivener con il salvataggio successivo.
- I testi (dalla 0.90.0) vengono riscritti con tutto ciò che Scrivener vi conserva: rientri destri sotto zero, elenchi con la loro numerazione, la sottolineatura dei collegamenti, immagini collegate a un file, caratteri indicati dal loro nome di tipo (come *Cambria-BoldItalic*) e la posizione di un'immagine i cui dati mancano. Commenti e note a piè di pagina mantengono i loro ID, se il loro riquadro è ripiegato e la loro formattazione, finché non li cambi.
- La bacheca (dalla 0.90.0): le schede che collochi liberamente mantengono la loro posizione in entrambi i programmi. Scrivener conserva una posizione in larghezze di scheda; {{APP}} la trasforma nella propria dimensione di scheda, così la disposizione resta la stessa qualunque sia la dimensione con cui ogni programma mostra le schede. Ogni cartella mantiene la sua disposizione (libera o a griglia), come in Scrivener: la scelta di **Libera** o della griglia per la bacheca di una cartella viene ricordata per quella cartella.
- Etichette e stati (dalla 0.90.0): l'etichetta e lo stato predefiniti dei nuovi documenti, e i nomi che Scrivener dà a "Nessuna etichetta" e "Nessuno stato", restano come li ha il progetto.
- Ricerche salvate (dalla 0.90.0): una ricerca salvata di Scrivener è una raccolta di ricerca in {{APP}}, e una raccolta di ricerca che crei o modifichi va nel *.scriv* come ricerca salvata. L'elenco di Scrivener dei risultati di ricerca recenti resta di Scrivener. Verificato con una ricerca salvata per una qualsiasi delle parole; come Scrivener chiami gli altri tipi (tutte le parole, la frase esatta, un'espressione regolare, solo i documenti inclusi o esclusi dalla compilazione) non è ancora stato visto in un progetto reale, quindi una ricerca salvata di quei tipi può risultare in Scrivener come ricerca di tutte le parole. Dove nei documenti cerca una ricerca non fa parte di ciò che passa: una ricerca salvata letta da Scrivener cerca ovunque.
- L'immagine di una scheda (dalla 0.90.0): una scheda che mostra un'immagine invece della sua sinossi la mantiene in entrambi i programmi; toglierla o metterne una in {{APP}} raggiunge Scrivener.

Ciò che resta solo dal lato di {{APP}} (in una copia di lavoro in *~/.local/share/{{APP}}/ScrivenerProjects*): formati di script, impostazioni di compilazione (tranne titolo, autore e lingua), oggetti (sezione 7) e layout della finestra; ciò che il *.scriv* ha già di tutto questo viene lasciato com'è.

La prima volta che un progetto Scrivener viene aperto o importato, il suo raccoglitore mostra le cartelle che erano aperte in Scrivener e i documenti lì selezionati (0.90.0; {{APP}} non scrive mai le impostazioni di finestra di Scrivener). La prima volta che apri un progetto in formato Scrivener con la 0.81.2 o successiva, {{APP}} rilegge il *.scriv* e unisce parole chiave, metadati personalizzati e raccolte della sua copia di lavoro con quelli del progetto, così non si perde nulla di quanto hai creato con una versione più vecchia; il salvataggio successivo li mette nel *.scriv*. La prima apertura con la 0.90.0 o successiva fa lo stesso per segnalibri, obiettivi dei documenti, scadenza, cronologia di scrittura e bacheca.

Se un salvataggio nel *.scriv* è stato interrotto (un arresto anomalo, un disco pieno) dopo che nuovi documenti erano stati scritti nella copia di lavoro, la prossima apertura mantiene quei documenti e lo dice; il salvataggio successivo li mette nel *.scriv*. I documenti eliminati in Scrivener restano eliminati.

**File > Salva con nome…** (**Ctrl+Shift+S**) salva il progetto aperto con un altro nome o in un altro posto, ed è il modo in cui un progetto cambia formato. Nella finestra, digita il nome e scegli il posto; il **tipo di file** decide il formato: *Progetto Scrivener 3 (\*.scriv)* oppure *Progetto {{APP}} (\*.qbx)* (un nome che termina con *.scriv* o *.qbx* decide da solo). Un progetto è una cartella: il programma la crea dal nome (*Nome.scriv*, oppure *Nome.quillbinder* con *Nome.qbx* all'interno). Un nome già esistente viene rifiutato.

- Da quel momento la finestra lavora sul nuovo progetto e ogni salvataggio successivo va lì. Il titolo del progetto è il nome che hai digitato. Le modifiche che non avevi ancora salvato vanno solo nel nuovo progetto.
- L'originale resta sul disco com'era in quel momento (tienilo come copia o eliminalo); è anche in **File > Apri recenti**, sotto il nuovo progetto. Il suo file di blocco viene rimosso quando la finestra lo abbandona, e il nuovo progetto ottiene il proprio.
- Il nuovo progetto è un progetto a sé, con i propri **backup**: inizia una nuova serie, e i backup fatti finora restano con l'originale (sezione 14, Backup). Un progetto in formato Scrivener riceve per questo un nuovo identificatore nel suo *.scrivx*.
- Salva con nome funziona allo stesso modo da un formato allo stesso formato (un nuovo nome o posto) e verso l'altro (il passaggio che il menu "Formato del progetto" faceva fino alla 0.92).

**File > Esporta > Progetto Scrivener…** scrive una **copia** in formato Scrivener, e la voce sotto, nello stesso menu, **Progetto {{APP}}…**, una copia nel formato proprio di {{APP}}; entrambe lasciano la finestra sull'originale: nel progetto su cui lavori non cambia nulla (prima della 0.93.0 erano "Salva una copia come ..." nel menu "Formato del progetto").

### Esportare documenti

**File > Esporta > File…** salva i documenti selezionati per altri programmi. Usa Compila per un manoscritto finito; usa Esporta per consegnare singoli documenti a qualcuno o per tenere una copia fuori da {{APP}}.

1. Nel raccoglitore, seleziona i documenti e le cartelle da esportare.
2. Scegli **File > Esporta > File…**.
3. **Esporta i file di testo come**: Rich Text (.rtf), Word (.docx), Markdown (.md), testo semplice (.txt), Fountain o Final Draft (per gli script, Fountain è preselezionato). Immagini, file PDF, pagine web, audio e video vengono esportati così come sono; il formato conta allora solo per le loro note.
4. **Metadati**: **Esporta metadati** scrive un file di testo accanto a ogni documento, "*nome* (metadata).txt", con titolo, tipo, etichetta, stato, parole chiave, inclusione nella compilazione, tipo di sezione, conteggio delle parole, obiettivo, date di creazione e di modifica, metadati personalizzati e segnalibri. **Esporta sinossi** vi aggiunge la sinossi (oppure scrive "*nome* (synopsis).txt" quando i metadati non vengono esportati).
5. **Opzioni**: **Includi sottodocumenti** (le cartelle diventano cartelle), **Esporta note** ("*nome* (notes)" nel formato del punto 3; testo semplice per gli script), **Esporta commenti e annotazioni in linea** (togli la spunta per consegnare il testo pulito; le parole commentate restano, le note a piè di pagina restano sempre), **Esporta istantanee** (una cartella "*nome* (snapshots)" con un file per istantanea, chiamato con data e titolo).
6. **Posizione**: la cartella in cui salvare. Quando sono selezionati più documenti, vanno in una nuova cartella di cui dai il nome (viene proposto il titolo del progetto; se la cartella esiste, viene aggiunto un numero).
7. Fai clic su **Esporta**. La cartella si apre nel gestore file.

Le scelte vengono ricordate per l'esportazione successiva.

Il modo rapido: **trascina i documenti fuori dal raccoglitore** nel gestore file o sul desktop. I documenti di testo vengono salvati come file RTF (oppure Word, Markdown o testo semplice: **File > Opzioni > Comportamento**), le immagini e gli altri file così come sono, e una cartella diventa una cartella con i suoi documenti dentro. Se il gestore file chiede se spostare o copiare, va bene entrambe: i documenti restano nel tuo progetto.

### Confronta e trasferisci: un file Word o LibreOffice e il progetto

Quando un manoscritto torna da un editor, o hai continuato a lavorarci in Word o LibreOffice, **File > Confronta e trasferisci > Confronta con file…** trova ciò che differisce tra quel file e il tuo progetto e recepisce il lato che scegli.

1. Scegli il file: un file Word (*.docx*) o un file LibreOffice (*.odt*). Anche i vecchi file Word (*.doc*), RTF, testo semplice e Markdown si possono confrontare, ma solo letti: il loro testo può andare nel progetto; il file stesso non può essere modificato.
2. {{APP}} confronta il file con **ciò che Compila ricava dal progetto**, non con l'intero progetto: i documenti del gruppo di compilazione (la Bozza, a meno che nella finestra di compilazione non sia scelto un altro gruppo) che hanno **Includi nella compilazione**, nell'ordine del raccoglitore. Parti iniziali e finali, la cartella Ricerca e i documenti esclusi da Compila non contano. Se meno del 65 % del testo del file vi si ritrova, {{APP}} lo dice e chiede se confrontare comunque (**Confronta comunque**); il file potrebbe appartenere a un altro progetto, oppure Compila lascia fuori ciò che il file ha. Una parola conta come ritrovata quando si trova in una sequenza di quattro parole che ha anche il testo compilato, quindi poche parole cambiate non rendono un file estraneo.
3. La finestra **Confronta e trasferisci** mostra le differenze. Scegli il lato che viene aggiornato:
   - **Aggiorna il progetto dal file**: le modifiche vengono fatte nei tuoi documenti. Prima di ciò, ogni documento modificato riceve un'istantanea, "Prima di Confronta e trasferisci (*nome del file*)" (**Documenti > Istantanee**), così puoi sempre tornare indietro. Il testo che si trova solo nel file diventa un nuovo documento (chiamato come il suo titolo nel file), collocato dopo il documento che lo precede.
   - **Aggiorna il file dal progetto**: il file che hai scelto resta com'è. {{APP}} scrive una copia accanto, "*nome* (Quillbinder).docx" (con un numero quando quel nome è già preso). In un file Word ogni modifica è una **modifica registrata** dell'autore "Quillbinder": apri la copia in Word o LibreOffice e accetta o rifiuta lì ogni modifica (in Word nella scheda Revisione; in LibreOffice con Modifica, Registra modifiche, Gestisci). Una copia LibreOffice (*.odt*) riceve le modifiche direttamente, senza segni.
4. Nella vista, il testo **verde** entra, il testo **rosso** barrato esce e il testo **blu** riceve il grassetto, il corsivo o il sottolineato dell'altro lato. I paragrafi grigi sono testo invariato attorno a una modifica, per mostrare dove si trova. Ogni punto ha un collegamento **recepisci** o **salta**; un clic lo commuta.
5. Fai clic su **Trasferisci ... nel progetto** o **Scrivi ... in una copia del file**. Un messaggio dice che cosa è stato fatto; per il file, **Apri il file** apre la copia.

**Per capitolo** (**File > Confronta e trasferisci > Per capitolo**, anche una casella nella finestra) elenca a sinistra i documenti, ciascuno con una casella e ciò che differisce ("2 punti", "solo nel file", "non nel file"). Togli la spunta a un documento per lasciarlo com'è; un clic mostra a destra le sue modifiche. Senza Per capitolo, tutte le modifiche vengono mostrate una sotto l'altra.

**Passo per passo** (**File > Confronta e trasferisci > Passo per passo**, anche nella finestra) mostra una modifica alla volta: **Recepiscila** o **Saltala**, oppure torna **Indietro**. Alla fine la finestra dice quante modifiche verranno recepite; poi fai clic su Trasferisci. Entrambe le scelte vengono ricordate per la volta successiva.

Che cosa viene confrontato: le parole, e grassetto, corsivo e sottolineato. Ogni paragrafo del file viene abbinato al documento che ha le sue parole, quindi una scena viene trovata anche dove è stata spostata nel file (viene allora contrassegnata "in un altro punto del file"; l'ordine del raccoglitore non cambia). I titoli del file che non sono testo di un documento, come i titoli dei capitoli creati da Compila, non vengono confrontati.

Che cosa è spuntato all'inizio: ogni modifica all'interno di un documento. Il testo che si trova da un solo lato è spuntato quando ha almeno 100 parole (un nuovo capitolo del file diventa un nuovo documento; una scena mancante dal file viene aggiunta alla copia). Un intero documento non viene mai spostato nel Cestino, e un'intera parte non viene mai tolta dal file, a meno che tu non la spunti.

Limiti: i nuovi paragrafi prendono l'aspetto del paragrafo accanto. Il testo nelle tabelle viene confrontato come gli altri paragrafi (una tabella stessa non viene mai tolta) e i campi contano con il testo che mostrano. Intestazioni e piè di pagina, caselle di testo e il testo delle note a piè di pagina e dei commenti del file non vengono confrontati e restano come sono; commenti e note a piè di pagina del progetto restano nel progetto e non vengono scritti nel file. Se il file è stato modificato dopo il confronto, la copia non viene scritta; confronta di nuovo.

### Stampa

**File > Stampa documento corrente…** (**Ctrl+P**) stampa ciò che mostra l'editor corrente:

- **Un testo** (un documento, o più nel testo combinato): il testo con la sua formattazione e le immagini, il titolo di ogni documento sopra il suo testo e le note a piè di pagina in fondo alla pagina. Con la tastiera in un copyholder, viene stampato il documento del copyholder.
- **La bacheca**: le schede nell'ordine mostrato, quante ne stanno su un foglio (con le linee lungo cui tagliare), oppure una scheda per foglio per cartoncino da schede.
- **La struttura**: le righe e le colonne come le mostra la struttura, con i titoli delle colonne in cima a ogni pagina.
- **Un'immagine** (grande quanto consente la pagina), **un file PDF** (le sue pagine) o **una pagina web** (il suo testo).

La finestra di stampa sceglie la stampante, le pagine e il numero di copie. **Selezione** stampa solo il testo selezionato, o solo le schede o le righe della struttura selezionate. Per salvare invece la stampa come file PDF, scegli **Stampa su file (PDF)** come stampante.

**File > Anteprima di stampa…** mostra prima le pagine; il pulsante della stampante dell'anteprima le stampa, **Chiudi** (o **Esc**) chiude l'anteprima.

**File > Impostazioni di stampa…** decide come le cose vanno sulla carta. La finestra si apre sulla scheda di ciò che mostra l'editor; **Anteprima…** mostra il risultato prima che tu faccia clic su **OK**. Le impostazioni appartengono al progetto; spunta **Usa queste impostazioni anche per i nuovi progetti** per renderle lo standard dei nuovi progetti.

- **Pagina**: il formato e l'orientamento della carta (il pulsante **Carta della vista pagina** riprende la carta e i margini di **Vista > Modifica del testo > Dimensione e margini della pagina…**), i margini, e l'intestazione e il piè di pagina, ciascuno con una parte sinistra, una centrale e una destra. Segnaposto: `<$title>` (ciò che viene stampato), `<$projecttitle>`, `<$author>`, `<$date>`, `<$p>` (il numero di pagina) e `<$pages>` (il numero di pagine). Togli la spunta a **Intestazione e piè di pagina anche sulla prima pagina** per una prima pagina pulita.
- **Testo**: ciò che viene stampato con il testo (il titolo di ogni documento, la sinossi sotto di esso, le note del documento dopo il testo, commenti e annotazioni in linea come note in fondo alla pagina, note a piè di pagina alla fine del testo invece che in fondo a ogni pagina, evidenziazioni); i caratteri (come nell'editor, oppure **Un solo carattere** a tua scelta); e l'interlinea (come nell'editor, singola, 1,5 righe o doppia, con spazio per le correzioni). I documenti script mantengono il loro layout da script.
- **Schede**: la dimensione della scheda (3 x 5 in, 4 x 6 in, 5 x 8 in, A7, A6 o una dimensione tua), quante schede per foglio ne stanno oppure una scheda per foglio, ciò che mostra ogni scheda (titolo, sinossi, le prime righe del testo quando non c'è sinossi, immagini, il colore dell'etichetta come striscia in alto, stato, parole chiave, le righe di una scheda, una linea attorno a ogni scheda) e il carattere.
- **Struttura**: le sinossi (come le mostra la struttura, sempre o mai), anche le righe all'interno di gruppi chiusi, i titoli delle colonne in cima a ogni pagina, una linea tra le righe, un punto del colore dell'etichetta e il carattere. Con molte colonne, **Orizzontale** nella scheda **Pagina** dà loro più spazio.

Stampa documento corrente stampa un documento come ci stai lavorando. Per un manoscritto finito con l'aspetto di un formato di compilazione, compilalo (sezione 13) e stampa il file PDF o Word.

---

## 15. Backup

{{APP}} crea un backup (un file ZIP dell'intero progetto) ogni volta che chiudi un progetto, e conserva gli ultimi cinque per progetto. Cambia questa impostazione in **File > Opzioni > Backup**, dove puoi anche scegliere la cartella dei backup (meglio su un altro disco, o in una cartella che il tuo programma di sincronizzazione copia).

Dalla 0.91 i backup di tutti i progetti si trovano direttamente nella cartella dei backup, ciascuno chiamato come il suo progetto con data e ora. Dalla 0.92.0 il nome contiene anche l'**ID** del progetto, cinque caratteri dopo un "#" (*My Novel #K7T2M bak 2026-11-20 15-40-00.zip*). L'ID appartiene al progetto, non al computer: è lo stesso su ogni computer su cui il progetto viene aperto, e resta quando rinomini il progetto o sposti la sua cartella. I backup di un progetto sono i file che portano il suo titolo e il suo ID, e solo questi vengono contati e ripuliti per esso, quindi due progetti con lo stesso titolo mantengono i propri backup e il proprio numero di backup conservati. Una copia o un nuovo progetto che crea il programma stesso è un progetto a sé con un proprio ID: **File > Salva con nome…** e la voce Progetto {{APP}} di **File > Esporta**, un progetto ripristinato con **Ripristina da backup** (riceve anch'esso un titolo proprio) e un progetto creato da un tuo modello. Una copia fatta fuori dal programma (una cartella copiata a mano, un backup decompresso a mano) ha l'ID del suo originale: dai a una tale copia un titolo proprio. Un progetto in formato Scrivener prende il suo ID dall'identificatore che Scrivener conserva nel *.scrivx*. Se la tua cartella dei backup è nel OneDrive di un'organizzazione, il suo amministratore potrebbe aver vietato "#" nei nomi dei file; OneDrive per uso domestico lo permette. I backup fatti da una versione precedente alla 0.92.0 non hanno ID nel nome: la pulizia non li elimina mai, **Ripristina da backup** li elenca ancora e li elimini tu quando non ti servono più. Quando cambi il titolo di un progetto, i backup fatti con il vecchio titolo restano finché non li elimini. Le versioni dalla 0.70 alla 0.90 conservavano i backup di ogni progetto in una cartella a sé all'interno della cartella dei backup (*My Novel 1a2b3c4d*); una tale cartella viene lasciata com'è, lì non viene più aggiunto né eliminato nulla, e **Ripristina da backup** ne elenca ancora i backup. Un backup gira accanto alla finestra: dopo un momento una finestra di avanzamento mostra a che punto è e può annullarlo. Chi ha il progetto aperto (il file di blocco con il nome del computer e del login) non è nel backup.

- Con **Esegui backup a ogni salvataggio manuale** (**File > Opzioni > Backup**), ogni **File > Salva** (**Ctrl+S**) crea anche un backup. Due backup nello stesso secondo ricevono un numero ("... (2)"). Quando quel backup non può essere creato, è incompleto o deve attendere fuori dalla cartella dei backup, un avviso accanto al testo lo dice (0.92.0; prima lo diceva solo la barra di stato, per pochi secondi). Il tuo testo viene comunque salvato. L'avviso compare una volta per causa finché non esci dal programma; la barra di stato lo dice a ogni salvataggio.
- **La riga dei messaggi** sotto il raccoglitore mostra brevi messaggi uno alla volta, per 5 secondi (10 con un collegamento) o finché non fai clic su di essa; gli altri attendono. La usano anche i componenti aggiuntivi (per esempio "Modello scaricato, uso di RAM minimo" quando Assistente di traduzione o Trascrizione vocale hanno liberato il proprio modello dopo 15 minuti senza uso; il tempo è nelle loro opzioni). **Aiuto > Risoluzione problemi e sviluppo > Informazioni di sistema** indica la memoria occupata dal programma.
- **Componenti aggiuntivi non installati.** **Modifica > Strumenti di scrittura** elenca "Assistente di traduzione" e "Trascrizione vocale" finché il componente aggiuntivo manca o è disattivato; ciascuno mostra un avviso con un pulsante che apre **Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi**.
- **File > Backup > Esegui backup ora** crea subito un backup. Dalla 0.94.0 l'esito positivo viene comunicato nella riga dei messaggi sotto il raccoglitore ("Backup salvato." con il collegamento **Mostra cartella**, che apre la cartella del backup; il suggerimento indica il file; un messaggio con un collegamento resta 10 secondi); nessuna finestra attende un OK. Una finestra compare solo quando qualcosa è fallito, o quando il backup è stato creato ma alcuni file non si sono potuti leggere o un backup più vecchio non si è potuto eliminare.
- **Un backup che uno spegnimento o un comando kill ha impedito di creare** viene annotato e creato alla prossima apertura (0.81). Dalla 0.93.0 la nota resta finché quel backup non viene creato (nella cartella dei backup, o in attesa accanto ad essa come descritto sopra): un'apertura il cui backup fallisce lascia la nota per l'apertura successiva. **Un backup più vecchio che la pulizia non riesce a eliminare** (un file bloccato o protetto) resta, e un messaggio nomina il file e il motivo una sola volta; eliminalo a mano. Mentre continui a scrivere, questo messaggio è un piccolo avviso accanto al testo che non richiede tasti.
- **Esegui backup in…** salva un backup dove scegli, per esempio su una chiavetta USB.
- **Ripristina da backup…** (0.70) elenca i backup del progetto aperto con data, dimensione e se sono completi; scegline uno e **Ripristina e apri**: viene decompresso in una nuova cartella accanto al progetto, *… (restored 2026-11-10 14-03)*, il suo titolo dice "(ripristinato …)" e si apre lì. Il progetto che avevi aperto non viene modificato. L'elenco contiene i backup con l'ID di questo progetto, quelli senza ID (fatti prima della 0.92.0) e quelli di un altro progetto con lo stesso titolo; il nome del file nell'elenco indica quale è quale, così nulla resta fuori portata. Dalla 0.93.0 elenca anche i backup che attendono nella cartella di lavoro del programma (fatti mentre la cartella dei backup non era raggiungibile; il backup successivo nella cartella dei backup li mette lì): sono contrassegnati "in attesa (non ancora nella cartella dei backup)" e si ripristinano allo stesso modo. Dalla 0.81 un ripristino di qualsiasi dimensione funziona (backup di 2 GB e più, file di più di 1 GB come i video, più di 65.535 file), ogni file viene controllato mentre viene scritto, un ripristino che non entra nell'unità non viene avviato, una finestra di avanzamento può annullarlo e, quando dei file non si possono ripristinare, il messaggio nomina ciascuno di essi con il motivo (un disco pieno, una voce danneggiata). Dalla 0.93.0 il messaggio ha tre forme: **ripristinato** (tutto è presente), **ripristinato in modo incompleto** (con questa intestazione e un segnale di avvertimento, i file mancanti nominati con il motivo: guarda il progetto ripristinato prima di fare affidamento su di esso) e **non ripristinato**; e quando il titolo del progetto ripristinato non ha potuto essere contrassegnato "(ripristinato …)" il messaggio lo dice.
- **Sposta i vecchi backup nella cartella dei backup…** (0.93) sposta i backup che le versioni dalla 0.70 alla 0.90 conservavano in una cartella a sé all'interno della cartella dei backup (*My Novel 1a2b3c4d*) nella cartella dei backup stessa, con i loro nomi, così che stiano insieme agli altri. È attivo solo finché il progetto aperto ha una tale cartella con dei backup. Chiede prima e dice quanti ce ne sono; un backup il cui nome è già nella cartella dei backup non viene mai sovrascritto e resta dov'era; il resoconto dice che cosa è stato spostato e che cosa è stato lasciato (con il motivo), e la vecchia cartella viene rimossa quando non resta più nulla al suo interno. Questi backup non hanno ID nel nome, quindi la pulizia non li conta né li elimina mai, come prima; **Ripristina da backup** li elenca in entrambi i posti.
- **Mostra cartella dei backup** apre la cartella dei backup.

**Dove si assembla un backup (dalla 0.90.3).** Un backup viene impacchettato e controllato in una cartella propria del programma, *~/.local/share/{{APP}}/backup-work* (su Windows *data\backup-work* nella cartella del programma), e solo il file finito viene messo nella cartella dei backup, in un solo passaggio e con il suo nome definitivo: spostato lì quando le due cartelle sono sullo stesso disco, altrimenti copiato direttamente con il suo nome definitivo e confrontato con l'originale. Così una cartella dei backup osservata da un programma di sincronizzazione, o che si trova su una chiavetta USB o su un'unità di rete, non contiene mai un backup scritto a metà o un file temporaneo, e un backup interrotto non somiglia mai a uno finito. (Prima della 0.90.3 il backup veniva scritto all'interno della cartella dei backup come *… .zip.part* e rinominato lì.) La cartella di lavoro è vuota tra un backup e l'altro, a parte una piccola nota della cartella dei backup che serve.

**Quando la cartella dei backup non è raggiungibile** (un'unità non collegata, una cartella che non si può creare o scrivere, nessuno spazio), il backup viene comunque creato e attende nella cartella di lavoro. Un messaggio lo dice e nomina entrambe le cartelle, e il backup successivo del progetto mette a posto i backup in attesa prima del proprio. Ne attendono tanti quanti sono i backup conservati. **Esegui backup in…** in una cartella non scrivibile si limita a dirlo con il motivo; scegli un'altra cartella.

**Copie fatte da un programma di sincronizzazione.** Solo i file chiamati come {{APP}} chiama i suoi backup (*Title #K7T2M bak 2026-11-20 07-56-13.zip*, con "(2)" o "(incomplete)" dove serve) contano come backup e possono essere eliminati dalla pulizia. Gli altri file nella cartella dei backup di un progetto vengono lasciati stare e non contano: per esempio le copie che un programma di sincronizzazione fa quando ha problemi con un file (*… -MyComputer-safeBackup-0001.zip*, *… (conflicted copy).zip*), o una copia che hai fatto a mano. (Prima della 0.90.3 tali copie contavano come backup della loro data, e diverse di esse potevano spingere fuori i veri backup più vecchi.) **Ripristina da backup…** le elenca ancora, così puoi ripristinare da una copia; elimina tu le copie che non ti servono.

Anche i progetti grandi (più di 4 GB, per esempio con audio e video) vengono salvati correttamente (dalla versione 0.20). Un file che non si può leggere (per esempio a causa dei suoi permessi, o un file di una cartella sincronizzata che non è ancora su questo computer) viene escluso; il backup viene comunque creato, ma si chiama *… (incomplete).zip*, un messaggio nomina i file mancanti (anche per i backup automatici all'apertura o alla chiusura di un progetto, 0.65) e per questo non viene eliminato alcun backup più vecchio: il numero di backup da conservare conta solo i backup completi. Quali backup siano i più vecchi lo dicono la data e l'ora nei loro nomi, quindi rinominare il progetto non cambia nulla su quali vengano conservati (0.81; prima, un nuovo titolo che precedeva quello vecchio nell'ordinamento faceva contare ogni nuovo backup come il più vecchio, ed esso veniva eliminato subito). Dopo la pulizia il programma controlla che il nuovo backup ci sia ancora. Tra i backup incompleti, restano i più recenti fino al numero da conservare (0.81). Una cartella nel progetto che è un collegamento a una cartella altrove viene salvata con i suoi file (0.81); un collegamento che non punta da nessuna parte, o che torna dentro il progetto, rende il backup incompleto. Se il programma viene terminato da un segnale (uno spegnimento che non attende, un comando kill) mentre **Crea un backup alla chiusura di un progetto** è attivo, non c'è tempo per il backup; alla successiva apertura del progetto il backup viene creato e la barra di stato lo dice (0.81). Il progetto viene salvato prima di ogni backup; se quel salvataggio fallisce (un disco pieno, una cartella non scrivibile), non viene creato alcun backup e un messaggio ne dice il motivo, così a un backup non mancano mai le tue ultime modifiche a tua insaputa. Il backup di un progetto in formato Scrivener contiene il *.scriv* e, accanto, la copia di lavoro di {{APP}} (*… - working copy of {{APP}}*), con le parole chiave, gli stili, gli oggetti e le altre cose che solo {{APP}} conosce.

**Per ripristinare un backup a mano** (per esempio su un altro computer): apri la cartella dei backup, decomprimi il file ZIP (clic destro > Estrai) e apri il progetto al suo interno con **File > Apri progetto**.

---

## 16. Opzioni

**File > Opzioni** (**Ctrl+,**) ha in alto una fila di icone per le sezioni, a sinistra un elenco di pagine (o schede) e a destra le opzioni. Non cambia nulla finché non fai clic su **OK** o **Applica**. In basso a sinistra, **Gestione** esporta il profilo in uso in un file e ne importa uno (**Esporta profilo...**, **Importa profilo...**, per esempio per portare le tue impostazioni su un altro computer; anche un vecchio file di opzioni (*.qboptions*) può essere importato come profilo; vedi "Profili" più sotto), apre **Gestisci profili...**, salva l'aspetto corrente come file di tema (un file già presente viene sostituito solo quando il nuovo è stato scritto completamente; se la scrittura fallisce, il vecchio file resta com'era e il messaggio lo dice) ed elenca i temi; **Predefiniti** riporta ogni opzione allo standard (i tuoi progetti recenti, i tuoi temi, layout, barre degli strumenti e scorciatoie restano).

**Dove viene conservata ogni opzione.** Passa con il puntatore su un'opzione per vedere dove è conservato il suo valore: il suggerimento dice se lo conserva il progetto aperto o il programma per tutti i progetti, e se quel posto è **fisso** (un'impostazione che solo il programma può contenere, come la lingua) o **dipende dall'impostazione** **Impostazioni per** in alto (e, nella pagina Aspetto, **Mostra e cambia l'aspetto di**). (La 0.90.1 mostrava una P verde o una A blu davanti a ogni opzione; la 0.90.2 lascia questo compito al suggerimento.) Su uno schermo piccolo le pagine scorrono, così OK, Annulla e Applica restano in vista.

Dalla 0.70 la finestra si apre su **Tutti i progetti** (le impostazioni del programma); scegli **Questo progetto** in alto per impostare qualcosa solo per il progetto aperto. Le impostazioni proprie di un progetto possono rendere il salvataggio più sicuro ma mai meno sicuro: vale il ritardo di salvataggio automatico più breve, un backup che il programma crea resta attivo, viene conservato il numero maggiore di backup e l'uscita automatica è solo del programma. L'ultimo salvataggio avviene al massimo 10 secondi dopo una modifica, qualunque sia il ritardo del salvataggio automatico.

- **Generale**: **Avvio** (riaprire l'ultimo progetto, le finestre di dialogo dei file del desktop, la **Lingua** del programma, dal prossimo avvio: English, Deutsch, Español, Français, Português (Brasil), Italiano, Русский, Bahasa Indonesia, Türkçe, Polski o Nederlands (le nove nuove sono traduzioni automatiche, contrassegnate come tali nell'elenco finché un madrelingua non le ha controllate; il manuale, la guida rapida e il progetto di benvenuto seguono la lingua, in inglese dove una lingua non li ha; al primo avvio viene presa la lingua del sistema quando è una di queste, e la pagina del primo avvio offre lo stesso elenco); **Raggruppa le cifre dei numeri grandi**: conteggi delle parole, obiettivi e statistiche con un separatore tra le migliaia, come il tuo sistema scrive i numeri (44,825 o 44.825), oppure 44825 se la spunta è tolta); **Salvataggio** (intervallo di salvataggio automatico, il numero di progetti recenti, istantanee a ogni File > Salva); **Informazioni sull'autore** (nome, indirizzo, telefono ed e-mail per i frontespizi, usati dai segnaposto `<$author>`, `<$street>`, `<$city>`, `<$state>`, `<$zip>`, `<$country>`, `<$phone>`, `<$email>`, `<$address>`); le opzioni degli strumenti di scrittura (**Blocco appunti**); **Modelli condivisi** (un'ulteriore cartella di modelli di progetto, per esempio una sincronizzata tra i tuoi computer; i suoi modelli compaiono in File > Nuovo progetto); **Citazioni** (il programma che **Formato > Bibliografia/Citazioni...** avvia, per esempio Zotero o JabRef); **Separatori** (che cosa va tra i documenti uniti e tra le note del blocco appunti aggiunte a un documento: un solo a capo, una riga vuota, un'interruzione di pagina, niente o un testo tuo); **Uscita automatica** (salva e chiude dopo un certo numero di minuti senza input da tastiera o mouse); **Avvisi** (**Mostra console del registro interno** mostra i messaggi del programma di questa sessione, **Ripristina tutti gli avvisi** rifà le domande a cui si era risposto con "Non chiedere più").
- **Modifica**: **Opzioni** (ricaricare l'ultimo zoom degli editor, lo zoom predefinito del testo di ogni area, centimetri o pollici e il passo del righello, il tasto Insert per la sovrascrittura, un formato di data tuo per Inserisci > Data corrente, che cosa mostra il conteggio in tempo reale, annotazioni in linea senza colore, commenti e note a piè di pagina che si fermano prima della punteggiatura, un punto di inserimento a blocco); **Formattazione** (la formattazione e gli stili dei nuovi progetti); **Revisioni** (i cinque colori di Formato > Modalità revisione, dalla tavolozza condivisa).
- **Comportamento**: **Modalità composizione** (schermo, finestra principale fuori mezzo, Esc); **Collegamenti ai documenti** (dove si aprono i collegamenti ai documenti, collegamenti inversi); **Doppio clic** (un doppio clic sulla bacheca crea una scheda); **Trascinamento** (documenti rilasciati su documenti di testo, il formato del testo trascinato in altri programmi); **Cartelle e file** (le cartelle si aprono come bacheca; un testo con sottodocumenti si apre come una cartella); **Riproduzione** di audio e video (sezione 6); **Riferimento rapido** (i pannelli si riaprono con il loro progetto); **Tasto Return** (Return termina la modifica di una sinossi; Return crea una nuova scheda o riga); **Istantanee** (un messaggio quando vengono create istantanee).
- **Aspetto**: il tema e undici parti della finestra (Interfaccia generale, Raccoglitore, Modalità composizione, Bacheca, Schede, Ispettore e note, Editor principale, Struttura, Testo combinato, Barre di avanzamento degli obiettivi, Segni testuali), ciascuna con le sue opzioni, i suoi caratteri e i suoi colori, e **Colori standard** (dalla 0.95: le sei etichette con cui partono i nuovi progetti, i sei colori della tavolozza del colore del testo e di quella dell'evidenziazione e i colori con cui partono i due pulsanti; **Predefiniti** ripristina lo standard del programma; i progetti esistenti conservano le proprie etichette). Vedi "Temi, colori e caratteri" qui sotto. **Interfaccia generale** ha anche il percorso completo del progetto nella barra del titolo e la dimensione delle icone delle barre degli strumenti.
- **Correzioni**: correzioni durante la digitazione (sezione 4) e **Ortografia** (il dizionario, parole con numeri, la sottolineatura, l'elenco personale di parole).
- **Tastiera**: la scorciatoia di ogni comando (sezione 19).
- **Condivisione**: **Importa** (commenti, note a piè di pagina, stili, la codifica del testo semplice, file HTML; sezione 14), **Esporta** (il formato di file e le note di File > Esporta > File), **Conversione** (come vengono letti i file *.doc*).
- **Backup**: quando eseguire il backup (all'apertura, alla chiusura, a ogni File > Salva), quanti backup conservare (una nuova installazione parte con gli ultimi 25; il campo contiene solo il numero, le parole gli stanno intorno: "Conserva gli ultimi [25] backup di ogni progetto") e dove.
- **Extra**, per le aggiunte proprie di {{APP}}: **Mappa mentale** (le opzioni delle lavagne, la forma e l'aspetto delle nuove note, lo sfondo delle lavagne senza un colore proprio nei temi chiari e scuri, la dimensione delle icone su note e linee), **Oggetti** (se i nuovi progetti ricevono i tuoi modelli di oggetto o quelli standard), **Componenti aggiuntivi e plugin** (i formati di compilazione e gli strumenti di scrittura installati, le cartelle dei plugin), **Rapporti di arresto anomalo** (dopo quanti secondi senza risposta viene scritto un rapporto di blocco, quanti rapporti vengono conservati).
- **Voce** (con il componente aggiuntivo Trascrizione vocale): il modello, il dispositivo, le lingue, il microfono e le pause (vedi sezione 10, *Trascrizione vocale*). Anche gli strumenti di scrittura di componenti aggiuntivi successivi possono portare una sezione propria.

Altre finestre del programma possono aprire le opzioni su una pagina, per esempio la pagina Riproduzione dal lettore multimediale.

Aggiornamenti e componenti aggiuntivi hanno finestre proprie: **Aiuto > Componenti aggiuntivi e aggiornamenti > Installa aggiornamento o componente aggiuntivo…** e **Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi…** (sezione 17).

Le impostazioni specifiche del progetto (etichette, stato, tipi di sezione, l'aspetto del testo, metadati personalizzati, cartelle speciali, titolo e autore per la compilazione) si trovano in **Progetto > Impostazioni del progetto**.

### Impostazioni per un progetto

In cima a **File > Opzioni**, **Impostazioni per:** sceglie dove viene conservato ciò che cambi:

- **Tutti i progetti (il programma)**: le impostazioni del programma, per ogni progetto. È qui che si apre la finestra.
- **Questo progetto**: il progetto aperto conserva ciò che cambi qui come impostazione propria (in *settings/options.ini* della sua cartella). Vale finché quel progetto è aperto; ciò che il progetto non imposta proviene dalle impostazioni del programma. Il titolo della finestra dice "Opzioni: questo progetto", e la riga sotto la scelta dice quante impostazioni proprie ha il progetto (fai clic sul numero per vederle). **Usa le impostazioni del programma** le rimuove tutte; le impostazioni del programma si applicano di nuovo al progetto. Una modifica fatta fuori dalle opzioni, per esempio una scorciatoia cambiata nella finestra della tastiera o una barra degli strumenti disposta, va al progetto quando il progetto ha già un proprio valore per essa, e al programma altrimenti.

Alcune impostazioni appartengono solo al programma e non vengono mostrate per un progetto: i progetti recenti, la riapertura dell'ultimo progetto, le finestre di dialogo dei file del desktop, le posizioni delle finestre e i layout, i tempi dei rapporti di arresto anomalo e l'aspetto per i nuovi progetti. L'aspetto di un progetto è sempre stato suo: la pagina **Aspetto** mostra l'aspetto del progetto aperto o l'aspetto per i nuovi progetti (**Mostra e cambia l'aspetto di** in alto); con **Questo progetto** scelto sopra, mostra solo l'aspetto del progetto. **Predefiniti** e **Gestione > Importa profilo…** lavorano solo sulle impostazioni del programma, quindi scegli **Tutti i progetti** per essi.

**Quali impostazioni hanno la precedenza** (**Generale > Avvio**, per tutti i progetti): con la spunta (l'impostazione standard), le impostazioni proprie di un progetto hanno la precedenza e il programma completa il resto. Senza la spunta, valgono sempre le impostazioni del programma: le impostazioni proprie dei progetti non vengono usate (restano nei progetti per dopo) e ogni progetto mostra l'aspetto per i nuovi progetti.

**Progetto > Salva impostazioni del progetto su file…** include le opzioni proprie del progetto (la sezione **Opzioni di questo progetto**), così possono essere portate in un altro progetto.

### Temi, colori e caratteri

Un tema imposta i colori e, se lo desidera, i caratteri di ogni area della finestra. Sceglierne uno in **Vista > Temi**, o con il pulsante del tema della barra Temi (attiva la barra in **Vista > Barre degli strumenti**); la modifica ha effetto subito, senza riavvio. Nove temi sono integrati:

| Tema | Aspetto |
|-------|------|
| Shelin's Dark Theme | finestra e raccoglitore scuri, una carta calda con testo nero, Cinzel per i titoli del testo combinato (dalla 0.95; sta in cima all'elenco e può essere eliminato in Gestisci temi; poi non torna con gli aggiornamenti) |
| Default (Follow the Desktop) | i colori del tuo desktop (la combinazione di colori di KDE) |
| Light | finestra grigio chiaro, carta bianca |
| Dark Mode | grigio scuro, con schede scure |
| Sepia | crema caldo e marrone |
| Slate | finestra grigia, carta bianca su una scrivania grigia |
| Midnight | blu scuro con un accento ambra |
| Ocean | blu-verde chiaro |
| High Contrast | bianco e giallo su nero |

I componenti aggiuntivi possono portare altri temi (il componente aggiuntivo "Extra themes" ha Northern Night, Forest, Graphite, Warm Paper e Rose Dawn).

**Il pulsante del tema** (0.90.2, modificato nella 0.91 e nella 0.94). Un solo pulsante, **Tema**, nella barra Temi, che all'inizio è disattivata (**Vista > Barre degli strumenti > Temi** la mostra; prima della 0.94 quattro pulsanti e un elenco occupavano un terzo della barra principale). Il suo menu elenca:

- **Light** e **Dark**: un clic mostra subito quel tema, così com'è.
- **My Look 1** e **My Look 2** conservano due aspetti tuoi. Scegliere uno vuoto (nel menu c'è scritto *empty*) vi salva l'aspetto che vedi ora (il tema con sopra ogni colore e carattere che hai cambiato); scegliere uno pieno ripristina il suo aspetto. **Salva l'aspetto corrente come My Look 1** e **...come My Look 2**, in fondo al menu, vi salvano l'aspetto che vedi ora (chiedono conferma prima di sostituire un aspetto). I due aspetti sono temi normali: sono nell'elenco qui sotto e in **Vista > Temi**, dove si possono anche esportare o eliminare (uno eliminato lascia vuota la sua voce).
- **Tutti i temi**, sotto, con quello in uso spuntato. Sceglierne uno fa ciò che fa **Vista > Temi**: lo usano il progetto aperto e i nuovi progetti, e la domanda se mantenere le tue modifiche sopra di esso arriva come prima.
- **Il tuo aspetto non si perde mai con una scelta.** Il menu mostra un tema così com'è, quindi ciò che vedi cambia davvero. Se colori o caratteri tuoi si trovano sopra il tema mostrato, andrebbero persi, quindi vengono salvati prima: in un My Look vuoto senza domanda (la barra di stato dice dove), e quando entrambi contengono un aspetto, {{APP}} chiede se salvare in My Look 1, in My Look 2 o passare senza salvare.

Come ogni controllo delle barre degli strumenti, il pulsante del tema si può spostare o togliere in **Vista > Barre degli strumenti > Personalizza barre degli strumenti**, e l'intera barra Temi si può attivare o disattivare in **Vista > Barre degli strumenti**.

**La pagina nei temi scuri** (0.70). La pagina standard di {{APP}} è una carta color oliva chiaro accanto a una scrivania grigia. Con un tema scuro, la pagina è invece un grigio molto scuro con testo bianco (oppure la pagina scura propria del tema, se ne ha una), così un tema scuro non lascia una pagina luminosa in una cornice scura. Un colore di pagina scelto da te (clic destro > Aspetto, oppure File > Opzioni > Aspetto) resta in ogni tema.

**Cambiare colori e caratteri.** **File > Opzioni > Aspetto** (oppure **Vista > Temi > Colori e caratteri…**) elenca a sinistra le parti della finestra. Ognuna mostra le sue opzioni, i suoi caratteri e i suoi colori (un clic su un colore mostra la tavolozza condivisa: i colori delle etichette, i sei colori standard e **Altro...**):

| Parte | Caratteri | Colori |
|------|-------|--------|
| Interfaccia generale | menu, pulsanti e pannelli | finestra, Sfondo dell'ispettore (gli elenchi e i campi di testo dell'ispettore), righe alternate, testo, pulsanti, selezione, collegamenti, suggerimenti, bordi, testo in grigio |
| Raccoglitore | raccoglitore | sfondo, titoli |
| Modalità composizione | (nessuno) | sfondo, carta, testo |
| Bacheca | (nessuno) | bacheca, timbro di stato |
| Schede | titoli delle schede, testo delle schede | scheda, titolo, testo, linea sotto il titolo, righe |
| Ispettore e note | sinossi | Strisce dell'ispettore (la riga delle schede, i titoli, la riga Etichetta e Stato), etichette, scheda e testo della sinossi, note e il loro testo |
| Editor principale | titolo della barra d'intestazione | carta, testo, lo spazio accanto alla colonna di testo, la scrivania attorno alle pagine (mostrata solo nella vista pagina), la barra d'intestazione (normale, editor attivo, bloccato in posizione) |
| Struttura | struttura | sfondo, righe alternate, testo |
| Testo combinato | titoli nel testo combinato | (nessuno) |
| Barre di avanzamento degli obiettivi | (nessuno) | sessione, bozza, obiettivo raggiunto |
| Segni testuali | (nessuno) | collegamenti nel testo, risultati di ricerca, sottolineatura ortografica, i segni di Mostra caratteri invisibili |

**Ogni progetto conserva il proprio aspetto.** Il tema del raccoglitore, dell'editor, della bacheca, della struttura, dell'ispettore e della modalità composizione, i tuoi colori e caratteri per essi, la larghezza della colonna di testo e il bordo accanto al testo appartengono al progetto: cambiarli in un progetto lascia gli altri come sono, e un aggiornamento di {{APP}} non cambia mai l'aspetto di un progetto esistente. L'elenco **Mostra e cambia l'aspetto di** in cima a **File > Opzioni > Aspetto** sceglie di chi la pagina mostra e cambia l'aspetto: **Questo progetto** (quello aperto; scelto per primo) o **Nuovi progetti** (l'aspetto con cui parte un nuovo progetto). Passare dall'uno all'altro mantiene ciò che hai cambiato in ciascuno finché non premi **Applica** o **OK**. I colori di menu e pannelli (**Interfaccia generale**) e il carattere di menu, pulsanti e pannelli sono uguali in ogni progetto. Un progetto aperto per la prima volta dalla versione 0.40.2 o successiva mantiene l'aspetto che aveva fino ad allora. **Vista > Temi** imposta il tema del progetto aperto e dei nuovi progetti insieme.

**Menu del clic destro.** Ovunque tu faccia clic destro, il menu ha un sottomenu **Aspetto** con i colori e i caratteri di ciò che si trova sotto il mouse: sul testo e sulla carta accanto, la carta, il testo, lo spazio accanto alla colonna, i collegamenti, i risultati di ricerca, la sottolineatura ortografica, la larghezza della colonna e il bordo; sulla barra d'intestazione sopra un editor i suoi colori e il suo carattere; nel raccoglitore il suo sfondo, i titoli, il carattere, il rientro e i colori delle etichette; sulla bacheca il sughero, le schede e i loro caratteri, i timbri di stato, la larghezza delle schede; nella struttura il suo sfondo, le righe, il testo e il carattere; nell'ispettore e nelle sue note le strisce, le etichette, la scheda della sinossi e le note, e lo Sfondo dell'ispettore, che è uguale in ogni progetto; nella modalità composizione il suo sfondo, la carta e il testo. Un colore mostra la tavolozza condivisa e **Il colore del tema**; **Opzioni di aspetto...** apre questa pagina delle opzioni. Come le opzioni, questi menu cambiano l'aspetto del progetto aperto (la prima riga del sottomenu lo dice) oppure, senza un progetto, l'aspetto dei nuovi progetti.

Fai clic su un colore per cambiarlo, su un pulsante del carattere per sceglierne uno. La piccola immagine in alto mostra il risultato prima che tu prema **Applica** o **OK**. Le tue modifiche stanno sopra il tema: una voce cambiata è mostrata in grassetto, il pulsante **↺** accanto la restituisce al tema e **Ripristina colori e caratteri del tema** lo fa per tutte. Un colore che non hai cambiato e che il tema non imposta segue gli altri; per esempio, il raccoglitore usa il colore degli elenchi finché non gliene dai uno proprio. Lo **Sfondo dell'ispettore** colora solo gli elenchi e i campi di testo dell'ispettore; i campi nelle barre degli strumenti, il righello e gli elenchi delle opzioni hanno il colore della finestra e dei menu.

Il carattere e la dimensione del testo che scrivi non fanno parte del tema: appartengono al progetto (**Progetto > Impostazioni del progetto > Formattazione**, sezione 4).

**Temi tuoi.** **Salva come tema…** nelle opzioni (oppure **Vista > Temi > Salva l'aspetto corrente come tema…**) memorizza il tema insieme alle tue modifiche con un nuovo nome. Compare poi in **Vista > Temi** come gli altri. **Vista > Temi > Gestisci temi…** elenca tutti i temi: **Usa questo tema**, **Duplica…** (anche per i temi integrati, per ottenere un punto di partenza tuo), **Rinomina…** ed **Elimina** (solo i tuoi temi), **Esporta…** (scrive un file *.qbtheme* che puoi passare ad altri) e **Importa…** (aggiunge un tale file).
Quando scegli un altro tema mentre hai modifiche tue, {{APP}} chiede se mantenerle con il nuovo tema o se usare il tema così com'è.

I collegamenti nel tuo testo sono memorizzati con un unico blu; su carta scura vengono mostrati in un blu più chiaro (**Editor principale > Collegamenti nel testo**). Il testo che hai colorato tu mantiene il suo colore in ogni tema.

### Profili

Un tema è solo un aspetto: colori e caratteri. Un **profilo** (dalla 0.95) conserva tutto il resto delle impostazioni del programma: la lingua, tutte le opzioni, le barre degli strumenti e le loro posizioni, la dimensione e la posizione della finestra, i pannelli mostrati, i divisori, il set di tastiera e le tue scorciatoie, i backup e la cartella dei backup, i controlli degli aggiornamenti, i colori standard, i layout salvati di **Vista > Layout**, e nomina il suo tema, così cambiare profilo cambia anche l'aspetto. Il tuo nome e indirizzo, il nome di questo computer, i progetti recenti e le posizioni del progetto di benvenuto non fanno parte di un profilo.

- **Cambiare.** **Vista > Profili** elenca i profili; un clic cambia subito, senza riavvio (la lingua è l'unica cosa che richiede un riavvio; il programma lo dice). Il profilo che lasci viene prima salvato. La barra degli strumenti **Profili** (**Vista > Barre degli strumenti > Profili**, inizialmente disattivata) mostra il profilo in uso; un clic sul suo pulsante elenca i profili. Le impostazioni proprie di un progetto (**Impostazioni per: Questo progetto**) restano comunque sopra il profilo. Il progetto aperto mostra l'aspetto del nuovo profilo, come quando scegli un tema in **Vista > Temi**.
- **Default.** Un programma già usato prende le sue impostazioni come profilo **Default** quando viene aggiornato alla 0.95, così nulla va perso. Una nuova installazione parte con le impostazioni standard del programma, nel tema chiaro o scuro a seconda di come è impostato il tuo sistema, dove il sistema lo dice. La pagina del primo avvio compila **Default**. **Default** non può essere eliminato.
- **Un nuovo profilo** dalle impostazioni com'è ora: **Vista > Profili > Salva le impostazioni come nuovo profilo…** (oppure **Gestisci profili… > Salva le impostazioni come nuovo profilo…**). Il nuovo profilo diventa quello in uso. **Gestisci profili…** rinomina, copia ed elimina anche i profili (il profilo in uso non può essere eliminato; cambia prima).
- **Shelin's** è un profilo fornito con il programma. Sta in cima all'elenco, imposta Sitka per il nuovo testo e un tema proprio, e può essere eliminato; poi non torna con gli aggiornamenti. I caratteri che nomina e che non sono installati (Cinzel, Sitka) ripiegano su altri. La sua scelta del motore di riproduzione richiede il componente aggiuntivo Extra Video and Audio Formats; le scelte dei modelli degli strumenti di scrittura valgono solo dove quei componenti aggiuntivi e quei modelli sono installati.
- **Dimensioni su un altro schermo.** Le larghezze del raccoglitore, dell'editor e dell'ispettore vengono conservate come quote della finestra; un profilo creato su uno schermo grande mostra le stesse quote su uno schermo più piccolo o su un tablet (le colonne laterali mantengono una larghezza leggibile), e la finestra viene riportata dentro lo schermo.
- **Esportare e importare.** **Gestisci profili… > Esporta…** (oppure **File > Opzioni > Gestione > Esporta profilo…**) scrive il profilo in un unico file (*.qbprofile*) con i suoi layout e, se è uno dei tuoi, il suo tema. **Importa…** aggiunge un tale file come nuovo profilo (non si passa ad esso). Un file può nominare programmi, cartelle, file o indirizzi; come per gli altri file che vengono da qualcun altro, ti viene chiesto prima che vengano accettati (una cartella che non esiste su questo computer non servirebbe a nulla). Un profilo che non è stato salvato da questo programma su questo computer (un file copiato o sincronizzato nella cartella dei profili) chiede allo stesso modo dei suoi programmi, cartelle, file e indirizzi quando passi ad esso. Anche un vecchio file di opzioni (*.qboptions*) può essere importato come profilo.

**Progetto > Salva impostazioni del progetto su file…** resta com'era.

---

## 17. Aggiornamenti e componenti aggiuntivi

**Aiuto > Risoluzione problemi e sviluppo > Guida per sviluppatori: aggiornamenti e componenti aggiuntivi** spiega, per chi costruisce {{APP}} o scrive componenti aggiuntivi, come funzionano aggiornamenti e componenti aggiuntivi e con quali strumenti si creano; contiene anche ciò che serve a un amico con competenze tecniche per configurare le cose (motori multimediali, plugin dei formati di compilazione, file di pacchetti, temi, layout e formati di script, i file all'interno della cartella di un progetto) e le variabili d'ambiente lette dal programma.

{{APP}} è un unico file di programma (l'AppImage). Aggiornamenti ed estensioni arrivano come piccoli file di pacchetto con estensione **.qbpkg**. Ce ne sono tre tipi, e tutti si installano allo stesso modo:

- **Aggiornamento del programma**: una versione più recente di {{APP}} stesso.
- **Componente aggiuntivo**: un'estensione che puoi attivare e disattivare, come temi di colori, modelli di progetto, formati di compilazione, dizionari, strumenti di scrittura (sezione 10), la riproduzione di audio e video e i suoi formati extra (sezione 6), importatori per vecchi file Word e pagine web (sezione 14).

Aggiornamenti del programma e componenti aggiuntivi si trovano nella pagina **Releases** della pagina del progetto (sezione 1), accanto all'AppImage.
- **Pacchetto di aggiornamento**: un unico file che contiene un aggiornamento del programma e diversi componenti aggiuntivi, installati insieme.

### Cercare aggiornamenti

Dalla 0.92.0 {{APP}} può cercare da solo gli aggiornamenti. È l'unica cosa per cui il programma contatta internet di sua iniziativa, e puoi disattivarla.

- **Aiuto > Componenti aggiuntivi e aggiornamenti > Cerca aggiornamenti all'avvio** è l'interruttore (all'inizio attivo). **Aiuto > Componenti aggiuntivi e aggiornamenti > Cerca aggiornamenti ora** cerca subito, qualunque cosa dica l'interruttore.
- **Il primo avvio** lo dice nella sua pagina, prima che venga contattato qualcosa: una casella *Cerca aggiornamenti all'avvio del programma* (spuntata) con la nota che non viene inviato nulla su di te; un solo **OK** chiude la pagina. Un programma che è stato avviato prima della 0.94.0 e non ha visto la nota la mostra come una finestra con **OK** e **Disattiva**.
- **Che cosa succede**: pochi secondi dopo l'avvio, al massimo una volta al giorno, il programma scarica un piccolo file dalla pagina del progetto (*shelin-boop.github.io/Quillbinder_Collection/updates-1.json*) e la firma di quel file. La richiesta nomina il programma ("{{APP}}") e nient'altro: nessuna versione, nessun sistema, nessun identificatore, nessun cookie. Come in ogni connessione, il server (GitHub) vede il tuo indirizzo internet e l'ora. Il file elenca la versione più recente del programma e dei componenti aggiuntivi del progetto; il tuo programma la confronta con ciò che è installato, sul tuo computer. Il file conta solo se porta la firma della chiave di rilascio del progetto, e un file più vecchio di uno già visto viene ignorato.
- **Quando c'è un aggiornamento**, una finestra lo nomina: il programma o il componente aggiuntivo, la tua versione e la nuova, le novità e il suo tipo. Un aggiornamento **funzionale** ha tre pulsanti: **Scarica e installa**, **Ignora questa versione** (nessun altro promemoria per quella versione) e **Rimanda**. Rimanda attende più a lungo ogni volta: 1 giorno, 1 settimana, 2 settimane, 4 settimane; poi c'è un ultimo promemoria, e dopo di esso i promemoria per quella versione finiscono. Un aggiornamento **consigliato (sicurezza)** ha solo due pulsanti, **Scarica e installa** e **Rimanda**: non si può ignorare, e dopo il quarto rinvio torna ogni 4 settimane. Quando ci sono entrambi i tipi, una sola finestra li elenca e segue le regole della sicurezza. La finestra compare accanto al tuo lavoro e non prende la tastiera.
- **Scarica e installa** scarica i file che l'elenco nomina, controlla ciascuno rispetto alla dimensione e al checksum nell'elenco firmato e lo installa: un componente aggiuntivo o un aggiornamento del programma come fa **Aiuto > Componenti aggiuntivi e aggiornamenti > Installa aggiornamento o componente aggiuntivo** (solo pacchetti con la firma del progetto). Poi propone di riavviare il programma.
- **Quando la nuova versione richiede una nuova AppImage** (altre librerie rispetto alla tua AppImage), {{APP}} la scarica, la mette al posto dell'AppImage che hai avviato, ovunque sia e con lo stesso nome di file, conserva quella che avevi accanto come *…AppImage.before-update* e si riavvia. Elimina il file conservato quando la nuova versione funziona per te. Se la cartella della tua AppImage non è scrivibile, il nuovo file va nella tua cartella Download e un messaggio dice che cosa fare.
- **Su Windows** (0.92.0) una tale versione arriva come programma di installazione. {{APP}} lo scarica nella propria cartella (*data\update-setup*), lo controlla rispetto all'elenco firmato del progetto e chiede. Con **Chiudi e aggiorna ora** {{APP}} si chiude e il programma di installazione decomprime la nuova versione nella cartella che usi, senza le sue pagine, e la avvia; le tue impostazioni, i componenti aggiuntivi e tutto il resto in *data* restano. Con **Più tardi** non cambia nulla e l'aggiornamento viene riproposto. Se Windows non avvia il programma di installazione (non ha un certificato di firma del codice), un messaggio nomina il file, e puoi anche scaricare la versione corrente dalla pagina del progetto.
- **Quando l'indirizzo non è raggiungibile**, il programma riprova due volte, a 5 secondi di distanza, e poi mostra un breve testo nella barra del piè di pagina del primo editor, al posto del testo del salvataggio, che compare e scompare due volte. Nessuna finestra, e al massimo una volta al giorno. (**Cerca aggiornamenti ora** risponde invece con un messaggio.)
- **Se disattivata**, il programma non contatta nulla, come prima della 0.92.0. Per i computer configurati per altri, la variabile d'ambiente `QB_NO_UPDATE_CHECK=1` impedisce l'esecuzione della ricerca automatica qualunque cosa dica l'interruttore.
- Un'AppImage di una versione precedente alla 0.92.0 che esegue un aggiornamento più recente del programma non può stabilire connessioni sicure; in quel caso la ricerca lo dice quando la richiedi, e altrimenti resta in silenzio. Scarica una volta l'AppImage corrente.
- Ciò che il programma ricorda in proposito (l'interruttore, l'ora dell'ultima ricerca, ciò che hai ignorato o rimandato) è in *updates-state.json* nella cartella dei dati del programma.

### Installare un pacchetto

1. Scegli **Aiuto > Componenti aggiuntivi e aggiornamenti > Installa aggiornamento o componente aggiuntivo…** e seleziona il file .qbpkg, oppure fai doppio clic sul file nel tuo gestore file.
2. {{APP}} mostra che cosa contiene il pacchetto e quali sono le novità. Fai clic su **Installa**.
3. Se il pacchetto contiene un aggiornamento del programma, {{APP}} chiede di riavviare. Il tuo progetto aperto viene salvato e riaperto.

{{APP}} controlla ogni pacchetto prima di installarlo: file danneggiati e pacchetti creati per un'altra versione vengono rifiutati. Un aggiornamento del programma viene avviato una volta in modo invisibile come prova; se non gira sul tuo computer, non viene installato. Le parti di un pacchetto di aggiornamento vengono installate insieme: quando una di esse non si può installare, non se ne installa nessuna (0.65).

**Firme** (0.65). Ogni pacchetto nella pagina dei rilasci del progetto è firmato con la chiave del progetto, e {{APP}} porta con sé la metà pubblica di quella chiave. La finestra di installazione dice **Firmato dal progetto Quillbinder** per un tale pacchetto: è il pacchetto pubblicato dal progetto, non modificato.

- Un pacchetto la cui firma non corrisponde (modificato dopo la firma, o danneggiato) non viene mai installato.
- Un **aggiornamento del programma** viene installato solo con la firma del progetto.
- Un **componente aggiuntivo** senza di essa (uno creato da te o da un'altra persona) viene installato solo dopo un chiaro avviso, con **Annulla** come scelta predefinita. Dalla riga di comando (`--install <file>`), un pacchetto senza la firma del progetto viene installato solo se aggiungi `--allow-unsigned`.

**Chiavi dei componenti aggiuntivi** (0.91.3). I componenti aggiuntivi ufficiali, anche quelli scritti da altri autori, possono essere firmati con una chiave propria invece che con quella del progetto. Il progetto garantisce per una tale chiave con la propria, in un certificato che arriva dentro il pacchetto e consente alla chiave di firmare componenti aggiuntivi e nient'altro. La finestra di installazione nomina allora la chiave: *Firmato con la chiave dei componenti aggiuntivi "Quillbinder official add-ons", per la quale il progetto Quillbinder garantisce*.

- Un **aggiornamento del programma** firmato con una chiave di componenti aggiuntivi viene rifiutato come uno non firmato: gli aggiornamenti del programma richiedono la chiave propria del progetto.
- Una chiave di componenti aggiuntivi per cui il progetto non garantisce conta come nessuna firma: il componente aggiuntivo viene installato solo dopo l'avviso.
- I componenti aggiuntivi firmati con la chiave propria del progetto, come lo erano tutti fino alla 0.91.2, restano affidabili. Dalla 0.91.3 in poi, anche i componenti aggiuntivi propri del progetto sono firmati con la chiave dei componenti aggiuntivi.
- Le versioni fino alla 0.91.2 non conoscono le chiavi dei componenti aggiuntivi e avvisano prima di installare un componente aggiuntivo firmato con una: installa prima l'aggiornamento del programma (è firmato con la chiave propria del progetto), poi i componenti aggiuntivi.

**La finestra che chiede prima di installare un componente aggiuntivo** (0.94.0) dice prima, in una o due frasi, che cosa fa il componente aggiuntivo. Le righe tecniche (le librerie che porta, il motore vocale o grafico, il runtime per cui è stato creato) sono sotto **Dettagli**, che resta chiuso finché non lo apri.

**Controllare tu stesso un download.** Ogni rilascio contiene anche *SHA256SUMS.txt* (il checksum di ogni file), la sua firma *SHA256SUMS.txt.sig* e la chiave pubblica *quillbinder-signing-public.pem*. In un terminale, nella cartella con i download:

    openssl pkeyutl -verify -pubin -inkey quillbinder-signing-public.pem -rawin -in SHA256SUMS.txt -sigfile SHA256SUMS.txt.sig
    sha256sum -c --ignore-missing SHA256SUMS.txt

Il primo comando risponde *Signature Verified Successfully*, il secondo *OK* per ogni file che hai scaricato. Per essere sicuro che il file della chiave sia davvero quello del progetto, confronta i suoi 32 byte (`openssl pkey -pubin -in quillbinder-signing-public.pem -outform DER | tail -c 32 | od -An -tx1`) con la chiave nel sorgente del programma (*src/core/Signing.cpp* e *packaging/quillbinder-signing-public.pem*): iniziano con *bc 3c 58 00 08 3f 66 e6*.

### La Gestione componenti aggiuntivi

**Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi…** mostra che cosa è installato.

- **Versione del programma**: la versione integrata nella tua AppImage e ogni aggiornamento installato. Quella contrassegnata *in uso ora* è quella in uso. Per tornare a una versione precedente, selezionala e fai clic su **Usa questa versione**, poi riavvia. Gli aggiornamenti installati che non ti servono più si possono rimuovere.
- **Componenti aggiuntivi**: togli la spunta a un componente aggiuntivo per disattivarlo senza rimuoverlo; **Rimuovi componente aggiuntivo** lo elimina. Temi, modelli e formati di compilazione di un componente aggiuntivo compaiono nei posti consueti: **Vista > Temi**, la finestra **Nuovo progetto** e la finestra **Compila**.
- Quando una scelta fatta qui non si può scrivere (un disco pieno, una cartella non scrivibile), un messaggio lo dice con il nome del file (0.92.0). Prima, la finestra mostrava la scelta come fatta e veniva annullata al successivo avvio senza una parola. Lo stesso vale per un'installazione: un componente aggiuntivo o una versione del programma la cui voce non si è potuta scrivere viene segnalato come non installato, con ciò che si può fare. Quando un'installazione fallisce a metà e la versione installata prima non si può rimettere a posto, il messaggio nomina la cartella in cui si trova e il nome da dare a quella cartella.

### Se un aggiornamento non si avvia

Se un aggiornamento del programma non si avvia per due volte di seguito, {{APP}} lo disattiva da sé e avvia la versione integrata nell'AppImage, con un messaggio che te lo dice. Nei tuoi progetti non cambia nulla.

### Quando serve una nuova AppImage

Gli aggiornamenti del programma girano sulle librerie all'interno della tua AppImage. Di tanto in tanto un aggiornamento più grande richiede una nuova AppImage; la finestra di installazione ti dice quando un pacchetto non si adatta alla tua. Sostituisci il vecchio file AppImage con quello nuovo; le tue impostazioni, i componenti aggiuntivi e i progetti restano come sono.

La 0.80.0 è una di queste versioni: la sua AppImage porta un nuovo Qt (la build propria di The Qt Company di Qt 6.12 invece del Qt 6.4.2 di Ubuntu). I componenti aggiuntivi con codice di programma (*Media Playback*, *Extra Video and Audio Formats*, *Speech Transcription*, gli strumenti di scrittura e gli importatori) vanno installati di nuovo nelle loro versioni per la 0.80.0 (il pacchetto di aggiornamento e **media-all-formats.qbpkg** nella pagina Releases li portano tutti); fino ad allora, la Gestione componenti aggiuntivi contrassegna i vecchi come creati per un programma più vecchio, e il loro codice non viene caricato. La versione completa della 0.80.0 porta ogni componente aggiuntivo.

La 0.93.0 è un'altra (Linux): la sua AppImage porta Qt 6.12.1, le cui librerie integrate (zlib, HarfBuzz, PCRE2, libpng, libjpeg-turbo, FreeType e altre) sono più recenti e hanno correzioni di sicurezza. Un aggiornamento del programma non può portarle, quindi la ricerca degli aggiornamenti propone la nuova AppImage. I componenti aggiuntivi con codice di programma della AppImage precedente (quelli elencati sopra) vengono installati di nuovo nelle loro versioni per la 0.93.0 allo stesso modo, e i componenti aggiuntivi multimediali portano ora FFmpeg 9.1.

Una nuova AppImage porta anche una nuova versione del programma propria (per esempio l'AppImage 0.20.0). Avvia quella versione, anche se avevi installato aggiornamenti nella vecchia AppImage: gli aggiornamenti più vecchi vengono disattivati e restano elencati in **Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi…**, dove puoi rimuoverli. Se il nuovo file ha un altro nome o si trova in un'altra cartella rispetto al vecchio, scegli una volta **Aiuto > Aggiungi al menu delle applicazioni**, così che la voce di menu e i progetti aperti con doppio clic usino il nuovo file.

---

## 18. Il dialetto Markdown

Non hai bisogno di saperlo per usare {{APP}}; l'editor mostra testo formattato. Conta solo se apri i file di testo con un altro programma. {{APP}} scrive Markdown con queste regole:

- **Ogni riga è un paragrafo**, come in un elaboratore di testi. Una riga vuota è un paragrafo vuoto.
- Titoli: da `# ` a `###### ` all'inizio della riga.
- Citazione in blocco: `> `. Paragrafo centrato: `->text<-`. Qualsiasi altro stile di paragrafo: `::Style Name:: text`.
- Stili di carattere (dalla versione 0.4): `<span class="Style Name">text</span>`. Nel testo vengono scritti solo i nomi degli stili. L'aspetto di uno stile è memorizzato una sola volta, nel file del progetto, quindi uno stile modificato non richiede alcuna modifica nei documenti.
- Elenchi: `- ` e `1. `; due spazi per livello di annidamento.
- `**bold**`, `*italic*`, `~~strikethrough~~`, `==highlight==`, `<u>underline</u>`, `<sup>superscript</sup>`, `<sub>subscript</sub>`.
- Colori di revisione: da `<rev1>…</rev1>` a `<rev5>…</rev5>`.
- Collegamenti: `[text](https://example.com)`; collegamenti ai documenti del progetto: `[text](qb://ID)`.
- Immagini: `![](images/picture.png){width=300}` (le immagini sono memorizzate nella cartella del documento). La descrizione di un'immagine (0.62) è il testo tra le parentesi: `![A brass lamp](images/lamp.png){width=300}`.
- Tabelle (0.62): una riga `:::table` con il design tra parentesi graffe (JSON), una riga per ogni riga della tabella con le celle tra `|` (i paragrafi di una cella separati da `<p/>`, una cella coperta da una cella unita lasciata vuota, un `|` nel testo scritto `\|`) e una riga `:::` alla fine.
- Un'immagine accanto al testo o centrata (0.62): `:::figure {"pos":1,"width":168}` (1 a sinistra, 2 a destra, 3 centrata), la riga dell'immagine, le righe della didascalia e `:::`.
- Un'interruzione di riga all'interno di un paragrafo: `<br>`.
- Caratteri impostati a mano (dalla versione 0.3): `<span style="font-family:'DejaVu Sans'; font-size:14pt; color:#aa0000">text</span>`. Le dichiarazioni sono normale CSS, quindi anche altri programmi Markdown che ammettono HTML le mostrano. Voci possibili: `font-family`, `font-size` (pt), `font-weight` (da 100 a 900), `color`, `letter-spacing` (em), `font-variant:small-caps`, `text-transform:uppercase`.
- Formattazione dei paragrafi impostata a mano (dalla versione 0.3): un elenco tra parentesi graffe all'inizio della riga, prima di qualsiasi altro prefisso: `{: align=right left=10mm right=5mm first=0mm before=6pt after=12pt line=1.5 tabs=20mm,50mmR} text`. `line` è un multiplo (`1.5`), un'altezza esatta (`14pt`) o un minimo (`14pt+`). Le tabulazioni terminano con `R` (destra), `C` (centrata) o `D` (punto decimale); nessuna lettera significa sinistra. Vengono scritte solo le voci che hai impostato.
- Gli elementi di script (dalla versione 0.10) sono stili di paragrafo chiamati come l'elemento: `::Script > Scene Heading:: INT. LIGHTHOUSE - NIGHT`. L'aspetto di un elemento è memorizzato una sola volta, nel file del progetto, con il formato di script.
- Un'interruzione di pagina prima di un paragrafo (da script e file RTF importati, dalla versione 0.10): `{: page=before} text`.
- Le versioni precedenti alla 0.62 mostrano tabelle e figure come righe di testo (`:::table` ...). Se torni a una versione più vecchia, non modificare documenti con tabelle o immagini accanto al testo.
- La versione 0.2 mostra entrambi i marcatori come testo semplice. Se torni alla versione 0.2 con la Gestione componenti aggiuntivi, non modificare documenti che li contengono.
- Le versioni precedenti alla 0.4 non conoscono i tuoi stili: mostrano `<span class=…>` come testo semplice e trattano i paragrafi dei tuoi stili come testo normale. Se torni a una versione più vecchia, non modificare documenti che usano stili tuoi.
- Le versioni precedenti alla 0.10 trattano gli elementi di script come testo normale, e rimuovono i nomi degli elementi da uno script non appena è stato mostrato nell'editor e il progetto viene salvato. Se torni a una versione più vecchia, non aprire lì i tuoi script (vedi sezione 20).
- Una barra rovesciata rende il carattere successivo testo normale: `\*` è un asterisco, non un corsivo.
- Le note seguono la convenzione CriticMarkup:
  - `{>>inline annotation<<}`
  - `^[inline footnote]`
  - `{==text==}{>>linked comment<<}`
  - `{==text==}^[linked footnote]`
  - Un commento può iniziare con un colore: `{>>@#ff8800 text<<}`.

**File > Converti file Markdown ⇄ RTF…** legge sia questo dialetto sia il Markdown standard (in cui i paragrafi sono separati da righe vuote) e, quando scrive Markdown, chiede quale dei due vuoi.

---

## 19. Scorciatoie da tastiera

### Cambiare le scorciatoie

**File > Opzioni > Tastiera** (oppure **Aiuto > Scorciatoie da tastiera**) elenca ogni comando, raggruppato per menu, con la sua scorciatoia e la scorciatoia standard con cui il programma è arrivato. Sono elencati anche gli stili del progetto aperto e i layout, così uno stile o un layout può avere una scorciatoia propria.

1. Trova il comando: scorri, oppure digita una parte del suo nome o una scorciatoia come "Ctrl+K" nel campo di ricerca.
2. Fai clic sul comando, fai clic nel campo della scorciatoia sotto l'elenco e premi i nuovi tasti.
3. Se un altro comando ha quella scorciatoia, ti viene chiesto se toglierla da lì. L'altro comando resta allora senza scorciatoia finché non gliene dai una nuova.

**Nessuna scorciatoia** rimuove la scorciatoia del comando selezionato, **Standard** gli restituisce quella con cui è arrivato e **Tutte allo standard** annulla tutte le tue modifiche. Le tue modifiche sono mostrate in grassetto. Una scorciatoia richiede Ctrl, Alt o Meta, oppure deve essere un tasto funzione, perché le lettere semplici servono per scrivere. **Esporta** e **Importa** salvano il tuo insieme di scorciatoie come file (*.qbkeys*) e ne caricano uno, per esempio per portarlo su un altro computer.

Le scorciatoie degli stili appartengono al nome dello stile: "Block Quote" ha la sua scorciatoia in ogni progetto che ha uno stile con quel nome.

### Scorciatoie e layout della tastiera

Alcune delle scorciatoie standard sono tasti che richiedono **AltGr** su una tastiera tedesca: **Ctrl+{**, **Ctrl+|** e **Ctrl+}** (allinea a sinistra, centra, allinea a destra), **Ctrl+[** e **Ctrl+]** (indietro e avanti), **Ctrl+\\** (l'elenco degli elementi di script) e **Ctrl+=** (pedice). Dalla 0.94.0 il programma controlla il layout di tastiera del sistema **al primissimo avvio** (quando non esistono né impostazioni delle scorciatoie né un avvio precedente) e conserva la risposta. Con un **layout US**, o qualsiasi layout diverso dal tedesco, resta l'insieme consueto qui sopra. Con un **layout tedesco (o austriaco)** questi comandi ricevono tasti che non richiedono AltGr (lo stesso set con un **layout francese, belga, spagnolo, italiano, portoghese, turco o olandese**, le cui parentesi quadre e graffe richiedono anch'esse AltGr; il layout polacco per programmatori, le cui lettere AltGr ć e ń collidono con Ctrl+Alt+C e Ctrl+Alt+N, sposta soltanto i quattro comandi nominati sotto la tabella):

| Comando | Insieme tedesco |
|---------|------------|
| Allinea a sinistra / centra / allinea a destra (anche nella mappa mentale) | Alt+Shift+L / Alt+Shift+E / Alt+Shift+R |
| Giustifica (mappa mentale) | Alt+Shift+J |
| Indietro / avanti | Alt+Shift+B / Alt+Shift+F |
| Elenco degli elementi di script | Ctrl+Shift+O |
| Pedice | Ctrl+Shift+D |
| Porta in primo piano / manda in secondo piano (mappa mentale) | Ctrl+Shift+PageUp / Ctrl+Shift+PageDown |

I layout che usano AltGr per le lettere (polacco, brasiliano e gli altri qui sopra) spostano anche quattro comandi da Ctrl+Alt+lettera, che Windows legge come AltGr: copia formattazione Ctrl+Shift+Y, incolla formattazione Ctrl+Shift+U, vai alle note del documento Ctrl+Shift+X e (non sul layout polacco) mostra righello Ctrl+Shift+W. Le tastiere russe e indonesiane mantengono l'insieme consueto.

I menu mostrano ogni tasto come lo chiama il layout. Un programma usato prima della 0.94.0 mantiene le sue scorciatoie com'erano (nulla cambia da solo), e così ogni scorciatoia che hai impostato tu in **File > Opzioni > Tastiera**: una scorciatoia memorizzata non viene mai sostituita. Dalla 0.95 il pulsante **Usa il set per il mio layout di tastiera…** in fondo a **File > Opzioni > Tastiera** applica l'insieme adatto al layout di tastiera in uso ora (per una tastiera tedesca, i tasti qui sopra). Elenca prima quali scorciatoie standard cambiano; quelle che hai impostato tu restano. Il set appartiene al tuo profilo. Dove il sistema non dice il suo layout, viene usato l'insieme consueto. (Linux: le impostazioni della tastiera del desktop o del server X; Windows: la lingua del layout di tastiera; Android: la lingua della tastiera.)

### Le scorciatoie standard

| Azione | Scorciatoia |
|--------|----------|
| Nuovo progetto / apri progetto | Ctrl+Shift+N / Ctrl+O |
| Salva / chiudi progetto / esci | Ctrl+S / Ctrl+W / Ctrl+Q |
| Nuovo documento / nuova cartella | Ctrl+N / Alt+Shift+N |
| Rinomina / duplica | F2 / Ctrl+D |
| Sposta elemento su / giù / fuori / dentro | Ctrl+Alt+Up / Down / Left / Right |
| Dividi alla selezione / con la selezione come titolo | Ctrl+K / Ctrl+Shift+K |
| Unisci documenti | Ctrl+Shift+M |
| Testo combinato / bacheca / struttura | Ctrl+1 / Ctrl+2 / Ctrl+3 |
| Mostra o nascondi raccoglitore / ispettore | Ctrl+Alt+B / Ctrl+Alt+I |
| Modalità composizione / schermo intero | F11 / Ctrl+Shift+F11 |
| Focus sul raccoglitore / focus sull'editor / altro editor | Alt+1 / Alt+2 / F6 |
| Indietro / avanti | Ctrl+[ / Ctrl+] (layout tedesco: Alt+Shift+B / Alt+Shift+F) |
| Documento precedente / successivo | Alt+Shift+Up / Alt+Shift+Down |
| Mostra nel raccoglitore | Ctrl+Shift+R |
| Trova / sostituisci / successivo / precedente | Ctrl+F / Ctrl+H / F3 / Shift+F3 |
| Ricerca nel progetto | Ctrl+Shift+F |
| Usa la selezione per la ricerca | Ctrl+E |
| Trova per formattazione / successiva / precedente | Ctrl+F3 / Ctrl+G / Ctrl+Shift+G |
| Blocco appunti (componente aggiuntivo) | Alt+Shift+Return; da ogni programma: vedi sezione 10 |
| Audio e video (componente aggiuntivo): riproduci / pausa | Ctrl+Return |
| Audio e video: salta indietro / salta avanti | Ctrl+Alt+, / Ctrl+Alt+. |
| Audio e video: inserisci marca temporale | Ctrl+Shift+Return |
| Filtra bacheca o struttura | Ctrl+F (mentre una delle due ha la tastiera) |
| Collega a documento | Ctrl+L |
| Commento / nota a piè di pagina | Shift+F2 / Ctrl+Shift+F2 |
| Annotazione in linea / nota a piè di pagina in linea | Ctrl+Shift+A / Ctrl+Alt+F |
| Crea istantanea / con titolo | Ctrl+5 / Ctrl+Shift+5 |
| Obiettivi del progetto / parole chiave del progetto | Ctrl+Shift+T / Ctrl+Alt+K |
| Importa file / compila | Ctrl+Shift+I / Ctrl+Shift+E |
| Stampa documento corrente | Ctrl+P |
| Finestra dei caratteri / finestra del paragrafo | Ctrl+T / Ctrl+Alt+P |
| Dimensione del carattere più grande / più piccola | Ctrl+> / Ctrl+< |
| Copia formattazione / incolla formattazione | Ctrl+Alt+C / Ctrl+Alt+V |
| Incolla con formattazione / incolla e adatta lo stile | Ctrl+Alt+Shift+V / Ctrl+Shift+V |
| Nessuno stile / Titolo 1 / Titolo 2 / Titolo 3 | Ctrl+Alt+0 / Ctrl+Alt+1 / Ctrl+Alt+2 / Ctrl+Alt+3 |
| I sei stili successivi dell'elenco degli stili (Formato > Stile > Personalizza l'elenco degli stili) | Ctrl+Alt+4 ... Ctrl+Alt+9 |
| Vai alla sinossi / vai alle note del documento (Naviga > Ispeziona) | Ctrl+Alt+Y / Ctrl+Alt+N |
| Esegui backup ora | Ctrl+Shift+B |
| Modalità script / elenco degli elementi di script | Ctrl+8 / Ctrl+\\ (layout tedesco: Ctrl+8 / Ctrl+Shift+O) |
| In uno script: elemento successivo / cambia elemento | Enter / Tab (vedi sezione 4, Scrittura di script) |
| Ingrandisci / riduci / ripristina zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom dell'area sotto il mouse | Ctrl+rotella del mouse |
| Mostra o nascondi il righello | Ctrl+Alt+R |
| Scorrimento macchina da scrivere | Meta+Ctrl+T |
| Opzioni | Ctrl+, |
| Questo manuale | F1 |

---

## 20. Risoluzione dei problemi

### Rapporti di arresto anomalo e informazioni di sistema

Se {{APP}} termina a causa di un errore, scrive un **rapporto di arresto anomalo** sul tuo computer prima di chiudersi. Se la sua finestra non risponde per 30 secondi (si blocca; il tempo si imposta in **File > Opzioni > Extra > Rapporti di arresto anomalo**), scrive un **rapporto di blocco** che mostra dove è bloccato; se si riprende, il rapporto dice dopo quanto tempo. Non viene inviato nulla da nessuna parte. La volta successiva che avvii {{APP}} dopo un arresto anomalo, o dopo che hai dovuto terminare un {{APP}} bloccato, un messaggio ti dice che cosa è successo. Se è terminato entro un minuto dall'avvio, il messaggio offre **Avvia senza l'ultimo progetto**, nel caso la causa sia il progetto.

- **Aiuto > Risoluzione problemi e sviluppo > Rapporti di arresto anomalo…** elenca i rapporti di arresto anomalo e di blocco con la loro data e il tipo di errore ("blocco (ripreso)" quando {{APP}} ha continuato a funzionare). **Copia negli appunti** copia quello selezionato, così puoi incollarlo in un messaggio a chi si occupa di {{APP}} per te; **Apri cartella** mostra i file; **Elimina** ed **Elimina tutti…** li rimuovono. Vengono conservati i 20 rapporti più recenti (**File > Opzioni > Extra > Rapporti di arresto anomalo** cambia il numero).
- **File > Opzioni > Generale > Avvisi > Mostra console del registro interno** mostra i messaggi del programma e di Qt dall'avvio (gli ultimi 1000), con i nuovi man mano che arrivano; utile quando qualcosa non funziona e nessun messaggio dice perché.
- **Aiuto > Risoluzione problemi e sviluppo > Informazioni di sistema…** mostra la stessa istantanea del programma e del computer che porta un rapporto. Inizia con **Cose da controllare**: ciò che potrebbe presto causare problemi, come un'unità quasi piena o in sola lettura, un progetto in una cartella di rete, un'unità di backup con meno spazio libero di quello che serve al progetto, poca memoria libera, un tempo di salvataggio automatico lungo, backup automatici disattivati, oppure rapporti recenti di arresto anomalo e di blocco. Seguono la versione del programma, il runtime (l'insieme di librerie all'interno dell'AppImage, con il giorno di rilascio del suo Qt e la sua età: un runtime vecchio significa che una AppImage più recente può portare correzioni di sicurezza) e gli aggiornamenti e i componenti aggiuntivi installati, il sistema operativo, il desktop, gli schermi (dalla 0.93.1 con l'area che una finestra può usare su ciascuno), i dispositivi di input (tastiera, mouse, touch screen, penna) e se la tastiera su schermo è mostrata (0.93.1), il processore e la memoria, le cartelle con il loro spazio libero, le impostazioni per salvataggio e backup e il progetto aperto. Usa **Copia negli appunti** o **Salva con nome…** quando segnali un problema che non è un arresto anomalo.

Un rapporto nomina file, cartelle e documenti, e i comandi e i tasti usati poco prima dell'errore (le lettere digitate vengono solo contate, mai registrate). Non contiene testo dei tuoi documenti. La tua cartella home è scritta come **~**.

Se {{APP}} è stato fermato dall'esterno o il computer è stato spento senza chiuderlo, non c'è alcun rapporto, ma anche in quel caso l'avvio successivo te lo dice. In entrambi i casi il tuo lavoro è salvato fino all'ultimo salvataggio automatico, pochi secondi prima della fine; guarda le istantanee del documento o il backup più recente se manca qualcosa (sezioni 11 e 15).

**"Impossibile leggere il file del progetto."** Se il file del progetto (*IlTuoTitolo*{{FILE}}) è danneggiato, per esempio da un arresto del computer mentre veniva salvato, {{APP}} offre la copia che ne conserva (dall'ultima volta che il progetto è stato aperto): **Usa la copia**. Il file danneggiato viene conservato accanto (*… .damaged-<data>*). Mancano le modifiche al raccoglitore da allora (titoli, ordine, etichette); i testi ci sono tutti, e i documenti creati da allora vengono messi nella cartella **Documenti recuperati** in Ricerca, chiamati con la loro prima riga.

**"Il file del progetto conteneva caratteri non ammessi."** Le versioni precedenti alla 0.20 potevano salvare caratteri di controllo invisibili (incollati da altri programmi in un titolo o in una sinossi), dopo di che il progetto non si apriva più. {{APP}} li rimuove quando apre il progetto e te lo dice; nient'altro viene cambiato. Dalla 0.20 non vengono mai salvati.

**Un documento è mostrato vuoto, con la nota "Impossibile leggere il file di questo documento".** Il suo file di testo c'è ma non si può leggere (i suoi permessi sono stati cambiati, o l'unità su cui si trova è scomparsa). {{APP}} lo mostra vuoto e non ti lascia modificarlo, così il file non viene mai sovrascritto. Controlla i permessi del file o l'unità, poi chiudi e riapri il progetto.

**Premere Page Down o Page Up ha chiuso il programma.** Era un errore delle versioni precedenti alla 0.19; aggiorna alla 0.19 o successiva. Fino ad allora, scorri con il mouse o con la barra di scorrimento.

**Il programma non si avvia quando faccio doppio clic.** Fai clic destro sul file > Proprietà > Permessi e spunta "È eseguibile" (in Dolphin: "Consenti l'esecuzione del file come programma"). Poi fai di nuovo doppio clic.

**Le finestre di dialogo dei file hanno un aspetto diverso dagli altri programmi KDE.** Attiva **Usa le finestre di dialogo dei file del desktop** in **File > Opzioni > Generale** e riavvia {{APP}}.

**Le icone delle barre degli strumenti si vedono male in un tema scuro o chiaro.** {{APP}} sceglie l'insieme chiaro o scuro delle icone del tuo desktop per adattarlo al tema. Se il desktop non ha un insieme adatto, usa le proprie icone. Se singole icone ancora non vanno bene, scegli **Default (Follow the Desktop)** in **Vista > Temi** e imposta i colori nelle impostazioni del desktop.

**Ho cambiato dei colori e voglio quelli del tema.** **File > Opzioni > Aspetto > Ripristina colori e caratteri del tema**, oppure il pulsante **↺** accanto a un singolo colore o carattere.

**Un aggiornamento del programma non si è avviato.** {{APP}} disattiva da sé un aggiornamento che non funziona (sezione 17). Puoi provare a installarlo di nuovo, oppure continuare a usare la versione integrata.

**Manca un formato di compilazione.** Controlla **File > Opzioni > Plugin**: elenca di quale programma il formato ha bisogno. Installalo in Discover e riavvia {{APP}}.

**Il controllo ortografico non conosce la mia lingua.** Per spagnolo, francese, portoghese brasiliano, italiano, russo, indonesiano, turco, polacco e olandese installa il componente aggiuntivo "dizionario ortografico <lingua>" (**Aiuto > Componenti aggiuntivi e aggiornamenti > Installa aggiornamento o componente aggiuntivo**; la pagina dei download li elenca), riavvia {{APP}} e scegli la lingua in **File > Opzioni > Correzioni** (pagina Ortografia). Per un'altra lingua installa in Discover il pacchetto del dizionario *hunspell* corrispondente.

**Il progetto "sembra essere aperto in un'altra finestra o su un altro computer".** Succede quando il progetto si trova in una cartella sincronizzata che è aperta altrove, oppure dopo un arresto anomalo su un altro computer. Se sei sicuro che non sia aperto da nessun'altra parte, scegli **Sì**.

**Alt+trascinamento non apre un copyholder.** Alcuni desktop usano Alt+trascinamento per qualcosa di proprio, come spostare le finestre. Usa invece **Naviga > Apri > nel copyholder** o il menu del clic destro del documento, oppure cambia il tasto nelle impostazioni delle finestre del desktop.

**Dopo essere tornato a una versione più vecchia del programma, uno script è testo semplice senza il suo layout.** Le versioni precedenti alla 0.10 non conoscono gli elementi di script e salvano come testo normale uno script che hanno mostrato. Passa di nuovo alla versione corrente (**Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi**). Il testo c'è tutto; per riavere il layout, ripristina il documento da un'istantanea o da un backup fatto prima (sezioni 11 e 15), oppure ridai ai paragrafi i loro elementi con il menu a comparsa nel piè di pagina.

**La scorciatoia del blocco appunti non fa nulla negli altri programmi.** Le scorciatoie impostate in **File > Opzioni > Tastiera** funzionano solo all'interno di {{APP}}. Per gli altri programmi, deve essere il desktop ad avviare il comando (sezione 10, *Da ogni programma*). Se sposti l'AppImage, inserisci di nuovo il comando (nomina il posto dell'AppImage).

**La fotocamera del blocco appunti dice di non poter leggere lo schermo.** Con Wayland può farlo solo il programma di schermate del desktop. Installa *Spectacle* (`sudo apt install kde-spectacle`) e riprova.

**Modifica > Strumenti di scrittura dice "Nessuno strumento di scrittura installato".** Gli strumenti sono componenti aggiuntivi: installa il pacchetto dell'aggiornamento 0.11 oppure attivali in **Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi**.

**Un file audio o video offre solo "Apri con l'applicazione predefinita".** La riproduzione all'interno di {{APP}} richiede il componente aggiuntivo *Media Playback* (nel pacchetto dell'aggiornamento 0.12); installalo oppure attivalo in **Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi**. La riga sotto il pulsante dice che cosa manca.

**Un video si riproduce ma non c'è audio, oppure un file audio dice "Nessuna uscita audio trovata".** {{APP}} riproduce il suono tramite il server audio del desktop (PipeWire o PulseAudio). Controlla che l'audio funzioni negli altri programmi e che nelle impostazioni audio del desktop sia scelta l'uscita giusta; {{APP}} segue subito un cambio di uscita.

**"Questo file non può essere riprodotto qui: usa H.264 e AAC ..."** Il file è in un formato soggetto a brevetti, che *Media Playback* esclude. Installa il componente aggiuntivo **Extra Video and Audio Formats**, oppure il pacchetto **media-all-formats.qbpkg**, che lo porta insieme a *Media Playback* (entrambi nella pagina Releases della pagina del progetto, sezione 1), e riavvia {{APP}}.

**Un file non si riproduce, o si riproduce senza immagine.** Alcuni tipi non fanno parte della decodifica integrata (video AV1, WMA Pro, WMV 9). Apri il file con un altro programma (fai clic destro sul lettore: **Apri con l'applicazione predefinita**), oppure convertilo in un altro formato.

**Un file .doc viene aggiunto come file, non come testo.** Il componente aggiuntivo **Word 97–2003 Import** non è installato o è disattivato (**Aiuto > Componenti aggiuntivi e aggiornamenti > Gestione componenti aggiuntivi**); dopo averlo installato, riavvia {{APP}}. Un messaggio dopo l'importazione nomina ciò che il componente aggiuntivo ha escluso. I file Word 6.0 e 95 richiedono LibreOffice (`sudo apt install libreoffice-writer`).

**Una pagina web arriva vuota o con la parte sbagliata.** Togli la spunta a **Solo l'articolo** nella finestra Pagina web per prendere l'intera pagina ed elimina ciò che non ti serve. Le pagine che richiedono un login o sono costruite interamente da script non si possono leggere senza un browser: apri la pagina nel tuo browser, seleziona il testo e trascinalo nel raccoglitore (diventa un nuovo documento). "Impossibile scaricare le pagine web sicure" significa che manca OpenSSL 3 (`sudo apt install libssl3`, presente in ogni Ubuntu) o che il componente aggiuntivo è danneggiato (installalo di nuovo).

**Il primo secondo di un file è silenzioso o il tempo resta fermo all'inizio.** L'uscita audio ha bisogno di un momento per avviarsi; con alcuni server audio più di un secondo. Riavvolgi un po' (Ctrl+Alt+,) se ti sei perso l'inizio.

**Le pagine nella vista pagina non corrispondono al mio manoscritto stampato.** Non sono pensate per farlo: la vista pagina mostra il tuo testo su fogli mentre scrivi. È il formato di compilazione a decidere le pagine del file finito (sezione 13).

**"Scrivener ha aperto questo progetto."** Scrivener lascia un file di blocco nel *.scriv* mentre ha il progetto aperto, e dopo un arresto anomalo. Chiudi il progetto in Scrivener (su ogni computer) e aprilo di nuovo qui; se Scrivener non è in esecuzione da nessuna parte, scegli **Sì**.

**Un progetto in formato Scrivener non mostra le mie ultime modifiche fatte in Scrivener.** {{APP}} rilegge il *.scriv* solo quando il progetto viene aperto. Chiudilo qui (**File > Chiudi progetto**), lascia che Scrivener o il servizio di sincronizzazione finisca e aprilo di nuovo.

**I menu Inserisci, Naviga, Progetto, Documenti e Formato sono grigi.** Nessun progetto è aperto. Aprine o creane uno (menu File).

**Dove segnalare un problema.** **Aiuto > Segnala un problema…** apre nel tuo browser la pagina delle segnalazioni del progetto (https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose), con un modulo per un problema e uno per un desiderio; ti serve un account GitHub e puoi scrivere in inglese o in tedesco. Le segnalazioni sono pubbliche: non allegare il tuo manoscritto o il tuo progetto; **Aiuto > Risoluzione problemi e sviluppo > Informazioni di sistema…** (**Copia negli appunti**) contiene ciò che serve a una segnalazione sul programma e sul computer. Un **problema di sicurezza** non si segnala lì ma in privato, come descritto in *SECURITY.md* del progetto (la scheda Security del repository, **Report a vulnerability**).

**L'indice di un file Word non ha numeri di pagina.** I numeri di pagina vengono inseriti dall'elaboratore di testi quando aggiorna il campo; aggiornalo lì.

**Qualcosa è andato storto con un documento.** Guarda nella scheda delle istantanee dell'ispettore, oppure ripristina un backup (sezione 15).

## Crediti e licenza

- Main Developer: Claude.AI w. Opus 5.5 Extra

{{APP}} è software libero con licenza Apache License 2.0: chiunque può usarlo, copiarlo, modificarlo e distribuirlo, con o senza compenso, mantenendo la licenza e il file NOTICE, che nomina il progetto e il suo sorgente; senza garanzia. Le versioni fino alla 0.70.0 erano pubblicate con la licenza Zero-Clause BSD (0BSD). La finestra Informazioni (nel menu Aiuto) mostra la versione e la licenza.

{{APP}} usa parti di altri progetti (Qt, Hunspell, ICU, le icone Breeze e le altre librerie all'interno dell'AppImage), ciascuna con la propria licenza. **Aiuto > Licenze di terze parti…** (anche un pulsante della finestra Informazioni) le elenca con i testi completi delle licenze; gli stessi testi sono all'interno dell'AppImage, in *usr/share/doc/quillbinder/licenses*. Il codice sorgente di ciascuna, esattamente nella versione usata, si trova nel rilascio **Third-party source code** della pagina del progetto (https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources); la finestra nomina il file per ogni libreria, e le note di licenza dei componenti aggiuntivi fanno lo stesso per le loro.

Scrivener e Scapple sono nomi di prodotti di Literature & Latte. {{APP}} è un programma indipendente, non realizzato da Literature & Latte né collegato ad essa; legge e scrive i loro formati di file affinché i tuoi progetti possano passare da un programma all'altro.
