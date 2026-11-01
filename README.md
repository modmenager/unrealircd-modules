# m_rootchannel - Blocco ASSOLUTO per canali critici

`m_rootchannel` è un modulo di terze parti per **UnrealIRCd 6** progettato per blindare in modo **assoluto e inviolabile** un canale specifico del server (es. `#root` o qualsiasi canale specificato nel file di configurazione). 

A differenza dei moduli standard, questo modulo implementa una logica di **Default Deny** applicata direttamente tramite l'override dei comandi nativi a livello core. Impedisce l'accesso e la manipolazione del canale a utenti normali e a **qualsiasi altra operclass** (come `netadmin`, `csop`, `ircop`), **anche se hanno i permessi di OperOverride attivi**.

Il modulo permette il passaggio e il controllo del canale **esclusivamente** agli operatori che appartengono alle classi:
* `root`
* `root-with-override`

---

## 🔒 Funzionalità e Protezioni

Il modulo intercetta alla fonte e distrugge i seguenti comandi se mirati al canale protetto e avviati da utenti non autorizzati:
1. **`/JOIN` standard:** Blocca l'ingresso classico degli utenti, ignorando eccezioni come inviti (`+i`), chiavi (`+k`) o moduli di override esterni (es. `operoverride_ext`).
2. **`/SAJOIN`:** Impedisce agli amministratori di rete non-root di forzare se stessi o altri utenti all'interno del canale.
3. **`/SAMODE`:** Impedisce la modifica dei modi del canale dall'esterno (es. impedisce a un netadmin di togliere il modo `+i` o `+s`).
4. **`/SAPART`:** Impedisce di forzare l'uscita dei Root Administrator presenti nel canale.
5. **`/INVITE`:** Blocca i tentativi di generazione di inviti non autorizzati verso il canale.

Quando un utente o un operatore non autorizzato tenta una di queste azioni, il comando viene cestinato e il server restituisce una notifica ed un errore numerico nativo (es. `ERR_BANNEDFROMCHAN`).

---

## 🛠️ Installazione Manuale

Se desideri installare il modulo manualmente sul tuo server senza passare dal gestore dei moduli:

1. Copia il file `m_rootchannel.c` nella cartella dei moduli di terze parti:
   ```bash
   cp m_rootchannel.c src/modules/third/
   ```
2. Compila il modulo tramite il terminale di UnrealIRCd:
   ```bash
   ./unrealircd modules
   ```
3. Apri il tuo file `unrealircd.conf` e carica il modulo aggiungendo questa riga:
   ```text
   loadmodule "third/m_rootchannel";
   ```
4. Esegui un `/REHASH` dal tuo client IRC per attivarlo a caldo.

---

## ⚙️ Configurazione del Canale

Il modulo legge il nome del canale da proteggere da un file di testo locale. Questo ti permette di cambiare il canale protetto in qualsiasi momento senza dover ricompilare il codice sorgente.

1. All'interno della cartella `conf/` del tuo UnrealIRCd, crea un file chiamato `rootchannel.txt`:
   ```bash
   nano conf/rootchannel.txt
   ```
2. Inserisci all'interno del file **solamente il nome del canale** comprensivo di cancelletto, ad esempio:
   ```text
   #root
   ```
3. Salva e chiudi il file. 

*Nota: Il file viene letto dal modulo all'avvio del server o ad ogni comando `/REHASH`.*

---

## 🌐 Installazione tramite Module Manager (Repository)

Se hai inserito questo modulo all'interno del tuo repository di rete (es. tramite GitHub Pages), puoi installarlo ed aggiornarlo su qualsiasi altro server del network semplicemente aggiungendo l'URL del tuo indice nel file `conf/modules.sources.list` e digitando:

```bash
./unrealircd module install third/m_rootchannel
```

---

## 📄 Licenza

Questo modulo è rilasciato sotto licenza **GPLv2 o successiva**. Sei libero di modificarlo e adattarlo alle esigenze della tua rete IRC.

