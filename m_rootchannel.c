/* Copyright (C) All Rights Reserved
 * Written by Server
 * Website: https://github.com
 * License: GPLv2 o successiva. Sei libero di modificarlo e adattarlo alle esigenze della tua rete IRC.
 */

/*
 * Blocco ASSOLUTO - Ammette SOLO root, root-with-override, techadmin e Services
 * Ottimizzato per server LINK e controlli nativi UnrealIRCd 6
 */

/*** <<<MODULE MANAGER START>>>
module
{
    documentation "https://github.comblob"
    troubleshooting "In caso di problemi con il modulo m_rootchannel, Contatta Server Su Irc.albnetwork.org."
    min-unrealircd-version "6.1.*"
    max-unrealircd-version "6.2.*"
    post-install-text {
        "Il modulo m_rootchannel e' stato installato con successo!"
        "Ora aggiungi la seguente riga nel tuo file unrealircd.conf:"
        "loadmodule \"third/m_rootchannel\";"
        "Ricordati di creare il file 'conf/rootchannel.txt' inserendo il nome del canale da blindare (es. #root)."
        "Infine, esegui /REHASH dal tuo client IRC per attivarlo."
    }
}
*** <<<MODULE MANAGER END>>>
*/

#include "unrealircd.h"

ModuleHeader MOD_HEADER = {
    "third/m_rootchannel",
    "20.1",
    "Blocco ASSOLUTO - Ammette SOLO root, root-with-override, techadmin e Services",
    "Server",
    "unrealircd-6"
};

static char restricted_channel[CHANNELLEN + 1] = "#root";

/* Prototipi delle funzioni */
int rootchannel_can_join(Client *client, Channel *channel, const char *key, char **errmsg);
void load_rootchannel_config(void);
int b_is_authorized_oper(Client *client);

MOD_TEST() { return MOD_SUCCESS; }

MOD_INIT()
{
    load_rootchannel_config();
    /* Controllo nativo ingresso canali stabilizzato per UnrealIRCd 6 */
    HookAdd(modinfo->handle, HOOKTYPE_CAN_JOIN, 90, rootchannel_can_join);
    return MOD_SUCCESS;
}

MOD_LOAD() { return MOD_SUCCESS; }
MOD_UNLOAD() { return MOD_SUCCESS; }

void load_rootchannel_config(void)
{
    FILE *f = fopen("conf/rootchannel.txt", "r");
    if (f)
    {
        char buf[CHANNELLEN + 1];
        if (fgets(buf, sizeof(buf), f))
        {
            buf[strcspn(buf, "\r\n")] = 0;
            if (buf[0] == '#')
            {
                strlcpy(restricted_channel, buf, sizeof(restricted_channel));
            }
        }
        fclose(f);
    }
}

int b_is_authorized_oper(Client *client)
{
    if (!client)
        return 0;

    /* I Servizi IRC e le connessioni U-Line (Anope/OperServ/BotServ) passano sempre */
    if (IsServer(client) || IsMe(client) || IsULine(client))
        return 1;

    if (!IsUser(client))
        return 0;

    /* Verifica se l'utente è un operatore di rete su UnrealIRCd 6 */
    if (IsOper(client))
    {
        /* Estrazione sicura del nome dell'operclass dalle API stabili di UnrealIRCd 6 */
        const char *class_name = get_operclass(client);
        if (class_name)
        {
            if (strcasecmp(class_name, "root") == 0 || 
                strcasecmp(class_name, "root-with-override") == 0 || 
                strcasecmp(class_name, "root_with_override") == 0 || 
                strcasecmp(class_name, "techadmin") == 0 || 
                strcasecmp(class_name, "netadmin") == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

int rootchannel_can_join(Client *client, Channel *channel, const char *key, char **errmsg)
{
    if (!client || !channel || channel->name[0] == '\0' || restricted_channel[0] == '\0')
        return HOOK_CONTINUE;

    /* Se l'utente tenta di entrare nel canale blindato configurato */
    if (strcasecmp(channel->name, restricted_channel) == 0)
    {
        /* Se l'operatore fa parte della lista autorizzata, l'accesso viene garantito */
        if (b_is_authorized_oper(client))
        {
            return HOOK_CONTINUE; 
        }

        /* Messaggi originali ripristinati in lingua albanese */
        sendnotice(client, "*** [Aksesi i ndaluar] Kanali %s eshte i rezervuar vetem per Root Administrator. Komanda JOIN u bllokua.", channel->name);
        sendnumeric(client, ERR_BANNEDFROMCHAN, channel->name);
        
        *errmsg = "Aksesi i ndaluar.";
        return ERR_OPERONLY; 
    }

    return HOOK_CONTINUE;
}
